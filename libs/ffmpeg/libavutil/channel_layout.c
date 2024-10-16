/*
 * Copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* audio channel layout utility functions */

#include <string.h>

#include "channel_layout.h"
#include "common.h"

struct channel_layout_name
{
	const char* name;
	AVChannelLayout layout;
};

static const struct channel_layout_name channel_layout_map[] = {
	{"mono",   AV_CHANNEL_LAYOUT_MONO  },
	{"stereo", AV_CHANNEL_LAYOUT_STEREO},
};

void av_channel_layout_uninit(AVChannelLayout* channel_layout)
{
	memset(channel_layout, 0, sizeof(*channel_layout));
}

int av_channel_layout_copy(AVChannelLayout* dst, const AVChannelLayout* src)
{
	av_channel_layout_uninit(dst);
	*dst = *src;
	return 0;
}

enum AVChannel av_channel_layout_channel_from_index(const AVChannelLayout* channel_layout, unsigned int idx)
{
	if (idx >= (unsigned int)channel_layout->nb_channels)
		return AV_CHAN_NONE;

	if (channel_layout->order == AV_CHANNEL_ORDER_NATIVE)
	{
		for (int i = 0; i < 64; i++)
		{
			if ((1ULL << i) & channel_layout->u.mask && !idx--)
				return i;
		}
	}

	return AV_CHAN_NONE;
}

int av_channel_layout_check(const AVChannelLayout* channel_layout)
{
	if (channel_layout->nb_channels <= 0)
		return 0;

	switch (channel_layout->order)
	{
		case AV_CHANNEL_ORDER_NATIVE:
			return av_popcount64(channel_layout->u.mask) == channel_layout->nb_channels;
		case AV_CHANNEL_ORDER_UNSPEC:
			return 1;
		default:
			return 0;
	}
}

int av_channel_layout_compare(const AVChannelLayout* chl, const AVChannelLayout* chl1)
{
	int i;

	/* different channel counts -> not equal */
	if (chl->nb_channels != chl1->nb_channels)
		return 1;

	/* if only one is unspecified -> not equal */
	if ((chl->order == AV_CHANNEL_ORDER_UNSPEC) != (chl1->order == AV_CHANNEL_ORDER_UNSPEC))
		return 1;
	/* both are unspecified -> equal */
	else if (chl->order == AV_CHANNEL_ORDER_UNSPEC)
		return 0;

	/* can compare masks directly */
	if (chl->order == AV_CHANNEL_ORDER_NATIVE && chl->order == chl1->order)
		return chl->u.mask != chl1->u.mask;

	/* compare channel by channel */
	for (i = 0; i < chl->nb_channels; i++)
		if (av_channel_layout_channel_from_index(chl, i) != av_channel_layout_channel_from_index(chl1, i))
			return 1;
	return 0;
}

void av_channel_layout_default(AVChannelLayout* ch_layout, int nb_channels)
{
	for (size_t i = 0; i < FF_ARRAY_ELEMS(channel_layout_map); i++)
		if (nb_channels == channel_layout_map[i].layout.nb_channels)
		{
			*ch_layout = channel_layout_map[i].layout;
			return;
		}

	ch_layout->order = AV_CHANNEL_ORDER_UNSPEC;
	ch_layout->nb_channels = nb_channels;
}
