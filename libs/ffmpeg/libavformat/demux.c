/*
 * Core demuxing component
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdint.h>

#include "libavutil/avassert.h"
#include "libavutil/mathematics.h"

#include "libavcodec/avcodec.h"
#include "libavcodec/bsf.h"
#include "libavcodec/internal.h"

#include "demux.h"
#include "internal.h"

static int64_t wrap_timestamp(const AVStream* st, int64_t timestamp)
{
	const FFStream* const sti = cffstream(st);
	if (sti->pts_wrap_behavior != AV_PTS_WRAP_IGNORE &&
		st->pts_wrap_bits < 64 &&
		sti->pts_wrap_reference != AV_NOPTS_VALUE &&
		timestamp != AV_NOPTS_VALUE)
	{
		if (sti->pts_wrap_behavior == AV_PTS_WRAP_ADD_OFFSET && timestamp < sti->pts_wrap_reference)
			return timestamp + (1ULL << st->pts_wrap_bits);
		else if (sti->pts_wrap_behavior == AV_PTS_WRAP_SUB_OFFSET && timestamp >= sti->pts_wrap_reference)
			return timestamp - (1ULL << st->pts_wrap_bits);
	}
	return timestamp;
}

int64_t ff_wrap_timestamp(const AVStream* st, int64_t timestamp)
{
	return wrap_timestamp(st, timestamp);
}

static const AVCodec* find_probe_decoder(AVFormatContext* s, const AVStream* st, enum AVCodecID codec_id)
{
	const AVCodec* codec;

	codec = ff_find_decoder(s, st, codec_id);
	if (!codec)
		return NULL;

	return codec;
}

static int set_codec_from_probe_data(AVFormatContext* s, AVStream* st, AVProbeData* pd)
{
	(void)s;
	(void)st;

	int score;
	(void)av_probe_input_format3(pd, 1, &score);
	return 0;
}

static int init_input(AVFormatContext* s, const char* filename, AVDictionary** options)
{
	(void)options;

	int ret;
	AVProbeData pd = {NULL, 0};
	int score = AVPROBE_SCORE_RETRY;

	if (s->pb)
	{
		s->flags |= AVFMT_FLAG_CUSTOM_IO;
		if (!s->iformat)
			return av_probe_input_buffer2(s->pb, &s->iformat, filename, s, 0, s->format_probesize);
		return 0;
	}

	if ((s->iformat && s->iformat->flags & AVFMT_NOFILE) ||
		(!s->iformat && (s->iformat = av_probe_input_format2(&pd, 0, &score))))
		return score;

	if ((ret = s->io_open(s, &s->pb, filename, AVIO_FLAG_READ, NULL)) < 0)
		return ret;

	if (s->iformat)
		return 0;
	return av_probe_input_buffer2(s->pb, &s->iformat, filename, s, 0, s->format_probesize);
}

static int update_stream_avctx(AVFormatContext* s)
{
	int ret;
	for (unsigned i = 0; i < s->nb_streams; i++)
	{
		AVStream* const st = s->streams[i];
		FFStream* const sti = ffstream(st);

		if (!sti->need_context_update)
			continue;

		/* update internal codec context, for the parser */
		ret = avcodec_parameters_to_context(sti->avctx, st->codecpar);
		if (ret < 0)
			return ret;

		sti->need_context_update = 0;
	}
	return 0;
}

int avformat_open_input(AVFormatContext** ps, const char* filename, const AVInputFormat* fmt, AVDictionary** options)
{
	(void)options;

	AVFormatContext* s = *ps;
	FFFormatContext* si;
	int ret = 0;

	if (!s && !(s = avformat_alloc_context()))
		return AVERROR(ENOMEM);
	si = ffformatcontext(s);
	if (fmt)
		s->iformat = fmt;

	if (s->pb) // must be before any goto fail
		s->flags |= AVFMT_FLAG_CUSTOM_IO;

	if ((ret = init_input(s, filename, NULL)) < 0)
		goto fail;
	s->probe_score = ret;
	s->duration = s->start_time = AV_NOPTS_VALUE;

	/* Allocate private data. */
	if (ffifmt(s->iformat)->priv_data_size > 0)
	{
		if (!(s->priv_data = av_mallocz(ffifmt(s->iformat)->priv_data_size)))
		{
			ret = AVERROR(ENOMEM);
			goto fail;
		}
	}

	if (ffifmt(s->iformat)->read_header)
		if ((ret = ffifmt(s->iformat)->read_header(s)) < 0)
		{
			goto fail;
		}

	if (s->pb && !si->data_offset)
		si->data_offset = avio_tell(s->pb);

	si->raw_packet_buffer_size = 0;

	update_stream_avctx(s);

	*ps = s;
	return 0;

fail:
	if (s->pb && !(s->flags & AVFMT_FLAG_CUSTOM_IO))
		avio_closep(&s->pb);
	avformat_free_context(s);
	*ps = NULL;
	return ret;
}

void avformat_close_input(AVFormatContext** ps)
{
	AVFormatContext* s;
	AVIOContext* pb;

	if (!ps || !*ps)
		return;

	s = *ps;
	pb = s->pb;

	if (s->flags & AVFMT_FLAG_CUSTOM_IO)
		pb = NULL;

	avformat_free_context(s);

	*ps = NULL;

	avio_close(pb);
}

static int probe_codec(AVFormatContext* s, AVStream* st, const AVPacket* pkt)
{
	FFFormatContext* const si = ffformatcontext(s);
	FFStream* const sti = ffstream(st);

	if (sti->request_probe > 0)
	{
		AVProbeData* const pd = &sti->probe_data;
		int end;
		--sti->probe_packets;

		if (pkt)
		{
			uint8_t* new_buf = av_realloc(pd->buf, pd->buf_size + pkt->size + AVPROBE_PADDING_SIZE);
			if (!new_buf)
			{
				goto no_packet;
			}
			pd->buf = new_buf;
			memcpy(pd->buf + pd->buf_size, pkt->data, pkt->size);
			pd->buf_size += pkt->size;
			memset(pd->buf + pd->buf_size, 0, AVPROBE_PADDING_SIZE);
		}
		else
		{
		no_packet:
			sti->probe_packets = 0;
		}

		end = si->raw_packet_buffer_size >= s->probesize || sti->probe_packets <= 0;

		if (end || av_log2(pd->buf_size) != av_log2(pd->buf_size - pkt->size))
		{
			int score = set_codec_from_probe_data(s, st, pd);
			if ((st->codecpar->codec_id != AV_CODEC_ID_NONE && score > AVPROBE_SCORE_STREAM_RETRY) || end)
			{
				pd->buf_size = 0;
				av_freep(&pd->buf);
				sti->request_probe = -1;
			}
		}
	}
	return 0;
}

static int update_wrap_reference(AVFormatContext* s, AVStream* st, int stream_index, AVPacket* pkt)
{
	(void)stream_index;

	FFStream* const sti = ffstream(st);
	int64_t ref = pkt->dts;
	int pts_wrap_behavior;
	int64_t pts_wrap_reference;

	if (ref == AV_NOPTS_VALUE)
		ref = pkt->pts;
	if (sti->pts_wrap_reference != AV_NOPTS_VALUE || st->pts_wrap_bits >= 63 || ref == AV_NOPTS_VALUE || !s->correct_ts_overflow)
		return 0;
	ref &= (1LL << st->pts_wrap_bits) - 1;

	// reference time stamp should be 60 s before first time stamp
	pts_wrap_reference = ref - av_rescale(60, st->time_base.den, st->time_base.num);
	// if first time stamp is not more than 1/8 and 60s before the wrap point, subtract rather than add wrap offset
	pts_wrap_behavior = (ref < (1LL << st->pts_wrap_bits) - (1LL << (st->pts_wrap_bits - 3))) ||
		(ref < (1LL << st->pts_wrap_bits) - av_rescale(60, st->time_base.den, st->time_base.num)) ?
		AV_PTS_WRAP_ADD_OFFSET : AV_PTS_WRAP_SUB_OFFSET;

	int default_stream_index = av_find_default_stream_index(s);
	FFStream* const default_sti = ffstream(s->streams[default_stream_index]);
	if (default_sti->pts_wrap_reference == AV_NOPTS_VALUE)
	{
		for (unsigned i = 0; i < s->nb_streams; i++)
		{
			FFStream* const sti2 = ffstream(s->streams[i]);
			sti2->pts_wrap_reference = pts_wrap_reference;
			sti2->pts_wrap_behavior = pts_wrap_behavior;
		}
	}
	else
	{
		sti->pts_wrap_reference = default_sti->pts_wrap_reference;
		sti->pts_wrap_behavior = default_sti->pts_wrap_behavior;
	}
	return 1;
}

static void update_timestamps(AVFormatContext* s, AVStream* st, AVPacket* pkt)
{
	FFStream* const sti = ffstream(st);

	if (update_wrap_reference(s, st, pkt->stream_index, pkt) && sti->pts_wrap_behavior == AV_PTS_WRAP_SUB_OFFSET)
	{
		// correct first time stamps to negative values
		if (!is_relative(sti->first_dts))
			sti->first_dts = wrap_timestamp(st, sti->first_dts);
		if (!is_relative(st->start_time))
			st->start_time = wrap_timestamp(st, st->start_time);
		if (!is_relative(sti->cur_dts))
			sti->cur_dts = wrap_timestamp(st, sti->cur_dts);
	}

	pkt->dts = wrap_timestamp(st, pkt->dts);
	pkt->pts = wrap_timestamp(st, pkt->pts);
}

/*
 * Handle a new packet and either return it directly if possible and
 * allow_passthrough is true or queue the packet (or drop the packet if corrupt).
 */
static int handle_new_packet(AVFormatContext* s, AVPacket* pkt, int allow_passthrough)
{
	FFFormatContext* const si = ffformatcontext(s);
	AVStream* st;
	FFStream* sti;
	int err;

	av_assert0((unsigned int)pkt->stream_index < s->nb_streams && "Invalid stream index.\n");

	if (pkt->flags & AV_PKT_FLAG_CORRUPT)
	{
		if (s->flags & AVFMT_FLAG_DISCARD_CORRUPT)
		{
			av_packet_unref(pkt);
			return 1;
		}
	}

	st = s->streams[pkt->stream_index];
	sti = ffstream(st);

	update_timestamps(s, st, pkt);

	if (sti->request_probe <= 0 && allow_passthrough && !si->raw_packet_buffer.head)
		return 0;

	err = avpriv_packet_list_put(&si->raw_packet_buffer, pkt, NULL, 0);
	if (err < 0)
	{
		av_packet_unref(pkt);
		return err;
	}

	pkt = &si->raw_packet_buffer.tail->pkt;
	si->raw_packet_buffer_size += pkt->size;

	err = probe_codec(s, st, pkt);
	if (err < 0)
		return err;

	return 1;
}

int ff_read_packet(AVFormatContext* s, AVPacket* pkt)
{
	FFFormatContext* const si = ffformatcontext(s);
	int err;

	av_packet_unref(pkt);

	for (;;)
	{
		PacketListEntry* pktl = si->raw_packet_buffer.head;

		if (pktl)
		{
			AVStream* const st = s->streams[pktl->pkt.stream_index];
			if (si->raw_packet_buffer_size >= s->probesize)
				if ((err = probe_codec(s, st, NULL)) < 0)
					return err;
			if (ffstream(st)->request_probe <= 0)
			{
				avpriv_packet_list_get(&si->raw_packet_buffer, pkt);
				si->raw_packet_buffer_size -= pkt->size;
				return 0;
			}
		}

		err = ffifmt(s->iformat)->read_packet(s, pkt);
		if (err < 0)
		{
			av_packet_unref(pkt);

			/* Some demuxers return FFERROR_REDO when they consume
			   data and discard it (ignored streams, junk, extradata).
			   We must re-call the demuxer to get the real packet. */
			if (err == FFERROR_REDO)
				continue;
			if (!pktl || err == AVERROR(EAGAIN))
				return err;
			for (unsigned i = 0; i < s->nb_streams; i++)
			{
				AVStream* const st = s->streams[i];
				FFStream* const sti = ffstream(st);
				if (sti->probe_packets || sti->request_probe > 0)
					if ((err = probe_codec(s, st, NULL)) < 0)
						return err;
				av_assert0(sti->request_probe <= 0);
			}
			continue;
		}

		err = av_packet_make_refcounted(pkt);
		if (err < 0)
		{
			av_packet_unref(pkt);
			return err;
		}

		err = handle_new_packet(s, pkt, 1);
		if (err <= 0) /* Error or passthrough */
			return err;
	}
}

/* Return the frame duration in seconds. Return 0 if not available. */
static void compute_frame_duration(AVFormatContext* s, int* pnum, int* pden,
	AVStream* st,
	AVPacket* pkt)
{
	FFStream* const sti = ffstream(st);
	AVRational codec_framerate = sti->avctx->framerate;
	int frame_size, sample_rate;

	*pnum = 0;
	*pden = 0;
	switch (st->codecpar->codec_type)
	{
		case AVMEDIA_TYPE_VIDEO:
			if (st->r_frame_rate.num)
			{
				*pnum = st->r_frame_rate.den;
				*pden = st->r_frame_rate.num;
			}
			else if ((s->iformat->flags & AVFMT_NOTIMESTAMPS) &&
				!codec_framerate.num &&
				st->avg_frame_rate.num && st->avg_frame_rate.den)
			{
				*pnum = st->avg_frame_rate.den;
				*pden = st->avg_frame_rate.num;
			}
			else if (st->time_base.num * 1000LL > st->time_base.den)
			{
				*pnum = st->time_base.num;
				*pden = st->time_base.den;
			}
			else if (codec_framerate.den * 1000LL > codec_framerate.num)
			{
				const int ticks_per_frame = 1;
				av_reduce(pnum, pden,
					codec_framerate.den,
					codec_framerate.num * (int64_t)ticks_per_frame,
					INT_MAX);
			}
			break;
		case AVMEDIA_TYPE_AUDIO:
			if (sti->avctx_inited)
			{
				frame_size = av_get_audio_frame_duration(sti->avctx, pkt->size);
				sample_rate = sti->avctx->sample_rate;
			}
			else
			{
				frame_size = av_get_audio_frame_duration2(st->codecpar, pkt->size);
				sample_rate = st->codecpar->sample_rate;
			}
			if (frame_size <= 0 || sample_rate <= 0)
				break;
			*pnum = frame_size;
			*pden = sample_rate;
			break;
		default:
			break;
	}
}

static PacketListEntry* get_next_pkt(AVFormatContext* s, AVStream* st, PacketListEntry* pktl)
{
	(void)st;

	FFFormatContext* const si = ffformatcontext(s);
	if (pktl->next)
		return pktl->next;
	if (pktl == si->packet_buffer.tail)
		return si->parse_queue.head;
	return NULL;
}

static int64_t select_from_pts_buffer(AVStream* st, int64_t* pts_buffer, int64_t dts)
{
	(void)st;

	if (dts == AV_NOPTS_VALUE)
		dts = pts_buffer[0];

	return dts;
}

/* Updates the dts of packets of a stream in pkt_buffer, by re-ordering the pts of the packets in a window. */
static void update_dts_from_pts(AVFormatContext* s, int stream_index,
	PacketListEntry* pkt_buffer)
{
	AVStream* const st = s->streams[stream_index];
	int64_t pts_buffer[MAX_REORDER_DELAY + 1];

	for (int i = 0; i < MAX_REORDER_DELAY + 1; i++)
		pts_buffer[i] = AV_NOPTS_VALUE;

	for (; pkt_buffer; pkt_buffer = get_next_pkt(s, st, pkt_buffer))
	{
		if (pkt_buffer->pkt.stream_index != stream_index)
			continue;

		if (pkt_buffer->pkt.pts != AV_NOPTS_VALUE)
		{
			pts_buffer[0] = pkt_buffer->pkt.pts;
			pkt_buffer->pkt.dts = select_from_pts_buffer(st, pts_buffer, pkt_buffer->pkt.dts);
		}
	}
}

static void update_initial_timestamps(AVFormatContext* s, int stream_index, int64_t dts, int64_t pts, AVPacket* pkt)
{
	FFFormatContext* const si = ffformatcontext(s);
	AVStream* const st = s->streams[stream_index];
	FFStream* const sti = ffstream(st);
	PacketListEntry* pktl = si->packet_buffer.head ? si->packet_buffer.head : si->parse_queue.head;

	uint64_t shift;

	if (sti->first_dts != AV_NOPTS_VALUE ||
		dts == AV_NOPTS_VALUE ||
		sti->cur_dts == AV_NOPTS_VALUE ||
		sti->cur_dts < INT_MIN + RELATIVE_TS_BASE ||
		dts < INT_MIN + (sti->cur_dts - RELATIVE_TS_BASE) ||
		is_relative(dts))
		return;

	sti->first_dts = dts - (sti->cur_dts - RELATIVE_TS_BASE);
	sti->cur_dts = dts;
	shift = (uint64_t)sti->first_dts - RELATIVE_TS_BASE;

	if (is_relative(pts))
		pts += shift;

	for (PacketListEntry* pktl_it = pktl; pktl_it; pktl_it = get_next_pkt(s, st, pktl_it))
	{
		if (pktl_it->pkt.stream_index != stream_index)
			continue;
		if (is_relative(pktl_it->pkt.pts))
			pktl_it->pkt.pts += shift;

		if (is_relative(pktl_it->pkt.dts))
			pktl_it->pkt.dts += shift;

		if (st->start_time == AV_NOPTS_VALUE && pktl_it->pkt.pts != AV_NOPTS_VALUE)
		{
			st->start_time = pktl_it->pkt.pts;
			if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && st->codecpar->sample_rate)
				st->start_time = av_sat_add64(st->start_time,
					av_rescale_q(0, (AVRational){1, st->codecpar->sample_rate}, st->time_base));
		}
	}

	update_dts_from_pts(s, stream_index, pktl);

	if (st->start_time == AV_NOPTS_VALUE)
	{
		if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO || !(pkt->flags & AV_PKT_FLAG_DISCARD))
		{
			st->start_time = pts;
		}
		if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && st->codecpar->sample_rate)
			st->start_time = av_sat_add64(st->start_time,
				av_rescale_q(0, (AVRational){1, st->codecpar->sample_rate}, st->time_base));
	}
}

static void update_initial_durations(AVFormatContext* s, AVStream* st, int stream_index, int64_t duration)
{
	FFFormatContext* const si = ffformatcontext(s);
	FFStream* const sti = ffstream(st);
	PacketListEntry* pktl = si->packet_buffer.head ? si->packet_buffer.head : si->parse_queue.head;
	int64_t cur_dts = RELATIVE_TS_BASE;

	if (sti->first_dts != AV_NOPTS_VALUE)
	{
		if (sti->update_initial_durations_done)
			return;
		sti->update_initial_durations_done = 1;
		cur_dts = sti->first_dts;
		for (; pktl; pktl = get_next_pkt(s, st, pktl))
		{
			if (pktl->pkt.stream_index == stream_index)
			{
				if (pktl->pkt.pts != pktl->pkt.dts || pktl->pkt.dts != AV_NOPTS_VALUE || pktl->pkt.duration)
					break;
				cur_dts -= duration;
			}
		}
		if (pktl && pktl->pkt.dts != sti->first_dts)
		{
			return;
		}
		if (!pktl)
		{
			return;
		}
		pktl = si->packet_buffer.head ? si->packet_buffer.head : si->parse_queue.head;
		sti->first_dts = cur_dts;
	}
	else if (sti->cur_dts != RELATIVE_TS_BASE)
		return;

	for (; pktl; pktl = get_next_pkt(s, st, pktl))
	{
		if (pktl->pkt.stream_index != stream_index)
			continue;
		if ((pktl->pkt.pts == pktl->pkt.dts || pktl->pkt.pts == AV_NOPTS_VALUE) &&
			(pktl->pkt.dts == AV_NOPTS_VALUE || pktl->pkt.dts == sti->first_dts || pktl->pkt.dts == RELATIVE_TS_BASE) &&
			!pktl->pkt.duration &&
			(uint64_t)av_sat_add64(cur_dts, duration) == cur_dts + (uint64_t)duration)
		{
			pktl->pkt.dts = cur_dts;
			pktl->pkt.pts = cur_dts;
			pktl->pkt.duration = duration;
		}
		else
			break;
		cur_dts = pktl->pkt.dts + pktl->pkt.duration;
	}
	if (!pktl)
		sti->cur_dts = cur_dts;
}

static void compute_pkt_fields(AVFormatContext* s, AVStream* st, AVPacket* pkt, int64_t next_dts, int64_t next_pts)
{
	FFFormatContext* const si = ffformatcontext(s);
	FFStream* const sti = ffstream(st);
	int num, den, presentation_delayed, delay;
	AVRational duration;
	const int onein_oneout = 1;

	if (s->flags & AVFMT_FLAG_NOFILLIN)
		return;

	if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && pkt->dts != AV_NOPTS_VALUE)
	{
		if (pkt->dts == pkt->pts && sti->last_dts_for_order_check != AV_NOPTS_VALUE)
		{
			if (sti->last_dts_for_order_check <= pkt->dts)
			{
				sti->dts_ordered++;
			}
			else
			{
				sti->dts_misordered++;
			}
			if (sti->dts_ordered + sti->dts_misordered > 250)
			{
				sti->dts_ordered >>= 1;
				sti->dts_misordered >>= 1;
			}
		}

		sti->last_dts_for_order_check = pkt->dts;
		if (sti->dts_ordered < 8 * sti->dts_misordered && pkt->dts == pkt->pts)
			pkt->dts = AV_NOPTS_VALUE;
	}

	if ((s->flags & AVFMT_FLAG_IGNDTS) && pkt->pts != AV_NOPTS_VALUE)
		pkt->dts = AV_NOPTS_VALUE;

	/* do we have a video B-frame ? */
	delay = 0;
	presentation_delayed = 0;

	if (pkt->pts != AV_NOPTS_VALUE && pkt->dts != AV_NOPTS_VALUE &&
		st->pts_wrap_bits < 63 && pkt->dts > INT64_MIN + (1LL << st->pts_wrap_bits) &&
		pkt->dts - (1LL << (st->pts_wrap_bits - 1)) > pkt->pts)
	{
		if (is_relative(sti->cur_dts) || pkt->dts - (1LL << (st->pts_wrap_bits - 1)) > sti->cur_dts)
		{
			pkt->dts -= 1LL << st->pts_wrap_bits;
		}
		else
			pkt->pts += 1LL << st->pts_wrap_bits;
	}

	duration = av_mul_q((AVRational){(int)pkt->duration, 1}, st->time_base);
	if (pkt->duration <= 0)
	{
		compute_frame_duration(s, &num, &den, st, pkt);
		if (den && num)
		{
			duration = (AVRational){num, den};
			pkt->duration = av_rescale_rnd(1, num * (int64_t)st->time_base.den, den * (int64_t)st->time_base.num, AV_ROUND_DOWN);
		}
	}

	if (pkt->duration > 0 && (si->packet_buffer.head || si->parse_queue.head))
		update_initial_durations(s, st, pkt->stream_index, pkt->duration);

	/* This may be redundant, but it should not hurt. */
	if (pkt->dts != AV_NOPTS_VALUE && pkt->pts != AV_NOPTS_VALUE && pkt->pts > pkt->dts)
		presentation_delayed = 1;

	/* Interpolate PTS and DTS if they are not present. We skip H264
	 * currently because delay and has_b_frames are not reliably set. */
	if (delay == 0 && onein_oneout)
	{
		if (presentation_delayed)
		{
			/* DTS = decompression timestamp */
			/* PTS = presentation timestamp */
			if (pkt->dts == AV_NOPTS_VALUE)
				pkt->dts = sti->last_IP_pts;
			update_initial_timestamps(s, pkt->stream_index, pkt->dts, pkt->pts, pkt);
			if (pkt->dts == AV_NOPTS_VALUE)
				pkt->dts = sti->cur_dts;

			/* This is tricky: the dts must be incremented by the duration
			 * of the frame we are displaying, i.e. the last I- or P-frame. */
			if (sti->last_IP_duration == 0 && (uint64_t)pkt->duration <= INT32_MAX)
				sti->last_IP_duration = (int)pkt->duration;
			if (pkt->dts != AV_NOPTS_VALUE)
				sti->cur_dts = av_sat_add64(pkt->dts, sti->last_IP_duration);
			if (pkt->dts != AV_NOPTS_VALUE &&
				pkt->pts == AV_NOPTS_VALUE &&
				sti->last_IP_duration > 0 &&
				((uint64_t)sti->cur_dts - (uint64_t)next_dts + 1) <= 2 &&
				next_dts != next_pts &&
				next_pts != AV_NOPTS_VALUE)
				pkt->pts = next_dts;

			if ((uint64_t)pkt->duration <= INT32_MAX)
				sti->last_IP_duration = (int)pkt->duration;
			sti->last_IP_pts = pkt->pts;
			/* Cannot compute PTS if not present (we can compute it only by knowing the future. */
		}
		else if (pkt->pts != AV_NOPTS_VALUE || pkt->dts != AV_NOPTS_VALUE || pkt->duration > 0)
		{
			/* presentation is not delayed : PTS and DTS are the same */
			if (pkt->pts == AV_NOPTS_VALUE)
				pkt->pts = pkt->dts;
			update_initial_timestamps(s, pkt->stream_index, pkt->pts,
				pkt->pts, pkt);
			if (pkt->pts == AV_NOPTS_VALUE)
				pkt->pts = sti->cur_dts;
			pkt->dts = pkt->pts;
			if (pkt->pts != AV_NOPTS_VALUE && duration.num >= 0)
				sti->cur_dts = av_add_stable(st->time_base, pkt->pts, duration, 1);
		}
	}

	if (pkt->pts != AV_NOPTS_VALUE && delay <= MAX_REORDER_DELAY)
	{
		sti->pts_buffer[0] = pkt->pts;
		for (int i = 0; i < delay && sti->pts_buffer[i] > sti->pts_buffer[i + 1]; i++)
			FFSWAP(int64_t, sti->pts_buffer[i], sti->pts_buffer[i + 1]);

		pkt->dts = select_from_pts_buffer(st, sti->pts_buffer, pkt->dts);
	}
	// We skipped it above so we try here.
	if (!onein_oneout)
		// This should happen on the first packet
		update_initial_timestamps(s, pkt->stream_index, pkt->dts, pkt->pts, pkt);
	if (pkt->dts > sti->cur_dts)
		sti->cur_dts = pkt->dts;
}

static int codec_close(FFStream* sti)
{
	AVCodecContext* avctx_new = NULL;
	AVCodecParameters* par_tmp = NULL;
	int ret;

	avctx_new = avcodec_alloc_context3(sti->avctx->codec);
	if (!avctx_new)
	{
		ret = AVERROR(ENOMEM);
		goto fail;
	}

	par_tmp = avcodec_parameters_alloc();
	if (!par_tmp)
	{
		ret = AVERROR(ENOMEM);
		goto fail;
	}

	ret = avcodec_parameters_from_context(par_tmp, sti->avctx);
	if (ret < 0)
		goto fail;

	ret = avcodec_parameters_to_context(avctx_new, par_tmp);
	if (ret < 0)
		goto fail;

	avcodec_free_context(&sti->avctx);
	sti->avctx = avctx_new;

	avctx_new = NULL;
	ret = 0;

fail:
	avcodec_free_context(&avctx_new);
	avcodec_parameters_free(&par_tmp);

	return ret;
}

static int read_frame_internal(AVFormatContext* s, AVPacket* pkt)
{
	FFFormatContext* const si = ffformatcontext(s);
	int ret = 0, got_packet = 0;

	while (!got_packet && !si->parse_queue.head)
	{
		AVStream* st;
		FFStream* sti;

		/* read next packet */
		ret = ff_read_packet(s, pkt);
		if (ret < 0)
		{
			if (ret == AVERROR(EAGAIN))
				return ret;
			break;
		}
		ret = 0;
		st = s->streams[pkt->stream_index];
		sti = ffstream(st);

		st->event_flags |= AVSTREAM_EVENT_FLAG_NEW_PACKETS;

		/* update context if required */
		if (sti->need_context_update)
		{
			if (avcodec_is_open(sti->avctx))
			{
				ret = codec_close(sti);
				sti->info->found_decoder = 0;
				if (ret < 0)
					return ret;
			}

			ret = avcodec_parameters_to_context(sti->avctx, st->codecpar);
			if (ret < 0)
			{
				av_packet_unref(pkt);
				return ret;
			}

			sti->need_context_update = 0;
		}

		/* no parsing needed: we just output the packet as is */
		compute_pkt_fields(s, st, pkt, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
		if ((s->iformat->flags & AVFMT_GENERIC_INDEX) && (pkt->flags & AV_PKT_FLAG_KEY) && pkt->dts != AV_NOPTS_VALUE)
		{
			ff_reduce_index(s, st->index);
			av_add_index_entry(st, pkt->pos, pkt->dts, 0, 0, AVINDEX_KEYFRAME);
		}
		got_packet = 1;

		if (pkt->flags & AV_PKT_FLAG_KEY)
			sti->skip_to_keyframe = 0;
		if (sti->skip_to_keyframe)
		{
			av_packet_unref(pkt);
			got_packet = 0;
		}
	}

	if (!got_packet && si->parse_queue.head)
		ret = avpriv_packet_list_get(&si->parse_queue, pkt);

	/* A demuxer might have returned EOF because of an IO error, let's
	 * propagate this back to the user. */
	if (ret == AVERROR_EOF && s->pb && s->pb->error < 0 && s->pb->error != AVERROR(EAGAIN))
		ret = s->pb->error;

	return ret;
}

int av_read_frame(AVFormatContext* s, AVPacket* pkt)
{
	FFFormatContext* const si = ffformatcontext(s);
	const int genpts = s->flags & AVFMT_FLAG_GENPTS;
	int eof = 0;
	int ret;
	AVStream* st;

	if (!genpts)
	{
		ret = si->packet_buffer.head ? avpriv_packet_list_get(&si->packet_buffer, pkt) : read_frame_internal(s, pkt);
		if (ret < 0)
			return ret;
		goto return_packet;
	}

	for (;;)
	{
		PacketListEntry* pktl = si->packet_buffer.head;

		if (pktl)
		{
			AVPacket* next_pkt = &pktl->pkt;

			if (next_pkt->dts != AV_NOPTS_VALUE)
			{
				int wrap_bits = s->streams[next_pkt->stream_index]->pts_wrap_bits;
				// last dts seen for this stream. if any of packets following
				// current one had no dts, we will set this to AV_NOPTS_VALUE.
				int64_t last_dts = next_pkt->dts;
				av_assert2(wrap_bits <= 64);
				while (pktl && next_pkt->pts == AV_NOPTS_VALUE)
				{
					if (pktl->pkt.stream_index == next_pkt->stream_index &&
						av_compare_mod(next_pkt->dts, pktl->pkt.dts, 2ULL << (wrap_bits - 1)) < 0)
					{
						if (av_compare_mod(pktl->pkt.pts, pktl->pkt.dts, 2ULL << (wrap_bits - 1)))
						{
							// not B-frame
							next_pkt->pts = pktl->pkt.dts;
						}
						if (last_dts != AV_NOPTS_VALUE)
						{
							// Once last dts was set to AV_NOPTS_VALUE, we don't change it.
							last_dts = pktl->pkt.dts;
						}
					}
					pktl = pktl->next;
				}
				if (eof && next_pkt->pts == AV_NOPTS_VALUE && last_dts != AV_NOPTS_VALUE)
				{
					// Fixing the last reference frame had none pts issue (For MXF etc).
					// We only do this when
					// 1. eof.
					// 2. we are not able to resolve a pts value for current packet.
					// 3. the packets for this stream at the end of the files had valid dts.
					next_pkt->pts = last_dts + next_pkt->duration;
				}
				pktl = si->packet_buffer.head;
			}

			/* read packet from packet buffer, if there is data */
			st = s->streams[next_pkt->stream_index];
			if (!(next_pkt->pts == AV_NOPTS_VALUE && st->discard < AVDISCARD_ALL && next_pkt->dts != AV_NOPTS_VALUE && !eof))
			{
				ret = avpriv_packet_list_get(&si->packet_buffer, pkt);
				goto return_packet;
			}
		}

		ret = read_frame_internal(s, pkt);
		if (ret < 0)
		{
			if (pktl && ret != AVERROR(EAGAIN))
			{
				eof = 1;
				continue;
			}
			else
				return ret;
		}

		ret = avpriv_packet_list_put(&si->packet_buffer, pkt, NULL, 0);
		if (ret < 0)
		{
			av_packet_unref(pkt);
			return ret;
		}
	}

return_packet:
	st = s->streams[pkt->stream_index];
	if ((s->iformat->flags & AVFMT_GENERIC_INDEX) && pkt->flags & AV_PKT_FLAG_KEY)
	{
		ff_reduce_index(s, st->index);
		av_add_index_entry(st, pkt->pos, pkt->dts, 0, 0, AVINDEX_KEYFRAME);
	}

	if (is_relative(pkt->dts))
		pkt->dts -= RELATIVE_TS_BASE;
	if (is_relative(pkt->pts))
		pkt->pts -= RELATIVE_TS_BASE;

	return ret;
}

/* Return TRUE if the stream has accurate duration in any stream. */
static int has_duration(AVFormatContext* ic)
{
	for (unsigned i = 0; i < ic->nb_streams; i++)
	{
		const AVStream* const st = ic->streams[i];
		if (st->duration != AV_NOPTS_VALUE)
			return 1;
	}
	if (ic->duration != AV_NOPTS_VALUE)
		return 1;
	return 0;
}

/* Estimate the stream timings from the one of each components. Also computes the global bitrate if possible. */
static void update_stream_timings(AVFormatContext* ic)
{
	int64_t start_time, start_time1, start_time_text, end_time, end_time1, end_time_text;
	int64_t duration, duration1, duration_text, filesize;

	start_time = INT64_MAX;
	start_time_text = INT64_MAX;
	end_time = INT64_MIN;
	end_time_text = INT64_MIN;
	duration = INT64_MIN;
	duration_text = INT64_MIN;

	for (unsigned i = 0; i < ic->nb_streams; i++)
	{
		AVStream* const st = ic->streams[i];

		if (st->start_time != AV_NOPTS_VALUE && st->time_base.den)
		{
			start_time1 = av_rescale_q(st->start_time, st->time_base, AV_TIME_BASE_Q);
			start_time = FFMIN(start_time, start_time1);
			end_time1 = av_rescale_q_rnd(st->duration, st->time_base, AV_TIME_BASE_Q, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
			if (end_time1 != AV_NOPTS_VALUE &&
				(end_time1 > 0 ? start_time1 <= INT64_MAX - end_time1 : start_time1 >= INT64_MIN - end_time1))
			{
				end_time1 += start_time1;
				end_time = FFMAX(end_time, end_time1);
			}
		}
		if (st->duration != AV_NOPTS_VALUE)
		{
			duration1 = av_rescale_q(st->duration, st->time_base, AV_TIME_BASE_Q);
			duration = FFMAX(duration, duration1);
		}
	}
	if (start_time == INT64_MAX || (start_time > start_time_text && start_time - (uint64_t)start_time_text < AV_TIME_BASE))
		start_time = start_time_text;

	if (end_time == INT64_MIN || (end_time < end_time_text && end_time_text - (uint64_t)end_time < AV_TIME_BASE))
		end_time = end_time_text;

	if (duration == INT64_MIN || (duration < duration_text && (uint64_t)duration_text - duration < AV_TIME_BASE))
		duration = duration_text;

	if (start_time != INT64_MAX)
	{
		ic->start_time = start_time;
		if (end_time != INT64_MIN)
		{
			if (end_time >= start_time && end_time - (uint64_t)start_time <= INT64_MAX)
			{
				duration = FFMAX(duration, end_time - start_time);
			}
		}
	}
	if (duration != INT64_MIN && duration > 0 && ic->duration == AV_NOPTS_VALUE)
	{
		ic->duration = duration;
	}
	if (ic->pb && (filesize = avio_size(ic->pb)) > 0 && ic->duration > 0)
	{
		/* compute the bitrate */
		double bitrate = (double)filesize * 8.0 * AV_TIME_BASE / (double)ic->duration;
		if (bitrate >= 0 && bitrate <= (double)INT64_MAX)
			ic->bit_rate = (int64_t)bitrate;
	}
}

static void fill_all_stream_timings(AVFormatContext* ic)
{
	update_stream_timings(ic);
	for (unsigned i = 0; i < ic->nb_streams; i++)
	{
		AVStream* const st = ic->streams[i];

		if (st->start_time == AV_NOPTS_VALUE)
		{
			if (ic->start_time != AV_NOPTS_VALUE)
				st->start_time = av_rescale_q(ic->start_time, AV_TIME_BASE_Q, st->time_base);
			if (ic->duration != AV_NOPTS_VALUE)
				st->duration = av_rescale_q(ic->duration, AV_TIME_BASE_Q, st->time_base);
		}
	}
}

static void estimate_timings_from_bit_rate(AVFormatContext* ic)
{
	FFFormatContext* const si = ffformatcontext(ic);

	/* if bit_rate is already set, we believe it */
	if (ic->bit_rate <= 0)
	{
		int64_t bit_rate = 0;
		for (unsigned i = 0; i < ic->nb_streams; i++)
		{
			const AVStream* const st = ic->streams[i];
			const FFStream* const sti = cffstream(st);
			if (st->codecpar->bit_rate <= 0 && sti->avctx->bit_rate > 0)
				st->codecpar->bit_rate = sti->avctx->bit_rate;
			if (st->codecpar->bit_rate > 0)
			{
				if (INT64_MAX - st->codecpar->bit_rate < bit_rate)
				{
					bit_rate = 0;
					break;
				}
				bit_rate += st->codecpar->bit_rate;
			}
			else if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && sti->codec_info_nb_frames > 1)
			{
				// If we have a videostream with packets but without a bitrate then consider the sum not known
				bit_rate = 0;
				break;
			}
		}
		ic->bit_rate = bit_rate;
	}

	/* if duration is already set, we believe it */
	if (ic->duration == AV_NOPTS_VALUE && ic->bit_rate != 0)
	{
		int64_t filesize = ic->pb ? avio_size(ic->pb) : 0;
		if (filesize > si->data_offset)
		{
			filesize -= si->data_offset;
			for (unsigned i = 0; i < ic->nb_streams; i++)
			{
				AVStream* const st = ic->streams[i];

				if (st->time_base.num <= INT64_MAX / ic->bit_rate && st->duration == AV_NOPTS_VALUE)
				{
					st->duration = av_rescale(filesize, 8LL * st->time_base.den, ic->bit_rate * (int64_t)st->time_base.num);
				}
			}
		}
	}
}

static void estimate_timings(AVFormatContext* ic, int64_t old_offset)
{
	(void)old_offset;

	int64_t file_size;

	/* get the file size, if possible */
	if (ic->iformat->flags & AVFMT_NOFILE)
	{
		file_size = 0;
	}
	else
	{
		file_size = avio_size(ic->pb);
		file_size = FFMAX(0, file_size);
	}

	if (has_duration(ic))
	{
		/* at least one component has timings - we use them for all the components */
		fill_all_stream_timings(ic);
	}
	else
	{
		/* less precise: use bitrate info */
		estimate_timings_from_bit_rate(ic);
	}
	update_stream_timings(ic);
}

static int has_codec_parameters(const AVStream* st, const char** errmsg_ptr)
{
	const FFStream* const sti = cffstream(st);
	const AVCodecContext* const avctx = sti->avctx;

#define FAIL(errmsg) do {         \
        if (errmsg_ptr)           \
            *errmsg_ptr = errmsg; \
        return 0;                 \
    } while (0)

	if (avctx->codec_id == AV_CODEC_ID_NONE)
		FAIL("unknown codec");
	switch (avctx->codec_type)
	{
		case AVMEDIA_TYPE_AUDIO:
			if (sti->info->found_decoder >= 0 && avctx->sample_fmt == AV_SAMPLE_FMT_NONE)
				FAIL("unspecified sample format");
			if (!avctx->sample_rate)
				FAIL("unspecified sample rate");
			if (!avctx->ch_layout.nb_channels)
				FAIL("unspecified number of channels");
			break;
		case AVMEDIA_TYPE_VIDEO:
			if (!avctx->width)
				FAIL("unspecified size");
			if (sti->info->found_decoder >= 0 && avctx->pix_fmt == AV_PIX_FMT_NONE)
				FAIL("unspecified pixel format");
			break;
		default:
			break;
	}

	return 1;
}

/* returns 1 or 0 if or if not decoded data was returned, or a negative error */
static int try_decode_frame(AVFormatContext* s, AVStream* st, const AVPacket* pkt, AVDictionary** options)
{
	(void)options;

	FFStream* const sti = ffstream(st);
	AVCodecContext* const avctx = sti->avctx;
	const AVCodec* codec;
	int got_picture = 1, ret = 0;
	AVFrame* frame = av_frame_alloc();
	int pkt_to_send = pkt->size > 0;

	if (!frame)
		return AVERROR(ENOMEM);

	if (!avcodec_is_open(avctx) &&
		sti->info->found_decoder <= 0 &&
		(st->codecpar->codec_id != (enum AVCodecID)(-sti->info->found_decoder) || !st->codecpar->codec_id))
	{
		codec = find_probe_decoder(s, st, st->codecpar->codec_id);

		if (!codec)
		{
			sti->info->found_decoder = -st->codecpar->codec_id;
			ret = -1;
			goto fail;
		}

		ret = avcodec_open2(avctx, codec, NULL);
		if (ret < 0)
		{
			sti->info->found_decoder = -avctx->codec_id;
			goto fail;
		}
		sti->info->found_decoder = 1;
	}
	else if (!sti->info->found_decoder)
		sti->info->found_decoder = 1;

	if (sti->info->found_decoder < 0)
	{
		ret = -1;
		goto fail;
	}

	(void)avpriv_codec_get_cap_skip_frame_fill_param(avctx->codec);

	while ((pkt_to_send || (!pkt->data && got_picture)) && ret >= 0 && (!has_codec_parameters(st, NULL)))
	{
		got_picture = 0;
		if (avctx->codec_type == AVMEDIA_TYPE_VIDEO || avctx->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			ret = avcodec_send_packet(avctx, pkt);
			if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
				break;
			if (ret >= 0)
				pkt_to_send = 0;
			ret = avcodec_receive_frame(avctx, frame);
			if (ret >= 0)
				got_picture = 1;
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				ret = 0;
		}
		if (ret >= 0)
		{
			if (got_picture)
				sti->nb_decoded_frames++;
			ret = got_picture;
		}
	}

fail:
	av_frame_free(&frame);
	return ret;
}

static int get_std_framerate(int i)
{
	if (i < 30 * 12)
		return (i + 1) * 1001;
	i -= 30 * 12;

	if (i < 30)
		return (i + 31) * 1001 * 12;
	i -= 30;

	if (i < 3)
		return ((const int[])
		{
			80, 120, 240
		})[i] * 1001 * 12;

	i -= 3;

	return ((const int[])
	{
		24, 30, 60, 12, 15, 48
	})[i] * 1000 * 12;
}

/* Is the time base unreliable?
 * This is a heuristic to balance between quick acceptance of the values in
 * the headers vs. some extra checks. */
static int tb_unreliable(AVFormatContext* ic, AVStream* st)
{
	FFStream* const sti = ffstream(st);
	AVCodecContext* c = sti->avctx;
	AVRational mul = (AVRational){1, 1};
	AVRational time_base = c->framerate.num ? av_inv_q(av_mul_q(c->framerate, mul)) :
		/* NOHEADER check added to not break existing behavior */
			(((ic->ctx_flags & AVFMTCTX_NOHEADER) || st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) ?
			(AVRational){0, 1} :
				st->time_base);

	if (time_base.den >= 101LL * time_base.num || time_base.den < 5LL * time_base.num)
		return 1;
	return 0;
}

void ff_rfps_calculate(AVFormatContext* ic)
{
	for (unsigned i = 0; i < ic->nb_streams; i++)
	{
		AVStream* const st = ic->streams[i];
		FFStream* const sti = ffstream(st);

		if (st->codecpar->codec_type != AVMEDIA_TYPE_VIDEO)
			continue;
		// the check for tb_unreliable() is not completely correct, since this is not about handling
		// an unreliable/inexact time base, but a time base that is finer than necessary, as e.g.
		// ipmovie.c produces.
		if (tb_unreliable(ic, st) &&
			sti->info->duration_count > 15 &&
			sti->info->duration_gcd > FFMAX(1, st->time_base.den / (500LL * st->time_base.num)) &&
			!st->r_frame_rate.num &&
			sti->info->duration_gcd < INT64_MAX / st->time_base.num)
			av_reduce(&st->r_frame_rate.num, &st->r_frame_rate.den, st->time_base.den,
				st->time_base.num * sti->info->duration_gcd, INT_MAX);
		if (sti->info->duration_count > 1 && !st->r_frame_rate.num && tb_unreliable(ic, st))
		{
			int num = 0;
			double best_error = 0.01;
			AVRational ref_rate = st->r_frame_rate.num ? st->r_frame_rate : av_inv_q(st->time_base);

			for (int j = 0; j < MAX_STD_TIMEBASES; j++)
			{
				if (sti->info->codec_info_duration &&
					sti->info->codec_info_duration * av_q2d(st->time_base) < (1001 * 11.5) / get_std_framerate(j))
					continue;
				if (!sti->info->codec_info_duration && get_std_framerate(j) < 1001 * 12)
					continue;

				if (av_q2d(st->time_base) * sti->info->rfps_duration_sum /
						sti->info->duration_count < (1001 * 12.0 * 0.8) / get_std_framerate(j))
					continue;

				for (int k = 0; k < 2; k++)
				{
					int n = sti->info->duration_count;
					double a = sti->info->duration_error[k][0][j] / n;
					double error = sti->info->duration_error[k][1][j] / n - a * a;

					if (error < best_error && best_error> 0.000000001)
					{
						best_error = error;
						num = get_std_framerate(j);
					}
				}
			}
			// do not increase frame rate by more than 1 % in order to match a standard rate.
			if (num && (!ref_rate.num || (double)num / (12 * 1001) < 1.01 * av_q2d(ref_rate)))
				av_reduce(&st->r_frame_rate.num, &st->r_frame_rate.den, num, 12 * 1001, INT_MAX);
		}
		if (!st->avg_frame_rate.num
			&& st->r_frame_rate.num && sti->info->rfps_duration_sum
			&& sti->info->codec_info_duration <= 0
			&& sti->info->duration_count > 2
			&& fabs(1.0 / (av_q2d(st->r_frame_rate) * av_q2d(st->time_base)) - sti->info->rfps_duration_sum /
				(double)sti->info->duration_count) <= 1.0
			)
		{
			st->avg_frame_rate = st->r_frame_rate;
		}

		av_freep(&sti->info->duration_error);
		sti->info->duration_count = 0;
		sti->info->rfps_duration_sum = 0;
	}
}

int avformat_find_stream_info(AVFormatContext* ic, AVDictionary** options)
{
	(void)options;

	FFFormatContext* const si = ffformatcontext(ic);
	int count = 0, ret = 0;
	int64_t read_size;
	AVPacket* pkt1 = si->pkt;
	int64_t old_offset = avio_tell(ic->pb);
	// new streams might appear, no options for those
	int flush_codecs;
	int64_t max_analyze_duration = 0;
	int64_t max_stream_analyze_duration;
	int64_t probesize = ic->probesize;
	int eof_reached = 0;

	flush_codecs = probesize > 0;

	max_stream_analyze_duration = max_analyze_duration;
	if (!max_analyze_duration)
	{
		max_stream_analyze_duration =
			max_analyze_duration = 5 * AV_TIME_BASE;
	}

	for (unsigned i = 0; i < ic->nb_streams; i++)
	{
		const AVCodec* codec;
		AVStream* const st = ic->streams[i];
		FFStream* const sti = ffstream(st);
		AVCodecContext* const avctx = sti->avctx;

		ret = avcodec_parameters_to_context(avctx, st->codecpar);
		if (ret < 0)
			goto find_stream_info_err;
		if (sti->request_probe <= 0)
			sti->avctx_inited = 1;

		codec = find_probe_decoder(ic, st, st->codecpar->codec_id);

		// Try to just open decoders, in case this is enough to get parameters.
		// Also ensure that subtitle_header is properly set.
		if (!has_codec_parameters(st, NULL) && sti->request_probe <= 0)
		{
			if (codec && !avctx->codec)
				avcodec_open2(avctx, codec, NULL);
		}
	}

	read_size = 0;
	for (;;)
	{
		const AVPacket* pkt;
		AVStream* st;
		FFStream* sti;
		AVCodecContext* avctx;
		int analyzed_all_streams;
		unsigned i;

		/* check if one codec still needs to be handled */
		for (i = 0; i < ic->nb_streams; i++)
		{
			AVStream* const st2 = ic->streams[i];
			FFStream* const sti2 = ffstream(st2);
			int fps_analyze_framecount = 20;
			int count2;

			if (!has_codec_parameters(st2, NULL))
				break;
			/* If the timebase is coarse (like the usual millisecond precision
			 * of mkv), we need to analyze more frames to reliably arrive at
			 * the correct fps. */
			if (av_q2d(st2->time_base) > 0.0005)
				fps_analyze_framecount *= 2;
			if (!tb_unreliable(ic, st2))
				fps_analyze_framecount = 0;
			if (ic->fps_probe_size >= 0)
				fps_analyze_framecount = ic->fps_probe_size;
			/* variable fps and no guess at the real fps */
			count2 = (int)((ic->iformat->flags & AVFMT_NOTIMESTAMPS) ?
				sti2->info->codec_info_duration_fields / 2 :
				sti2->info->duration_count);
			if (!(st2->r_frame_rate.num && st2->avg_frame_rate.num) &&
				st2->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				if (count2 < fps_analyze_framecount)
					break;
			}
			// Look at the first 3 frames if there is evidence of frame delay
			// but the decoder delay is not set.
			if (sti2->info->frame_delay_evidence && count2 < 2)
				break;
			if (sti2->first_dts == AV_NOPTS_VALUE &&
				(!(ic->iformat->flags & AVFMT_NOTIMESTAMPS)) &&
				sti2->codec_info_nb_frames < ic->max_ts_probe &&
				(st2->codecpar->codec_type == AVMEDIA_TYPE_VIDEO ||
					st2->codecpar->codec_type == AVMEDIA_TYPE_AUDIO))
				break;
		}
		analyzed_all_streams = 0;
		if (i == ic->nb_streams)
		{
			analyzed_all_streams = 1;
			/* NOTE: If the format has no header, then we need to read some
			 * packets to get most of the streams, so we cannot stop here. */
			if (!(ic->ctx_flags & AVFMTCTX_NOHEADER))
			{
				/* If we found the info for all the codecs, we can stop. */
				ret = count;
				flush_codecs = 0;
				break;
			}
		}
		/* We did not get all the codec info, but we read too much data. */
		if (read_size >= probesize)
		{
			ret = count;
			break;
		}

		/* NOTE: A new stream can be added there if no header in file (AVFMTCTX_NOHEADER). */
		ret = read_frame_internal(ic, pkt1);
		if (ret == AVERROR(EAGAIN))
			continue;

		if (ret < 0)
		{
			/* EOF or error*/
			eof_reached = 1;
			break;
		}

		if (!(ic->flags & AVFMT_FLAG_NOBUFFER))
		{
			ret = avpriv_packet_list_put(&si->packet_buffer, pkt1, NULL, 0);
			if (ret < 0)
				goto unref_then_goto_end;

			pkt = &si->packet_buffer.tail->pkt;
		}
		else
		{
			pkt = pkt1;
		}

		st = ic->streams[pkt->stream_index];
		sti = ffstream(st);
		read_size += pkt->size;

		avctx = sti->avctx;
		if (!sti->avctx_inited)
		{
			ret = avcodec_parameters_to_context(avctx, st->codecpar);
			if (ret < 0)
				goto unref_then_goto_end;
			sti->avctx_inited = 1;
		}

		if (pkt->dts != AV_NOPTS_VALUE && sti->codec_info_nb_frames > 1)
		{
			/* check for non-increasing dts */
			if (sti->info->fps_last_dts != AV_NOPTS_VALUE && sti->info->fps_last_dts >= pkt->dts)
			{
				sti->info->fps_first_dts =
					sti->info->fps_last_dts = AV_NOPTS_VALUE;
			}
			/* Check for a discontinuity in dts. If the difference in dts
			 * is more than 1000 times the average packet duration in the
			 * sequence, we treat it as a discontinuity. */
			if (sti->info->fps_last_dts != AV_NOPTS_VALUE &&
				sti->info->fps_last_dts_idx > sti->info->fps_first_dts_idx &&
				(pkt->dts - (uint64_t)sti->info->fps_last_dts) / 1000 >
					(sti->info->fps_last_dts - (uint64_t)sti->info->fps_first_dts) /
						(sti->info->fps_last_dts_idx - sti->info->fps_first_dts_idx))
			{
				sti->info->fps_first_dts =
					sti->info->fps_last_dts = AV_NOPTS_VALUE;
			}

			/* update stored dts values */
			if (sti->info->fps_first_dts == AV_NOPTS_VALUE)
			{
				sti->info->fps_first_dts = pkt->dts;
				sti->info->fps_first_dts_idx = sti->codec_info_nb_frames;
			}
			sti->info->fps_last_dts = pkt->dts;
			sti->info->fps_last_dts_idx = sti->codec_info_nb_frames;
		}
		if (sti->codec_info_nb_frames > 1)
		{
			int64_t t = 0;
			int64_t limit;

			if (st->time_base.den > 0)
				t = av_rescale_q(sti->info->codec_info_duration, st->time_base, AV_TIME_BASE_Q);
			if (st->avg_frame_rate.num > 0)
				t = FFMAX(t, av_rescale_q(sti->codec_info_nb_frames, av_inv_q(st->avg_frame_rate), AV_TIME_BASE_Q));

			if (t == 0
				&& sti->codec_info_nb_frames > 30
				&& sti->info->fps_first_dts != AV_NOPTS_VALUE
				&& sti->info->fps_last_dts != AV_NOPTS_VALUE)
			{
				int64_t dur = av_sat_sub64(sti->info->fps_last_dts, sti->info->fps_first_dts);
				t = FFMAX(t, av_rescale_q(dur, st->time_base, AV_TIME_BASE_Q));
			}

			if (analyzed_all_streams) limit = max_analyze_duration;
			else                      limit = max_stream_analyze_duration;

			if (t >= limit)
			{
				if (ic->flags & AVFMT_FLAG_NOBUFFER)
					av_packet_unref(pkt1);
				break;
			}
			if (pkt->duration > 0)
			{
				sti->info->codec_info_duration += pkt->duration;
				sti->info->codec_info_duration_fields += 2;
			}
		}
		if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			if (pkt->dts != pkt->pts && pkt->dts != AV_NOPTS_VALUE && pkt->pts != AV_NOPTS_VALUE)
				sti->info->frame_delay_evidence = 1;
		}

		/* If still no information, we try to open the codec and to
		 * decompress the frame. We try to avoid that in most cases as
		 * it takes longer and uses more memory. For MPEG-4, we need to
		 * decompress for QuickTime.
		 *
		 * If AV_CODEC_CAP_CHANNEL_CONF is set this will force decoding of at
		 * least one frame of codec data, this makes sure the codec initializes
		 * the channel configuration and does not only trust the values from
		 * the container. */
		try_decode_frame(ic, st, pkt, NULL);

		if (ic->flags & AVFMT_FLAG_NOBUFFER)
			av_packet_unref(pkt1);

		sti->codec_info_nb_frames++;
		count++;
	}

	if (eof_reached)
	{
		for (unsigned stream_index = 0; stream_index < ic->nb_streams; stream_index++)
		{
			AVStream* const st = ic->streams[stream_index];
			AVCodecContext* const avctx = ffstream(st)->avctx;
			if (!has_codec_parameters(st, NULL))
			{
				const AVCodec* codec = find_probe_decoder(ic, st, st->codecpar->codec_id);
				if (codec && !avctx->codec)
				{
					(void)avcodec_open2(avctx, codec, NULL);
				}
			}
		}
	}

	if (flush_codecs)
	{
		AVPacket* empty_pkt = si->pkt;
		av_packet_unref(empty_pkt);

		for (unsigned i = 0; i < ic->nb_streams; i++)
		{
			AVStream* const st = ic->streams[i];
			FFStream* const sti = ffstream(st);

			/* flush the decoders */
			if (sti->info->found_decoder == 1)
			{
				(void)try_decode_frame(ic, st, empty_pkt, NULL);
			}
		}
	}

	ff_rfps_calculate(ic);

	for (unsigned i = 0; i < ic->nb_streams; i++)
	{
		AVStream* const st = ic->streams[i];
		FFStream* const sti = ffstream(st);
		AVCodecContext* const avctx = sti->avctx;

		if (avctx->codec_type == AVMEDIA_TYPE_VIDEO)
		{
/* estimate average framerate if not set by demuxer */
			if (sti->info->codec_info_duration_fields && !st->avg_frame_rate.num && sti->info->codec_info_duration)
			{
				int best_fps = 0;
				double best_error = 0.01;

				if (sti->info->codec_info_duration >= INT64_MAX / st->time_base.num / 2 ||
					sti->info->codec_info_duration_fields >= INT64_MAX / st->time_base.den ||
					sti->info->codec_info_duration < 0)
					continue;
				av_reduce(&st->avg_frame_rate.num, &st->avg_frame_rate.den,
					sti->info->codec_info_duration_fields * (int64_t)st->time_base.den,
					sti->info->codec_info_duration * 2 * (int64_t)st->time_base.num, 60000);

				/* Round guessed framerate to a "standard" framerate if it's within 1% of the original estimate. */
				for (int j = 0; j < MAX_STD_TIMEBASES; j++)
				{
					AVRational std_fps = {get_std_framerate(j), 12 * 1001};
					double error = fabs(av_q2d(st->avg_frame_rate) /
						av_q2d(std_fps) - 1);

					if (error < best_error)
					{
						best_error = error;
						best_fps = std_fps.num;
					}
				}
				if (best_fps)
					av_reduce(&st->avg_frame_rate.num, &st->avg_frame_rate.den, best_fps, 12 * 1001, INT_MAX);
			}
			if (!st->r_frame_rate.num)
			{
				AVRational mul = (AVRational){1, 1};
				AVRational  fr = av_mul_q(avctx->framerate, mul);

				if (fr.num && fr.den && av_cmp_q(st->time_base, av_inv_q(fr)) <= 0)
				{
					st->r_frame_rate = fr;
				}
				else
				{
					st->r_frame_rate.num = st->time_base.den;
					st->r_frame_rate.den = st->time_base.num;
				}
			}
			st->codecpar->framerate = avctx->framerate;
			if (sti->display_aspect_ratio.num && sti->display_aspect_ratio.den)
			{
				AVRational hw_ratio = {avctx->height, avctx->width};
				st->sample_aspect_ratio = av_mul_q(sti->display_aspect_ratio,
					hw_ratio);
			}
		}
	}

	if (probesize)
		estimate_timings(ic, old_offset);

	if (ret >= 0 && ic->nb_streams)
		/* We could not have all the codec parameters before EOF. */
		ret = -1;
	for (unsigned i = 0; i < ic->nb_streams; i++)
	{
		AVStream* const st = ic->streams[i];
		FFStream* const sti = ffstream(st);
		const char* errmsg;

		/* if no packet was ever seen, update context now for has_codec_parameters */
		if (!sti->avctx_inited)
		{
			if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO &&
				st->codecpar->format == AV_SAMPLE_FMT_NONE)
				st->codecpar->format = sti->avctx->sample_fmt;
			ret = avcodec_parameters_to_context(sti->avctx, st->codecpar);
			if (ret < 0)
				goto find_stream_info_err;
		}
		if (!has_codec_parameters(st, &errmsg))
		{
		}
		else
		{
			ret = 0;
		}
	}

	/* update the stream parameters from the internal codec contexts */
	for (unsigned i = 0; i < ic->nb_streams; i++)
	{
		AVStream* const st = ic->streams[i];
		FFStream* const sti = ffstream(st);

		if (sti->avctx_inited)
		{
			ret = avcodec_parameters_from_context(st->codecpar, sti->avctx);
			if (ret < 0)
				goto find_stream_info_err;
		}

		sti->avctx_inited = 0;
	}

find_stream_info_err:
	for (unsigned i = 0; i < ic->nb_streams; i++)
	{
		AVStream* const st = ic->streams[i];
		FFStream* const sti = ffstream(st);
		int err;

		if (sti->info)
		{
			av_freep(&sti->info->duration_error);
			av_freep(&sti->info);
		}

		err = codec_close(sti);
		if (err < 0 && ret >= 0)
			ret = err;
		av_bsf_free(&sti->extract_extradata.bsf);
	}
	return ret;

unref_then_goto_end:
	av_packet_unref(pkt1);
	goto find_stream_info_err;
}
