/*
 * copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVUTIL_PIXFMT_H
#define AVUTIL_PIXFMT_H

/* pixel format definitions */

#include "version.h"

/* Pixel format. */
enum AVPixelFormat
{
	AV_PIX_FMT_NONE = -1,
	AV_PIX_FMT_YUV420P,   // planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
	AV_PIX_FMT_YUVA420P,  // planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
	AV_PIX_FMT_NB         // number of pixel formats
};

/* Visual content value range. */
enum AVColorRange
{
	AVCOL_RANGE_UNSPECIFIED = 0,

	/*
	 * Narrow or limited range content.
	 * - For luma planes:
	 *       (219 * E + 16) * 2^(n-8)
	 *   F.ex. the range of 16-235 for 8 bits
	 * - For chroma planes:
	 *       (224 * E + 128) * 2^(n-8)
	 *   F.ex. the range of 16-240 for 8 bits
	 */
	AVCOL_RANGE_MPEG = 1,

	/*
	 * Full range content.
	 * - For RGB and luma planes:
	 *       (2^n - 1) * E
	 *   F.ex. the range of 0-255 for 8 bits
	 * - For chroma planes:
	 *       (2^n - 1) * E + 2^(n - 1)
	 *   F.ex. the range of 1-255 for 8 bits
	 */
	AVCOL_RANGE_JPEG = 2
};

#endif /* AVUTIL_PIXFMT_H */
