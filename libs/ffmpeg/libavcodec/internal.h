/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* common internal api header. */

#ifndef AVCODEC_INTERNAL_H
#define AVCODEC_INTERNAL_H

#include <stdint.h>

#include "libavutil/channel_layout.h"
#include "avcodec.h"

#define FF_SANE_NB_CHANNELS 512U

#define STRIDE_ALIGN 8

typedef struct AVCodecInternal
{
	struct FramePool* pool;

	/* This packet is used to hold the packet given to decoders implementing the .decode API. */
	AVPacket* in_pkt;
	struct AVBSFContext* bsf;

	/* Properties (timestamps+side data) extracted from the last packet passed for decoding. */
	AVPacket* last_pkt_props;

	/* If this is set, then FFCodec->close (if existing) needs to be called for the parent AVCodecContext. */
	int needs_close;

	/* Number of audio samples to skip at the start of the next decoded frame */
	int skip_samples;

	/* checks API usage: after codec draining, flush is required to resume operation */
	int draining;

	/* Temporary buffers for newly received or not yet output packets/frames. */
	AVPacket* buffer_pkt;
	AVFrame* buffer_frame;
	int draining_done;
} AVCodecInternal;

int avpriv_codec_get_cap_skip_frame_fill_param(const AVCodec* codec);

#endif /* AVCODEC_INTERNAL_H */
