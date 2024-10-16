/*
 * copyright (c) 2001 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVFORMAT_AVFORMAT_H
#define AVFORMAT_AVFORMAT_H

/* Main libavformat public API header */

#include <stdio.h> /* FILE */

#include "libavcodec/codec_par.h"
#include "libavcodec/defs.h"
#include "libavcodec/packet.h"

#include "libavutil/dict.h"
#include "libavutil/log.h"

#include "avio.h"
#include "libavformat/version_major.h"
#include "libavformat/version.h"

#include "libavutil/frame.h"
#include "libavcodec/codec.h"

struct AVFormatContext;
struct AVFrame;

/* Allocate and read the payload of a packet and initialize its fields with default values. */
int av_get_packet(AVIOContext* s, AVPacket* pkt, int size);

struct AVCodecTag;

/* This structure contains the data a format has to probe a file. */
typedef struct AVProbeData
{
	unsigned char* buf; /* Buffer must have AVPROBE_PADDING_SIZE of extra allocated bytes filled with zero. */
	int buf_size;       /* Size of buf except extra allocated bytes */
} AVProbeData;

#define AVPROBE_SCORE_RETRY (AVPROBE_SCORE_MAX/4)
#define AVPROBE_SCORE_STREAM_RETRY (AVPROBE_SCORE_MAX/4-1)

#define AVPROBE_SCORE_EXTENSION  50 // score for file extension
#define AVPROBE_SCORE_MAX       100 // maximum score

#define AVPROBE_PADDING_SIZE 32 // extra allocated bytes at the end of the probe buffer

/// Demuxer will use avio_open, no opened file should be provided by the caller.
#define AVFMT_NOFILE        0x0001
#define AVFMT_SHOW_IDS      0x0008 /* Show format stream IDs numbers. */
#define AVFMT_NOTIMESTAMPS  0x0080 /* Format does not need / have any timestamps. */
#define AVFMT_GENERIC_INDEX 0x0100 /* Use generic index building code. */

typedef struct AVInputFormat
{
	/* A comma separated list of short names for the format. */
	const char* name;

	/* Descriptive name for the format, meant to be more human-readable than name. */
	const char* long_name;

	int flags;
} AVInputFormat;

typedef struct AVIndexEntry
{
	int64_t pos;
	int64_t timestamp;

#define AVINDEX_KEYFRAME 0x0001
#define AVINDEX_DISCARD_FRAME 0x0002 /* Flag is used to indicate which frame should be discarded after decoding. */
	int flags : 2;
	int size : 30; // Yeah, trying to keep the size of this small to reduce memory requirements (it is 24 vs. 32 bytes due to possible 8-byte alignment).
	int min_distance; /* Minimum distance between this and the previous keyframe, used to avoid unneeded searching. */
} AVIndexEntry;

/* Options for behavior on timestamp wrap detection. */

#define AV_PTS_WRAP_IGNORE      0   // ignore the wrap
#define AV_PTS_WRAP_ADD_OFFSET  1   // add the format specific offset on wrap detection
#define AV_PTS_WRAP_SUB_OFFSET  -1  // subtract the format specific offset on wrap detection

/* Stream structure. */
typedef struct AVStream
{
	int index; /* stream index in AVFormatContext */
	/* Format-specific stream ID. */
	int id;

	/* Codec parameters associated with this stream.  */
	AVCodecParameters* codecpar;

	/* This is the fundamental unit of time (in seconds) in terms of which frame timestamps are represented. */
	AVRational time_base;

	/* PTS of the first frame of the stream in presentation order, in stream time base. */
	int64_t start_time;

	/* Duration of the stream, in stream time base. */
	int64_t duration;

	enum AVDiscard discard; // Selects which packets can be discarded at will and do not need to be demuxed.

	/* sample aspect ratio (0 if unknown) */
	AVRational sample_aspect_ratio;

	/* Average framerate */
	AVRational avg_frame_rate;

	/* Flags indicating events happening on the stream, a combination of AVSTREAM_EVENT_FLAG_*. */
	int event_flags;

/* New packets for this stream were read from the file.  */
#define AVSTREAM_EVENT_FLAG_NEW_PACKETS (1 << 1)

	/* Real base framerate of the stream. */
	AVRational r_frame_rate;

	/* Number of bits in timestamps. Used for wrapping control. */
	int pts_wrap_bits;
} AVStream;

#define AVFMTCTX_NOHEADER 0x0001 /* signal that no header is present (streams are added dynamically) */

/* Format I/O context. */
typedef struct AVFormatContext
{
	/* The input container format. */
	const struct AVInputFormat* iformat;

	/* Format private data. */
	void* priv_data;

	/* I/O context. */
	AVIOContext* pb;

	/* Flags signalling stream properties. A combination of AVFMTCTX_*. */
	int ctx_flags;

	/* Number of elements in AVFormatContext.streams. */
	unsigned int nb_streams;
	/* A list of all streams in the file. */
	AVStream** streams;

	/* Position of the first frame of the component, in AV_TIME_BASE fractional seconds. */
	int64_t start_time;

	/* Duration of the stream, in AV_TIME_BASE fractional seconds. */
	int64_t duration;

	/* Total stream bitrate in bit/s, 0 if not available. */
	int64_t bit_rate;

	/* Flags modifying the (de)muxer behaviour. A combination of AVFMT_FLAG_*. */
	int flags;
#define AVFMT_FLAG_GENPTS       0x0001 // Generate missing pts even if it requires parsing future frames.
#define AVFMT_FLAG_IGNDTS       0x0008 // Ignore DTS on frames that contain both DTS & PTS
#define AVFMT_FLAG_NOFILLIN     0x0010 // Do not infer any values from other values, just return what is stored in the container
#define AVFMT_FLAG_NOBUFFER     0x0040 // Do not buffer frames when possible
#define AVFMT_FLAG_CUSTOM_IO    0x0080 // The caller has supplied a custom AVIOContext, don't avio_close() it.
#define AVFMT_FLAG_DISCARD_CORRUPT  0x0100 // Discard frames marked corrupted

	/* Maximum number of bytes read from input in order to determine stream properties. */
	int64_t probesize;

	/* The number of frames used for determining the framerate in avformat_find_stream_info(). */
	int fps_probe_size;

	/* Maximum amount of memory in bytes to use for the index of each stream. */
	unsigned int max_index_size;

	/* Maximum number of packets to read while waiting for the first timestamp. */
	int max_ts_probe;

	/* Flags indicating events happening on the file, a combination of AVFMT_EVENT_FLAG_*. */
	int event_flags;
/* the demuxer read new metadata from the file and updated AVFormatContext.metadata accordingly */
#define AVFMT_EVENT_FLAG_METADATA_UPDATED 0x0001

	/* Correct single timestamp overflows */
	unsigned int correct_ts_overflow;

	/* format probing score. */
	int probe_score;

	/* Maximum number of bytes read from input in order to identify the "input format". */
	int format_probesize;

	/* A callback for opening new IO streams. */
	int (*io_open)(struct AVFormatContext* s, AVIOContext** pb, const char* url, int flags, AVDictionary** options);

	/* A callback for closing the streams opened with AVFormatContext.io_open(). */
	int (*io_close2)(struct AVFormatContext* s, AVIOContext* pb);
} AVFormatContext;

/* Return the LIBAVFORMAT_VERSION_INT constant. */
unsigned avformat_version(void);

/* Iterate over all registered demuxers. */
const AVInputFormat* av_demuxer_iterate(void** opaque);

/* Allocate an AVFormatContext. */
AVFormatContext* avformat_alloc_context(void);

/* Free an AVFormatContext and all its streams. */
void avformat_free_context(AVFormatContext* s);

/* Add a new stream to a media file. */
AVStream* avformat_new_stream(AVFormatContext* s, const struct AVCodec* c);

/* Find AVInputFormat based on the short name of the input format. */
const AVInputFormat* av_find_input_format(const char* short_name);

/* Guess the file format. */
const AVInputFormat* av_probe_input_format2(const AVProbeData* pd, int is_opened, int* score_max);

/* Guess the file format. */
const AVInputFormat* av_probe_input_format3(const AVProbeData* pd, int is_opened, int* score_ret);

/* Probe a bytestream to determine the input format. */
int av_probe_input_buffer2(AVIOContext* pb, const AVInputFormat** fmt,
	const char* url, void* logctx,
	unsigned int offset, unsigned int max_probe_size);

/* Open an input stream and read the header. The codecs are not opened. */
int avformat_open_input(AVFormatContext** ps, const char* url, const AVInputFormat* fmt, AVDictionary** options);

/* Read packets of a media file to get stream information. */
int avformat_find_stream_info(AVFormatContext* ic, AVDictionary** options);

/* Find the "best" stream in the file. */
int av_find_best_stream(AVFormatContext* ic,
	enum AVMediaType type,
	int wanted_stream_nb,
	int related_stream,
	const struct AVCodec** decoder_ret,
	int flags);

/* Return the next frame of a stream. */
int av_read_frame(AVFormatContext* s, AVPacket* pkt);

/* Close an opened input AVFormatContext. Free it and all its contents */
void avformat_close_input(AVFormatContext** s);

#define AVSEEK_FLAG_BACKWARD 1 // seek backward
#define AVSEEK_FLAG_ANY      4 // seek to any frame, even non-keyframes

/* Miscellaneous utility functions related to both muxing and demuxing */

int av_find_default_stream_index(AVFormatContext* s);

/* Get the index for a specific timestamp. */
int av_index_search_timestamp(AVStream* st, int64_t timestamp, int flags);

/* Add an index entry into a sorted list. Update the entry if the list already contains it. */
int av_add_index_entry(AVStream* st, int64_t pos, int64_t timestamp, int size, int distance, int flags);

#endif /* AVFORMAT_AVFORMAT_H */
