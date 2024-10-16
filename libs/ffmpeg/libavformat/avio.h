/*
 * copyright (c) 2001 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef AVFORMAT_AVIO_H
#define AVFORMAT_AVIO_H

/* Buffered I/O operations */

#include <stdint.h>
#include <stdio.h>

#include "libavutil/dict.h"
#include "libavutil/log.h"

#include "libavformat/version_major.h"

/* Seeking works like for a local file. */
#define AVIO_SEEKABLE_NORMAL (1 << 0)

/* Different data types that can be returned via the AVIO write_data_type callback. */
enum AVIODataMarkerType
{
	/* A point in the output bytestream where a decoder can start decoding (i.e. a keyframe). */
	AVIO_DATA_MARKER_SYNC_POINT,
	/* A point in the output bytestream where a demuxer can start parsing (for non self synchronizing bytestream formats). */
	AVIO_DATA_MARKER_BOUNDARY_POINT,
	/* This is any, unlabelled data. */
	AVIO_DATA_MARKER_UNKNOWN,
};

/* Bytestream IO Context. */
typedef struct AVIOContext
{
/*
 * The following shows the relationship between buffer, buf_ptr,
 * buf_ptr_max, buf_end, buf_size, and pos, when reading and when writing
 * (since AVIOContext is used for both):
 *
 **********************************************************************************
 *                                   READING
 **********************************************************************************
 *
 *                            |              buffer_size              |
 *                            |---------------------------------------|
 *                            |                                       |
 *
 *                         buffer          buf_ptr       buf_end
 *                            +---------------+-----------------------+
 *                            |/ / / / / / / /|/ / / / / / /|         |
 *  read buffer:              |/ / consumed / | to be read /|         |
 *                            |/ / / / / / / /|/ / / / / / /|         |
 *                            +---------------+-----------------------+
 *
 *                                                         pos
 *              +-------------------------------------------+-----------------+
 *  input file: |                                           |                 |
 *              +-------------------------------------------+-----------------+
 *
 *
 **********************************************************************************
 *                                   WRITING
 **********************************************************************************
 *
 *                             |          buffer_size                 |
 *                             |--------------------------------------|
 *                             |                                      |
 *
 *                                                buf_ptr_max
 *                          buffer                 (buf_ptr)       buf_end
 *                             +-----------------------+--------------+
 *                             |/ / / / / / / / / / / /|              |
 *  write buffer:              | / / to be flushed / / |              |
 *                             |/ / / / / / / / / / / /|              |
 *                             +-----------------------+--------------+
 *                               buf_ptr can be in this
 *                               due to a backward seek
 *
 *                            pos
 *               +-------------+----------------------------------------------+
 *  output file: |             |                                              |
 *               +-------------+----------------------------------------------+
 *
 */
	unsigned char* buffer;	/* Start of the buffer. */
	int buffer_size;		/* Maximum buffer size */
	unsigned char* buf_ptr;	/* Current position in the buffer */
	unsigned char* buf_end;	/* End of the data, may be less than
							 * buffer+buffer_size if the read function returned
							 * less data than requested, e.g. for streams where
							 * no more data has been received yet. */
	void* opaque;			/* A private pointer, passed to the read/write/seek/... functions. */
	int (*read_packet)(void* opaque, uint8_t* buf, int buf_size);
	int (*write_packet)(void* opaque, const uint8_t* buf, int buf_size);
	int64_t(*seek)(void* opaque, int64_t offset, int whence);
	int64_t pos;			/* position in the file of the current buffer */
	int eof_reached;		/* true if was unable to read due to error or eof */
	int error;				/* contains the error code or 0 if no error happened */
	int write_flag;			/* true if open for writing */
	int max_packet_size;
	int min_packet_size;	/* Try to buffer at least this amount of data before flushing it. */
	/* A combination of AVIO_SEEKABLE_ flags or 0 when the stream is not seekable. */
	int seekable;

	/*
	 * Maximum reached position before a backward seek in the write buffer,
	 * used keeping track of already written data for a later flush.
	 */
	unsigned char* buf_ptr_max;

	/* Read-only statistic of bytes read for this AVIOContext. */
	int64_t bytes_read;

	/* Read-only statistic of bytes written for this AVIOContext. */
	int64_t bytes_written;
} AVIOContext;

/* Allocate and initialize an AVIOContext for buffered I/O. */
AVIOContext* avio_alloc_context(
	unsigned char* buffer,
	int buffer_size,
	int write_flag,
	void* opaque,
	int (*read_packet)(void* opaque, uint8_t* buf, int buf_size),
	int (*write_packet)(void* opaque, const uint8_t* buf, int buf_size),
	int64_t(*seek)(void* opaque, int64_t offset, int whence));

/* Free the supplied IO context and everything associated with it. */
void avio_context_free(AVIOContext** s);

/*
 * ORing this as the "whence" parameter to a seek function causes it to
 * return the filesize without seeking anywhere.
 */
#define AVSEEK_SIZE 0x10000

/*
 * Passing this flag as the "whence" parameter to a seek function causes it to
 * seek by any means (like reopening and linear reading) or other normally unreasonable
 * means that can be extremely slow.
 */
#define AVSEEK_FORCE 0x20000

/* fseek() equivalent for AVIOContext. */
int64_t avio_seek(AVIOContext* s, int64_t offset, int whence);

/* Skip given number of bytes forward */
int64_t avio_skip(AVIOContext* s, int64_t offset);

/* ftell() equivalent for AVIOContext. */
static inline int64_t avio_tell(AVIOContext* s)
{
	return avio_seek(s, 0, SEEK_CUR);
}

/* Get the filesize. */
int64_t avio_size(AVIOContext* s);

/* Similar to feof() but also returns nonzero on read errors. */
int avio_feof(AVIOContext* s);

/* Force flushing of buffered data. */
void avio_flush(AVIOContext* s);

/* Read size bytes from AVIOContext into buf. */
int avio_read(AVIOContext* s, unsigned char* buf, int size);

/* Functions for reading from AVIOContext */

int          avio_r8(AVIOContext* s);
unsigned int avio_rl16(AVIOContext* s);
unsigned int avio_rl32(AVIOContext* s);

/* URL open modes */

#define AVIO_FLAG_READ  1 /* read-only */
#define AVIO_FLAG_WRITE 2 /* write-only */

/* Close the resource accessed by the AVIOContext s and free it. */
int avio_close(AVIOContext* s);

/* Close the resource accessed by the AVIOContext *s, free it and set the pointer pointing to it to NULL. */
int avio_closep(AVIOContext** s);

#endif /* AVFORMAT_AVIO_H */
