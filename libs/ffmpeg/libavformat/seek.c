/*
 * Seeking and index-related functions
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdint.h>

#include "libavutil/avassert.h"
#include "libavcodec/avcodec.h"

#include "demux.h"
#include "internal.h"

void ff_reduce_index(AVFormatContext* s, int stream_index)
{
	AVStream* const st = s->streams[stream_index];
	FFStream* const sti = ffstream(st);
	unsigned int max_entries = s->max_index_size / sizeof(AVIndexEntry);

	if ((unsigned)sti->nb_index_entries >= max_entries)
	{
		int i;
		for (i = 0; 2 * i < sti->nb_index_entries; i++)
			sti->index_entries[i] = sti->index_entries[2 * i];
		sti->nb_index_entries = i;
	}
}

int ff_add_index_entry(AVIndexEntry** index_entries,
	int* nb_index_entries,
	unsigned int* index_entries_allocated_size,
	int64_t pos, int64_t timestamp,
	int size, int distance, int flags)
{
	AVIndexEntry* entries, * ie;
	int index;

	if ((unsigned)*nb_index_entries + 1 >= UINT_MAX / sizeof(AVIndexEntry))
		return -1;

	if (timestamp == AV_NOPTS_VALUE)
		return AVERROR(EINVAL);

	if (size < 0 || size > 0x3FFFFFFF)
		return AVERROR(EINVAL);

	if (is_relative(timestamp)) // FIXME this maintains previous behavior but we should shift by the correct offset once known
		timestamp -= RELATIVE_TS_BASE;

	entries = av_fast_realloc(*index_entries, index_entries_allocated_size, (*nb_index_entries + 1) * sizeof(AVIndexEntry));
	if (!entries)
		return -1;

	*index_entries = entries;

	index = ff_index_search_timestamp(*index_entries, *nb_index_entries, timestamp, AVSEEK_FLAG_ANY);
	if (index < 0)
	{
		index = (*nb_index_entries)++;
		ie = &entries[index];
		av_assert0(index == 0 || ie[-1].timestamp < timestamp);
	}
	else
	{
		ie = &entries[index];
		if (ie->timestamp != timestamp)
		{
			if (ie->timestamp <= timestamp)
				return -1;
			memmove(entries + index + 1, entries + index, sizeof(AVIndexEntry) * (*nb_index_entries - index));
			(*nb_index_entries)++;
		}
		else if (ie->pos == pos && distance < ie->min_distance)
			// do not reduce the distance
			distance = ie->min_distance;
	}

	ie->pos = pos;
	ie->timestamp = timestamp;
	ie->min_distance = distance;
	ie->size = size;
	ie->flags = flags;

	return index;
}

int av_add_index_entry(AVStream* st, int64_t pos, int64_t timestamp, int size, int distance, int flags)
{
	FFStream* const sti = ffstream(st);
	timestamp = ff_wrap_timestamp(st, timestamp);
	return ff_add_index_entry(&sti->index_entries, &sti->nb_index_entries,
		&sti->index_entries_allocated_size, pos,
		timestamp, size, distance, flags);
}

int ff_index_search_timestamp(const AVIndexEntry* entries, int nb_entries, int64_t wanted_timestamp, int flags)
{
	int a, b, m;
	int64_t timestamp;

	a = -1;
	b = nb_entries;

	// Optimize appending index entries at the end.
	if (b && entries[b - 1].timestamp < wanted_timestamp)
		a = b - 1;

	while (b - a > 1)
	{
		m = (a + b) >> 1;

		// Search for the next non-discarded packet.
		while ((entries[m].flags & AVINDEX_DISCARD_FRAME) && m < b && m < nb_entries - 1)
		{
			m++;
			if (m == b && entries[m].timestamp >= wanted_timestamp)
			{
				m = b - 1;
				break;
			}
		}

		timestamp = entries[m].timestamp;
		if (timestamp >= wanted_timestamp)
			b = m;
		if (timestamp <= wanted_timestamp)
			a = m;
	}
	m = (flags & AVSEEK_FLAG_BACKWARD) ? a : b;

	if (!(flags & AVSEEK_FLAG_ANY))
		while (m >= 0 && m < nb_entries && !(entries[m].flags & AVINDEX_KEYFRAME))
			m += (flags & AVSEEK_FLAG_BACKWARD) ? -1 : 1;

	if (m == nb_entries)
		return -1;
	return m;
}

int av_index_search_timestamp(AVStream* st, int64_t wanted_timestamp, int flags)
{
	const FFStream* const sti = ffstream(st);
	return ff_index_search_timestamp(sti->index_entries, sti->nb_index_entries, wanted_timestamp, flags);
}
