/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Utility Preprocessor macros */

#ifndef AVUTIL_MACROS_H
#define AVUTIL_MACROS_H

#if LTJS_FFMPEG_AV_HAVE_BIGENDIAN
#   define AV_NE(be, le) (be)
#else
#   define AV_NE(be, le) (le)
#endif

/**
 * Comparator.
 * For two numerical expressions x and y, gives 1 if x > y, -1 if x < y, and 0
 * if x == y. This is useful for instance in a qsort comparator callback.
 */
#define FFDIFFSIGN(x,y) (((x)>(y)) - ((x)<(y)))

#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))

#define FFSWAP(type,a,b) do{type SWAP_tmp= b; b= a; a= SWAP_tmp;}while(0)
#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

#define MKTAG(a,b,c,d)   ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))

#define FFALIGN(x, a) (((x)+(a)-1)&~((a)-1))

#endif /* AVUTIL_MACROS_H */
