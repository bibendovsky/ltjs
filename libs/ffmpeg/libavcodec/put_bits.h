/*
 * copyright (c) 2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* bitstream writer API */

#ifndef AVCODEC_PUT_BITS_H
#define AVCODEC_PUT_BITS_H

#include <stdint.h>
#include <stddef.h>

#include "libavutil/intreadwrite.h"
#include "libavutil/avassert.h"
#include "libavutil/common.h"

typedef uint32_t BitBuf;
#define AV_WBBUF AV_WB32
#define AV_WLBUF AV_WL32

static const int BUF_BITS = 8 * sizeof(BitBuf);

typedef struct PutBitContext
{
	BitBuf bit_buf;
	int bit_left;
	uint8_t* buf, * buf_ptr, * buf_end;
} PutBitContext;

/* Return the total number of bits written to the bitstream. */
static inline int put_bits_count(PutBitContext* s)
{
	return (int)((s->buf_ptr - s->buf) * 8 + BUF_BITS - s->bit_left);
}

/* Return the number of bits available in the bitstream. */
static inline int put_bits_left(PutBitContext* s)
{
	return (int)((s->buf_end - s->buf_ptr) * 8 - BUF_BITS + s->bit_left);
}

/* Pad the end of the output stream with zeros. */
static inline void flush_put_bits(PutBitContext* s)
{
#ifndef BITSTREAM_WRITER_LE
	if (s->bit_left < BUF_BITS)
		s->bit_buf <<= s->bit_left;
#endif
	while (s->bit_left < BUF_BITS)
	{
		av_assert0(s->buf_ptr < s->buf_end);
#ifdef BITSTREAM_WRITER_LE
		* s->buf_ptr++ = s->bit_buf;
		s->bit_buf >>= 8;
#else
		* s->buf_ptr++ = (uint8_t)(s->bit_buf >> (BUF_BITS - 8));
		s->bit_buf <<= 8;
#endif
		s->bit_left += 8;
	}
	s->bit_left = BUF_BITS;
	s->bit_buf = 0;
}

static inline void put_bits_no_assert(PutBitContext* s, int n, BitBuf value)
{
	BitBuf bit_buf;
	int bit_left;

	bit_buf = s->bit_buf;
	bit_left = s->bit_left;

	/* XXX: optimize */
#ifdef BITSTREAM_WRITER_LE
	bit_buf |= value << (BUF_BITS - bit_left);
	if (n >= bit_left)
	{
		if (s->buf_end - s->buf_ptr >= sizeof(BitBuf))
		{
			AV_WLBUF(s->buf_ptr, bit_buf);
			s->buf_ptr += sizeof(BitBuf);
		}
		else
		{
			av_assert2(0);
		}
		bit_buf = value >> bit_left;
		bit_left += BUF_BITS;
	}
	bit_left -= n;
#else
	if (n < bit_left)
	{
		bit_buf = (bit_buf << n) | value;
		bit_left -= n;
	}
	else
	{
		bit_buf <<= bit_left;
		bit_buf |= value >> (n - bit_left);
		if (s->buf_end - s->buf_ptr >= (ptrdiff_t)sizeof(BitBuf))
		{
			AV_WBBUF(s->buf_ptr, bit_buf);
			s->buf_ptr += sizeof(BitBuf);
		}
		else
		{
			av_assert2(0);
		}
		bit_left += BUF_BITS - n;
		bit_buf = value;
	}
#endif

	s->bit_buf = bit_buf;
	s->bit_left = bit_left;
}

/* Write up to 31 bits into a bitstream. */
static inline void put_bits(PutBitContext* s, int n, BitBuf value)
{
	av_assert2(n <= 31 && value < (1UL << n));
	put_bits_no_assert(s, n, value);
}

/* Return the pointer to the byte where the bitstream writer will put the next bit. */
static inline uint8_t* put_bits_ptr(PutBitContext* s)
{
	return s->buf_ptr;
}

/* Skip the given number of bytes. */
static inline void skip_put_bytes(PutBitContext* s, int n)
{
	av_assert2((put_bits_count(s) & 7) == 0);
	av_assert2(s->bit_left == BUF_BITS);
	av_assert0(n <= s->buf_end - s->buf_ptr);
	s->buf_ptr += n;
}

#undef AV_WBBUF
#undef AV_WLBUF

#endif /* AVCODEC_PUT_BITS_H */
