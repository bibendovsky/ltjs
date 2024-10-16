/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* reference-counted frame API */

#ifndef AVUTIL_FRAME_H
#define AVUTIL_FRAME_H

#include <stddef.h>
#include <stdint.h>

#include "avutil.h"
#include "buffer.h"
#include "channel_layout.h"
#include "dict.h"
#include "rational.h"
#include "samplefmt.h"
#include "pixfmt.h"
#include "version.h"

/* This structure describes decoded (raw) audio or video data. */
typedef struct AVFrame
{
#define AV_NUM_DATA_POINTERS 8
	/* pointer to the picture/channel planes. */
	uint8_t* data[AV_NUM_DATA_POINTERS];

	/* strides */
	int linesize[AV_NUM_DATA_POINTERS];

	/* pointers to the data planes/channels. */
	uint8_t** extended_data;

	/* Video dimensions */
	int width, height;

	/* number of audio samples (per channel) described by this frame */
	int nb_samples;

	/* format of the frame, -1 if unknown or unset */
	int format;

	/* Sample aspect ratio for the video frame, 0/1 if unknown/unspecified. */
	AVRational sample_aspect_ratio;

	/* Presentation timestamp in time_base units (time when frame should be shown to user). */
	int64_t pts;

	/* DTS copied from the AVPacket that triggered returning this frame. */
	int64_t pkt_dts;

	/* Time base for the timestamps in this frame. */
	AVRational time_base;

	/* Sample rate of the audio data. */
	int sample_rate;

	/* AVBuffer references backing the data for this frame. */
	AVBufferRef* buf[AV_NUM_DATA_POINTERS];

/* Flags describing additional frame properties. */

/* A flag to mark frames that are keyframes. */
#define AV_FRAME_FLAG_KEY (1 << 1)
/* A flag to mark the frames which need to be decoded, but shouldn't be output. */
#define AV_FRAME_FLAG_DISCARD   (1 << 2)

	/* Frame flags */
	int flags;

	/* MPEG vs JPEG YUV range. */
	enum AVColorRange color_range;

	/* frame timestamp estimated using various heuristics, in stream time base */
	int64_t best_effort_timestamp;

	/* Channel layout of the audio data. */
	AVChannelLayout ch_layout;

	/* Duration of the frame, in the same units as pts. 0 if unknown. */
	int64_t duration;
} AVFrame;

/* Allocate an AVFrame and set its fields to default values. */
AVFrame* av_frame_alloc(void);

/* Free the frame and any dynamically allocated objects in it, e.g. extended_data. */
void av_frame_free(AVFrame** frame);

/* Set up a new reference to the data described by the source frame. */
int av_frame_ref(AVFrame* dst, const AVFrame* src);

/**
 * Ensure the destination frame refers to the same data described by the source
 * frame, either by creating a new reference for each AVBufferRef from src if
 * they differ from those in dst, by allocating new buffers and copying data if
 * src is not reference counted, or by unrefencing it if src is empty.
 */
int av_frame_replace(AVFrame* dst, const AVFrame* src);

/* Unreference all the buffers referenced by frame and reset the frame fields. */
void av_frame_unref(AVFrame* frame);

/* Move everything contained in src to dst and reset src. */
void av_frame_move_ref(AVFrame* dst, AVFrame* src);

/* Allocate new buffer(s) for audio or video data. */
int av_frame_get_buffer(AVFrame* frame, int align);

/* Check if the frame data is writable. */
int av_frame_is_writable(AVFrame* frame);

/* Copy the frame data from src to dst. */
int av_frame_copy(AVFrame* dst, const AVFrame* src);

#endif /* AVUTIL_FRAME_H */
