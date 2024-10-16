/*
 * pixel format descriptor
 * Copyright (c) 2009 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stddef.h>

#include "common.h"
#include "pixfmt.h"
#include "pixdesc.h"

static const AVPixFmtDescriptor av_pix_fmt_descriptors[AV_PIX_FMT_NB] = {
	[AV_PIX_FMT_YUV420P] = {
		.name = "yuv420p",
		.nb_components = 3,
		.flags = AV_PIX_FMT_FLAG_PLANAR,
	},
	[AV_PIX_FMT_YUVA420P] = {
		.name = "yuva420p",
		.nb_components = 4,
		.flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
	},
};

const AVPixFmtDescriptor* av_pix_fmt_desc_get(enum AVPixelFormat pix_fmt)
{
	if (pix_fmt < 0 || pix_fmt >= AV_PIX_FMT_NB)
		return NULL;
	return &av_pix_fmt_descriptors[pix_fmt];
}

int av_pix_fmt_count_planes(enum AVPixelFormat pix_fmt)
{
	const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(pix_fmt);

	if (!desc)
		return AVERROR(EINVAL);

	return desc->nb_components;
}
