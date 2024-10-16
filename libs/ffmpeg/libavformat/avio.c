/*
 * unbuffered I/O
 * Copyright (c) 2001 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "libavutil/dict.h"
#include "avio_internal.h"
#include "internal.h"

int avio_close(AVIOContext* s)
{
	int error;

	if (!s)
		return 0;

	avio_flush(s);
	s->opaque = NULL;

	av_freep(&s->buffer);

	error = s->error;
	avio_context_free(&s);

	return error;
}

int avio_closep(AVIOContext** s)
{
	int ret = avio_close(*s);
	*s = NULL;
	return ret;
}
