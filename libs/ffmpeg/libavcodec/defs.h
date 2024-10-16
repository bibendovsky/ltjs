/*
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVCODEC_DEFS_H
#define AVCODEC_DEFS_H

/* Misc types and constants that do not belong anywhere else. */

/* Required number of additionally allocated bytes at the end of the input bitstream for decoding. */
#define AV_INPUT_BUFFER_PADDING_SIZE 64

enum AVDiscard
{
	AVDISCARD_DEFAULT = 0, /* discard useless packets like 0 size packets in avi */
	AVDISCARD_ALL = 48, /* discard all */
};

#endif // AVCODEC_DEFS_H
