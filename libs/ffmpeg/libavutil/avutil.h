/*
 * copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVUTIL_AVUTIL_H
#define AVUTIL_AVUTIL_H

/* Convenience header that includes @ref lavu "libavutil"'s core. */

/* Return the LIBAVUTIL_VERSION_INT constant. */
unsigned avutil_version(void);

enum AVMediaType
{
	AVMEDIA_TYPE_UNKNOWN = -1,
	AVMEDIA_TYPE_VIDEO,
	AVMEDIA_TYPE_AUDIO,
};

/* Undefined timestamp value */
#define AV_NOPTS_VALUE ((int64_t)UINT64_C(0x8000000000000000))

/* Internal time base represented as integer */
#define AV_TIME_BASE 1000000

/* Internal time base represented as fractional value */

#ifdef __cplusplus
/* ISO C++ forbids compound-literals. */
#define AV_TIME_BASE_Q av_make_q(1, AV_TIME_BASE)
#else
#define AV_TIME_BASE_Q (AVRational){1, AV_TIME_BASE}
#endif

#include "common.h"
#include "rational.h"
#include "version.h"
#include "macros.h"
#include "mathematics.h"
#include "log.h"
#include "pixfmt.h"

#endif /* AVUTIL_AVUTIL_H */
