/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVCODEC_BLOCKDSP_H
#define AVCODEC_BLOCKDSP_H

#include <stddef.h>
#include <stdint.h>

/* add and put pixel */
typedef void (*op_fill_func)(uint8_t* block /* align width (8 or 16) */,
	uint8_t value, ptrdiff_t line_size, int h);

typedef struct BlockDSPContext
{
	void (*clear_block)(int16_t* block /* align 32 */);
	void (*clear_blocks)(int16_t* blocks /* align 32 */);

	op_fill_func fill_block_tab[2];
} BlockDSPContext;

void ff_blockdsp_init(BlockDSPContext* c);

#endif /* AVCODEC_BLOCKDSP_H */
