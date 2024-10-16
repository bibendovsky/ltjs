/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "error.h"
#include "samplefmt.h"

#include <limits.h>
#include <string.h>

int av_get_bytes_per_sample(enum AVSampleFormat sample_fmt)
{
	return sample_fmt < 0 || sample_fmt >= AV_SAMPLE_FMT_NB ? 0 : 4;
}

int av_sample_fmt_is_planar(enum AVSampleFormat sample_fmt)
{
	return sample_fmt == AV_SAMPLE_FMT_FLTP;
}

int av_samples_get_buffer_size(int* linesize, int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt, int align)
{
	int line_size;
	int sample_size = av_get_bytes_per_sample(sample_fmt);
	int planar = av_sample_fmt_is_planar(sample_fmt);

	/* validate parameter ranges */
	if (!sample_size || nb_samples <= 0 || nb_channels <= 0)
		return AVERROR(EINVAL);

	/* auto-select alignment if not specified */
	if (!align)
	{
		if (nb_samples > INT_MAX - 31)
			return AVERROR(EINVAL);
		align = 1;
		nb_samples = FFALIGN(nb_samples, 32);
	}

	/* check for integer overflow */
	if (nb_channels > INT_MAX / align || (int64_t)nb_channels * nb_samples > (INT_MAX - (align * nb_channels)) / sample_size)
		return AVERROR(EINVAL);

	line_size = planar ? FFALIGN(nb_samples * sample_size, align) : FFALIGN(nb_samples * sample_size * nb_channels, align);
	if (linesize)
		*linesize = line_size;

	return planar ? line_size * nb_channels : line_size;
}

int av_samples_copy(uint8_t* const* dst, uint8_t* const* src, int dst_offset,
	int src_offset, int nb_samples, int nb_channels,
	enum AVSampleFormat sample_fmt)
{
	int planar = av_sample_fmt_is_planar(sample_fmt);
	int planes = planar ? nb_channels : 1;
	int block_align = av_get_bytes_per_sample(sample_fmt) * (planar ? 1 : nb_channels);
	int data_size = nb_samples * block_align;
	int i;

	dst_offset *= block_align;
	src_offset *= block_align;

	if ((dst[0] < src[0] ? src[0] - dst[0] : dst[0] - src[0]) >= data_size)
	{
		for (i = 0; i < planes; i++)
			memcpy(dst[i] + dst_offset, src[i] + src_offset, data_size);
	}
	else
	{
		for (i = 0; i < planes; i++)
			memmove(dst[i] + dst_offset, src[i] + src_offset, data_size);
	}

	return 0;
}
