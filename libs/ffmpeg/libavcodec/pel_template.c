/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stddef.h>
#include <stdint.h>

#include "libavutil/intreadwrite.h"
#include "pixels.h"
#include "rnd_avg.h"

#include "bit_depth_template.c"

#define DEF_PEL(OPNAME, OP)                                             \
static inline void FUNCC(OPNAME ## _pixels2)(uint8_t *block,            \
                                             const uint8_t *pixels,     \
                                             ptrdiff_t line_size,       \
                                             int h)                     \
{                                                                       \
    int i;                                                              \
    for (i = 0; i < h; i++) {                                           \
        OP(*((pixel2 *) block), AV_RN2P(pixels));                       \
        pixels += line_size;                                            \
        block  += line_size;                                            \
    }                                                                   \
}                                                                       \
                                                                        \
static inline void FUNCC(OPNAME ## _pixels4)(uint8_t *block,            \
                                             const uint8_t *pixels,     \
                                             ptrdiff_t line_size,       \
                                             int h)                     \
{                                                                       \
    int i;                                                              \
    for (i = 0; i < h; i++) {                                           \
        OP(*((pixel4 *) block), AV_RN4P(pixels));                       \
        pixels += line_size;                                            \
        block  += line_size;                                            \
    }                                                                   \
}                                                                       \
                                                                        \
static inline void FUNCC(OPNAME ## _pixels8)(uint8_t *block,            \
                                             const uint8_t *pixels,     \
                                             ptrdiff_t line_size,       \
                                             int h)                     \
{                                                                       \
    int i;                                                              \
    for (i = 0; i < h; i++) {                                           \
        OP(*((pixel4 *) block), AV_RN4P(pixels));                       \
        OP(*((pixel4 *) (block + 4 * sizeof(pixel))),                   \
           AV_RN4P(pixels + 4 * sizeof(pixel)));                        \
        pixels += line_size;                                            \
        block  += line_size;                                            \
    }                                                                   \
}                                                                       \
                                                                        \
CALL_2X_PIXELS(FUNCC(OPNAME ## _pixels16),                              \
               FUNCC(OPNAME ## _pixels8),                               \
               8 * sizeof(pixel))

#define op_avg(a, b) a = rnd_avg_pixel4(a, b)
#define op_put(a, b) a = b

DEF_PEL(avg, op_avg)
DEF_PEL(put, op_put)
#undef op_avg
#undef op_put
