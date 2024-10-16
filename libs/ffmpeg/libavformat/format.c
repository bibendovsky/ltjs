/*
 * Format register and lookup
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "libavutil/avstring.h"
#include "avio_internal.h"
#include "avformat.h"
#include "demux.h"
#include "internal.h"

/* Format register and lookup */

const AVInputFormat* av_find_input_format(const char* short_name)
{
	const AVInputFormat* fmt = NULL;
	void* i = 0;
	while ((fmt = av_demuxer_iterate(&i)))
		if (av_match_name(short_name, fmt->name))
			return fmt;
	return NULL;
}

const AVInputFormat* av_probe_input_format3(const AVProbeData* pd, int is_opened, int* score_ret)
{
	(void)is_opened;

	static const uint8_t zerobuffer[AVPROBE_PADDING_SIZE] = {0};

	AVProbeData lpd = *pd;
	const AVInputFormat* fmt1 = NULL;
	const AVInputFormat* fmt = NULL;
	int score, score_max = 0;
	void* i = 0;

	if (!lpd.buf)
		lpd.buf = (unsigned char*)zerobuffer;

	while ((fmt1 = av_demuxer_iterate(&i)))
	{
		score = 0;
		if (ffifmt(fmt1)->read_probe)
		{
			score = ffifmt(fmt1)->read_probe(&lpd);
		}
		if (score > score_max)
		{
			score_max = score;
			fmt = fmt1;
		}
		else if (score == score_max)
			fmt = NULL;
	}
	*score_ret = score_max;

	return fmt;
}

const AVInputFormat* av_probe_input_format2(const AVProbeData* pd, int is_opened, int* score_max)
{
	int score_ret;
	const AVInputFormat* fmt = av_probe_input_format3(pd, is_opened, &score_ret);
	if (score_ret > *score_max)
	{
		*score_max = score_ret;
		return fmt;
	}
	else
		return NULL;
}

int av_probe_input_buffer2(AVIOContext* pb, const AVInputFormat** fmt,
	const char* filename, void* logctx,
	unsigned int offset, unsigned int max_probe_size)
{
	(void)filename;
	(void)logctx;

	AVProbeData pd = {NULL, 0};
	uint8_t* buf = NULL;
	int ret = 0, probe_size, buf_offset = 0;
	int score = 0;
	int ret2;
	int eof = 0;

	if (!max_probe_size)
		max_probe_size = PROBE_BUF_MAX;
	else if (max_probe_size < PROBE_BUF_MIN)
	{
		return AVERROR(EINVAL);
	}

	if (offset >= max_probe_size)
		return AVERROR(EINVAL);

	for (probe_size = PROBE_BUF_MIN;
		probe_size <= (int)max_probe_size && !*fmt && !eof;
		probe_size = FFMIN(probe_size << 1, FFMAX((int)max_probe_size, probe_size + 1)))
	{
		score = probe_size < (int)max_probe_size ? AVPROBE_SCORE_RETRY : 0;

		/* Read probe data. */
		if ((ret = av_reallocp(&buf, probe_size + AVPROBE_PADDING_SIZE)) < 0)
			goto fail;
		if ((ret = avio_read(pb, buf + buf_offset, probe_size - buf_offset)) < 0)
		{
			/* Fail if error was not end of file, otherwise, lower score. */
			if (ret != AVERROR_EOF)
				goto fail;

			score = 0;
			ret = 0; /* error was end of file, nothing read */
			eof = 1;
		}
		buf_offset += ret;
		if ((unsigned int)buf_offset < offset)
			continue;
		pd.buf_size = buf_offset - offset;
		pd.buf = &buf[offset];

		memset(pd.buf + pd.buf_size, 0, AVPROBE_PADDING_SIZE);

		/* Guess file format. */
		*fmt = av_probe_input_format2(&pd, 1, &score);
	}

	if (!*fmt)
		ret = AVERROR_INVALIDDATA;

fail:
	/* Rewind. Reuse probe buffer to avoid seeking. */
	ret2 = ffio_rewind_with_probe_data(pb, &buf, buf_offset);
	if (ret >= 0)
		ret = ret2;

	return ret < 0 ? ret : score;
}
