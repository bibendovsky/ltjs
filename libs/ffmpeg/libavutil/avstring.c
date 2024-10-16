/*
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 * Copyright (c) 2007 Mans Rullgard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "avstring.h"
#include "macros.h"

int av_strncasecmp(const char* a, const char* b, size_t n)
{
	uint8_t c1, c2;
	if (n <= 0)
		return 0;
	do
	{
		c1 = (uint8_t)(av_tolower(*a++));
		c2 = (uint8_t)(av_tolower(*b++));
	} while (--n && c1 && c1 == c2);
	return c1 - c2;
}

int av_match_name(const char* name, const char* names)
{
	const char* p;
	size_t len, namelen;

	if (!name || !names)
		return 0;

	namelen = strlen(name);
	while (*names)
	{
		int negate = '-' == *names;
		p = strchr(names, ',');
		if (!p)
			p = names + strlen(names);
		names += negate;
		len = FFMAX((size_t)(p - names), namelen);
		if (!av_strncasecmp(name, names, len) || !strncmp("ALL", names, FFMAX(3, p - names)))
			return !negate;
		names = p + (*p == ',');
	}
	return 0;
}
