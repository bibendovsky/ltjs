/*
 * AVCodecContext functions for libavcodec
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* AVCodecContext functions for libavcodec */

#include "libavutil/channel_layout.h"
#include "libavutil/imgutils.h"
#include "avcodec.h"
#include "avcodec_internal.h"
#include "bsf.h"
#include "codec_internal.h"
#include "decode.h"
#include "internal.h"
#include "refstruct.h"

/*
 * Maximum size in bytes of extradata.
 * This value was chosen such that every bit of the buffer is
 * addressable by a 32-bit signed integer as used by get_bits.
 */
#define FF_MAX_EXTRADATA_SIZE ((1 << 28) - AV_INPUT_BUFFER_PADDING_SIZE)

static int64_t get_bit_rate(AVCodecContext* ctx)
{
	int64_t bit_rate;
	int bits_per_sample;

	switch (ctx->codec_type)
	{
		case AVMEDIA_TYPE_VIDEO:
			bit_rate = ctx->bit_rate;
			break;
		case AVMEDIA_TYPE_AUDIO:
			bits_per_sample = 0;
			if (bits_per_sample)
			{
				bit_rate = ctx->sample_rate * (int64_t)ctx->ch_layout.nb_channels;
				if (bit_rate > INT64_MAX / bits_per_sample)
				{
					bit_rate = 0;
				}
				else
					bit_rate *= bits_per_sample;
			}
			else
				bit_rate = ctx->bit_rate;
			break;
		default:
			bit_rate = 0;
			break;
	}
	return bit_rate;
}

int avcodec_open2(AVCodecContext* avctx, const AVCodec* codec, AVDictionary** options)
{
	(void)options;

	int ret = 0;
	AVCodecInternal* avci;
	const FFCodec* codec2;

	if (avcodec_is_open(avctx))
		return 0;

	if (!codec && !avctx->codec)
	{
		return AVERROR(EINVAL);
	}
	if (codec && avctx->codec && codec != avctx->codec)
	{
		return AVERROR(EINVAL);
	}
	if (!codec)
		codec = avctx->codec;
	codec2 = ffcodec(codec);

	if ((avctx->codec_type != AVMEDIA_TYPE_UNKNOWN && avctx->codec_type != codec->type) ||
		(avctx->codec_id != AV_CODEC_ID_NONE && avctx->codec_id != codec->id))
	{
		return AVERROR(EINVAL);
	}

	avctx->codec_type = codec->type;
	avctx->codec_id = codec->id;
	avctx->codec = codec;

	if (avctx->extradata_size < 0 || avctx->extradata_size >= FF_MAX_EXTRADATA_SIZE)
		return AVERROR(EINVAL);

	avci = ff_decode_internal_alloc();
	if (!avci)
	{
		ret = AVERROR(ENOMEM);
		goto end;
	}
	avctx->internal = avci;

	avci->buffer_frame = av_frame_alloc();
	avci->buffer_pkt = av_packet_alloc();
	if (!avci->buffer_frame || !avci->buffer_pkt)
	{
		ret = AVERROR(ENOMEM);
		goto free_and_end;
	}

	if (codec2->priv_data_size > 0)
	{
		if (!avctx->priv_data)
		{
			avctx->priv_data = av_mallocz(codec2->priv_data_size);
			if (!avctx->priv_data)
			{
				ret = AVERROR(ENOMEM);
				goto free_and_end;
			}
		}
	}
	else
	{
		avctx->priv_data = NULL;
	}

	// only call ff_set_dimensions() for non H.264/VP6F/DXV codecs so as not to overwrite previously setup dimensions
	if (avctx->width && avctx->height)
		ret = ff_set_dimensions(avctx, avctx->width, avctx->height);
	if (ret < 0)
		goto free_and_end;

	if ((avctx->width || avctx->height)
		&& (av_image_check_size2(avctx->width, avctx->height, AV_PIX_FMT_NONE, 0, avctx) < 0))
	{
		ff_set_dimensions(avctx, 0, 0);
	}

	if (avctx->width > 0 && avctx->height > 0)
	{
		if (av_image_check_sar(avctx->width, avctx->height, avctx->sample_aspect_ratio) < 0)
		{
			avctx->sample_aspect_ratio = (AVRational){0, 1};
		}
	}

	if (avctx->sample_rate < 0)
	{
		ret = AVERROR(EINVAL);
		goto free_and_end;
	}

	/* AV_CODEC_CAP_CHANNEL_CONF is a decoder-only flag; so the code below
	 * in particular checks that nb_channels is set for all audio encoders. */
	if (avctx->codec_type == AVMEDIA_TYPE_AUDIO && !avctx->ch_layout.nb_channels)
	{
		ret = AVERROR(EINVAL);
		goto free_and_end;
	}
	if (avctx->ch_layout.nb_channels && !av_channel_layout_check(&avctx->ch_layout))
	{
		ret = AVERROR(EINVAL);
		goto free_and_end;
	}
	if (avctx->ch_layout.nb_channels > (int)FF_SANE_NB_CHANNELS)
	{
		ret = AVERROR(EINVAL);
		goto free_and_end;
	}

	avctx->frame_num = 0;

	ret = ff_decode_preinit(avctx);
	if (ret < 0)
		goto free_and_end;

	if (codec2->init)
	{
		ret = codec2->init(avctx);
		if (ret < 0)
		{
			avci->needs_close = codec2->caps_internal & FF_CODEC_CAP_INIT_CLEANUP;
			goto free_and_end;
		}
	}
	avci->needs_close = 1;

	ret = 0;

	if (av_codec_is_decoder(avctx->codec))
	{
		if (!avctx->bit_rate)
			avctx->bit_rate = get_bit_rate(avctx);

		/* validate channel layout from the decoder */
		if ((avctx->ch_layout.nb_channels && !av_channel_layout_check(&avctx->ch_layout)) ||
			avctx->ch_layout.nb_channels > (int)FF_SANE_NB_CHANNELS)
		{
			ret = AVERROR(EINVAL);
			goto free_and_end;
		}
	}

end:
	return ret;

free_and_end:
	ff_codec_close(avctx);
	goto end;
}

void ff_codec_close(AVCodecContext* avctx)
{
	if (!avctx)
		return;

	if (avcodec_is_open(avctx))
	{
		AVCodecInternal* avci = avctx->internal;

		if (avci->needs_close && ffcodec(avctx->codec)->close)
			ffcodec(avctx->codec)->close(avctx);
		av_frame_free(&avci->buffer_frame);
		av_packet_free(&avci->buffer_pkt);
		av_packet_free(&avci->last_pkt_props);
		av_packet_free(&avci->in_pkt);
		ff_refstruct_unref(&avci->pool);
		av_bsf_free(&avci->bsf);
		av_freep(&avctx->internal);
	}

	avctx->codec = NULL;
}

int avcodec_is_open(AVCodecContext* s)
{
	return !!s->internal;
}

int avcodec_receive_frame(AVCodecContext* avctx, AVFrame* frame)
{
	av_frame_unref(frame);

	if (av_codec_is_decoder(avctx->codec))
		return ff_decode_receive_frame(avctx, frame);
	return AVERROR_ENCODER_NOT_FOUND;
}
