/*
 * Various functions used by both muxers and demuxers
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <math.h>
#include "libavutil/avassert.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/bsf.h"
#include "avformat.h"
#include "demux.h"
#include "internal.h"

void ff_free_stream(AVStream** pst)
{
	AVStream* st = *pst;
	FFStream* const sti = ffstream(st);

	if (!st)
		return;

	avcodec_free_context(&sti->avctx);
	av_freep(&sti->index_entries);
	av_freep(&sti->probe_data.buf);

	av_bsf_free(&sti->extract_extradata.bsf);

	if (sti->info)
	{
		av_freep(&sti->info->duration_error);
		av_freep(&sti->info);
	}

	avcodec_parameters_free(&st->codecpar);

	av_freep(pst);
}

/* XXX: suppress the packet queue */
void ff_flush_packet_queue(AVFormatContext* s)
{
	FFFormatContext* const si = ffformatcontext(s);
	avpriv_packet_list_free(&si->parse_queue);
	avpriv_packet_list_free(&si->packet_buffer);
	avpriv_packet_list_free(&si->raw_packet_buffer);

	si->raw_packet_buffer_size = 0;
}

void avformat_free_context(AVFormatContext* s)
{
	FFFormatContext* si;

	if (!s)
		return;
	si = ffformatcontext(s);

	for (unsigned i = 0; i < s->nb_streams; i++)
		ff_free_stream(&s->streams[i]);
	s->nb_streams = 0;

	av_freep(&s->priv_data);
	av_packet_free(&si->pkt);
	av_freep(&s->streams);
	ff_flush_packet_queue(s);
	av_free(s);
}

int av_find_default_stream_index(AVFormatContext* s)
{
	int best_stream = 0;
	int best_score = INT_MIN;

	if (s->nb_streams <= 0)
		return -1;
	for (unsigned i = 0; i < s->nb_streams; i++)
	{
		const AVStream* const st = s->streams[i];
		const FFStream* const sti = cffstream(st);
		int score = 0;
		if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			if (st->codecpar->width && st->codecpar->height)
				score += 50;
			score += 25;
		}
		if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			if (st->codecpar->sample_rate)
				score += 50;
		}
		if (sti->codec_info_nb_frames)
			score += 12;

		if (st->discard != AVDISCARD_ALL)
			score += 200;

		if (score > best_score)
		{
			best_score = score;
			best_stream = i;
		}
	}
	return best_stream;
}

int av_find_best_stream(AVFormatContext* ic, enum AVMediaType type,
	int wanted_stream_nb, int related_stream,
	const AVCodec** decoder_ret, int flags)
{
	(void)related_stream;
	(void)flags;

	int nb_streams = ic->nb_streams;
	int ret = AVERROR_STREAM_NOT_FOUND;
	int best_count = -1, best_multiframe = -1, best_disposition = -1;
	int count, multiframe, disposition;
	int64_t best_bitrate = -1;
	int64_t bitrate;
	unsigned* program = NULL;
	const AVCodec* decoder = NULL, * best_decoder = NULL;

	for (unsigned i = 0; i < (unsigned int)nb_streams; i++)
	{
		int real_stream_index = program ? program[i] : i;
		AVStream* st = ic->streams[real_stream_index];
		AVCodecParameters* par = st->codecpar;
		if (par->codec_type != type)
			continue;
		if (wanted_stream_nb >= 0 && real_stream_index != wanted_stream_nb)
			continue;
		if (type == AVMEDIA_TYPE_AUDIO && !(par->ch_layout.nb_channels && par->sample_rate))
			continue;
		if (decoder_ret)
		{
			decoder = ff_find_decoder(ic, st, par->codec_id);
			if (!decoder)
			{
				if (ret < 0)
					ret = AVERROR_DECODER_NOT_FOUND;
				continue;
			}
		}
		disposition = 0;
		count = ffstream(st)->codec_info_nb_frames;
		bitrate = par->bit_rate;
		multiframe = FFMIN(5, count);
		if ((best_disposition > disposition) ||
			(best_disposition == disposition && best_multiframe > multiframe) ||
			(best_disposition == disposition && best_multiframe == multiframe && best_bitrate > bitrate) ||
			(best_disposition == disposition && best_multiframe == multiframe && best_bitrate == bitrate && best_count >= count))
			continue;
		best_disposition = disposition;
		best_count = count;
		best_bitrate = bitrate;
		best_multiframe = multiframe;
		ret = real_stream_index;
		best_decoder = decoder;
		if (program && i == (unsigned int)(nb_streams - 1) && ret < 0)
		{
			program = NULL;
			nb_streams = ic->nb_streams;
			/* no related stream found, try again with everything */
			i = 0;
		}
	}
	if (decoder_ret)
		*decoder_ret = best_decoder;
	return ret;
}

void avpriv_set_pts_info(AVStream* st, int pts_wrap_bits, unsigned int pts_num, unsigned int pts_den)
{
	AVRational new_tb;
	(void)av_reduce(&new_tb.num, &new_tb.den, pts_num, pts_den, INT_MAX);

	if (new_tb.num <= 0 || new_tb.den <= 0)
	{
		return;
	}
	st->time_base = new_tb;
	st->pts_wrap_bits = pts_wrap_bits;
}

const AVCodec* ff_find_decoder(AVFormatContext* s, const AVStream* st, enum AVCodecID codec_id)
{
	(void)s;
	(void)st;

	return avcodec_find_decoder(codec_id);
}
