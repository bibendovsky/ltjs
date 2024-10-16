/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVCODEC_PIXELS_H
#define AVCODEC_PIXELS_H

#include <stddef.h>
#include <stdint.h>

/* pixel operations */
#define CALL_2X_PIXELS_MACRO(STATIC, a, b, n)        \
STATIC void a(uint8_t *block, const uint8_t *pixels, \
              ptrdiff_t line_size, int h)            \
{                                                    \
    b(block, pixels, line_size, h);                  \
    b(block + n, pixels + n, line_size, h);          \
}

#define CALL_2X_PIXELS(a, b, n) CALL_2X_PIXELS_MACRO(static, a, b, n)

#endif /* AVCODEC_PIXELS_H */
