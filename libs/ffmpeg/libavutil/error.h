/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**
 * @file
 * error code definitions
 */

#ifndef AVUTIL_ERROR_H
#define AVUTIL_ERROR_H

#include <errno.h>
#include <stddef.h>

#include "macros.h"

/* error handling */
#if EDOM > 0
#define AVERROR(e) (-(e))   // Returns a negative error code from a POSIX error code, to return from library functions.
#define AVUNERROR(e) (-(e)) // Returns a POSIX error code from a library function error return value.
#else
/* Some platforms have E* and errno already negated. */
#define AVERROR(e) (e)
#define AVUNERROR(e) (e)
#endif

#define FFERRTAG(a, b, c, d) (-(int)MKTAG(a, b, c, d))

#define AVERROR_BUG                FFERRTAG( 'B','U','G','!') // Internal bug, also see AVERROR_BUG2
#define AVERROR_DECODER_NOT_FOUND  FFERRTAG(0xF8,'D','E','C') // Decoder not found
#define AVERROR_ENCODER_NOT_FOUND  FFERRTAG(0xF8,'E','N','C') // Encoder not found
#define AVERROR_EOF                FFERRTAG( 'E','O','F',' ') // End of file
#define AVERROR_INVALIDDATA        FFERRTAG( 'I','N','D','A') // Invalid data found when processing input
#define AVERROR_PATCHWELCOME       FFERRTAG( 'P','A','W','E') // Not yet implemented in FFmpeg, patches welcome
#define AVERROR_PROTOCOL_NOT_FOUND FFERRTAG(0xF8,'P','R','O') // Protocol not found

#define AVERROR_STREAM_NOT_FOUND   FFERRTAG(0xF8,'S','T','R') // Stream not found
#define AVERROR_UNKNOWN            FFERRTAG( 'U','N','K','N') // Unknown error, typically from an external library

#define AV_ERROR_MAX_STRING_SIZE 64

#endif /* AVUTIL_ERROR_H */
