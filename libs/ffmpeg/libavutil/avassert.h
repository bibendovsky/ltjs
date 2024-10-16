/*
 * copyright (c) 2010 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* simple assert() macros that are a bit more flexible than ISO C assert(). */

#ifndef AVUTIL_AVASSERT_H
#define AVUTIL_AVASSERT_H

#include <stdlib.h>
#include "log.h"
#include "macros.h"

/* assert() equivalent, that is always enabled. */
#define av_assert0(cond) do { \
    if (!(cond)) {            \
        abort();              \
    }                         \
} while (0)


/* These asserts() thus can be enabled without fearing speed loss. */
#if defined(ASSERT_LEVEL) && ASSERT_LEVEL > 0
#define av_assert1(cond) av_assert0(cond)
#else
#define av_assert1(cond) ((void)0)
#endif

/* assert() equivalent, that does lie in speed critical code. */
#if defined(ASSERT_LEVEL) && ASSERT_LEVEL > 1
#define av_assert2(cond) av_assert0(cond)
#else
#define av_assert2(cond) ((void)0)
#endif

#endif /* AVUTIL_AVASSERT_H */
