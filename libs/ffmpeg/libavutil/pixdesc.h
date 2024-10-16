/*
 * pixel format descriptor
 * Copyright (c) 2009 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVUTIL_PIXDESC_H
#define AVUTIL_PIXDESC_H

#include <stdint.h>

#include "pixfmt.h"

/*
 * Descriptor that unambiguously describes how the bits of a pixel are
 * stored in the up to 4 data planes of an image. It also stores the
 * subsampling factors and number of components.
 */
typedef struct AVPixFmtDescriptor
{
	const char* name;
	uint8_t nb_components;  // The number of components each pixel has, (1-4)

	/* Combination of AV_PIX_FMT_FLAG_... flags. */
	uint64_t flags;
} AVPixFmtDescriptor;

/* At least one pixel component is not in the first data plane. */
#define AV_PIX_FMT_FLAG_PLANAR (1 << 4)
/* The pixel format has an alpha channel. */
#define AV_PIX_FMT_FLAG_ALPHA  (1 << 7)

/* Return a pixel format descriptor for provided pixel format or NULL if this pixel format is unknown. */
const AVPixFmtDescriptor* av_pix_fmt_desc_get(enum AVPixelFormat pix_fmt);

/* Return number of planes in pix_fmt, a negative AVERROR if pix_fmt is not a valid pixel format. */
int av_pix_fmt_count_planes(enum AVPixelFormat pix_fmt);

#endif /* AVUTIL_PIXDESC_H */
