/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* APIs internal to the generic codec layer. */

#ifndef AVCODEC_AVCODEC_INTERNAL_H
#define AVCODEC_AVCODEC_INTERNAL_H

struct AVCodecContext;
struct AVFrame;

/* avcodec_receive_frame() implementation for decoders. */
int ff_decode_receive_frame(struct AVCodecContext *avctx, struct AVFrame *frame);

/* Perform decoder initialization and validation. */
int ff_decode_preinit(struct AVCodecContext *avctx);

struct AVCodecInternal *ff_decode_internal_alloc(void);

void ff_codec_close(struct AVCodecContext *avctx);

#endif // AVCODEC_AVCODEC_INTERNAL_H
