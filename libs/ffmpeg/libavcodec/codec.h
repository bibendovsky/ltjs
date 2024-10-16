/*
 * AVCodec public API
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVCODEC_CODEC_H
#define AVCODEC_CODEC_H

#include <stdint.h>

#include "libavutil/avutil.h"
#include "libavutil/channel_layout.h"
#include "libavutil/log.h"
#include "libavutil/pixfmt.h"
#include "libavutil/rational.h"
#include "libavutil/samplefmt.h"

#include "libavcodec/codec_id.h"
#include "libavcodec/version_major.h"

/* Codec uses get_buffer() or get_encode_buffer() for allocating buffers and supports custom allocators. */
#define AV_CODEC_CAP_DR1 (1 << 1)

typedef struct AVCodec
{
	/* Name of the codec implementation. */
	const char* name;
	/* Descriptive name for the codec, meant to be more human readable than name. */
	const char* long_name;
	enum AVMediaType type;
	enum AVCodecID id;
	/* Codec capabilities. See AV_CODEC_CAP_*. */
	int capabilities;
} AVCodec;

/* Iterate over all registered codecs. */
const AVCodec* av_codec_iterate(void** opaque);

/* Find a registered decoder with a matching codec ID. */
const AVCodec* avcodec_find_decoder(enum AVCodecID id);

/* Return a non-zero number if codec is a decoder, zero otherwise. */
int av_codec_is_decoder(const AVCodec* codec);

#endif /* AVCODEC_CODEC_H */
