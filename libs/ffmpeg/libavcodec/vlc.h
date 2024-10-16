/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVCODEC_VLC_H
#define AVCODEC_VLC_H

#include <stddef.h>
#include <stdint.h>

#include "libavutil/macros.h"

#define VLC_MULTI_MAX_SYMBOLS 6

// When changing this, be sure to also update tableprint_vlc.h accordingly.
typedef int16_t VLCBaseType;

typedef struct VLCElem
{
	VLCBaseType sym, len;
} VLCElem;

typedef struct VLC
{
	int bits;
	VLCElem* table;
	int table_size, table_allocated;
} VLC;

typedef struct VLC_MULTI_ELEM
{
	uint8_t val[VLC_MULTI_MAX_SYMBOLS];
	int8_t len; // -31,32
	uint8_t num;
} VLC_MULTI_ELEM;

typedef struct VLC_MULTI
{
	VLC_MULTI_ELEM* table;
	int table_size, table_allocated;
} VLC_MULTI;

typedef struct RL_VLC_ELEM
{
	int16_t level;
	int8_t len;
	uint8_t run;
} RL_VLC_ELEM;

#define vlc_init(vlc, nb_bits, nb_codes,                \
                 bits, bits_wrap, bits_size,            \
                 codes, codes_wrap, codes_size,         \
                 flags)                                 \
    ff_vlc_init_sparse(vlc, nb_bits, nb_codes,          \
                       bits, bits_wrap, bits_size,      \
                       codes, codes_wrap, codes_size,   \
                       NULL, 0, 0, flags)

/* Build VLC decoding tables suitable for use with get_vlc2(). */
int ff_vlc_init_sparse(VLC* vlc, int nb_bits, int nb_codes,
	const void* bits, int bits_wrap, int bits_size,
	const void* codes, int codes_wrap, int codes_size,
	const void* symbols, int symbols_wrap, int symbols_size,
	int flags);

#define VLC_INIT_USE_STATIC      1
#define VLC_INIT_STATIC_OVERLONG (2 | VLC_INIT_USE_STATIC)
/* If VLC_INIT_INPUT_LE is set, the LSB bit of the codes used to initialize the VLC table is the first bit to be read. */
#define VLC_INIT_INPUT_LE        4
/* If set the VLC is intended for a little endian bitstream reader. */
#define VLC_INIT_OUTPUT_LE       8
#define VLC_INIT_LE              (VLC_INIT_INPUT_LE | VLC_INIT_OUTPUT_LE)

#endif /* AVCODEC_VLC_H */
