/*
 * Register all the formats and protocols
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stddef.h>
#include <stdint.h>

#include "libavformat/internal.h"
#include "avformat.h"
#include "demux.h"

extern const FFInputFormat ff_bink_demuxer;

static const FFInputFormat* const demuxer_list[] = {
	&ff_bink_demuxer,
	NULL};

static intptr_t indev_list_intptr = 0;

const AVInputFormat* av_demuxer_iterate(void** opaque)
{
	static const uintptr_t size = sizeof(demuxer_list) / sizeof(demuxer_list[0]) - 1;
	uintptr_t i = (uintptr_t)*opaque;
	const FFInputFormat* f = NULL;
	uintptr_t tmp;

	if (i < size)
	{
		f = demuxer_list[i];
	}
	else if ((tmp = indev_list_intptr))
	{
		const FFInputFormat* const* indev_list = (const FFInputFormat* const*)tmp;
		f = indev_list[i - size];
	}

	if (f)
	{
		*opaque = (void*)(i + 1);
		return &f->p;
	}
	return NULL;
}
