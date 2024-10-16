/*
 * Half-pel DSP functions.
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Half-pel DSP functions. */

#ifndef AVCODEC_HPELDSP_H
#define AVCODEC_HPELDSP_H

#include <stdint.h>
#include <stddef.h>

/* add and put pixel (decoding) */
typedef void (*op_pixels_func)(uint8_t* block /*align width (8 or 16)*/,
	const uint8_t* pixels /*align 1*/,
	ptrdiff_t line_size, int h);

/* Half-pel DSP context. */
typedef struct HpelDSPContext
{
	/* Halfpel motion compensation with rounding (a+b+1)>>1. */
	op_pixels_func put_pixels_tab[4][4];

	/* Halfpel motion compensation with rounding (a+b+1)>>1. */
	op_pixels_func avg_pixels_tab[4][4];

	/* Halfpel motion compensation with no rounding (a+b)>>1. */
	op_pixels_func put_no_rnd_pixels_tab[4][4];

	/* Halfpel motion compensation with no rounding (a+b)>>1. */
	op_pixels_func avg_no_rnd_pixels_tab[4];
} HpelDSPContext;

void ff_hpeldsp_init(HpelDSPContext* c, int flags);

#endif /* AVCODEC_HPELDSP_H */
