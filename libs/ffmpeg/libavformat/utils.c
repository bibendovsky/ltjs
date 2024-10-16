/*
 * various utility functions for use within FFmpeg
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdint.h>

#include "libavcodec/internal.h"

#include "avformat.h"
#include "avio_internal.h"

/* various utility functions for use within FFmpeg */

/* an arbitrarily chosen "sane" max packet size -- 50M */
#define SANE_CHUNK_SIZE (50000000)

/* Read the data in sane-sized chunks and append to pkt.
 * Return the number of bytes read or an error. */
static int append_packet_chunked(AVIOContext* s, AVPacket* pkt, int size)
{
	int orig_size = pkt->size;
	int ret;

	do
	{
		int prev_size = pkt->size;
		int read_size;

		/* When the caller requests a lot of data, limit it to the amount
		 * left in file or SANE_CHUNK_SIZE when it is not known. */
		read_size = size;
		if (read_size > SANE_CHUNK_SIZE / 10)
		{
			read_size = ffio_limit(s, read_size);
			// If filesize/maxsize is unknown, limit to SANE_CHUNK_SIZE
			if (ffiocontext(s)->maxsize < 0)
				read_size = FFMIN(read_size, SANE_CHUNK_SIZE);
		}

		ret = av_grow_packet(pkt, read_size);
		if (ret < 0)
			break;

		ret = avio_read(s, pkt->data + prev_size, read_size);
		if (ret != read_size)
		{
			av_shrink_packet(pkt, prev_size + FFMAX(ret, 0));
			break;
		}

		size -= read_size;
	} while (size > 0);
	if (size > 0)
		pkt->flags |= AV_PKT_FLAG_CORRUPT;

	if (!pkt->size)
		av_packet_unref(pkt);
	return pkt->size > orig_size ? pkt->size - orig_size : ret;
}

int av_get_packet(AVIOContext* s, AVPacket* pkt, int size)
{
	av_packet_unref(pkt);
	pkt->pos = avio_tell(s);

	return append_packet_chunked(s, pkt, size);
}

int ff_alloc_extradata(AVCodecParameters* par, int size)
{
	av_freep(&par->extradata);
	par->extradata_size = 0;

	if (size < 0 || size >= INT32_MAX - AV_INPUT_BUFFER_PADDING_SIZE)
		return AVERROR(EINVAL);

	par->extradata = av_malloc(size + AV_INPUT_BUFFER_PADDING_SIZE);
	if (!par->extradata)
		return AVERROR(ENOMEM);

	memset(par->extradata + size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
	par->extradata_size = size;

	return 0;
}
