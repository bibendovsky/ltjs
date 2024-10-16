/*
 * Various utility demuxing functions
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "libavutil/version.h"

#include "demux.h"
#include "avio_internal.h"
#include "internal.h"

int ff_get_extradata(void* logctx, AVCodecParameters* par, AVIOContext* pb, int size)
{
	(void)logctx;

	int ret = ff_alloc_extradata(par, size);
	if (ret < 0)
		return ret;
	ret = ffio_read_size(pb, par->extradata, size);
	if (ret < 0)
	{
		av_freep(&par->extradata);
		par->extradata_size = 0;
		return ret;
	}

	return ret;
}
