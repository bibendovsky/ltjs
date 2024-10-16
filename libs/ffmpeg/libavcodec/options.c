/*
 * Copyright (c) 2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Options definition for AVCodecContext. */

#include "avcodec.h"
#include "codec_internal.h"
#include "avcodec_internal.h"

static void ltjs_av_codec_context_set_defaults(AVCodecContext* ctx)
{
	ctx->bit_rate = 200000;
	ctx->pix_fmt = AV_PIX_FMT_NONE;
}

static int init_context_defaults(AVCodecContext* s, const AVCodec* codec)
{
	const FFCodec* const codec2 = ffcodec(codec);
	memset(s, 0, sizeof(AVCodecContext));

	s->codec_type = codec ? codec->type : AVMEDIA_TYPE_UNKNOWN;
	if (codec)
	{
		s->codec = codec;
		s->codec_id = codec->id;
	}

	ltjs_av_codec_context_set_defaults(s);
	av_channel_layout_uninit(&s->ch_layout);

	s->framerate = (AVRational){0, 1};
	s->get_buffer2 = avcodec_default_get_buffer2;
	s->sample_aspect_ratio = (AVRational){0,1};
	s->ch_layout.order = AV_CHANNEL_ORDER_UNSPEC;
	s->pix_fmt = AV_PIX_FMT_NONE;
	s->sample_fmt = AV_SAMPLE_FMT_NONE;

	if (codec && codec2->priv_data_size)
	{
		s->priv_data = av_mallocz(codec2->priv_data_size);
		if (!s->priv_data)
			return AVERROR(ENOMEM);
	}
	return 0;
}

AVCodecContext* avcodec_alloc_context3(const AVCodec* codec)
{
	AVCodecContext* avctx = av_malloc(sizeof(AVCodecContext));

	if (!avctx)
		return NULL;

	if (init_context_defaults(avctx, codec) < 0)
	{
		av_free(avctx);
		return NULL;
	}

	return avctx;
}

void avcodec_free_context(AVCodecContext** pavctx)
{
	AVCodecContext* avctx = *pavctx;

	if (!avctx)
		return;

	ff_codec_close(avctx);
	av_freep(&avctx->extradata);
	av_channel_layout_uninit(&avctx->ch_layout);
	av_freep(pavctx);
}
