/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVFORMAT_AVIO_INTERNAL_H
#define AVFORMAT_AVIO_INTERNAL_H

#include "avio.h"

#include "libavutil/log.h"

typedef struct FFIOContext
{
	AVIOContext pub;
	/* A callback that is used instead of short_seek_threshold. */
	int (*short_seek_get)(void* opaque);

	/* Threshold to favor readahead over seek. */
	int short_seek_threshold;

	enum AVIODataMarkerType current_type;

	/* max filesize, used to limit allocations */
	int64_t maxsize;

	/* Bytes read statistic */
	int64_t bytes_read;

	/* Bytes written statistic */
	int64_t bytes_written;

	/* Original buffer size used after probing to ensure seekback and to reset the buffer size */
	int orig_buffer_size;

	/* Written output size is updated each time a successful writeout ends up further position-wise */
	int64_t written_output_size;
} FFIOContext;

static inline FFIOContext* ffiocontext(AVIOContext* ctx)
{
	return (FFIOContext*)ctx;
}

void ffio_init_context(FFIOContext* s,
	unsigned char* buffer,
	int buffer_size,
	int write_flag,
	void* opaque,
	int (*read_packet)(void* opaque, uint8_t* buf, int buf_size),
	int (*write_packet)(void* opaque, const uint8_t* buf, int buf_size),
	int64_t(*seek)(void* opaque, int64_t offset, int whence));

/* Rewind the AVIOContext using the specified buffer containing the first buf_size bytes of the file. */
int ffio_rewind_with_probe_data(AVIOContext* s, unsigned char** buf, int buf_size);

/* Read size bytes from AVIOContext into buf. */
int ffio_read_size(AVIOContext* s, unsigned char* buf, int size);

int ffio_limit(AVIOContext* s, int size);

#endif /* AVFORMAT_AVIO_INTERNAL_H */
