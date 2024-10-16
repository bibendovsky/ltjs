/*
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "demux.h"
#include "internal.h"

#include "libavcodec/avcodec.h"

/* Options definition for AVFormatContext. */

static int io_open_default(AVFormatContext* s, AVIOContext** pb, const char* url, int flags, AVDictionary** options)
{
	(void)s;
	(void)pb;
	(void)url;
	(void)flags;
	(void)options;

	return AVERROR_PROTOCOL_NOT_FOUND;
}

static int io_close2_default(AVFormatContext* s, AVIOContext* pb)
{
	(void)s;

	return avio_close(pb);
}

static void ltjs_av_format_context_set_defaults(AVFormatContext* ctx)
{
	ctx->probesize = 5000000;
	ctx->format_probesize = PROBE_BUF_MAX;
	ctx->max_index_size = 1 << 20;
	ctx->fps_probe_size = -1;
	ctx->correct_ts_overflow = 1;
	ctx->max_ts_probe = 50;
}

AVFormatContext* avformat_alloc_context(void)
{
	FFFormatContext* const si = av_mallocz(sizeof(*si));
	AVFormatContext* s;

	if (!si)
		return NULL;

	s = &si->pub;
	s->io_open = io_open_default;
	s->io_close2 = io_close2_default;

	ltjs_av_format_context_set_defaults(s);

	si->pkt = av_packet_alloc();
	if (!si->pkt)
	{
		avformat_free_context(s);
		return NULL;
	}

	return s;
}

AVStream* avformat_new_stream(AVFormatContext* s, const AVCodec* c)
{
	(void)c;

	FFStream* sti;
	AVStream* st;
	AVStream** streams;

	streams = av_realloc_array(s->streams, s->nb_streams + 1, sizeof(*streams));
	if (!streams)
		return NULL;
	s->streams = streams;

	sti = av_mallocz(sizeof(*sti));
	if (!sti)
		return NULL;
	st = &sti->pub;

	st->codecpar = avcodec_parameters_alloc();
	if (!st->codecpar)
		goto fail;

	sti->fmtctx = s;

	if (s->iformat)
	{
		sti->avctx = avcodec_alloc_context3(NULL);
		if (!sti->avctx)
			goto fail;

		sti->info = av_mallocz(sizeof(*sti->info));
		if (!sti->info)
			goto fail;

		sti->info->fps_first_dts = AV_NOPTS_VALUE;
		sti->info->fps_last_dts = AV_NOPTS_VALUE;

		/* default pts setting is MPEG-like */
		avpriv_set_pts_info(st, 33, 1, 90000);
		/* we set the current DTS to 0 so that formats without any timestamps
		 * but durations get some timestamps, formats with some unknown
		 * timestamps have their first few packets buffered and the
		 * timestamps corrected before they are returned to the user */
		sti->cur_dts = RELATIVE_TS_BASE;
	}
	else
	{
		sti->cur_dts = AV_NOPTS_VALUE;
	}

	st->index = s->nb_streams;
	st->start_time = AV_NOPTS_VALUE;
	st->duration = AV_NOPTS_VALUE;
	sti->first_dts = AV_NOPTS_VALUE;
	sti->pts_wrap_reference = AV_NOPTS_VALUE;
	sti->pts_wrap_behavior = AV_PTS_WRAP_IGNORE;

	sti->last_IP_pts = AV_NOPTS_VALUE;
	sti->last_dts_for_order_check = AV_NOPTS_VALUE;
	for (int i = 0; i < MAX_REORDER_DELAY + 1; i++)
		sti->pts_buffer[i] = AV_NOPTS_VALUE;

	st->sample_aspect_ratio = (AVRational){0, 1};
	sti->need_context_update = 1;

	s->streams[s->nb_streams++] = st;
	return st;
fail:
	ff_free_stream(&st);
	return NULL;
}
