/*
 * Version macros.
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVFORMAT_VERSION_H
#define AVFORMAT_VERSION_H

#include "libavutil/version.h"

#include "version_major.h"

#define LIBAVFORMAT_VERSION_MINOR   1
#define LIBAVFORMAT_VERSION_MICRO 100

#define LIBAVFORMAT_VERSION_INT \
	AV_VERSION_INT(LIBAVFORMAT_VERSION_MAJOR, LIBAVFORMAT_VERSION_MINOR, LIBAVFORMAT_VERSION_MICRO)

#endif /* AVFORMAT_VERSION_H */
