/*
 * Bink DSP routines
 * Copyright (c) 2009 Konstantin Shishkov
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Bink DSP routines */

#ifndef AVCODEC_BINKDSP_H
#define AVCODEC_BINKDSP_H

#include <stdint.h>

typedef struct BinkDSPContext
{
	void (*idct_put)(uint8_t* dest /*align 8*/, int line_size, int32_t* block /*align 16*/);
	void (*idct_add)(uint8_t* dest /*align 8*/, int line_size, int32_t* block /*align 16*/);
	void (*scale_block)(const uint8_t src[64]/*align 8*/, uint8_t* dst/*align 8*/, int linesize);
	void (*add_pixels8)(uint8_t* pixels, int16_t* block, int line_size);
} BinkDSPContext;

void ff_binkdsp_init(BinkDSPContext* c);

#endif /* AVCODEC_BINKDSP_H */
