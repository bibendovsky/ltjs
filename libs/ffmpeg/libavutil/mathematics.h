/*
 * copyright (c) 2005-2012 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Mathematical utilities for working with timestamp and time base. */

#ifndef AVUTIL_MATHEMATICS_H
#define AVUTIL_MATHEMATICS_H

#include <stdint.h>
#include <math.h>
#include "rational.h"
#include "intfloat.h"

#ifndef M_E
#define M_E 2.7182818284590452354   /* e */
#endif

/* Rounding methods. */
enum AVRounding
{
	AV_ROUND_ZERO = 0, // Round toward zero.
	AV_ROUND_DOWN = 2, // Round toward -infinity.
	AV_ROUND_UP = 3, // Round toward +infinity.
	AV_ROUND_NEAR_INF = 5, // Round to nearest and halfway cases away from zero.
	/*
	 * Flag telling rescaling functions to pass `INT64_MIN`/`MAX` through
	 * unchanged, avoiding special cases for #AV_NOPTS_VALUE.
	 *
	 * Unlike other values of the enumeration AVRounding, this value is a
	 * bitmask that must be used in conjunction with another value of the
	 * enumeration through a bitwise OR, in order to set behavior for normal
	 * cases.
	 *
	 * av_rescale_rnd(3, 1, 2, AV_ROUND_UP | AV_ROUND_PASS_MINMAX);
	 * // Rescaling 3:
	 * //     Calculating 3 * 1 / 2
	 * //     3 / 2 is rounded up to 2
	 * //     => 2
	 *
	 * av_rescale_rnd(AV_NOPTS_VALUE, 1, 2, AV_ROUND_UP | AV_ROUND_PASS_MINMAX);
	 * // Rescaling AV_NOPTS_VALUE:
	 * //     AV_NOPTS_VALUE == INT64_MIN
	 * //     AV_NOPTS_VALUE is passed through
	 * //     => AV_NOPTS_VALUE
	 * @endcode
	 */
	AV_ROUND_PASS_MINMAX = 8192,
};

/* Compute the greatest common divisor of two integer operands. */
int64_t av_gcd(int64_t a, int64_t b);

/*
 * Rescale a 64-bit integer with rounding to nearest.
 * The operation is mathematically equivalent to `a * b / c`, but writing that directly can overflow.
 */
int64_t av_rescale(int64_t a, int64_t b, int64_t c);

/*
 * Rescale a 64-bit integer with specified rounding.
 *
 * The operation is mathematically equivalent to `a * b / c`, but writing that
 * directly can overflow, and does not support different rounding methods.
 * If the result is not representable then INT64_MIN is returned.
 */
int64_t av_rescale_rnd(int64_t a, int64_t b, int64_t c, enum AVRounding rnd);

/*
 * Rescale a 64-bit integer by 2 rational numbers.
 * The operation is mathematically equivalent to `a * bq / cq`.
 * This function is equivalent to av_rescale_q_rnd() with #AV_ROUND_NEAR_INF.
 */
int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq);

/*
 * Rescale a 64-bit integer by 2 rational numbers with specified rounding.
 * The operation is mathematically equivalent to `a * bq / cq`.
 */
int64_t av_rescale_q_rnd(int64_t a, AVRational bq, AVRational cq, enum AVRounding rnd);

/* Compare the remainders of two integer operands divided by a common divisor. */
int64_t av_compare_mod(uint64_t a, uint64_t b, uint64_t mod);

/* Add a value to a timestamp. */
int64_t av_add_stable(AVRational ts_tb, int64_t ts, AVRational inc_tb, int64_t inc);

#endif /* AVUTIL_MATHEMATICS_H */
