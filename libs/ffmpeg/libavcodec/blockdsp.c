/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdint.h>
#include <string.h>

#include "blockdsp.h"

static void clear_block_c(int16_t* block)
{
	memset(block, 0, sizeof(int16_t) * 64);
}

static void clear_blocks_c(int16_t* blocks)
{
	memset(blocks, 0, sizeof(int16_t) * 6 * 64);
}

static void fill_block16_c(uint8_t* block, uint8_t value, ptrdiff_t line_size, int h)
{
	int i;

	for (i = 0; i < h; i++)
	{
		memset(block, value, 16);
		block += line_size;
	}
}

static void fill_block8_c(uint8_t* block, uint8_t value, ptrdiff_t line_size, int h)
{
	int i;

	for (i = 0; i < h; i++)
	{
		memset(block, value, 8);
		block += line_size;
	}
}

void ff_blockdsp_init(BlockDSPContext* c)
{
	c->clear_block = clear_block_c;
	c->clear_blocks = clear_blocks_c;

	c->fill_block_tab[0] = fill_block16_c;
	c->fill_block_tab[1] = fill_block8_c;
}
