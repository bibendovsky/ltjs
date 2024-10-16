/*
 * Copyright (c) 2001-2003 BERO <bero@geocities.co.jp>
 * Copyright (c) 2011 Oskar Arvidsson
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVCODEC_RND_AVG_H
#define AVCODEC_RND_AVG_H

#include <stdint.h>

#define BYTE_VEC32(c) ((c) * 0x01010101UL)

static inline uint32_t rnd_avg32(uint32_t a, uint32_t b)
{
	return (a | b) - (((a ^ b) & ~BYTE_VEC32(0x01)) >> 1);
}

static inline uint32_t no_rnd_avg32(uint32_t a, uint32_t b)
{
	return (a & b) + (((a ^ b) & ~BYTE_VEC32(0x01)) >> 1);
}

#endif /* AVCODEC_RND_AVG_H */
