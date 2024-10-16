/*
 * copyright (c) 2001 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVFORMAT_INTERNAL_H
#define AVFORMAT_INTERNAL_H

#include <stdint.h>

#include "libavcodec/packet_internal.h"

#include "avformat.h"

/* size of probe buffer, for guessing file type from file contents */
#define PROBE_BUF_MIN 2048
#define PROBE_BUF_MAX (1 << 20)

/*************************************************/

typedef struct FFFormatContext
{
	/* The public context. */
	AVFormatContext pub;

	/* This buffer is only needed when packets were already buffered but not decoded. */
	PacketList packet_buffer;

	/* av_seek_frame() support */
	int64_t data_offset; /* offset of the first packet */

	/* Raw packets from the demuxer, prior to parsing and decoding. */
	PacketList raw_packet_buffer;
	/* Packets split by the parser get queued here. */
	PacketList parse_queue;

	/* Used to hold temporary packets for the generic demuxing code. */
	AVPacket* pkt;
	/* Sum of the size of packets in raw_packet_buffer, in bytes. */
	int raw_packet_buffer_size;
} FFFormatContext;

static inline FFFormatContext* ffformatcontext(AVFormatContext* s)
{
	return (FFFormatContext*)s;
}

typedef struct FFStream
{
	/* The public context. */
	AVStream pub;

	AVFormatContext* fmtctx;

	/* The codec context used by avformat_find_stream_info, the parser, etc. */
	struct AVCodecContext* avctx;
	/* 1 if avctx has been initialized with the values from the codec parameters */
	int avctx_inited;

	/* the context for extracting extradata in find_stream_info() */
	struct
	{
		struct AVBSFContext* bsf;
		int inited;
	} extract_extradata;

	/* Whether the internal avctx needs to be updated from codecpar (after a late change to codecpar) */
	int need_context_update;

	/* Stream information used internally by avformat_find_stream_info() */
	struct FFStreamInfo* info;

	AVIndexEntry* index_entries; /* Only used if the format does not support seeking natively. */
	int nb_index_entries;
	unsigned int index_entries_allocated_size;

	/**
	 * stream probing state
	 * -1   -> probing finished
	 *  0   -> no probing requested
	 * rest -> perform probing with request_probe being the minimum score to accept.
	 */
	int request_probe;
	/* Indicates that everything up to the next keyframe should be discarded. */
	int skip_to_keyframe;

	/* Number of internally decoded frames. */
	int nb_decoded_frames;

	/* Internal data to check for wrapping of the time stamp */
	int64_t pts_wrap_reference;

	/* Options for behavior, when a wrap is detected. Defined by AV_PTS_WRAP_ values. */
	int pts_wrap_behavior;

	/* Internal data to prevent doing update_initial_durations() twice */
	int update_initial_durations_done;

#define MAX_REORDER_DELAY 16

	/* Internal data to generate dts from pts */

	int64_t pts_buffer[MAX_REORDER_DELAY + 1];

	/* Internal data to analyze DTS and detect faulty mpeg streams */

	int64_t last_dts_for_order_check;
	uint8_t dts_ordered;
	uint8_t dts_misordered;

	/* display aspect ratio (0 if unknown) */
	AVRational display_aspect_ratio;

	AVProbeData probe_data;

	int64_t last_IP_pts;
	int last_IP_duration;

	/* Number of packets to buffer for codec probing */
	int probe_packets;

	/* Number of frames that have been demuxed during avformat_find_stream_info() */
	int codec_info_nb_frames;

	/* Timestamp corresponding to the last dts sync point. */

	int64_t first_dts;
	int64_t cur_dts;
} FFStream;

static inline FFStream* ffstream(AVStream* st)
{
	return (FFStream*)st;
}

static inline const FFStream* cffstream(const AVStream* st)
{
	return (const FFStream*)st;
}

void ff_flush_packet_queue(AVFormatContext* s);

const struct AVCodec* ff_find_decoder(AVFormatContext* s, const AVStream* st, enum AVCodecID codec_id);

/* Set the time base and wrapping info for a given stream. */
void avpriv_set_pts_info(AVStream* st, int pts_wrap_bits, unsigned int pts_num, unsigned int pts_den);

/* Frees a stream without modifying the corresponding AVFormatContext. */
void ff_free_stream(AVStream** st);

/* Allocate extradata with additional AV_INPUT_BUFFER_PADDING_SIZE at end which is always set to 0. */
int ff_alloc_extradata(AVCodecParameters* par, int size);

#endif /* AVFORMAT_INTERNAL_H */
