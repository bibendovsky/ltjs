/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVUTIL_IMGUTILS_H
#define AVUTIL_IMGUTILS_H

/* misc image utilities */

#include <stddef.h>
#include <stdint.h>
#include "pixdesc.h"
#include "pixfmt.h"
#include "rational.h"

/* Compute the max pixel step for each plane of an image with a format described by pixdesc. */
void av_image_fill_max_pixsteps(int max_pixsteps[4], int max_pixstep_comps[4], const AVPixFmtDescriptor* pixdesc);

/* Compute the size of an image line with format pix_fmt and width width for the plane plane. */
int av_image_get_linesize(enum AVPixelFormat pix_fmt, int width, int plane);

/* Fill plane linesizes for an image with pixel format pix_fmt and width width. */
int av_image_fill_linesizes(int linesizes[4], enum AVPixelFormat pix_fmt, int width);

/* Fill plane sizes for an image with pixel format pix_fmt and height height. */
int av_image_fill_plane_sizes(size_t size[4], enum AVPixelFormat pix_fmt, int height, const ptrdiff_t linesizes[4]);

/* Fill plane data pointers for an image with pixel format pix_fmt and height height. */
int av_image_fill_pointers(uint8_t* data[4], enum AVPixelFormat pix_fmt, int height,
	uint8_t* ptr, const int linesizes[4]);

/* Copy image in src_data to dst_data. */
void av_image_copy(uint8_t* const dst_data[4], const int dst_linesizes[4],
	const uint8_t* const src_data[4], const int src_linesizes[4],
	enum AVPixelFormat pix_fmt, int width, int height);

/* Wrapper around av_image_copy(). */
static inline
void av_image_copy2(uint8_t* const dst_data[4], const int dst_linesizes[4],
	uint8_t* const src_data[4], const int src_linesizes[4],
	enum AVPixelFormat pix_fmt, int width, int height)
{
	av_image_copy(dst_data, dst_linesizes,
		(const uint8_t* const*)src_data, src_linesizes,
		pix_fmt, width, height);
}

/*
 * Check if the given dimension of an image is valid, meaning that all
 * bytes of the image can be addressed with a signed int.
 */
int av_image_check_size(unsigned int w, unsigned int h, int log_offset, void* log_ctx);

/* Check if the given dimension of an image is valid. */
int av_image_check_size2(unsigned int w, unsigned int h, enum AVPixelFormat pix_fmt, int log_offset, void* log_ctx);

/* Check if the given sample aspect ratio of an image is valid. */
int av_image_check_sar(unsigned int w, unsigned int h, AVRational sar);

#endif /* AVUTIL_IMGUTILS_H */
