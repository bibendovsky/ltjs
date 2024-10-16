/*
 * generic decoding-related code
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdint.h>
#include <string.h>

#include "libavutil/avassert.h"
#include "libavutil/imgutils.h"

#include "bsf.h"
#include "codec_internal.h"
#include "decode.h"
#include "internal.h"
#include "packet_internal.h"

typedef struct DecodeContext
{
	AVCodecInternal avci;

	/* to prevent infinite loop on errors when draining */
	int nb_draining_errors;

	/**
	 * The caller has submitted a NULL packet on input.
	 */
	int draining_started;

	int64_t pts_correction_num_faulty_pts; /// Number of incorrect PTS values so far
	int64_t pts_correction_num_faulty_dts; /// Number of incorrect DTS values so far
	int64_t pts_correction_last_pts;       /// PTS of the last frame
	int64_t pts_correction_last_dts;       /// DTS of the last frame
} DecodeContext;

static DecodeContext* decode_ctx(AVCodecInternal* avci)
{
	return (DecodeContext*)avci;
}

static int extract_packet_props(AVCodecInternal* avci, const AVPacket* pkt)
{
	int ret = 0;

	av_packet_unref(avci->last_pkt_props);
	if (pkt)
	{
		ret = av_packet_copy_props(avci->last_pkt_props, pkt);
	}
	return ret;
}

static int decode_bsfs_init(AVCodecContext* avctx)
{
	AVCodecInternal* avci = avctx->internal;
	int ret;

	if (avci->bsf)
		return 0;

	ret = av_bsf_list_parse_str(NULL, &avci->bsf);
	if (ret < 0)
	{
		if (ret != AVERROR(ENOMEM))
			ret = AVERROR_BUG;
		goto fail;
	}

	/* We do not currently have an API for passing the input timebase into decoders,
	 * but no filters used here should actually need it.
	 * So we make up some plausible-looking number (the MPEG 90kHz timebase) */
	avci->bsf->time_base_in = (AVRational){1, 90000};
	ret = avcodec_parameters_from_context(avci->bsf->par_in, avctx);
	if (ret < 0)
		goto fail;

	ret = av_bsf_init(avci->bsf);
	if (ret < 0)
		goto fail;

	return 0;
fail:
	av_bsf_free(&avci->bsf);
	return ret;
}

static int decode_get_packet(AVCodecContext* avctx, AVPacket* pkt)
{
	AVCodecInternal* avci = avctx->internal;
	int ret;

	ret = av_bsf_receive_packet(avci->bsf, pkt);
	if (ret == AVERROR_EOF)
		avci->draining = 1;
	if (ret < 0)
		return ret;

	if (!(ffcodec(avctx->codec)->caps_internal & FF_CODEC_CAP_SETS_FRAME_PROPS))
	{
		ret = extract_packet_props(avctx->internal, pkt);
		if (ret < 0)
			goto finish;
	}

	return 0;
finish:
	av_packet_unref(pkt);
	return ret;
}

int ff_decode_get_packet(AVCodecContext* avctx, AVPacket* pkt)
{
	AVCodecInternal* avci = avctx->internal;
	DecodeContext* dc = decode_ctx(avci);

	if (avci->draining)
		return AVERROR_EOF;

	while (1)
	{
		int ret = decode_get_packet(avctx, pkt);
		if (ret == AVERROR(EAGAIN) &&
			(!AVPACKET_IS_EMPTY(avci->buffer_pkt) || dc->draining_started))
		{
			ret = av_bsf_send_packet(avci->bsf, avci->buffer_pkt);
			if (ret < 0)
			{
				av_packet_unref(avci->buffer_pkt);
				return ret;
			}

			continue;
		}

		return ret;
	}
}

/* Attempt to guess proper monotonic timestamps for decoded video frames which might have incorrect times. */
static int64_t guess_correct_pts(DecodeContext* dc, int64_t reordered_pts, int64_t dts)
{
	int64_t pts = AV_NOPTS_VALUE;

	if (dts != AV_NOPTS_VALUE)
	{
		dc->pts_correction_num_faulty_dts += dts <= dc->pts_correction_last_dts;
		dc->pts_correction_last_dts = dts;
	}
	else if (reordered_pts != AV_NOPTS_VALUE)
		dc->pts_correction_last_dts = reordered_pts;

	if (reordered_pts != AV_NOPTS_VALUE)
	{
		dc->pts_correction_num_faulty_pts += reordered_pts <= dc->pts_correction_last_pts;
		dc->pts_correction_last_pts = reordered_pts;
	}
	else if (dts != AV_NOPTS_VALUE)
		dc->pts_correction_last_pts = dts;

	if ((dc->pts_correction_num_faulty_pts <= dc->pts_correction_num_faulty_dts || dts == AV_NOPTS_VALUE)
		&& reordered_pts != AV_NOPTS_VALUE)
		pts = reordered_pts;
	else
		pts = dts;

	return pts;
}

static int discard_samples(AVCodecContext* avctx, AVFrame* frame, int64_t* discarded_samples)
{
	AVCodecInternal* avci = avctx->internal;
	uint32_t discard_padding = 0;

	if ((frame->flags & AV_FRAME_FLAG_DISCARD))
	{
		avci->skip_samples = FFMAX(0, avci->skip_samples - frame->nb_samples);
		*discarded_samples += frame->nb_samples;
		return AVERROR(EAGAIN);
	}

	if (avci->skip_samples > 0)
	{
		if (frame->nb_samples <= avci->skip_samples)
		{
			*discarded_samples += frame->nb_samples;
			avci->skip_samples -= frame->nb_samples;
			return AVERROR(EAGAIN);
		}
		else
		{
			av_samples_copy(frame->extended_data, frame->extended_data, 0, avci->skip_samples,
				frame->nb_samples - avci->skip_samples, avctx->ch_layout.nb_channels, frame->format);

			*discarded_samples += avci->skip_samples;
			frame->nb_samples -= avci->skip_samples;
			avci->skip_samples = 0;
		}
	}

	if (discard_padding > 0 && discard_padding <= (uint32_t)frame->nb_samples)
	{
		if (discard_padding == (uint32_t)frame->nb_samples)
		{
			*discarded_samples += frame->nb_samples;
			return AVERROR(EAGAIN);
		}
		else
		{
			frame->nb_samples -= discard_padding;
		}
	}

	return 0;
}

/* The core of the receive_frame_wrapper for the decoders implementing the simple API. */
static inline int decode_simple_internal(AVCodecContext* avctx, AVFrame* frame, int64_t* discarded_samples)
{
	AVCodecInternal* avci = avctx->internal;
	AVPacket* const pkt = avci->in_pkt;
	const FFCodec* const codec = ffcodec(avctx->codec);
	int got_frame, consumed;
	int ret;

	if (!pkt->data && !avci->draining)
	{
		av_packet_unref(pkt);
		ret = ff_decode_get_packet(avctx, pkt);
		if (ret < 0 && ret != AVERROR_EOF)
			return ret;
	}

	// Some codecs (at least wma lossless) will crash when feeding drain packets
	// after EOF was signaled.
	if (avci->draining_done)
		return AVERROR_EOF;

	if (!pkt->data)
		return AVERROR_EOF;

	got_frame = 0;

	{
		consumed = codec->cb.decode(avctx, frame, &got_frame, pkt);

		if (!(codec->caps_internal & FF_CODEC_CAP_SETS_PKT_DTS))
			frame->pkt_dts = pkt->dts;
	}

	if (avctx->codec->type == AVMEDIA_TYPE_VIDEO)
	{
		ret = (!got_frame || frame->flags & AV_FRAME_FLAG_DISCARD) ? AVERROR(EAGAIN) : 0;
	}
	else if (avctx->codec->type == AVMEDIA_TYPE_AUDIO)
	{
		ret = !got_frame ? AVERROR(EAGAIN) : discard_samples(avctx, frame, discarded_samples);
	}
	else
		av_assert0(0);

	if (ret == AVERROR(EAGAIN))
		av_frame_unref(frame);

	// FF_CODEC_CB_TYPE_DECODE decoders must not return AVERROR EAGAIN
	// code later will add AVERROR(EAGAIN) to a pointer
	av_assert0(consumed != AVERROR(EAGAIN));
	if (consumed < 0)
		ret = consumed;
	if (consumed >= 0 && avctx->codec->type == AVMEDIA_TYPE_VIDEO)
		consumed = pkt->size;

	if (!ret)
		av_assert0(frame->buf[0]);
	if (ret == AVERROR(EAGAIN))
		ret = 0;

	/* do not stop draining when got_frame != 0 or ret < 0 */
	if (avci->draining && !got_frame)
	{
		if (ret < 0)
		{
			/* prevent infinite loop if a decoder wrongly always return error on draining */
			/* reasonable nb_errors_max = maximum b frames + thread count */
			int nb_errors_max = 20 + 1;

			if (decode_ctx(avci)->nb_draining_errors++ >= nb_errors_max)
			{
				avci->draining_done = 1;
				ret = AVERROR_BUG;
			}
		}
		else
		{
			avci->draining_done = 1;
		}
	}

	if (consumed >= pkt->size || ret < 0)
	{
		av_packet_unref(pkt);
	}
	else
	{
		pkt->data += consumed;
		pkt->size -= consumed;
		pkt->pts = AV_NOPTS_VALUE;
		pkt->dts = AV_NOPTS_VALUE;
		if (!(codec->caps_internal & FF_CODEC_CAP_SETS_FRAME_PROPS))
		{
			avci->last_pkt_props->pts = AV_NOPTS_VALUE;
			avci->last_pkt_props->dts = AV_NOPTS_VALUE;
		}
	}

	return ret;
}

static int fill_frame_props(const AVCodecContext* avctx, AVFrame* frame)
{
	int ret;

	if (frame->color_range == AVCOL_RANGE_UNSPECIFIED)
		frame->color_range = avctx->color_range;

	if (avctx->codec_type == AVMEDIA_TYPE_VIDEO)
	{
		if (!frame->sample_aspect_ratio.num)  frame->sample_aspect_ratio = avctx->sample_aspect_ratio;
		if (frame->format == AV_PIX_FMT_NONE) frame->format = avctx->pix_fmt;
	}
	else if (avctx->codec->type == AVMEDIA_TYPE_AUDIO)
	{
		if (frame->format == AV_SAMPLE_FMT_NONE)
			frame->format = avctx->sample_fmt;
		if (!frame->ch_layout.nb_channels)
		{
			ret = av_channel_layout_copy(&frame->ch_layout, &avctx->ch_layout);
			if (ret < 0)
				return ret;
		}
		if (!frame->sample_rate)
			frame->sample_rate = avctx->sample_rate;
	}

	return 0;
}

static int decode_simple_receive_frame(AVCodecContext* avctx, AVFrame* frame)
{
	int ret;
	int64_t discarded_samples = 0;

	while (!frame->buf[0])
	{
		ret = decode_simple_internal(avctx, frame, &discarded_samples);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int decode_receive_frame_internal(AVCodecContext* avctx, AVFrame* frame)
{
	AVCodecInternal* avci = avctx->internal;
	DecodeContext* dc = decode_ctx(avci);
	const FFCodec* const codec = ffcodec(avctx->codec);
	int ret, ok;

	av_assert0(!frame->buf[0]);

	if (codec->cb_type == FF_CODEC_CB_TYPE_RECEIVE_FRAME)
	{
		ret = codec->cb.receive_frame(avctx, frame);
		if (!ret)
		{
			if (avctx->codec->type == AVMEDIA_TYPE_VIDEO)
				ret = (frame->flags & AV_FRAME_FLAG_DISCARD) ? AVERROR(EAGAIN) : 0;
			else if (avctx->codec->type == AVMEDIA_TYPE_AUDIO)
			{
				int64_t discarded_samples = 0;
				ret = discard_samples(avctx, frame, &discarded_samples);
			}
		}
	}
	else
		ret = decode_simple_receive_frame(avctx, frame);

	if (ret == AVERROR_EOF)
		avci->draining_done = 1;

	/* preserve ret */
	ok = 0;
	if (ok < 0)
	{
		av_frame_unref(frame);
		return ok;
	}

	if (!ret)
	{
		if (avctx->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			if (!frame->width)
				frame->width = avctx->width;
			if (!frame->height)
				frame->height = avctx->height;
		}
		else
			frame->flags |= AV_FRAME_FLAG_KEY;

		ret = fill_frame_props(avctx, frame);
		if (ret < 0)
		{
			av_frame_unref(frame);
			return ret;
		}

		frame->best_effort_timestamp = guess_correct_pts(dc, frame->pts, frame->pkt_dts);
	}

	return ret;
}

int avcodec_send_packet(AVCodecContext* avctx, const AVPacket* avpkt)
{
	AVCodecInternal* avci = avctx->internal;
	DecodeContext* dc = decode_ctx(avci);
	int ret;

	if (!avcodec_is_open(avctx) || !av_codec_is_decoder(avctx->codec))
		return AVERROR(EINVAL);

	if (dc->draining_started)
		return AVERROR_EOF;

	if (avpkt && !avpkt->size && avpkt->data)
		return AVERROR(EINVAL);

	if (avpkt && avpkt->data)
	{
		if (!AVPACKET_IS_EMPTY(avci->buffer_pkt))
			return AVERROR(EAGAIN);
		ret = av_packet_ref(avci->buffer_pkt, avpkt);
		if (ret < 0)
			return ret;
	}
	else
		dc->draining_started = 1;

	if (!avci->buffer_frame->buf[0] && !dc->draining_started)
	{
		ret = decode_receive_frame_internal(avctx, avci->buffer_frame);
		if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
			return ret;
	}

	return 0;
}

// make sure frames returned to the caller are valid
static int frame_validate(AVCodecContext* avctx, AVFrame* frame)
{
	if (!frame->buf[0] || frame->format < 0)
		goto fail;

	switch (avctx->codec_type)
	{
		case AVMEDIA_TYPE_VIDEO:
			if (frame->width <= 0 || frame->height <= 0)
				goto fail;
			break;
		case AVMEDIA_TYPE_AUDIO:
			if (!av_channel_layout_check(&frame->ch_layout) ||
				frame->sample_rate <= 0)
				goto fail;

			break;
		default: av_assert0(0);
	}

	return 0;
fail:
	return AVERROR_BUG;
}

int ff_decode_receive_frame(AVCodecContext* avctx, AVFrame* frame)
{
	AVCodecInternal* avci = avctx->internal;
	int ret;

	if (!avcodec_is_open(avctx) || !av_codec_is_decoder(avctx->codec))
		return AVERROR(EINVAL);

	if (avci->buffer_frame->buf[0])
	{
		av_frame_move_ref(frame, avci->buffer_frame);
	}
	else
	{
		ret = decode_receive_frame_internal(avctx, frame);
		if (ret < 0)
			return ret;
	}

	ret = frame_validate(avctx, frame);
	if (ret < 0)
		goto fail;

	avctx->frame_num++;

	return 0;
fail:
	av_frame_unref(frame);
	return ret;
}

int ff_decode_frame_props_from_pkt(const AVCodecContext* avctx, AVFrame* frame, const AVPacket* pkt)
{
	(void)avctx;

	frame->pts = pkt->pts;
	frame->duration = pkt->duration;

	if (pkt->flags & AV_PKT_FLAG_DISCARD)
	{
		frame->flags |= AV_FRAME_FLAG_DISCARD;
	}
	else
	{
		frame->flags = (frame->flags & ~AV_FRAME_FLAG_DISCARD);
	}

	return 0;
}

int ff_decode_frame_props(AVCodecContext* avctx, AVFrame* frame)
{
	int ret;

	if (!(ffcodec(avctx->codec)->caps_internal & FF_CODEC_CAP_SETS_FRAME_PROPS))
	{
		const AVPacket* pkt = avctx->internal->last_pkt_props;

		ret = ff_decode_frame_props_from_pkt(avctx, frame, pkt);
		if (ret < 0)
			return ret;
	}

	ret = fill_frame_props(avctx, frame);
	if (ret < 0)
		return ret;

	if (avctx->codec->type == AVMEDIA_TYPE_VIDEO)
	{
		if (frame->width && frame->height &&
			av_image_check_sar(frame->width, frame->height, frame->sample_aspect_ratio) < 0)
		{
			frame->sample_aspect_ratio = (AVRational){0, 1};
		}
	}
	return 0;
}

static void validate_avframe_allocation(AVCodecContext* avctx, AVFrame* frame)
{
	if (avctx->codec_type == AVMEDIA_TYPE_VIDEO)
	{
		int i;
		int num_planes = av_pix_fmt_count_planes(frame->format);
		for (i = 0; i < num_planes; i++)
		{
			av_assert0(frame->data[i]);
		}
		// For formats without data like hwaccel allow unused pointers to be non-NULL.
		for (i = num_planes; num_planes > 0 && (size_t)i < FF_ARRAY_ELEMS(frame->data); i++)
		{
			frame->data[i] = NULL;
		}
	}
}

int ff_get_buffer(AVCodecContext* avctx, AVFrame* frame, int flags)
{
	int override_dimensions = 1;
	int ret;

	av_assert0(av_codec_is_decoder(avctx->codec));

	if (avctx->codec_type == AVMEDIA_TYPE_VIDEO)
	{
		if ((unsigned)avctx->width > INT_MAX - STRIDE_ALIGN ||
			(ret = av_image_check_size2(FFALIGN(avctx->width, STRIDE_ALIGN), avctx->height, AV_PIX_FMT_NONE, 0, avctx)) < 0 ||
			avctx->pix_fmt < 0)
		{
			ret = AVERROR(EINVAL);
			goto fail;
		}

		if (frame->width <= 0 || frame->height <= 0)
		{
			frame->width = avctx->width;
			frame->height = avctx->height;
			override_dimensions = 0;
		}

		if (frame->data[0] || frame->data[1] || frame->data[2] || frame->data[3])
		{
			ret = AVERROR(EINVAL);
			goto fail;
		}
	}
	ret = ff_decode_frame_props(avctx, frame);
	if (ret < 0)
		goto fail;

	ret = avctx->get_buffer2(avctx, frame, flags);
	if (ret < 0)
		goto fail;

	validate_avframe_allocation(avctx, frame);

	if (avctx->codec_type == AVMEDIA_TYPE_VIDEO && !override_dimensions &&
		!(ffcodec(avctx->codec)->caps_internal & FF_CODEC_CAP_EXPORTS_CROPPING))
	{
		frame->width = avctx->width;
		frame->height = avctx->height;
	}

fail:
	if (ret < 0)
	{
		av_frame_unref(frame);
	}

	return ret;
}

static int reget_buffer_internal(AVCodecContext* avctx, AVFrame* frame, int flags)
{
	(void)flags;

	AVFrame* tmp;
	int ret;

	av_assert0(avctx->codec_type == AVMEDIA_TYPE_VIDEO);

	if (frame->data[0] && (frame->width != avctx->width || frame->height != avctx->height || frame->format != avctx->pix_fmt))
	{
		av_frame_unref(frame);
	}

	if (!frame->data[0])
		return ff_get_buffer(avctx, frame, AV_GET_BUFFER_FLAG_REF);

	if (av_frame_is_writable(frame))
		return ff_decode_frame_props(avctx, frame);

	tmp = av_frame_alloc();
	if (!tmp)
		return AVERROR(ENOMEM);

	av_frame_move_ref(tmp, frame);

	ret = ff_get_buffer(avctx, frame, AV_GET_BUFFER_FLAG_REF);
	if (ret < 0)
	{
		av_frame_free(&tmp);
		return ret;
	}

	av_frame_copy(frame, tmp);
	av_frame_free(&tmp);

	return 0;
}

int ff_reget_buffer(AVCodecContext* avctx, AVFrame* frame, int flags)
{
	int ret = reget_buffer_internal(avctx, frame, flags);
	return ret;
}

int ff_decode_preinit(AVCodecContext* avctx)
{
	AVCodecInternal* avci = avctx->internal;
	DecodeContext* dc = decode_ctx(avci);
	int ret = 0;

	dc->pts_correction_num_faulty_pts =
		dc->pts_correction_num_faulty_dts = 0;
	dc->pts_correction_last_pts =
		dc->pts_correction_last_dts = INT64_MIN;

	avci->in_pkt = av_packet_alloc();
	avci->last_pkt_props = av_packet_alloc();
	if (!avci->in_pkt || !avci->last_pkt_props)
		return AVERROR(ENOMEM);

	ret = decode_bsfs_init(avctx);
	if (ret < 0)
		return ret;

	return 0;
}

AVCodecInternal* ff_decode_internal_alloc(void)
{
	return av_mallocz(sizeof(DecodeContext));
}
