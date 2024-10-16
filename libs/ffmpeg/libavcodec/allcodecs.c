/*
 * Provide registration of all codecs, parsers and bitstream filters for libavcodec.
 * Copyright (c) 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Provide registration of all codecs, parsers and bitstream filters for libavcodec. */

#include <stdint.h>
#include <string.h>

#include "codec.h"
#include "codec_id.h"
#include "avcodec_internal.h"
#include "codec_internal.h"

extern const FFCodec ff_bink_decoder;
extern const FFCodec ff_binkaudio_dct_decoder;
extern const FFCodec ff_binkaudio_rdft_decoder;

static const FFCodec* const codec_list[] = {
	&ff_bink_decoder,
	&ff_binkaudio_dct_decoder,
	&ff_binkaudio_rdft_decoder,
	NULL};

static int av_codec_static_init = 0;
static void av_codec_init_static(void)
{
	for (int i = 0; codec_list[i]; i++)
	{
		if (codec_list[i]->init_static_data)
			codec_list[i]->init_static_data((FFCodec*)codec_list[i]);
	}
}

const AVCodec* av_codec_iterate(void** opaque)
{
	uintptr_t i = (uintptr_t)*opaque;
	const FFCodec* c = codec_list[i];

	if (!av_codec_static_init)
	{
		av_codec_static_init = 1;
		av_codec_init_static();
	}

	if (c)
	{
		*opaque = (void*)(i + 1);
		return &c->p;
	}
	return NULL;
}

static const AVCodec* find_codec(enum AVCodecID id, int (*x)(const AVCodec*))
{
	const AVCodec* p;
	void* i = 0;

	while ((p = av_codec_iterate(&i)))
	{
		if (!x(p))
			continue;
		if (p->id == id)
		{
			return p;
		}
	}

	return NULL;
}

const AVCodec* avcodec_find_decoder(enum AVCodecID id)
{
	return find_codec(id, av_codec_is_decoder);
}
