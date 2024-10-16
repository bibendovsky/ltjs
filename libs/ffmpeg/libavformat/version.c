/*
 * Version functions.
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "avformat.h"
#include "version.h"

unsigned avformat_version(void)
{
	return LIBAVFORMAT_VERSION_INT;
}
