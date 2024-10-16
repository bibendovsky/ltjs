/*
 * AVPacket functions for libavcodec
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <string.h>

#include "libavutil/avassert.h"
#include "libavutil/avutil.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"
#include "libavutil/mem.h"
#include "libavutil/rational.h"

#include "defs.h"
#include "packet.h"
#include "packet_internal.h"

static void get_packet_defaults(AVPacket* pkt)
{
	memset(pkt, 0, sizeof(*pkt));

	pkt->pts = AV_NOPTS_VALUE;
	pkt->dts = AV_NOPTS_VALUE;
	pkt->pos = -1;
	pkt->time_base = av_make_q(0, 1);
}

AVPacket* av_packet_alloc(void)
{
	AVPacket* pkt = av_malloc(sizeof(AVPacket));
	if (!pkt)
		return pkt;

	get_packet_defaults(pkt);

	return pkt;
}

void av_packet_free(AVPacket** pkt)
{
	if (!pkt || !*pkt)
		return;

	av_packet_unref(*pkt);
	av_freep(pkt);
}

static int packet_alloc(AVBufferRef** buf, int size)
{
	int ret;
	if (size < 0 || size >= INT_MAX - AV_INPUT_BUFFER_PADDING_SIZE)
		return AVERROR(EINVAL);

	ret = av_buffer_realloc(buf, size + AV_INPUT_BUFFER_PADDING_SIZE);
	if (ret < 0)
		return ret;

	memset((*buf)->data + size, 0, AV_INPUT_BUFFER_PADDING_SIZE);

	return 0;
}

void av_shrink_packet(AVPacket* pkt, int size)
{
	if (pkt->size <= size)
		return;
	pkt->size = size;
	memset(pkt->data + size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
}

int av_grow_packet(AVPacket* pkt, int grow_by)
{
	int new_size;
	av_assert0((unsigned)pkt->size <= INT_MAX - AV_INPUT_BUFFER_PADDING_SIZE);
	if (grow_by > INT_MAX - (pkt->size + AV_INPUT_BUFFER_PADDING_SIZE))
		return AVERROR(ENOMEM);

	new_size = pkt->size + grow_by + AV_INPUT_BUFFER_PADDING_SIZE;
	if (pkt->buf)
	{
		size_t data_offset;
		uint8_t* old_data = pkt->data;
		if (pkt->data == NULL)
		{
			data_offset = 0;
			pkt->data = pkt->buf->data;
		}
		else
		{
			data_offset = pkt->data - pkt->buf->data;
			if (data_offset > (size_t)(INT_MAX - new_size))
				return AVERROR(ENOMEM);
		}

		if (new_size + data_offset > pkt->buf->size || !av_buffer_is_writable(pkt->buf))
		{
			int ret;

			// allocate slightly more than requested to avoid excessive reallocations
			if (new_size + data_offset < (size_t)(INT_MAX - new_size / 16))
				new_size += new_size / 16;

			ret = av_buffer_realloc(&pkt->buf, new_size + data_offset);
			if (ret < 0)
			{
				pkt->data = old_data;
				return ret;
			}
			pkt->data = pkt->buf->data + data_offset;
		}
	}
	else
	{
		pkt->buf = av_buffer_alloc(new_size);
		if (!pkt->buf)
			return AVERROR(ENOMEM);
		if (pkt->size > 0)
			memcpy(pkt->buf->data, pkt->data, pkt->size);
		pkt->data = pkt->buf->data;
	}
	pkt->size += grow_by;
	memset(pkt->data + pkt->size, 0, AV_INPUT_BUFFER_PADDING_SIZE);

	return 0;
}

int av_packet_copy_props(AVPacket* dst, const AVPacket* src)
{
	dst->pts = src->pts;
	dst->dts = src->dts;
	dst->pos = src->pos;
	dst->duration = src->duration;
	dst->flags = src->flags;
	dst->stream_index = src->stream_index;
	dst->time_base = src->time_base;

	return 0;
}

void av_packet_unref(AVPacket* pkt)
{
	av_buffer_unref(&pkt->buf);
	get_packet_defaults(pkt);
}

int av_packet_ref(AVPacket* dst, const AVPacket* src)
{
	int ret;

	dst->buf = NULL;

	ret = av_packet_copy_props(dst, src);
	if (ret < 0)
		goto fail;

	if (!src->buf)
	{
		ret = packet_alloc(&dst->buf, src->size);
		if (ret < 0)
			goto fail;
		av_assert1(!src->size || src->data);
		if (src->size)
			memcpy(dst->buf->data, src->data, src->size);

		dst->data = dst->buf->data;
	}
	else
	{
		dst->buf = av_buffer_ref(src->buf);
		if (!dst->buf)
		{
			ret = AVERROR(ENOMEM);
			goto fail;
		}
		dst->data = src->data;
	}

	dst->size = src->size;

	return 0;
fail:
	av_packet_unref(dst);
	return ret;
}

void av_packet_move_ref(AVPacket* dst, AVPacket* src)
{
	*dst = *src;
	get_packet_defaults(src);
}

int av_packet_make_refcounted(AVPacket* pkt)
{
	int ret;

	if (pkt->buf)
		return 0;

	ret = packet_alloc(&pkt->buf, pkt->size);
	if (ret < 0)
		return ret;
	av_assert1(!pkt->size || pkt->data);
	if (pkt->size)
		memcpy(pkt->buf->data, pkt->data, pkt->size);

	pkt->data = pkt->buf->data;

	return 0;
}

int avpriv_packet_list_put(PacketList* packet_buffer,
	AVPacket* pkt,
	int (*copy)(AVPacket* dst, const AVPacket* src),
	int flags)
{
	(void)flags;

	PacketListEntry* pktl = av_malloc(sizeof(*pktl));
	int ret;

	if (!pktl)
		return AVERROR(ENOMEM);

	if (copy)
	{
		get_packet_defaults(&pktl->pkt);
		ret = copy(&pktl->pkt, pkt);
		if (ret < 0)
		{
			av_free(pktl);
			return ret;
		}
	}
	else
	{
		ret = av_packet_make_refcounted(pkt);
		if (ret < 0)
		{
			av_free(pktl);
			return ret;
		}
		av_packet_move_ref(&pktl->pkt, pkt);
	}

	pktl->next = NULL;

	if (packet_buffer->head)
		packet_buffer->tail->next = pktl;
	else
		packet_buffer->head = pktl;

	/* Add the packet in the buffered packet list. */
	packet_buffer->tail = pktl;
	return 0;
}

int avpriv_packet_list_get(PacketList* pkt_buffer,
	AVPacket* pkt)
{
	PacketListEntry* pktl = pkt_buffer->head;
	if (!pktl)
		return AVERROR(EAGAIN);
	*pkt = pktl->pkt;
	pkt_buffer->head = pktl->next;
	if (!pkt_buffer->head)
		pkt_buffer->tail = NULL;
	av_freep(&pktl);
	return 0;
}

void avpriv_packet_list_free(PacketList* pkt_buf)
{
	PacketListEntry* tmp = pkt_buf->head;

	while (tmp)
	{
		PacketListEntry* pktl = tmp;
		tmp = pktl->next;
		av_packet_unref(&pktl->pkt);
		av_freep(&pktl);
	}
	pkt_buf->head = pkt_buf->tail = NULL;
}
