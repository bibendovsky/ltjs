/*
 * Version functions.
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "avutil.h"
#include "version.h"

unsigned avutil_version(void)
{
    return LIBAVUTIL_VERSION_INT;
}
