/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "intmath.h"

/* undef these to get the function prototypes from common.h */
#undef av_log2
#undef av_log2_16bit
#include "common.h"

int av_log2(unsigned v)
{
	return ff_log2(v);
}

int av_log2_16bit(unsigned v)
{
	return ff_log2_16bit(v);
}
