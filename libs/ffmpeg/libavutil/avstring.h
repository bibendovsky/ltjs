/*
 * Copyright (c) 2007 Mans Rullgard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVUTIL_AVSTRING_H
#define AVUTIL_AVSTRING_H

#include <stddef.h>
#include <stdint.h>

/* Locale-independent conversion of ASCII characters to lowercase. */
static inline int av_tolower(int c)
{
	if (c >= 'A' && c <= 'Z')
		c ^= 0x20;
	return c;
}

/* Locale-independent case-insensitive compare. */
int av_strncasecmp(const char* a, const char* b, size_t n);

/* Match instances of a name in a comma-separated list of names. */
int av_match_name(const char* name, const char* names);

#endif /* AVUTIL_AVSTRING_H */
