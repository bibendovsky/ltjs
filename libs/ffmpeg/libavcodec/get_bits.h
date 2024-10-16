/*
 * Copyright (c) 2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* bitstream reader API header. */

#ifndef AVCODEC_GET_BITS_H
#define AVCODEC_GET_BITS_H

#include <stdint.h>

#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/avassert.h"

#include "defs.h"
#include "mathops.h"
#include "vlc.h"

#ifndef UNCHECKED_BITSTREAM_READER
#define UNCHECKED_BITSTREAM_READER 0
#endif

typedef struct GetBitContext
{
	const uint8_t* buffer, *buffer_end;
	int index;
	int size_in_bits;
	int size_in_bits_plus8;
} GetBitContext;

static inline unsigned int get_bits(GetBitContext* s, int n);
static inline void skip_bits(GetBitContext* s, int n);

/* Bitstream reader API docs:
 * name
 *   arbitrary name which is used as prefix for the internal variables
 *
 * gb
 *   getbitcontext
 *
 * OPEN_READER(name, gb)
 *   load gb into local variables
 *
 * CLOSE_READER(name, gb)
 *   store local vars in gb
 *
 * UPDATE_CACHE(name, gb)
 *   Refill the internal cache from the bitstream.
 *   After this call at least MIN_CACHE_BITS will be available.
 *
 * GET_CACHE(name, gb)
 *   Will output the contents of the internal cache,
 *   next bit is MSB of 32 or 64 bits (FIXME 64 bits).
 *
 * SHOW_UBITS(name, gb, num)
 *   Will return the next num bits.
 *
 * SHOW_SBITS(name, gb, num)
 *   Will return the next num bits and do sign extension.
 *
 * SKIP_BITS(name, gb, num)
 *   Will skip over the next num bits.
 *   Note, this is equivalent to SKIP_CACHE; SKIP_COUNTER.
 *
 * SKIP_CACHE(name, gb, num)
 *   Will remove the next num bits from the cache (note SKIP_COUNTER
 *   MUST be called before UPDATE_CACHE / CLOSE_READER).
 *
 * SKIP_COUNTER(name, gb, num)
 *   Will increment the internal bit counter (see SKIP_CACHE & SKIP_BITS).
 *
 * LAST_SKIP_BITS(name, gb, num)
 *   Like SKIP_BITS, to be used if next call is UPDATE_CACHE or CLOSE_READER.
 *
 * BITS_LEFT(name, gb)
 *   Return the number of bits left
 *
 * For examples see get_bits, show_bits, skip_bits, get_vlc.
 */

#if defined LONG_BITSTREAM_READER
#   define MIN_CACHE_BITS 32
#else
#   define MIN_CACHE_BITS 25
#endif

#define OPEN_READER_NOSIZE(name, gb)            \
    unsigned int name ## _index = (gb)->index;  \
    unsigned int name ## _cache

#if UNCHECKED_BITSTREAM_READER
#define OPEN_READER(name, gb) OPEN_READER_NOSIZE(name, gb)

#define BITS_AVAILABLE(name, gb) 1
#else
#define OPEN_READER(name, gb)                   \
    OPEN_READER_NOSIZE(name, gb);               \
    unsigned int name ## _size_plus8 = (gb)->size_in_bits_plus8

#define BITS_AVAILABLE(name, gb) name ## _index < name ## _size_plus8
#endif

#define CLOSE_READER(name, gb) (gb)->index = name ## _index

#define UPDATE_CACHE_BE_EXT(name, gb, bits, dst_bits) name ## _cache = \
    AV_RB ## bits((gb)->buffer + (name ## _index >> 3)) << (name ## _index & 7) >> (bits - dst_bits)

#define UPDATE_CACHE_LE_EXT(name, gb, bits, dst_bits) name ## _cache = \
    (uint ## dst_bits ## _t)(AV_RL ## bits((gb)->buffer + (name ## _index >> 3)) >> (name ## _index & 7))

/* Using these two macros ensures that 32 bits are available. */
# define UPDATE_CACHE_LE_32(name, gb) UPDATE_CACHE_LE_EXT(name, (gb), 64, 32)

# define UPDATE_CACHE_BE_32(name, gb) UPDATE_CACHE_BE_EXT(name, (gb), 64, 32)

# ifdef LONG_BITSTREAM_READER

# define UPDATE_CACHE_LE(name, gb) UPDATE_CACHE_LE_32(name, (gb))

# define UPDATE_CACHE_BE(name, gb) UPDATE_CACHE_BE_32(name, (gb))

#else

# define UPDATE_CACHE_LE(name, gb) UPDATE_CACHE_LE_EXT(name, (gb), 32, 32)

# define UPDATE_CACHE_BE(name, gb) UPDATE_CACHE_BE_EXT(name, (gb), 32, 32)

#endif


#ifdef BITSTREAM_READER_LE

# define UPDATE_CACHE(name, gb) UPDATE_CACHE_LE(name, gb)
# define UPDATE_CACHE_32(name, gb) UPDATE_CACHE_LE_32(name, (gb))

# define SKIP_CACHE(name, gb, num) name ## _cache >>= (num)

#else

# define UPDATE_CACHE(name, gb) UPDATE_CACHE_BE(name, gb)
# define UPDATE_CACHE_32(name, gb) UPDATE_CACHE_BE_32(name, (gb))

# define SKIP_CACHE(name, gb, num) name ## _cache <<= (num)

#endif

#if UNCHECKED_BITSTREAM_READER
#   define SKIP_COUNTER(name, gb, num) name ## _index += (num)
#else
#   define SKIP_COUNTER(name, gb, num) \
    name ## _index = FFMIN(name ## _size_plus8, name ## _index + (num))
#endif

#define SKIP_BITS(name, gb, num)                \
    do {                                        \
        SKIP_CACHE(name, gb, num);              \
        SKIP_COUNTER(name, gb, num);            \
    } while (0)

#define LAST_SKIP_BITS(name, gb, num) SKIP_COUNTER(name, gb, num)

#define SHOW_UBITS_LE(name, gb, num) zero_extend(name ## _cache, num)
#define SHOW_UBITS_BE(name, gb, num) NEG_USR32(name ## _cache, num)

#ifdef BITSTREAM_READER_LE
#   define SHOW_UBITS(name, gb, num) SHOW_UBITS_LE(name, gb, num)
#else
#   define SHOW_UBITS(name, gb, num) SHOW_UBITS_BE(name, gb, num)
#endif

static inline int get_bits_count(const GetBitContext* s)
{
	return s->index;
}

/* Skips the specified number of bits. */
static inline void skip_bits_long(GetBitContext* s, int n)
{
#if UNCHECKED_BITSTREAM_READER
	s->index += n;
#else
	s->index += av_clip(n, -s->index, s->size_in_bits_plus8 - s->index);
#endif
}

/* Read 1-25 bits. */
static inline unsigned int get_bits(GetBitContext* s, int n)
{
	register unsigned int tmp;
	OPEN_READER(re, s);
	(void)re_cache;
	av_assert2(n > 0 && n <= 25);
	UPDATE_CACHE(re, s);
	tmp = SHOW_UBITS(re, s, n);
	LAST_SKIP_BITS(re, s, n);
	CLOSE_READER(re, s);
	av_assert2(tmp < UINT64_C(1) << n);
	return tmp;
}

static inline void skip_bits(GetBitContext* s, int n)
{
	OPEN_READER(re, s);
	(void)re_cache;
	LAST_SKIP_BITS(re, s, n);
	CLOSE_READER(re, s);
}

static inline unsigned int get_bits1(GetBitContext* s)
{
	unsigned int index = s->index;
	uint8_t result = s->buffer[index >> 3];
#ifdef BITSTREAM_READER_LE
	result >>= index & 7;
	result &= 1;
#else
	result <<= index & 7;
	result >>= 8 - 1;
#endif
#if !UNCHECKED_BITSTREAM_READER
	if (s->index < s->size_in_bits_plus8)
#endif
		index++;
	s->index = index;

	return result;
}

/* Read 0-32 bits. */
static inline unsigned int get_bits_long(GetBitContext* s, int n)
{
	av_assert2(n >= 0 && n <= 32);
	if (!n)
	{
		return 0;
	}
	else
	{
#ifdef BITSTREAM_READER_LE
		unsigned ret = get_bits(s, 16);
		return ret | (get_bits(s, n - 16) << 16);
#else
		unsigned ret = get_bits(s, 16) << (n - 16);
		return ret | get_bits(s, n - 16);
#endif
	}
}

/* Initialize GetBitContext. */
static inline int init_get_bits(GetBitContext* s, const uint8_t* buffer, int bit_size)
{
	int buffer_size;
	int ret = 0;

	if (bit_size >= INT_MAX - FFMAX(7, AV_INPUT_BUFFER_PADDING_SIZE * 8) || bit_size < 0 || !buffer)
	{
		bit_size = 0;
		buffer = NULL;
		ret = AVERROR_INVALIDDATA;
	}

	buffer_size = (bit_size + 7) >> 3;

	s->buffer = buffer;
	s->size_in_bits = bit_size;
	s->size_in_bits_plus8 = bit_size + 8;
	s->buffer_end = buffer + buffer_size;
	s->index = 0;

	return ret;
}

/* Initialize GetBitContext. */
static inline int init_get_bits8(GetBitContext* s, const uint8_t* buffer, int byte_size)
{
	if (byte_size > INT_MAX / 8 || byte_size < 0)
		byte_size = -1;
	return init_get_bits(s, buffer, byte_size * 8);
}

/**
 * If the vlc code is invalid and max_depth=1, then no bits will be removed.
 * If the vlc code is invalid and max_depth>1, then the number of bits removed
 * is undefined.
 */
#define GET_VLC(code, name, gb, table, bits, max_depth)         \
    do {                                                        \
        int n, nb_bits;                                         \
        unsigned int index;                                     \
                                                                \
        index = SHOW_UBITS(name, gb, bits);                     \
        code  = table[index].sym;                               \
        n     = table[index].len;                               \
                                                                \
        if (max_depth > 1 && n < 0) {                           \
            LAST_SKIP_BITS(name, gb, bits);                     \
            UPDATE_CACHE(name, gb);                             \
                                                                \
            nb_bits = -n;                                       \
                                                                \
            index = SHOW_UBITS(name, gb, nb_bits) + code;       \
            code  = table[index].sym;                           \
            n     = table[index].len;                           \
            if (max_depth > 2 && n < 0) {                       \
                LAST_SKIP_BITS(name, gb, nb_bits);              \
                UPDATE_CACHE(name, gb);                         \
                                                                \
                nb_bits = -n;                                   \
                                                                \
                index = SHOW_UBITS(name, gb, nb_bits) + code;   \
                code  = table[index].sym;                       \
                n     = table[index].len;                       \
            }                                                   \
        }                                                       \
        SKIP_BITS(name, gb, n);                                 \
    } while (0)

/* Parse a vlc code. */
static inline int get_vlc2(GetBitContext* s, const VLCElem* table, int bits, int max_depth)
{
	int code;
	OPEN_READER(re, s);
	UPDATE_CACHE(re, s);
	GET_VLC(code, re, s, table, bits, max_depth);
	CLOSE_READER(re, s);
	return code;
}

static inline int get_bits_left(GetBitContext* gb)
{
	return gb->size_in_bits - get_bits_count(gb);
}

#endif /* AVCODEC_GET_BITS_H */
