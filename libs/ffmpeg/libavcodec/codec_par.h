/*
 * Codec parameters public API
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVCODEC_CODEC_PAR_H
#define AVCODEC_CODEC_PAR_H

#include <stdint.h>

#include "libavutil/avutil.h"
#include "libavutil/channel_layout.h"
#include "libavutil/rational.h"
#include "libavutil/pixfmt.h"

#include "codec_id.h"
#include "defs.h"
#include "packet.h"

/* This struct describes the properties of an encoded stream. */
typedef struct AVCodecParameters
{
	/* General type of the encoded data. */
	enum AVMediaType codec_type;
	/* Specific type of the encoded data (the codec used). */
	enum AVCodecID codec_id;
	/* Additional information about the codec (corresponds to the AVI FOURCC). */
	uint32_t codec_tag;

	/* Extra binary data needed for initializing the decoder, codec-dependent. */
	uint8_t* extradata;
	/* Size of the extradata content in bytes. */
	int extradata_size;

	/**
	 * - video: the pixel format, the value corresponds to enum AVPixelFormat.
	 * - audio: the sample format, the value corresponds to enum AVSampleFormat.
	 */
	int format;

	/* The average bitrate of the encoded data (in bits per second). */
	int64_t bit_rate;

	/* The number of bits per sample in the codedwords. */
	int bits_per_coded_sample;

	/* Video only. The dimensions of the video frame in pixels. */
	int width;
	int height;

	/* Video only. The aspect ratio (width / height) which a single pixel should have when displayed. */
	AVRational sample_aspect_ratio;

	/* Video only. Number of frames per second, for streams with constant frame durations. */
	AVRational framerate;

	/* Video only. Additional colorspace characteristics. */
	enum AVColorRange color_range;

	/* Audio only. The channel layout and number of channels. */
	AVChannelLayout ch_layout;
	/* Audio only. The number of audio samples per second. */
	int sample_rate;
	/*Audio only. The number of bytes per coded audio frame, required by some formats. */
	int block_align;
	/* Audio only. Audio frame size, if known. Required by some formats to be static. */
	int frame_size;
} AVCodecParameters;

/* Allocate a new AVCodecParameters and set its fields to default values (unknown/invalid/0). */
AVCodecParameters* avcodec_parameters_alloc(void);

/* Free an AVCodecParameters instance and everything associated with it and write NULL to the supplied pointer. */
void avcodec_parameters_free(AVCodecParameters** par);

/* Copy the contents of src to dst. */
int avcodec_parameters_copy(AVCodecParameters* dst, const AVCodecParameters* src);

/* Same as av_get_audio_frame_duration(), except it works  with AVCodecParameters instead of an AVCodecContext. */
int av_get_audio_frame_duration2(AVCodecParameters* par, int frame_bytes);

#endif // AVCODEC_CODEC_PAR_H
