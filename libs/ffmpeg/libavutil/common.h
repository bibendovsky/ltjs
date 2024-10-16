/*
 * copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* common internal and external API header */

#ifndef AVUTIL_COMMON_H
#define AVUTIL_COMMON_H

#if defined(__cplusplus) && !defined(__STDC_CONSTANT_MACROS) && !defined(UINT64_C)
#error missing -D__STDC_CONSTANT_MACROS / #define __STDC_CONSTANT_MACROS
#endif

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "macros.h"
#include "mem.h"

#include "intmath.h"

/* Fast a/(1<<b) rounded toward +inf. Assume a>=0 and b>=0 */
#define AV_CEIL_RSHIFT(a,b) (-((-(a)) >> (b)))

/*Absolute value */
#define FFABS(a) ((a) >= 0 ? (a) : (-(a)))

/* Absolute value 64-bit */
#define FFABS64(a) ((a) <= 0 ? -(int64_t)(a) : (int64_t)(a))

/* misc math functions */

#ifndef av_clip
#   define av_clip          av_clip_c
#endif
#ifndef av_clip64
#   define av_clip64        av_clip64_c
#endif
#ifndef av_mod_uintp2
#   define av_mod_uintp2    av_mod_uintp2_c
#endif
#ifndef av_sat_add64
#   define av_sat_add64     av_sat_add64_c
#endif
#ifndef av_sat_sub64
#   define av_sat_sub64     av_sat_sub64_c
#endif
#ifndef av_popcount
#   define av_popcount      av_popcount_c
#endif
#ifndef av_popcount64
#   define av_popcount64    av_popcount64_c
#endif

#ifndef av_log2
int av_log2(unsigned v);
#endif

#ifndef av_log2_16bit
int av_log2_16bit(unsigned v);
#endif

/* Clip a signed integer value into the amin-amax range. */
static inline int av_clip_c(int a, int amin, int amax)
{
#if defined(ASSERT_LEVEL) && ASSERT_LEVEL >= 2
	if (amin > amax) abort();
#endif
	if (a < amin) return amin;
	else if (a > amax) return amax;
	else               return a;
}

/* Clip a signed 64bit integer value into the amin-amax range. */
static inline int64_t av_clip64_c(int64_t a, int64_t amin, int64_t amax)
{
#if defined(ASSERT_LEVEL) && ASSERT_LEVEL >= 2
	if (amin > amax) abort();
#endif
	if (a < amin) return amin;
	else if (a > amax) return amax;
	else               return a;
}

/* Clear high bits from an unsigned integer starting with specific bit position */
static inline unsigned av_mod_uintp2_c(unsigned a, unsigned p)
{
	return a & ((1U << p) - 1);
}

/* Add two signed 64-bit values with saturation. */
static inline int64_t av_sat_add64_c(int64_t a, int64_t b)
{
	int64_t s = a + (uint64_t)b;
	if ((int64_t)((a ^ b) | (~s ^ b)) >= 0)
		return INT64_MAX ^ (b >> 63);
	return s;
}

/* Subtract two signed 64-bit values with saturation. */
static inline int64_t av_sat_sub64_c(int64_t a, int64_t b)
{
	if (b <= 0 && a >= INT64_MAX + b)
		return INT64_MAX;
	if (b >= 0 && a <= INT64_MIN + b)
		return INT64_MIN;
	return a - b;
}

/* Count number of bits set to one in x */
static inline int av_popcount_c(uint32_t x)
{
	x -= (x >> 1) & 0x55555555;
	x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
	x = (x + (x >> 4)) & 0x0F0F0F0F;
	x += x >> 8;
	return (x + (x >> 16)) & 0x3F;
}

/* Count number of bits set to one in x */
static inline int av_popcount64_c(uint64_t x)
{
	return av_popcount((uint32_t)x) + av_popcount((uint32_t)(x >> 32));
}

#endif /* AVUTIL_COMMON_H */
