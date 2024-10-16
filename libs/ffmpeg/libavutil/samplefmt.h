/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVUTIL_SAMPLEFMT_H
#define AVUTIL_SAMPLEFMT_H

#include <stdint.h>

/* Audio sample format enumeration and related convenience functions. */

/* Audio sample formats */
enum AVSampleFormat
{
	AV_SAMPLE_FMT_NONE = -1,
	AV_SAMPLE_FMT_FLT,  // float
	AV_SAMPLE_FMT_FLTP, // float, planar
	AV_SAMPLE_FMT_NB    // Number of sample formats. DO NOT USE if linking dynamically
};

/* Return number of bytes per sample. */
int av_get_bytes_per_sample(enum AVSampleFormat sample_fmt);

/* Check if the sample format is planar. */
int av_sample_fmt_is_planar(enum AVSampleFormat sample_fmt);

/* Get the required buffer size for the given audio parameters. */
int av_samples_get_buffer_size(int* linesize, int nb_channels, int nb_samples,
	enum AVSampleFormat sample_fmt, int align);

/* Functions that manipulate audio samples */

/* Copy samples from src to dst. */
int av_samples_copy(uint8_t* const* dst, uint8_t* const* src, int dst_offset,
	int src_offset, int nb_samples, int nb_channels,
	enum AVSampleFormat sample_fmt);

#endif /* AVUTIL_SAMPLEFMT_H */
