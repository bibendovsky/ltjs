/*
 * rational numbers
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* rational numbers */

#include "avassert.h"
#include "common.h"
#include "mathematics.h"
#include "rational.h"

int av_reduce(int* dst_num, int* dst_den, int64_t num, int64_t den, int64_t max)
{
	AVRational a0 = {0, 1}, a1 = {1, 0};
	int sign = (num < 0) ^ (den < 0);
	int64_t gcd = av_gcd(FFABS(num), FFABS(den));

	if (gcd)
	{
		num = FFABS(num) / gcd;
		den = FFABS(den) / gcd;
	}
	if (num <= max && den <= max)
	{
		a1 = (AVRational){(int)num, (int)den};
		den = 0;
	}

	while (den)
	{
		uint64_t x = num / den;
		int64_t next_den = num - den * x;
		int64_t a2n = x * a1.num + a0.num;
		int64_t a2d = x * a1.den + a0.den;

		if (a2n > max || a2d > max)
		{
			if (a1.num) x = (max - a0.num) / a1.num;
			if (a1.den) x = FFMIN(x, (uint64_t)((max - a0.den) / a1.den));

			if (den * (2 * x * a1.den + a0.den) > (uint64_t)(num * a1.den))
				a1 = (AVRational){(int)(x * a1.num + a0.num), (int)(x * a1.den + a0.den)};
			break;
		}

		a0 = a1;
		a1 = (AVRational){(int)a2n, (int)a2d};
		num = den;
		den = next_den;
	}
	av_assert2(av_gcd(a1.num, a1.den) <= 1U);
	av_assert2(a1.num <= max && a1.den <= max);

	*dst_num = sign ? -a1.num : a1.num;
	*dst_den = a1.den;

	return den == 0;
}

AVRational av_mul_q(AVRational b, AVRational c)
{
	av_reduce(&b.num, &b.den, b.num * (int64_t)c.num, b.den * (int64_t)c.den, INT_MAX);
	return b;
}

AVRational av_add_q(AVRational b, AVRational c)
{
	av_reduce(&b.num, &b.den, b.num * (int64_t)c.den + c.num * (int64_t)b.den, b.den * (int64_t)c.den, INT_MAX);
	return b;
}

AVRational av_gcd_q(AVRational a, AVRational b, int max_den, AVRational def)
{
	int64_t gcd, lcm;

	gcd = av_gcd(a.den, b.den);
	lcm = (a.den / gcd) * b.den;
	return lcm < max_den ? av_make_q((int)av_gcd(a.num, b.num), (int)lcm) : def;
}
