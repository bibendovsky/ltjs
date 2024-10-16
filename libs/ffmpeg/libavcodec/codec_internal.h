/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVCODEC_CODEC_INTERNAL_H
#define AVCODEC_CODEC_INTERNAL_H

#include <stdint.h>

#include "libavutil/frame.h"
#include "codec.h"

/* The codec is not known to be init-threadsafe. */
#define FF_CODEC_CAP_NOT_INIT_THREADSAFE    (1 << 0)
/*
 * The codec allows calling the close function for deallocation even if
 * the init function returned a failure.
 */
#define FF_CODEC_CAP_INIT_CLEANUP           (1 << 1)
/* Decoders want to set AVFrame.pkt_dts manually. */
#define FF_CODEC_CAP_SETS_PKT_DTS           (1 << 2)
/*
 * The decoder extracts and fills its parameters even if the frame is
 * skipped due to the skip_frame setting.
 */
#define FF_CODEC_CAP_SKIP_FRAME_FILL_PARAM  (1 << 3)
/* The decoder sets the cropping fields in the output frames manually. */
#define FF_CODEC_CAP_EXPORTS_CROPPING       (1 << 4)
/* Codec handles output frame properties internally. */
#define FF_CODEC_CAP_SETS_FRAME_PROPS       (1 << 8)

typedef struct FFCodecDefault
{
	const char* key;
	const char* value;
} FFCodecDefault;

struct AVCodecContext;
struct AVPacket;

enum FFCodecType
{
	/* The codec is a decoder using the decode callback. */
	FF_CODEC_CB_TYPE_DECODE,
	/* The codec is a decoder using the receive_frame callback. */
	FF_CODEC_CB_TYPE_RECEIVE_FRAME,
};

typedef struct FFCodec
{
	/* The public AVCodec. See codec.h for it. */
	AVCodec p;

	/* Internal codec capabilities FF_CODEC_CAP_*. */
	unsigned caps_internal : 29;

	/* This field determines the type of the codec. */
	unsigned cb_type : 3;

	int priv_data_size;

	/* Private codec-specific defaults. */
	const FFCodecDefault* defaults;

	/* Initialize codec static data, called from av_codec_iterate(). */
	void (*init_static_data)(struct FFCodec* codec);

	int (*init)(struct AVCodecContext*);

	union
	{
		/* Decode to an AVFrame. */
		int (*decode)(struct AVCodecContext* avctx, struct AVFrame* frame, int* got_frame_ptr, struct AVPacket* avpkt);
		/* Decode API with decoupled packet/frame dataflow. */
		int (*receive_frame)(struct AVCodecContext* avctx, struct AVFrame* frame);
	} cb;

	int (*close)(struct AVCodecContext*);

	/* Flush buffers. */
	void (*flush)(struct AVCodecContext*);

	/* List of supported codec_tags, terminated by FF_CODEC_TAGS_END. */
	const uint32_t* codec_tags;
} FFCodec;

#define CODEC_LONG_NAME(str) .p.long_name = str

#define FF_CODEC_DECODE_CB(func)                          \
    .cb_type           = FF_CODEC_CB_TYPE_DECODE,         \
    .cb.decode         = (func)
#define FF_CODEC_RECEIVE_FRAME_CB(func)                   \
    .cb_type           = FF_CODEC_CB_TYPE_RECEIVE_FRAME,  \
    .cb.receive_frame  = (func)

static inline const FFCodec* ffcodec(const AVCodec* codec)
{
	return (const FFCodec*)codec;
}

#endif /* AVCODEC_CODEC_INTERNAL_H */
