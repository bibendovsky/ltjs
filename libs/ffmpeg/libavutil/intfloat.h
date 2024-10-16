/*
 * Copyright (c) 2011 Mans Rullgard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVUTIL_INTFLOAT_H
#define AVUTIL_INTFLOAT_H

#include <stdint.h>

union av_intfloat32
{
	uint32_t i;
	float    f;
};

/* Reinterpret a 32-bit integer as a float. */
static inline float av_int2float(uint32_t i)
{
	union av_intfloat32 v;
	v.i = i;
	return v.f;
}

#endif /* AVUTIL_INTFLOAT_H */
