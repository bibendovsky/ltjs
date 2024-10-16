/*
 * copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVUTIL_LOG_H
#define AVUTIL_LOG_H

/* Print no output. */
#define AV_LOG_QUIET -8

/* Standard information. */
#define AV_LOG_INFO 32

/* Extremely verbose debugging, useful for libav* development. */
#define AV_LOG_TRACE 56

#define AV_LOG_MAX_OFFSET (AV_LOG_TRACE - AV_LOG_QUIET)

/* Set the log level */
void av_log_set_level(int level);

#endif /* AVUTIL_LOG_H */
