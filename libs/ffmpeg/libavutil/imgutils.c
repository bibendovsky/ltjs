/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* misc image utilities */

#include "avassert.h"
#include "common.h"
#include "imgutils.h"
#include "mathematics.h"

void av_image_fill_max_pixsteps(int max_pixsteps[4], int max_pixstep_comps[4], const AVPixFmtDescriptor* pixdesc)
{
	(void)pixdesc;

	int i;
	memset(max_pixsteps, 0, 4 * sizeof(max_pixsteps[0]));
	if (max_pixstep_comps)
		memset(max_pixstep_comps, 0, 4 * sizeof(max_pixstep_comps[0]));

	for (i = 0; i < 4; i++)
	{
		if (1 > max_pixsteps[i])
		{
			max_pixsteps[i] = 1;
			if (max_pixstep_comps)
				max_pixstep_comps[i] = i;
		}
	}
}

static inline
int image_get_linesize(int width, int plane, int max_step, int max_step_comp, const AVPixFmtDescriptor* desc)
{
	(void)plane;

	int s, shifted_w, linesize;

	if (!desc)
		return AVERROR(EINVAL);

	if (width < 0)
		return AVERROR(EINVAL);
	s = (max_step_comp == 1 || max_step_comp == 2) ? 1 : 0;
	shifted_w = ((width + (1 << s) - 1)) >> s;
	if (shifted_w && max_step > INT_MAX / shifted_w)
		return AVERROR(EINVAL);
	linesize = max_step * shifted_w;
	return linesize;
}

int av_image_get_linesize(enum AVPixelFormat pix_fmt, int width, int plane)
{
	const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(pix_fmt);
	int max_step[4]; /* max pixel step for each plane */
	int max_step_comp[4]; /* the component for each plane which has the max pixel step */

	if (!desc)
		return AVERROR(EINVAL);

	av_image_fill_max_pixsteps(max_step, max_step_comp, desc);
	return image_get_linesize(width, plane, max_step[plane], max_step_comp[plane], desc);
}

int av_image_fill_linesizes(int linesizes[4], enum AVPixelFormat pix_fmt, int width)
{
	int i, ret;
	const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(pix_fmt);
	int max_step[4]; /* max pixel step for each plane */
	int max_step_comp[4]; /* the component for each plane which has the max pixel step */

	memset(linesizes, 0, 4 * sizeof(linesizes[0]));

	if (!desc)
		return AVERROR(EINVAL);

	av_image_fill_max_pixsteps(max_step, max_step_comp, desc);
	for (i = 0; i < 4; i++)
	{
		if ((ret = image_get_linesize(width, i, max_step[i], max_step_comp[i], desc)) < 0)
			return ret;
		linesizes[i] = ret;
	}

	return 0;
}

int av_image_fill_plane_sizes(size_t sizes[4], enum AVPixelFormat pix_fmt, int height, const ptrdiff_t linesizes[4])
{
	int i;

	const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(pix_fmt);
	memset(sizes, 0, sizeof(sizes[0]) * 4);

	if (!desc)
		return AVERROR(EINVAL);

	if ((size_t)linesizes[0] > SIZE_MAX / height)
		return AVERROR(EINVAL);
	sizes[0] = linesizes[0] * (size_t)height;

	for (i = 1; i < desc->nb_components; i++)
	{
		int h, s = (i == 1 || i == 2) ? 1 : 0;
		h = (height + (1 << s) - 1) >> s;
		if ((size_t)linesizes[i] > SIZE_MAX / h)
			return AVERROR(EINVAL);
		sizes[i] = (size_t)h * linesizes[i];
	}

	return 0;
}

int av_image_fill_pointers(uint8_t* data[4], enum AVPixelFormat pix_fmt, int height, uint8_t* ptr, const int linesizes[4])
{
	int i, ret;
	ptrdiff_t linesizes1[4];
	size_t sizes[4];

	memset(data, 0, sizeof(data[0]) * 4);

	for (i = 0; i < 4; i++)
		linesizes1[i] = linesizes[i];

	ret = av_image_fill_plane_sizes(sizes, pix_fmt, height, linesizes1);
	if (ret < 0)
		return ret;

	ret = 0;
	for (i = 0; i < 4; i++)
	{
		if (sizes[i] > (size_t)(INT_MAX - ret))
			return AVERROR(EINVAL);
		ret += (int)sizes[i];
	}

	if (!ptr)
		return ret;

	data[0] = ptr;
	for (i = 1; i < 4 && sizes[i]; i++)
		data[i] = data[i - 1] + sizes[i - 1];

	return ret;
}

int av_image_check_size2(unsigned int w, unsigned int h, enum AVPixelFormat pix_fmt, int log_offset, void* log_ctx)
{
	(void)log_offset;
	(void)log_ctx;

	int64_t stride = av_image_get_linesize(pix_fmt, w, 0);
	if (stride <= 0)
		stride = 8LL * w;
	stride += 128 * 8;

	if (w == 0 || h == 0 || w > INT32_MAX || h > INT32_MAX || stride >= INT_MAX || stride * (h + 128ULL) >= INT_MAX)
	{
		return AVERROR(EINVAL);
	}

	return 0;
}

int av_image_check_size(unsigned int w, unsigned int h, int log_offset, void* log_ctx)
{
	return av_image_check_size2(w, h, AV_PIX_FMT_NONE, log_offset, log_ctx);
}

int av_image_check_sar(unsigned int w, unsigned int h, AVRational sar)
{
	int64_t scaled_dim;

	if (sar.den <= 0 || sar.num < 0)
		return AVERROR(EINVAL);

	if (!sar.num || sar.num == sar.den)
		return 0;

	if (sar.num < sar.den)
		scaled_dim = av_rescale_rnd(w, sar.num, sar.den, AV_ROUND_ZERO);
	else
		scaled_dim = av_rescale_rnd(h, sar.den, sar.num, AV_ROUND_ZERO);

	if (scaled_dim > 0)
		return 0;

	return AVERROR(EINVAL);
}

static void image_copy_plane(uint8_t* dst, ptrdiff_t dst_linesize,
	const uint8_t* src, ptrdiff_t src_linesize,
	ptrdiff_t bytewidth, int height)
{
	if (!dst || !src)
		return;
	av_assert0(FFABS(src_linesize) >= bytewidth);
	av_assert0(FFABS(dst_linesize) >= bytewidth);
	for (; height > 0; height--)
	{
		memcpy(dst, src, bytewidth);
		dst += dst_linesize;
		src += src_linesize;
	}
}

static void image_copy(uint8_t* const dst_data[4], const ptrdiff_t dst_linesizes[4],
	const uint8_t* const src_data[4], const ptrdiff_t src_linesizes[4],
	enum AVPixelFormat pix_fmt, int width, int height,
	void (*copy_plane)(uint8_t*, ptrdiff_t, const uint8_t*,
	ptrdiff_t, ptrdiff_t, int))
{
	const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(pix_fmt);

	if (!desc)
		return;

	int i, planes_nb = 0;

	for (i = 0; i < desc->nb_components; i++)
		planes_nb = FFMAX(planes_nb, i + 1);

	for (i = 0; i < planes_nb; i++)
	{
		int h = height;
		ptrdiff_t bwidth = av_image_get_linesize(pix_fmt, width, i);
		if (bwidth < 0)
		{
			return;
		}
		if (i == 1 || i == 2)
		{
			h = AV_CEIL_RSHIFT(height, 1);
		}
		copy_plane(dst_data[i], dst_linesizes[i], src_data[i], src_linesizes[i], bwidth, h);
	}
}

void av_image_copy(uint8_t* const dst_data[4], const int dst_linesizes[4],
	const uint8_t* const src_data[4], const int src_linesizes[4],
	enum AVPixelFormat pix_fmt, int width, int height)
{
	ptrdiff_t dst_linesizes1[4], src_linesizes1[4];
	int i;

	for (i = 0; i < 4; i++)
	{
		dst_linesizes1[i] = dst_linesizes[i];
		src_linesizes1[i] = src_linesizes[i];
	}

	image_copy(dst_data, dst_linesizes1, src_data, src_linesizes1, pix_fmt, width, height, image_copy_plane);
}
