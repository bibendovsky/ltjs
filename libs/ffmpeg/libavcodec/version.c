/*
 * Version functions.
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "version.h"

unsigned avcodec_version(void)
{
	return LIBAVCODEC_VERSION_INT;
}
