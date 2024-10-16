/*
 * AVPacket public API
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVCODEC_PACKET_H
#define AVCODEC_PACKET_H

#include <stddef.h>
#include <stdint.h>

#include "libavutil/buffer.h"
#include "libavutil/dict.h"
#include "libavutil/rational.h"
#include "libavutil/version.h"

#include "libavcodec/version_major.h"

/* Types and functions for working with AVPacket. */

/* This structure stores compressed data. */
typedef struct AVPacket
{
	/* A reference to the reference-counted buffer where the packet data is stored. */
	AVBufferRef* buf;
	/* Presentation timestamp in AVStream->time_base units */
	int64_t pts;
	/* Decompression timestamp in AVStream->time_base units */
	int64_t dts;
	uint8_t* data;
	int size;
	int stream_index;
	/* A combination of AV_PKT_FLAG values */
	int flags;

	/* Duration of this packet in AVStream->time_base units, 0 if unknown. */
	int64_t duration;

	int64_t pos; /* byte position in stream, -1 if unknown */

	/* Time base of the packet's timestamps. */
	AVRational time_base;
} AVPacket;

#define AV_PKT_FLAG_KEY     0x0001 /* The packet contains a keyframe */
#define AV_PKT_FLAG_CORRUPT 0x0002 /* The packet content is corrupted */
/*
 * Flag is used to discard packets which are required to maintain valid
 * decoder state but are not required for output and should be dropped
 * after decoding.
 */
#define AV_PKT_FLAG_DISCARD 0x0004

/* Allocate an AVPacket and set its fields to default values. */
AVPacket* av_packet_alloc(void);

/* Free the packet, if the packet is reference counted, it will be unreferenced first. */
void av_packet_free(AVPacket** pkt);

/* Reduce packet size, correctly zeroing padding */
void av_shrink_packet(AVPacket* pkt, int size);

/* Increase packet size, correctly zeroing padding */
int av_grow_packet(AVPacket* pkt, int grow_by);

/* Setup a new reference to the data described by a given packet. */
int av_packet_ref(AVPacket* dst, const AVPacket* src);

/* Wipe the packet. */
void av_packet_unref(AVPacket* pkt);

/* Move every field in src to dst and reset src. */
void av_packet_move_ref(AVPacket* dst, AVPacket* src);

/* Copy only "properties" fields from src to dst. */
int av_packet_copy_props(AVPacket* dst, const AVPacket* src);

/* Ensure the data described by a given packet is reference counted. */
int av_packet_make_refcounted(AVPacket* pkt);

#endif // AVCODEC_PACKET_H
