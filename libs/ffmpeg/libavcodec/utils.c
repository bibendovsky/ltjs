/*
 * utils for libavcodec
 * Copyright (c) 2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* utils. */

#include "libavutil/imgutils.h"
#include "avcodec.h"
#include "codec_internal.h"
#include "codec_par.h"
#include "internal.h"
#include <stdlib.h>
#include <limits.h>

int av_codec_is_decoder(const AVCodec* avcodec)
{
	const FFCodec* const codec = ffcodec(avcodec);
	return codec && (codec->cb_type == FF_CODEC_CB_TYPE_DECODE ||
		codec->cb_type == FF_CODEC_CB_TYPE_RECEIVE_FRAME);
}

int ff_set_dimensions(AVCodecContext* s, int width, int height)
{
	int ret = av_image_check_size2(width, height, AV_PIX_FMT_NONE, 0, s);

	if (ret < 0)
		width = height = 0;

	s->width = width;
	s->height = height;

	return ret;
}

void avcodec_align_dimensions2(AVCodecContext* s, int* width, int* height,
	int linesize_align[AV_NUM_DATA_POINTERS])
{
	int i;
	int w_align = 1;
	int h_align = 1;
	AVPixFmtDescriptor const* desc = av_pix_fmt_desc_get(s->pix_fmt);

	if (desc)
	{
		w_align = 1 << 1;
		h_align = 1 << 1;
	}

	switch (s->pix_fmt)
	{
		case AV_PIX_FMT_YUV420P:
		case AV_PIX_FMT_YUVA420P:
			w_align = 16; //FIXME assume 16 pixel per macroblock
			h_align = 16 * 2; // interlaced needs 2 macroblocks height
			if (s->codec_id == AV_CODEC_ID_BINKVIDEO)
				w_align = 16 * 2;
			break;
		default:
			break;
	}

	*width = FFALIGN(*width, w_align);
	*height = FFALIGN(*height, h_align);

	for (i = 0; i < 4; i++)
		linesize_align[i] = STRIDE_ALIGN;
}

int avpriv_codec_get_cap_skip_frame_fill_param(const AVCodec* codec)
{
	return !!(ffcodec(codec)->caps_internal & FF_CODEC_CAP_SKIP_FRAME_FILL_PARAM);
}

static int get_audio_frame_duration(enum AVCodecID id, int sr, int ch, int ba,
	uint32_t tag, int bits_per_coded_sample, int64_t bitrate,
	uint8_t* extradata, int frame_size, int frame_bytes)
{
	(void)ba;
	(void)tag;
	(void)bitrate;
	(void)extradata;

	int bps = 0;

	/* codecs with an exact constant bits per sample */
	if (bps > 0 && ch > 0 && frame_bytes > 0 && ch < 32768 && bps < 32768)
		return (frame_bytes * 8LL) / (bps * ch);
	bps = bits_per_coded_sample;

	/* codecs with a fixed packet duration */
	if (sr > 0)
	{
		/* calc from sample rate */
		if (id == AV_CODEC_ID_BINKAUDIO_DCT)
		{
			if (sr / 22050 > 22)
				return 0;
			return (480 << (sr / 22050));
		}
	}

	/* Fall back on using frame_size */
	if (frame_size > 1 && frame_bytes)
		return frame_size;

	return 0;
}

int av_get_audio_frame_duration(AVCodecContext* avctx, int frame_bytes)
{
	int channels = avctx->ch_layout.nb_channels;
	int duration;

	duration = get_audio_frame_duration(avctx->codec_id, avctx->sample_rate,
		channels, 0,
		avctx->codec_tag, 0,
		avctx->bit_rate, avctx->extradata, 0,
		frame_bytes);
	return FFMAX(0, duration);
}

int av_get_audio_frame_duration2(AVCodecParameters* par, int frame_bytes)
{
	int channels = par->ch_layout.nb_channels;
	int duration;

	duration = get_audio_frame_duration(par->codec_id, par->sample_rate,
		channels, par->block_align,
		par->codec_tag, par->bits_per_coded_sample,
		par->bit_rate, par->extradata, par->frame_size,
		frame_bytes);
	return FFMAX(0, duration);
}
