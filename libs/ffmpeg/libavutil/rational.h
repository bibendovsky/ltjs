/*
 * rational numbers
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Utilties for rational number calculation. */

#ifndef AVUTIL_RATIONAL_H
#define AVUTIL_RATIONAL_H

#include <stdint.h>
#include <limits.h>

/* Rational number (pair of numerator and denominator). */
typedef struct AVRational
{
	int num; // Numerator
	int den; // Denominator
} AVRational;

/* Create an AVRational. */
static inline AVRational av_make_q(int num, int den)
{
	AVRational r = {num, den};
	return r;
}

/* Compare two rationals. */
static inline int av_cmp_q(AVRational a, AVRational b)
{
	const int64_t tmp = a.num * (int64_t)b.den - b.num * (int64_t)a.den;

	if (tmp) return (int)((tmp ^ a.den ^ b.den) >> 63) | 1;
	else if (b.den && a.den) return 0;
	else if (a.num && b.num) return (a.num >> 31) - (b.num >> 31);
	else return INT_MIN;
}

/* Convert an AVRational to a `double`. */
static inline double av_q2d(AVRational a)
{
	return a.num / (double)a.den;
}

/* Reduce a fraction. */
int av_reduce(int* dst_num, int* dst_den, int64_t num, int64_t den, int64_t max);

/* Multiply two rationals. */
AVRational av_mul_q(AVRational b, AVRational c);

/* Add two rationals. */
AVRational av_add_q(AVRational b, AVRational c);

/* Invert a rational. */
static inline AVRational av_inv_q(AVRational q)
{
	AVRational r = {q.den, q.num};
	return r;
}

/* Return the best rational so that a and b are multiple of it. */
AVRational av_gcd_q(AVRational a, AVRational b, int max_den, AVRational def);

#endif /* AVUTIL_RATIONAL_H */
