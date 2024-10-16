/*
 * Copyright (c) 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVUTIL_MEM_INTERNAL_H
#define AVUTIL_MEM_INTERNAL_H

#include <stdint.h>

#include "macros.h"

#define ALIGN_64 16

#define DECLARE_ALIGNED_T(n,t,v)    t v
#define DECLARE_ALIGNED(n,t,v) DECLARE_ALIGNED_V(n,t,v)
#define DECLARE_ALIGNED_V(n,t,v) DECLARE_ALIGNED_##n(t,v)
#define DECLARE_ALIGNED_32(t,v) DECLARE_ALIGNED_T(ALIGN_32, t, v)

// Some broken preprocessors need a second expansion
// to be forced to tokenize __VA_ARGS__
#define E1(x) x

#define LOCAL_ALIGNED_A(a, t, v, s, o, ...)             \
    uint8_t la_##v[sizeof(t s o) + (a)];                \
    t (*v) o = (void *)FFALIGN((uintptr_t)la_##v, a)

#define LOCAL_ALIGNED(a, t, v, ...) LOCAL_ALIGNED_##a(t, v, __VA_ARGS__)
#define LOCAL_ALIGNED_16(t, v, ...) E1(LOCAL_ALIGNED_A(16, t, v, __VA_ARGS__,,))
#define LOCAL_ALIGNED_32(t, v, ...) E1(LOCAL_ALIGNED_A(32, t, v, __VA_ARGS__,,))

#endif /* AVUTIL_MEM_INTERNAL_H */
