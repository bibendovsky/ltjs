/*
 * Common bit i/o utils
 * Copyright (c) 2000, 2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 * Copyright (c) 2010 Loren Merritt
 *
 * alternative bitstream reader & writer by Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* bitstream api. */

#include <stdint.h>
#include <string.h>

#include "libavutil/avassert.h"
#include "libavutil/intreadwrite.h"
#include "put_bits.h"

void ff_put_string(PutBitContext* pb, const char* string, int terminate_string)
{
	while (*string)
	{
		put_bits(pb, 8, *string);
		string++;
	}
	if (terminate_string)
		put_bits(pb, 8, 0);
}

void ff_copy_bits(PutBitContext* pb, const uint8_t* src, int length)
{
	int words = length >> 4;
	int bits = length & 15;
	int i;

	if (length == 0)
		return;

	av_assert0(length <= put_bits_left(pb));

	if (words < 16 || put_bits_count(pb) & 7)
	{
		for (i = 0; i < words; i++)
			put_bits(pb, 16, AV_RB16(src + 2 * i));
	}
	else
	{
		for (i = 0; put_bits_count(pb) & 31; i++)
			put_bits(pb, 8, src[i]);
		flush_put_bits(pb);
		memcpy(put_bits_ptr(pb), src + i, 2 * words - i);
		skip_put_bytes(pb, 2 * words - i);
	}

	put_bits(pb, bits, AV_RB16(src + 2 * words) >> (16 - bits));
}
