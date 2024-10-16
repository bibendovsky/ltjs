/*
 * simple math operations
 * Copyright (c) 2001, 2002 Fabrice Bellard
 * Copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at> et al
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef AVCODEC_MATHOPS_H
#define AVCODEC_MATHOPS_H

#include <stdint.h>

#ifndef zero_extend
static inline unsigned zero_extend(unsigned val, unsigned bits)
{
	return (val << ((8 * sizeof(int)) - bits)) >> ((8 * sizeof(int)) - bits);
}
#endif

#ifndef NEG_USR32
#   define NEG_USR32(a,s) (((uint32_t)(a))>>(32-(s)))
#endif

#endif /* AVCODEC_MATHOPS_H */
