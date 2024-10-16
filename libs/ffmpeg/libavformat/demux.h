/*
 * copyright (c) 2001 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVFORMAT_DEMUX_H
#define AVFORMAT_DEMUX_H

#include <stdint.h>
#include "libavcodec/packet.h"
#include "avformat.h"

typedef struct FFInputFormat
{
	/* The public AVInputFormat. */
	AVInputFormat p;

	/* Raw demuxers store their codec ID here. */
	enum AVCodecID raw_codec_id;

	/* Size of private data so that it can be allocated in the wrapper. */
	int priv_data_size;

	/* Tell if a given file has a chance of being parsed as this format. */
	int (*read_probe)(const AVProbeData*);

	/* Read the format header and initialize the AVFormatContext structure. */
	int (*read_header)(struct AVFormatContext*);

	/* Read one packet and put it in 'pkt'. */
	int (*read_packet)(struct AVFormatContext*, AVPacket* pkt);

	/* Seek to a given timestamp relative to the frames in stream component stream_index. */
	int (*read_seek)(struct AVFormatContext*, int stream_index, int64_t timestamp, int flags);
} FFInputFormat;

static inline const FFInputFormat* ffifmt(const AVInputFormat* fmt)
{
	return (const FFInputFormat*)fmt;
}

#define MAX_STD_TIMEBASES (30*12+30+3+6)
typedef struct FFStreamInfo
{
	int64_t duration_gcd;
	int duration_count;
	int64_t rfps_duration_sum;
	double (*duration_error)[2][MAX_STD_TIMEBASES];
	int64_t codec_info_duration;
	int64_t codec_info_duration_fields;
	int frame_delay_evidence;

	/**
	 * 0  -> decoder has not been searched for yet.
	 * >0 -> decoder found
	 * <0 -> decoder with codec_id == -found_decoder has not been found
	 */
	int found_decoder;

	/* Those are used for average framerate estimation. */

	int64_t fps_first_dts;
	int     fps_first_dts_idx;
	int64_t fps_last_dts;
	int     fps_last_dts_idx;
} FFStreamInfo;

/*
 * Returned by demuxers to indicate that data was consumed but discarded
 * (ignored streams or junk data). The framework will re-call the demuxer.
 */
#define FFERROR_REDO FFERRTAG('R','E','D','O')

#define RELATIVE_TS_BASE (INT64_MAX - (1LL << 48))

static inline int is_relative(int64_t ts)
{
	return ts > (RELATIVE_TS_BASE - (1LL << 48));
}

/* Wrap a given time stamp, if there is an indication for an overflow */
int64_t ff_wrap_timestamp(const AVStream* st, int64_t timestamp);

/* Read a transport packet from a media file. */
int ff_read_packet(AVFormatContext* s, AVPacket* pkt);

/* Internal version of av_index_search_timestamp */
int ff_index_search_timestamp(const AVIndexEntry* entries, int nb_entries, int64_t wanted_timestamp, int flags);

/* Internal version of av_add_index_entry */
int ff_add_index_entry(AVIndexEntry** index_entries,
	int* nb_index_entries,
	unsigned int* index_entries_allocated_size,
	int64_t pos, int64_t timestamp, int size, int distance, int flags);

/*
 * Ensure the index uses less memory than the maximum specified in
 * AVFormatContext.max_index_size by discarding entries if it grows
 * too large.
 */
void ff_reduce_index(AVFormatContext* s, int stream_index);

void ff_rfps_calculate(AVFormatContext* ic);

/*
 * Allocate extradata with additional AV_INPUT_BUFFER_PADDING_SIZE at end
 * which is always set to 0 and fill it from pb.
 */
int ff_get_extradata(void* logctx, AVCodecParameters* par, AVIOContext* pb, int size);

#endif /* AVFORMAT_DEMUX_H */
