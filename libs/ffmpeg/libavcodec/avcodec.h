/*
 * copyright (c) 2001 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVCODEC_AVCODEC_H
#define AVCODEC_AVCODEC_H

/* Libavcodec external API header */

#include "libavutil/samplefmt.h"
#include "libavutil/avutil.h"
#include "libavutil/channel_layout.h"
#include "libavutil/dict.h"
#include "libavutil/frame.h"
#include "libavutil/pixfmt.h"

#include "codec.h"
#include "codec_id.h"
#include "defs.h"
#include "packet.h"
#include "version_major.h"
#include "version.h"

#include "codec_par.h"

struct AVCodecParameters;

/* The decoder will keep a reference to the frame and may reuse it later. */
#define AV_GET_BUFFER_FLAG_REF (1 << 0)

/* main external API structure. */
typedef struct AVCodecContext
{
	enum AVMediaType codec_type; /* see AVMEDIA_TYPE_xxx */
	const struct AVCodec* codec;
	enum AVCodecID codec_id; /* see AV_CODEC_ID_xxx */

	/* fourcc (LSB first, so "ABCD" -> ('D'<<24) + ('C'<<16) + ('B'<<8) + 'A'). */
	unsigned int codec_tag;

	void* priv_data;

	/* Private context used for internal data. */
	struct AVCodecInternal* internal;

	/* the average bitrate */
	int64_t bit_rate;

	/* some codecs need / can use extradata like Huffman tables. */
	uint8_t* extradata;
	int extradata_size;

	/**
	 * - decoding: For codecs that store a framerate value in the compressed
	 *             bitstream, the decoder may export it here. { 0, 1} when
	 *             unknown.
	 */
	AVRational framerate;

	/* picture width / height. */
	int width, height;

	/* sample aspect ratio (0 if unknown) */
	AVRational sample_aspect_ratio;

	/* Pixel format, see AV_PIX_FMT_xxx. */
	enum AVPixelFormat pix_fmt;

	/* MPEG vs JPEG YUV range. */
	enum AVColorRange color_range;

	/* samples per second */
	int sample_rate; 

	/* audio sample format */
	enum AVSampleFormat sample_fmt;

	/* Audio channel layout. */
	AVChannelLayout ch_layout;

	/* This callback is called at the beginning of each frame to get data buffer(s) for it. */
	int (*get_buffer2)(struct AVCodecContext* s, AVFrame* frame, int flags);

	/* Frame counter, set by libavcodec. */
	int64_t frame_num;
} AVCodecContext;

/* Return the LIBAVCODEC_VERSION_INT constant. */
unsigned avcodec_version(void);

/* Allocate an AVCodecContext and set its fields to default values. */
AVCodecContext* avcodec_alloc_context3(const AVCodec* codec);

/* Free the codec context and everything associated with it and write NULL to the provided pointer. */
void avcodec_free_context(AVCodecContext** avctx);

/* Fill the parameters struct based on the values from the supplied codec context. */
int avcodec_parameters_from_context(struct AVCodecParameters* par, const AVCodecContext* codec);

/* Fill the codec context based on the values from the supplied codec parameters. */
int avcodec_parameters_to_context(AVCodecContext* codec, const struct AVCodecParameters* par);

/* Initialize the AVCodecContext to use the given AVCodec. */
int avcodec_open2(AVCodecContext* avctx, const AVCodec* codec, AVDictionary** options);

/* The default callback for AVCodecContext.get_buffer2(). */
int avcodec_default_get_buffer2(AVCodecContext* s, AVFrame* frame, int flags);

/*
 * Modify width and height values so that they will result in a memory
 * buffer that is acceptable for the codec if you also ensure that all
 * line sizes are a multiple of the respective linesize_align[i].
 */
void avcodec_align_dimensions2(AVCodecContext* s, int* width, int* height, int linesize_align[AV_NUM_DATA_POINTERS]);

/* Supply raw packet data as input to a decoder. */
int avcodec_send_packet(AVCodecContext* avctx, const AVPacket* avpkt);

/* Return decoded output data from a decoder. */
int avcodec_receive_frame(AVCodecContext* avctx, AVFrame* frame);

/* Return audio frame duration. */
int av_get_audio_frame_duration(AVCodecContext* avctx, int frame_bytes);

/* Return a positive value if s is open, 0 otherwise. */
int avcodec_is_open(AVCodecContext* s);

#endif /* AVCODEC_AVCODEC_H */
