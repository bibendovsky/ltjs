/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <string.h>

#include "libavutil/avassert.h"
#include "libavutil/log.h"
#include "libavutil/mem.h"

#include "bsf.h"
#include "bsf_internal.h"
#include "codec_par.h"
#include "packet_internal.h"

static inline const FFBitStreamFilter* ff_bsf(const AVBitStreamFilter* bsf)
{
	return (const FFBitStreamFilter*)bsf;
}

typedef struct FFBSFContext
{
	AVBSFContext pub;
	AVPacket* buffer_pkt;
	int eof;
} FFBSFContext;

static inline FFBSFContext* ffbsfcontext(AVBSFContext* ctx)
{
	return (FFBSFContext*)ctx;
}

void av_bsf_free(AVBSFContext** pctx)
{
	AVBSFContext* ctx;
	FFBSFContext* bsfi;

	if (!pctx || !*pctx)
		return;
	ctx = *pctx;
	bsfi = ffbsfcontext(ctx);

	if (ctx->priv_data)
	{
		if (ff_bsf(ctx->filter)->close)
			ff_bsf(ctx->filter)->close(ctx);
		av_freep(&ctx->priv_data);
	}
	av_packet_free(&bsfi->buffer_pkt);

	avcodec_parameters_free(&ctx->par_in);
	avcodec_parameters_free(&ctx->par_out);

	av_freep(pctx);
}

int av_bsf_alloc(const AVBitStreamFilter* filter, AVBSFContext** pctx)
{
	AVBSFContext* ctx;
	FFBSFContext* bsfi;
	int ret;

	bsfi = av_mallocz(sizeof(*bsfi));
	if (!bsfi)
		return AVERROR(ENOMEM);
	ctx = &bsfi->pub;

	ctx->filter = filter;

	ctx->par_in = avcodec_parameters_alloc();
	ctx->par_out = avcodec_parameters_alloc();
	if (!ctx->par_in || !ctx->par_out)
	{
		ret = AVERROR(ENOMEM);
		goto fail;
	}
	/* allocate priv data and init private options */
	if (ff_bsf(filter)->priv_data_size)
	{
		ctx->priv_data = av_mallocz(ff_bsf(filter)->priv_data_size);
		if (!ctx->priv_data)
		{
			ret = AVERROR(ENOMEM);
			goto fail;
		}
	}
	bsfi->buffer_pkt = av_packet_alloc();
	if (!bsfi->buffer_pkt)
	{
		ret = AVERROR(ENOMEM);
		goto fail;
	}

	*pctx = ctx;
	return 0;
fail:
	av_bsf_free(&ctx);
	return ret;
}

int av_bsf_init(AVBSFContext* ctx)
{
	int ret, i;

	/* check that the codec is supported */
	if (ctx->filter->codec_ids)
	{
		for (i = 0; ctx->filter->codec_ids[i] != AV_CODEC_ID_NONE; i++)
			if (ctx->par_in->codec_id == ctx->filter->codec_ids[i])
				break;
		if (ctx->filter->codec_ids[i] == AV_CODEC_ID_NONE)
		{
			return AVERROR(EINVAL);
		}
	}

	/* initialize output parameters to be the same as input init below might overwrite that */
	ret = avcodec_parameters_copy(ctx->par_out, ctx->par_in);
	if (ret < 0)
		return ret;

	ctx->time_base_out = ctx->time_base_in;

	if (ff_bsf(ctx->filter)->init)
	{
		ret = ff_bsf(ctx->filter)->init(ctx);
		if (ret < 0)
			return ret;
	}

	return 0;
}

void av_bsf_flush(AVBSFContext* ctx)
{
	FFBSFContext* const bsfi = ffbsfcontext(ctx);

	bsfi->eof = 0;

	av_packet_unref(bsfi->buffer_pkt);

	if (ff_bsf(ctx->filter)->flush)
		ff_bsf(ctx->filter)->flush(ctx);
}

int av_bsf_send_packet(AVBSFContext* ctx, AVPacket* pkt)
{
	FFBSFContext* const bsfi = ffbsfcontext(ctx);
	int ret;

	if (!pkt || AVPACKET_IS_EMPTY(pkt))
	{
		if (pkt)
			av_packet_unref(pkt);
		bsfi->eof = 1;
		return 0;
	}

	if (bsfi->eof)
	{
		return AVERROR(EINVAL);
	}

	if (!AVPACKET_IS_EMPTY(bsfi->buffer_pkt))
		return AVERROR(EAGAIN);

	ret = av_packet_make_refcounted(pkt);
	if (ret < 0)
		return ret;
	av_packet_move_ref(bsfi->buffer_pkt, pkt);

	return 0;
}

int av_bsf_receive_packet(AVBSFContext* ctx, AVPacket* pkt)
{
	return ff_bsf(ctx->filter)->filter(ctx, pkt);
}

int ff_bsf_get_packet_ref(AVBSFContext* ctx, AVPacket* pkt)
{
	FFBSFContext* const bsfi = ffbsfcontext(ctx);

	if (bsfi->eof)
		return AVERROR_EOF;

	if (AVPACKET_IS_EMPTY(bsfi->buffer_pkt))
		return AVERROR(EAGAIN);

	av_packet_move_ref(pkt, bsfi->buffer_pkt);

	return 0;
}

typedef struct BSFListContext
{
	AVBSFContext** bsfs;
	int nb_bsfs;

	unsigned idx; // index of currently processed BSF

	char* item_name;
} BSFListContext;

static int bsf_list_init(AVBSFContext* bsf)
{
	BSFListContext* lst = bsf->priv_data;
	int ret, i;
	const AVCodecParameters* cod_par = bsf->par_in;
	AVRational tb = bsf->time_base_in;

	for (i = 0; i < lst->nb_bsfs; ++i)
	{
		ret = avcodec_parameters_copy(lst->bsfs[i]->par_in, cod_par);
		if (ret < 0)
			goto fail;

		lst->bsfs[i]->time_base_in = tb;

		ret = av_bsf_init(lst->bsfs[i]);
		if (ret < 0)
			goto fail;

		cod_par = lst->bsfs[i]->par_out;
		tb = lst->bsfs[i]->time_base_out;
	}

	bsf->time_base_out = tb;
	ret = avcodec_parameters_copy(bsf->par_out, cod_par);

fail:
	return ret;
}

static int bsf_list_filter(AVBSFContext* bsf, AVPacket* out)
{
	BSFListContext* lst = bsf->priv_data;
	int ret, eof = 0;

	if (!lst->nb_bsfs)
		return ff_bsf_get_packet_ref(bsf, out);

	while (1)
	{
		/* get a packet from the previous filter up the chain */
		if (lst->idx)
			ret = av_bsf_receive_packet(lst->bsfs[lst->idx - 1], out);
		else
			ret = ff_bsf_get_packet_ref(bsf, out);
		if (ret == AVERROR(EAGAIN))
		{
			if (!lst->idx)
				return ret;
			lst->idx--;
			continue;
		}
		else if (ret == AVERROR_EOF)
		{
			eof = 1;
		}
		else if (ret < 0)
			return ret;

		/* send it to the next filter down the chain */
		if (lst->idx < (unsigned int)lst->nb_bsfs)
		{
			ret = av_bsf_send_packet(lst->bsfs[lst->idx], eof ? NULL : out);
			av_assert1(ret != AVERROR(EAGAIN));
			if (ret < 0)
			{
				av_packet_unref(out);
				return ret;
			}
			lst->idx++;
			eof = 0;
		}
		else if (eof)
		{
			return ret;
		}
		else
		{
			return 0;
		}
	}
}

static void bsf_list_flush(AVBSFContext* bsf)
{
	BSFListContext* lst = bsf->priv_data;

	for (int i = 0; i < lst->nb_bsfs; i++)
		av_bsf_flush(lst->bsfs[i]);
	lst->idx = 0;
}

static void bsf_list_close(AVBSFContext* bsf)
{
	BSFListContext* lst = bsf->priv_data;
	int i;

	for (i = 0; i < lst->nb_bsfs; ++i)
		av_bsf_free(&lst->bsfs[i]);
	av_freep(&lst->bsfs);
	av_freep(&lst->item_name);
}

static const FFBitStreamFilter list_bsf = {
	.p.name = "bsf_list",
	.priv_data_size = sizeof(BSFListContext),
	.init = bsf_list_init,
	.filter = bsf_list_filter,
	.flush = bsf_list_flush,
	.close = bsf_list_close,
};

int av_bsf_list_parse_str(const char* str, AVBSFContext** bsf_lst)
{
	(void)str;

	return av_bsf_get_null_filter(bsf_lst);
}

int av_bsf_get_null_filter(AVBSFContext** bsf)
{
	return av_bsf_alloc(&list_bsf.p, bsf);
}
