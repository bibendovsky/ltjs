/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "channel_layout.h"
#include "avassert.h"
#include "buffer.h"
#include "common.h"
#include "dict.h"
#include "frame.h"
#include "imgutils.h"
#include "mem.h"
#include "samplefmt.h"

static void get_frame_defaults(AVFrame* frame)
{
	memset(frame, 0, sizeof(*frame));

	frame->pts =
		frame->pkt_dts = AV_NOPTS_VALUE;
	frame->best_effort_timestamp = AV_NOPTS_VALUE;
	frame->duration = 0;
	frame->time_base = (AVRational){0, 1};
	frame->sample_aspect_ratio = (AVRational){0, 1};
	frame->format = -1; /* unknown */
	frame->extended_data = frame->data;
	frame->color_range = AVCOL_RANGE_UNSPECIFIED;
	frame->flags = 0;
}

AVFrame* av_frame_alloc(void)
{
	AVFrame* frame = av_malloc(sizeof(*frame));

	if (!frame)
		return NULL;

	get_frame_defaults(frame);

	return frame;
}

void av_frame_free(AVFrame** frame)
{
	if (!frame || !*frame)
		return;

	av_frame_unref(*frame);
	av_freep(frame);
}

static int get_video_buffer(AVFrame* frame, int align)
{
	const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(frame->format);
	int ret, padded_height, total_size;
	int plane_padding = FFMAX(16 + 16/*STRIDE_ALIGN*/, align);
	ptrdiff_t linesizes[4];
	size_t sizes[4];

	if (!desc)
		return AVERROR(EINVAL);

	if ((ret = av_image_check_size(frame->width, frame->height, 0, NULL)) < 0)
		return ret;

	if (!frame->linesize[0])
	{
		if (align <= 0)
			align = 32; /* STRIDE_ALIGN. Should be av_cpu_max_align() */

		for (int i = 1; i <= align; i += i)
		{
			ret = av_image_fill_linesizes(frame->linesize, frame->format, FFALIGN(frame->width, i));
			if (ret < 0)
				return ret;
			if (!(frame->linesize[0] & (align - 1)))
				break;
		}

		for (int i = 0; i < 4 && frame->linesize[i]; i++)
			frame->linesize[i] = FFALIGN(frame->linesize[i], align);
	}

	for (int i = 0; i < 4; i++)
		linesizes[i] = frame->linesize[i];

	padded_height = FFALIGN(frame->height, 32);
	if ((ret = av_image_fill_plane_sizes(sizes, frame->format, padded_height, linesizes)) < 0)
		return ret;

	total_size = 4 * plane_padding;
	for (int i = 0; i < 4; i++)
	{
		if (sizes[i] > (size_t)(INT_MAX - total_size))
			return AVERROR(EINVAL);
		total_size += (int)sizes[i];
	}

	frame->buf[0] = av_buffer_alloc(total_size);
	if (!frame->buf[0])
	{
		ret = AVERROR(ENOMEM);
		goto fail;
	}

	if ((ret = av_image_fill_pointers(frame->data, frame->format, padded_height, frame->buf[0]->data, frame->linesize)) < 0)
		goto fail;

	for (int i = 1; i < 4; i++)
	{
		if (frame->data[i])
			frame->data[i] += i * plane_padding;
	}

	frame->extended_data = frame->data;

	return 0;
fail:
	av_frame_unref(frame);
	return ret;
}

static int get_audio_buffer(AVFrame* frame, int align)
{
	int planar = av_sample_fmt_is_planar(frame->format);
	int channels, planes;
	int ret;

	channels = frame->ch_layout.nb_channels;
	planes = planar ? channels : 1;
	if (!frame->linesize[0])
	{
		ret = av_samples_get_buffer_size(&frame->linesize[0], channels, frame->nb_samples, frame->format, align);
		if (ret < 0)
			return ret;
	}

	frame->extended_data = frame->data;

	for (int i = 0; i < FFMIN(planes, AV_NUM_DATA_POINTERS); i++)
	{
		frame->buf[i] = av_buffer_alloc(frame->linesize[0]);
		if (!frame->buf[i])
		{
			av_frame_unref(frame);
			return AVERROR(ENOMEM);
		}
		frame->extended_data[i] = frame->data[i] = frame->buf[i]->data;
	}
	return 0;

}

int av_frame_get_buffer(AVFrame* frame, int align)
{
	if (frame->format < 0)
		return AVERROR(EINVAL);

	if (frame->width > 0 && frame->height > 0)
		return get_video_buffer(frame, align);
	else if (frame->nb_samples > 0 && (av_channel_layout_check(&frame->ch_layout)))
		return get_audio_buffer(frame, align);

	return AVERROR(EINVAL);
}

static int frame_copy_props(AVFrame* dst, const AVFrame* src, int force_copy)
{
	(void)force_copy;

	dst->sample_aspect_ratio = src->sample_aspect_ratio;
	dst->pts = src->pts;
	dst->duration = src->duration;
	dst->sample_rate = src->sample_rate;
	dst->pkt_dts = src->pkt_dts;
	dst->time_base = src->time_base;
	dst->best_effort_timestamp = src->best_effort_timestamp;
	dst->flags = src->flags;
	dst->color_range = src->color_range;

	return 0;
}

int av_frame_ref(AVFrame* dst, const AVFrame* src)
{
	int ret = 0;

	av_assert1(dst->width == 0 && dst->height == 0);
	av_assert1(dst->ch_layout.nb_channels == 0 && dst->ch_layout.order == AV_CHANNEL_ORDER_UNSPEC);

	dst->format = src->format;
	dst->width = src->width;
	dst->height = src->height;
	dst->nb_samples = src->nb_samples;

	ret = frame_copy_props(dst, src, 0);
	if (ret < 0)
		goto fail;

	ret = av_channel_layout_copy(&dst->ch_layout, &src->ch_layout);
	if (ret < 0)
		goto fail;

	/* duplicate the frame data if it's not refcounted */
	if (!src->buf[0])
	{
		ret = av_frame_get_buffer(dst, 0);
		if (ret < 0)
			goto fail;

		ret = av_frame_copy(dst, src);
		if (ret < 0)
			goto fail;

		return 0;
	}

	/* ref the buffers */
	for (size_t i = 0; i < FF_ARRAY_ELEMS(src->buf); i++)
	{
		if (!src->buf[i])
			continue;
		dst->buf[i] = av_buffer_ref(src->buf[i]);
		if (!dst->buf[i])
		{
			ret = AVERROR(ENOMEM);
			goto fail;
		}
	}

	/* duplicate extended data */
	if (src->extended_data != src->data)
	{
		int ch = dst->ch_layout.nb_channels;

		if (!ch)
		{
			ret = AVERROR(EINVAL);
			goto fail;
		}

		dst->extended_data = av_malloc_array(ch, sizeof(*dst->extended_data));
		if (!dst->extended_data)
		{
			ret = AVERROR(ENOMEM);
			goto fail;
		}
		memcpy(dst->extended_data, src->extended_data, sizeof(*src->extended_data) * ch);
	}
	else
		dst->extended_data = dst->data;

	memcpy(dst->data, src->data, sizeof(src->data));
	memcpy(dst->linesize, src->linesize, sizeof(src->linesize));

	return 0;

fail:
	av_frame_unref(dst);
	return ret;
}

int av_frame_replace(AVFrame* dst, const AVFrame* src)
{
	int ret = 0;

	if (dst == src)
		return AVERROR(EINVAL);

	if (!src->buf[0])
	{
		av_frame_unref(dst);

		/* duplicate the frame data if it's not refcounted */
		if (src->data[0] || src->data[1] || src->data[2] || src->data[3])
			return av_frame_ref(dst, src);

		ret = frame_copy_props(dst, src, 0);
		if (ret < 0)
			goto fail;
	}

	dst->format = src->format;
	dst->width = src->width;
	dst->height = src->height;
	dst->nb_samples = src->nb_samples;

	ret = av_channel_layout_copy(&dst->ch_layout, &src->ch_layout);
	if (ret < 0)
		goto fail;

	ret = frame_copy_props(dst, src, 0);
	if (ret < 0)
		goto fail;

	/* replace the buffers */
	for (size_t i = 0; i < FF_ARRAY_ELEMS(src->buf); i++)
	{
		ret = av_buffer_replace(&dst->buf[i], src->buf[i]);
		if (ret < 0)
			goto fail;
	}

	if (dst->extended_data != dst->data)
		av_freep(&dst->extended_data);

	if (src->extended_data != src->data)
	{
		int ch = dst->ch_layout.nb_channels;

		if (!ch)
		{
			ret = AVERROR(EINVAL);
			goto fail;
		}

		if ((size_t)ch > SIZE_MAX / sizeof(*dst->extended_data))
			goto fail;

		dst->extended_data = av_memdup(src->extended_data, sizeof(*dst->extended_data) * ch);
		if (!dst->extended_data)
		{
			ret = AVERROR(ENOMEM);
			goto fail;
		}
	}
	else
		dst->extended_data = dst->data;

	memcpy(dst->data, src->data, sizeof(src->data));
	memcpy(dst->linesize, src->linesize, sizeof(src->linesize));

	return 0;

fail:
	av_frame_unref(dst);
	return ret;
}

void av_frame_unref(AVFrame* frame)
{
	if (!frame)
		return;

	for (size_t i = 0; i < FF_ARRAY_ELEMS(frame->buf); i++)
		av_buffer_unref(&frame->buf[i]);

	if (frame->extended_data != frame->data)
		av_freep(&frame->extended_data);

	av_channel_layout_uninit(&frame->ch_layout);

	get_frame_defaults(frame);
}

void av_frame_move_ref(AVFrame* dst, AVFrame* src)
{
	av_assert1(dst->width == 0 && dst->height == 0);
	av_assert1(dst->ch_layout.nb_channels == 0 && dst->ch_layout.order == AV_CHANNEL_ORDER_UNSPEC);

	*dst = *src;
	if (src->extended_data == src->data)
		dst->extended_data = dst->data;
	get_frame_defaults(src);
}

int av_frame_is_writable(AVFrame* frame)
{
	int ret = 1;

	/* assume non-refcounted frames are not writable */
	if (!frame->buf[0])
		return 0;

	for (size_t i = 0; i < FF_ARRAY_ELEMS(frame->buf); i++)
		if (frame->buf[i])
			ret &= !!av_buffer_is_writable(frame->buf[i]);

	return ret;
}

static int frame_copy_video(AVFrame* dst, const AVFrame* src)
{
	int planes;

	if (dst->width < src->width || dst->height < src->height)
		return AVERROR(EINVAL);

	planes = av_pix_fmt_count_planes(dst->format);
	for (int i = 0; i < planes; i++)
		if (!dst->data[i] || !src->data[i])
			return AVERROR(EINVAL);

	av_image_copy2(dst->data, dst->linesize, src->data, src->linesize, dst->format, src->width, src->height);

	return 0;
}

static int frame_copy_audio(AVFrame* dst, const AVFrame* src)
{
	int planar = av_sample_fmt_is_planar(dst->format);
	int channels = dst->ch_layout.nb_channels;
	int planes = planar ? channels : 1;

	if (dst->nb_samples != src->nb_samples || av_channel_layout_compare(&dst->ch_layout, &src->ch_layout))
		return AVERROR(EINVAL);

	for (int i = 0; i < planes; i++)
		if (!dst->extended_data[i] || !src->extended_data[i])
			return AVERROR(EINVAL);

	av_samples_copy(dst->extended_data, src->extended_data, 0, 0, dst->nb_samples, channels, dst->format);

	return 0;
}

int av_frame_copy(AVFrame* dst, const AVFrame* src)
{
	if (dst->format != src->format || dst->format < 0)
		return AVERROR(EINVAL);

	if (dst->width > 0 && dst->height > 0)
		return frame_copy_video(dst, src);
	else if (dst->nb_samples > 0 && (av_channel_layout_check(&dst->ch_layout)))
		return frame_copy_audio(dst, src);

	return AVERROR(EINVAL);
}
