/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdint.h>
#include <string.h>

#include "libavutil/log.h"

#include "bsf.h"
#include "bsf_internal.h"

static const FFBitStreamFilter* const bitstream_filters[] = {
	NULL};

const AVBitStreamFilter* av_bsf_iterate(void** opaque)
{
	uintptr_t i = (uintptr_t)*opaque;
	const FFBitStreamFilter* f = bitstream_filters[i];

	if (f)
	{
		*opaque = (void*)(i + 1);
		return &f->p;
	}
	return NULL;
}

const AVBitStreamFilter* av_bsf_get_by_name(const char* name)
{
	const AVBitStreamFilter* f = NULL;
	void* i = 0;

	if (!name)
		return NULL;

	while ((f = av_bsf_iterate(&i)))
	{
		if (!strcmp(f->name, name))
			return f;
	}

	return NULL;
}
