/*
 * AVCodecParameters functions for libavcodec
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* AVCodecParameters functions for libavcodec. */

#include <string.h>
#include "libavutil/mem.h"
#include "avcodec.h"
#include "codec_par.h"
#include "packet.h"

static void codec_parameters_reset(AVCodecParameters* par)
{
	av_freep(&par->extradata);
	av_channel_layout_uninit(&par->ch_layout);

	memset(par, 0, sizeof(*par));

	par->codec_type = AVMEDIA_TYPE_UNKNOWN;
	par->codec_id = AV_CODEC_ID_NONE;
	par->format = -1;
	par->ch_layout.order = AV_CHANNEL_ORDER_UNSPEC;
	par->color_range = AVCOL_RANGE_UNSPECIFIED;
	par->sample_aspect_ratio = (AVRational){0, 1};
	par->framerate = (AVRational){0, 1};
}

AVCodecParameters* avcodec_parameters_alloc(void)
{
	AVCodecParameters* par = av_mallocz(sizeof(*par));

	if (!par)
		return NULL;
	codec_parameters_reset(par);
	return par;
}

void avcodec_parameters_free(AVCodecParameters** ppar)
{
	AVCodecParameters* par = *ppar;

	if (!par)
		return;
	codec_parameters_reset(par);

	av_freep(ppar);
}

int avcodec_parameters_copy(AVCodecParameters* dst, const AVCodecParameters* src)
{
	int ret;

	codec_parameters_reset(dst);
	memcpy(dst, src, sizeof(*dst));

	dst->ch_layout = (AVChannelLayout){0};
	dst->extradata = NULL;
	dst->extradata_size = 0;
	if (src->extradata)
	{
		dst->extradata = av_mallocz(src->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
		if (!dst->extradata)
			return AVERROR(ENOMEM);
		memcpy(dst->extradata, src->extradata, src->extradata_size);
		dst->extradata_size = src->extradata_size;
	}

	ret = av_channel_layout_copy(&dst->ch_layout, &src->ch_layout);
	if (ret < 0)
		return ret;

	return 0;
}

int avcodec_parameters_from_context(AVCodecParameters* par, const AVCodecContext* codec)
{
	int ret;

	codec_parameters_reset(par);

	par->codec_type = codec->codec_type;
	par->codec_id = codec->codec_id;
	par->codec_tag = codec->codec_tag;

	par->bit_rate = codec->bit_rate;

	switch (par->codec_type)
	{
		case AVMEDIA_TYPE_VIDEO:
			par->format = codec->pix_fmt;
			par->width = codec->width;
			par->height = codec->height;
			par->color_range = codec->color_range;
			par->sample_aspect_ratio = codec->sample_aspect_ratio;
			par->framerate = codec->framerate;
			break;
		case AVMEDIA_TYPE_AUDIO:
			par->format = codec->sample_fmt;
			ret = av_channel_layout_copy(&par->ch_layout, &codec->ch_layout);
			if (ret < 0)
				return ret;
			par->sample_rate = codec->sample_rate;
			break;
		default:
			break;
	}

	if (codec->extradata)
	{
		par->extradata = av_mallocz(codec->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
		if (!par->extradata)
			return AVERROR(ENOMEM);
		memcpy(par->extradata, codec->extradata, codec->extradata_size);
		par->extradata_size = codec->extradata_size;
	}

	return 0;
}

int avcodec_parameters_to_context(AVCodecContext* codec, const AVCodecParameters* par)
{
	int ret;

	codec->codec_type = par->codec_type;
	codec->codec_id = par->codec_id;
	codec->codec_tag = par->codec_tag;

	codec->bit_rate = par->bit_rate;

	switch (par->codec_type)
	{
		case AVMEDIA_TYPE_VIDEO:
			codec->pix_fmt = par->format;
			codec->width = par->width;
			codec->height = par->height;
			codec->color_range = par->color_range;
			codec->sample_aspect_ratio = par->sample_aspect_ratio;
			codec->framerate = par->framerate;
			break;
		case AVMEDIA_TYPE_AUDIO:
			codec->sample_fmt = par->format;
			ret = av_channel_layout_copy(&codec->ch_layout, &par->ch_layout);
			if (ret < 0)
				return ret;
			codec->sample_rate = par->sample_rate;
			break;
		default:
			break;
	}

	av_freep(&codec->extradata);
	codec->extradata_size = 0;
	if (par->extradata)
	{
		codec->extradata = av_mallocz(par->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
		if (!codec->extradata)
			return AVERROR(ENOMEM);
		memcpy(codec->extradata, par->extradata, par->extradata_size);
		codec->extradata_size = par->extradata_size;
	}

	return 0;
}
