/*
 * generic decoding-related code
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVCODEC_DECODE_H
#define AVCODEC_DECODE_H

#include "libavutil/frame.h"

#include "avcodec.h"

/* Called by decoders to get the next packet for decoding. */
int ff_decode_get_packet(AVCodecContext* avctx, AVPacket* pkt);

/* Set various frame properties from the provided packet. */
int ff_decode_frame_props_from_pkt(const AVCodecContext* avctx, AVFrame* frame, const AVPacket* pkt);

/* Set various frame properties from the codec context / packet data. */
int ff_decode_frame_props(AVCodecContext* avctx, AVFrame* frame);

/* Check that the provided frame dimensions are valid and set them on the codec context. */
int ff_set_dimensions(AVCodecContext* s, int width, int height);

/* Get a buffer for a frame.  */
int ff_get_buffer(AVCodecContext* avctx, AVFrame* frame, int flags);

/* Identical in function to ff_get_buffer(), except it reuses the existing buffer if available. */
int ff_reget_buffer(AVCodecContext* avctx, AVFrame* frame, int flags);

#endif /* AVCODEC_DECODE_H */
