/*
 * buffered I/O
 * Copyright (c) 2000,2001 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "libavutil/avassert.h"
#include "libavcodec/defs.h"
#include "avio.h"
#include "avio_internal.h"
#include "internal.h"

#define IO_BUFFER_SIZE 32768

/**
 * Do seeks within this distance ahead of the current buffer by skipping
 * data instead of calling the protocol seek function, for seekable
 * protocols.
 */
#define SHORT_SEEK_THRESHOLD 32768

static void fill_buffer(AVIOContext* s);
static int url_resetbuf(AVIOContext* s, int flags);
static int set_buf_size(AVIOContext* s, int buf_size);

void ffio_init_context(FFIOContext* ctx,
	unsigned char* buffer,
	int buffer_size,
	int write_flag,
	void* opaque,
	int (*read_packet)(void* opaque, uint8_t* buf, int buf_size),
	int (*write_packet)(void* opaque, const uint8_t* buf, int buf_size),
	int64_t(*seek)(void* opaque, int64_t offset, int whence))
{
	AVIOContext* const s = &ctx->pub;

	memset(ctx, 0, sizeof(*ctx));

	s->buffer = buffer;
	ctx->orig_buffer_size =
		s->buffer_size = buffer_size;
	s->buf_ptr = buffer;
	s->buf_ptr_max = buffer;
	s->opaque = opaque;

	url_resetbuf(s, write_flag ? AVIO_FLAG_WRITE : AVIO_FLAG_READ);

	s->write_packet = write_packet;
	s->read_packet = read_packet;
	s->seek = seek;
	s->pos = 0;
	s->eof_reached = 0;
	s->error = 0;
	s->seekable = seek ? AVIO_SEEKABLE_NORMAL : 0;
	s->min_packet_size = 0;
	s->max_packet_size = 0;
	ctx->short_seek_threshold = SHORT_SEEK_THRESHOLD;

	if (!read_packet && !write_flag)
	{
		s->pos = buffer_size;
		s->buf_end = s->buffer + buffer_size;
	}

	ctx->current_type = AVIO_DATA_MARKER_UNKNOWN;
	ctx->short_seek_get = NULL;
}

AVIOContext* avio_alloc_context(
	unsigned char* buffer,
	int buffer_size,
	int write_flag,
	void* opaque,
	int (*read_packet)(void* opaque, uint8_t* buf, int buf_size),
	int (*write_packet)(void* opaque, const uint8_t* buf, int buf_size),
	int64_t(*seek)(void* opaque, int64_t offset, int whence))
{
	FFIOContext* s = av_malloc(sizeof(*s));
	if (!s)
		return NULL;
	ffio_init_context(s, buffer, buffer_size, write_flag, opaque, read_packet, write_packet, seek);
	return &s->pub;
}

void avio_context_free(AVIOContext** ps)
{
	av_freep(ps);
}

static void writeout(AVIOContext* s, const uint8_t* data, int len)
{
	FFIOContext* const ctx = ffiocontext(s);
	if (!s->error)
	{
		int ret = 0;
		if (s->write_packet)
			ret = s->write_packet(s->opaque, data, len);
		if (ret < 0)
		{
			s->error = ret;
		}
		else
		{
			ctx->bytes_written += len;
			s->bytes_written = ctx->bytes_written;

			if (s->pos + len > ctx->written_output_size)
			{
				ctx->written_output_size = s->pos + len;
			}
		}
	}
	if (ctx->current_type == AVIO_DATA_MARKER_SYNC_POINT || ctx->current_type == AVIO_DATA_MARKER_BOUNDARY_POINT)
	{
		ctx->current_type = AVIO_DATA_MARKER_UNKNOWN;
	}
	s->pos += len;
}

static void flush_buffer(AVIOContext* s)
{
	s->buf_ptr_max = FFMAX(s->buf_ptr, s->buf_ptr_max);
	if (s->write_flag && s->buf_ptr_max > s->buffer)
	{
		writeout(s, s->buffer, (int)(s->buf_ptr_max - s->buffer));
	}
	s->buf_ptr = s->buf_ptr_max = s->buffer;
	if (!s->write_flag)
		s->buf_end = s->buffer;
}

void avio_flush(AVIOContext* s)
{
	int seekback = (int)(s->write_flag ? FFMIN(0, s->buf_ptr - s->buf_ptr_max) : 0);
	flush_buffer(s);
	if (seekback)
		avio_seek(s, seekback, SEEK_CUR);
}

int64_t avio_seek(AVIOContext* s, int64_t offset, int whence)
{
	FFIOContext* const ctx = ffiocontext(s);
	int64_t offset1;
	int64_t pos;
	int force = whence & AVSEEK_FORCE;
	int buffer_size;
	int short_seek;
	whence &= ~AVSEEK_FORCE;

	if (!s)
		return AVERROR(EINVAL);

	if ((whence & AVSEEK_SIZE))
		return s->seek ? s->seek(s->opaque, offset, AVSEEK_SIZE) : AVERROR(ENOSYS);

	buffer_size = (int)(s->buf_end - s->buffer);
	// pos is the absolute position that the beginning of s->buffer corresponds to in the file
	pos = s->pos - (s->write_flag ? 0 : buffer_size);

	if (whence != SEEK_CUR && whence != SEEK_SET)
		return AVERROR(EINVAL);

	if (whence == SEEK_CUR)
	{
		offset1 = pos + (s->buf_ptr - s->buffer);
		if (offset == 0)
			return offset1;
		if (offset > INT64_MAX - offset1)
			return AVERROR(EINVAL);
		offset += offset1;
	}
	if (offset < 0)
		return AVERROR(EINVAL);

	short_seek = ctx->short_seek_threshold;
	if (ctx->short_seek_get)
	{
		int tmp = ctx->short_seek_get(s->opaque);
		short_seek = FFMAX(tmp, short_seek);
	}

	offset1 = offset - pos; // "offset1" is the relative offset from the beginning of s->buffer
	s->buf_ptr_max = FFMAX(s->buf_ptr_max, s->buf_ptr);
	if ((!s->seek) &&
		offset1 >= 0 && offset1 <= (s->write_flag ? s->buf_ptr_max - s->buffer : buffer_size))
	{
		/* can do the seek inside the buffer */
		s->buf_ptr = s->buffer + offset1;
	}
	else if ((!(s->seekable & AVIO_SEEKABLE_NORMAL) ||
		offset1 <= buffer_size + short_seek) &&
		!s->write_flag && offset1 >= 0 &&
		(!s->seek) &&
		(whence != SEEK_END || force))
	{
		while (s->pos < offset && !s->eof_reached)
			fill_buffer(s);
		if (s->eof_reached)
			return AVERROR_EOF;
		s->buf_ptr = s->buf_end - (s->pos - offset);
	}
	else if (!s->write_flag && offset1 < 0 && -offset1 < buffer_size>>1 && s->seek && offset > 0)
	{
		int64_t res;

		pos -= FFMIN(buffer_size >> 1, pos);
		if ((res = s->seek(s->opaque, pos, SEEK_SET)) < 0)
			return res;
		s->buf_end =
			s->buf_ptr = s->buffer;
		s->pos = pos;
		s->eof_reached = 0;
		fill_buffer(s);
		return avio_seek(s, offset, SEEK_SET | force);
	}
	else
	{
		int64_t res;
		if (s->write_flag)
		{
			flush_buffer(s);
		}
		if (!s->seek)
			return AVERROR(EPIPE);
		if ((res = s->seek(s->opaque, offset, SEEK_SET)) < 0)
			return res;
		if (!s->write_flag)
			s->buf_end = s->buffer;
		s->buf_ptr = s->buf_ptr_max = s->buffer;
		s->pos = offset;
	}
	s->eof_reached = 0;
	return offset;
}

int64_t avio_skip(AVIOContext* s, int64_t offset)
{
	return avio_seek(s, offset, SEEK_CUR);
}

int64_t avio_size(AVIOContext* s)
{
	FFIOContext* const ctx = ffiocontext(s);
	int64_t size;

	if (!s)
		return AVERROR(EINVAL);

	if (ctx->written_output_size)
		return ctx->written_output_size;

	if (!s->seek)
		return AVERROR(ENOSYS);
	size = s->seek(s->opaque, 0, AVSEEK_SIZE);
	if (size < 0)
	{
		if ((size = s->seek(s->opaque, -1, SEEK_END)) < 0)
			return size;
		size++;
		s->seek(s->opaque, s->pos, SEEK_SET);
	}
	return size;
}

int avio_feof(AVIOContext* s)
{
	if (!s)
		return 0;
	if (s->eof_reached)
	{
		s->eof_reached = 0;
		fill_buffer(s);
	}
	return s->eof_reached;
}

static int read_packet_wrapper(AVIOContext* s, uint8_t* buf, int size)
{
	int ret;

	if (!s->read_packet)
		return AVERROR(EINVAL);
	ret = s->read_packet(s->opaque, buf, size);
	av_assert2(ret || s->max_packet_size);
	return ret;
}

static void fill_buffer(AVIOContext* s)
{
	FFIOContext* const ctx = (FFIOContext*)s;
	int max_buffer_size = s->max_packet_size ?
		s->max_packet_size : IO_BUFFER_SIZE;
	uint8_t* dst = s->buf_end - s->buffer + max_buffer_size <= s->buffer_size ?
		s->buf_end : s->buffer;
	int len = (int)(s->buffer_size - (dst - s->buffer));

	/* can't fill the buffer without read_packet, just set EOF if appropriate */
	if (!s->read_packet && s->buf_ptr >= s->buf_end)
		s->eof_reached = 1;

	/* no need to do anything if EOF already reached */
	if (s->eof_reached)
		return;

	/* make buffer smaller in case it ended up large after probing */
	if (s->read_packet && ctx->orig_buffer_size &&
		s->buffer_size > ctx->orig_buffer_size && len >= ctx->orig_buffer_size)
	{
		if (dst == s->buffer && s->buf_ptr != dst)
		{
			(void)set_buf_size(s, ctx->orig_buffer_size);
			dst = s->buffer;
		}
		len = ctx->orig_buffer_size;
	}

	len = read_packet_wrapper(s, dst, len);
	if (len == AVERROR_EOF)
	{
		/* do not modify buffer if EOF reached so that a seek back can be done without rereading data */
		s->eof_reached = 1;
	}
	else if (len < 0)
	{
		s->eof_reached = 1;
		s->error = len;
	}
	else
	{
		s->pos += len;
		s->buf_ptr = dst;
		s->buf_end = dst + len;
		ffiocontext(s)->bytes_read += len;
		s->bytes_read = ffiocontext(s)->bytes_read;
	}
}

/* XXX: put an inline version */
int avio_r8(AVIOContext* s)
{
	if (s->buf_ptr >= s->buf_end)
		fill_buffer(s);
	if (s->buf_ptr < s->buf_end)
		return *s->buf_ptr++;
	return 0;
}

int avio_read(AVIOContext* s, unsigned char* buf, int size)
{
	int len, size1;

	size1 = size;
	while (size > 0)
	{
		len = (int)(FFMIN(s->buf_end - s->buf_ptr, size));
		if (len == 0 || s->write_flag)
		{
			if (size > s->buffer_size && s->read_packet)
			{
				// bypass the buffer and read data directly into buf
				len = read_packet_wrapper(s, buf, size);
				if (len == AVERROR_EOF)
				{
					/* do not modify buffer if EOF reached so that a seek back can be done without rereading data */
					s->eof_reached = 1;
					break;
				}
				else if (len < 0)
				{
					s->eof_reached = 1;
					s->error = len;
					break;
				}
				else
				{
					s->pos += len;
					ffiocontext(s)->bytes_read += len;
					s->bytes_read = ffiocontext(s)->bytes_read;
					size -= len;
					buf += len;
					// reset the buffer
					s->buf_ptr = s->buffer;
					s->buf_end = s->buffer /* + len*/;
				}
			}
			else
			{
				fill_buffer(s);
				len = (int)(s->buf_end - s->buf_ptr);
				if (len == 0)
					break;
			}
		}
		else
		{
			memcpy(buf, s->buf_ptr, len);
			buf += len;
			s->buf_ptr += len;
			size -= len;
		}
	}
	if (size1 == size)
	{
		if (s->error)      return s->error;
		if (avio_feof(s))  return AVERROR_EOF;
	}
	return size1 - size;
}

int ffio_read_size(AVIOContext* s, unsigned char* buf, int size)
{
	int ret = avio_read(s, buf, size);
	if (ret == size)
		return ret;
	if (ret < 0 && ret != AVERROR_EOF)
		return ret;
	return AVERROR_INVALIDDATA;
}

unsigned int avio_rl16(AVIOContext* s)
{
	unsigned int val;
	val = avio_r8(s);
	val |= avio_r8(s) << 8;
	return val;
}

unsigned int avio_rl32(AVIOContext* s)
{
	unsigned int val;
	val = avio_rl16(s);
	val |= avio_rl16(s) << 16;
	return val;
}

int ffio_limit(AVIOContext* s, int size)
{
	FFIOContext* const ctx = ffiocontext(s);
	if (ctx->maxsize >= 0)
	{
		int64_t pos = avio_tell(s);
		int64_t remaining = ctx->maxsize - pos;
		if (remaining < size)
		{
			int64_t newsize = avio_size(s);
			if (!ctx->maxsize || ctx->maxsize < newsize)
				ctx->maxsize = newsize - !newsize;
			if (pos > ctx->maxsize && ctx->maxsize >= 0)
				ctx->maxsize = AVERROR(EIO);
			if (ctx->maxsize >= 0)
				remaining = ctx->maxsize - pos;
		}

		if (ctx->maxsize >= 0 && remaining < size && size > 1)
		{
			size = (int)(remaining + !remaining);
		}
	}
	return size;
}

static int set_buf_size(AVIOContext* s, int buf_size)
{
	uint8_t* buffer;
	buffer = av_malloc(buf_size);
	if (!buffer)
		return AVERROR(ENOMEM);

	av_free(s->buffer);
	s->buffer = buffer;
	ffiocontext(s)->orig_buffer_size =
		s->buffer_size = buf_size;
	s->buf_ptr = s->buf_ptr_max = buffer;
	url_resetbuf(s, s->write_flag ? AVIO_FLAG_WRITE : AVIO_FLAG_READ);
	return 0;
}

static int url_resetbuf(AVIOContext* s, int flags)
{
	av_assert1(flags == AVIO_FLAG_WRITE || flags == AVIO_FLAG_READ);

	if (flags & AVIO_FLAG_WRITE)
	{
		s->buf_end = s->buffer + s->buffer_size;
		s->write_flag = 1;
	}
	else
	{
		s->buf_end = s->buffer;
		s->write_flag = 0;
	}
	return 0;
}

int ffio_rewind_with_probe_data(AVIOContext* s, unsigned char** bufp, int buf_size)
{
	int64_t buffer_start;
	int buffer_size;
	int overlap, new_size, alloc_size;
	uint8_t* buf = *bufp;

	if (s->write_flag)
	{
		av_freep(bufp);
		return AVERROR(EINVAL);
	}

	buffer_size = (int)(s->buf_end - s->buffer);

	/* the buffers must touch or overlap */
	if ((buffer_start = s->pos - buffer_size) > buf_size)
	{
		av_freep(bufp);
		return AVERROR(EINVAL);
	}

	overlap = (int)(buf_size - buffer_start);
	new_size = buf_size + buffer_size - overlap;

	alloc_size = FFMAX(s->buffer_size, new_size);
	if (alloc_size > buf_size)
		if (!(buf = (*bufp) = av_realloc_f(buf, 1, alloc_size)))
			return AVERROR(ENOMEM);

	if (new_size > buf_size)
	{
		memcpy(buf + buf_size, s->buffer + overlap, buffer_size - overlap);
		buf_size = new_size;
	}

	av_free(s->buffer);
	s->buf_ptr = s->buffer = buf;
	s->buffer_size = alloc_size;
	s->pos = buf_size;
	s->buf_end = s->buf_ptr + buf_size;
	s->eof_reached = 0;

	return 0;
}
