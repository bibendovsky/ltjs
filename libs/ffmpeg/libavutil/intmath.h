/*
 * Copyright (c) 2010 Mans Rullgard <mans@mansr.com>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVUTIL_INTMATH_H
#define AVUTIL_INTMATH_H

#include <stdint.h>

extern const uint8_t ff_log2_tab[256];

#ifndef ff_log2
#define ff_log2 ff_log2_c
static inline int ff_log2_c(unsigned int v)
{
	int n = 0;
	if (v & 0xffff0000)
	{
		v >>= 16;
		n += 16;
	}
	if (v & 0xff00)
	{
		v >>= 8;
		n += 8;
	}
	n += ff_log2_tab[v];

	return n;
}
#endif

#ifndef ff_log2_16bit
#define ff_log2_16bit ff_log2_16bit_c
static inline int ff_log2_16bit_c(unsigned int v)
{
	int n = 0;
	if (v & 0xff00)
	{
		v >>= 8;
		n += 8;
	}
	n += ff_log2_tab[v];

	return n;
}
#endif

#define av_log2       ff_log2
#define av_log2_16bit ff_log2_16bit

#ifndef ff_ctz
#define ff_ctz ff_ctz_c
/*
 * Trailing zero bit count.
 * We use the De-Bruijn method outlined in:
 * http://supertech.csail.mit.edu/papers/debruijn.pdf.
 */
static inline int ff_ctz_c(int v)
{
	static const uint8_t debruijn_ctz32[32] = {
		0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
		31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
	};
	return debruijn_ctz32[(uint32_t)((v & -v) * 0x077CB531U) >> 27];
}
#endif

#ifndef ff_ctzll
#define ff_ctzll ff_ctzll_c
/*
 * We use the De-Bruijn method outlined in:
 * http://supertech.csail.mit.edu/papers/debruijn.pdf.
 */
static inline int ff_ctzll_c(long long v)
{
	static const uint8_t debruijn_ctz64[64] = {
		0, 1, 2, 53, 3, 7, 54, 27, 4, 38, 41, 8, 34, 55, 48, 28,
		62, 5, 39, 46, 44, 42, 22, 9, 24, 35, 59, 56, 49, 18, 29, 11,
		63, 52, 6, 26, 37, 40, 33, 47, 61, 45, 43, 21, 23, 58, 17, 10,
		51, 25, 36, 32, 60, 20, 57, 16, 50, 31, 19, 15, 30, 14, 13, 12
	};
	return debruijn_ctz64[(uint64_t)((v & -v) * 0x022FDD63CC95386DU) >> 58];
}
#endif

#endif /* AVUTIL_INTMATH_H */
