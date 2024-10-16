/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVCODEC_VERSION_H
#define AVCODEC_VERSION_H

/* Libavcodec version macros. */

#include "libavutil/version.h"

#include "version_major.h"

#define LIBAVCODEC_VERSION_MINOR   3
#define LIBAVCODEC_VERSION_MICRO 100

#define LIBAVCODEC_VERSION_INT \
	AV_VERSION_INT(LIBAVCODEC_VERSION_MAJOR, LIBAVCODEC_VERSION_MINOR, LIBAVCODEC_VERSION_MICRO)

#endif /* AVCODEC_VERSION_H */
