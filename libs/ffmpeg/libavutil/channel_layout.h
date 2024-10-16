/*
 * Copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
 * Copyright (c) 2008 Peter Ross
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVUTIL_CHANNEL_LAYOUT_H
#define AVUTIL_CHANNEL_LAYOUT_H

#include <stdint.h>

#include "version.h"

/* Public libavutil channel layout APIs header. */

enum AVChannel
{
	// Invalid channel index
	AV_CHAN_NONE = -1,
	AV_CHAN_FRONT_LEFT,
	AV_CHAN_FRONT_RIGHT,
	AV_CHAN_FRONT_CENTER,

	/* Channel is empty can be safely skipped. */
	AV_CHAN_UNUSED = 0x200,

	/* Channel contains data, but its position is unknown. */
	AV_CHAN_UNKNOWN = 0x300,
};

enum AVChannelOrder
{
	/* Only the channel count is specified, without any further information about the channel order. */
	AV_CHANNEL_ORDER_UNSPEC,
	/* The native channel order */
	AV_CHANNEL_ORDER_NATIVE,
	/* Number of channel orders, not part of ABI/API */
	FF_CHANNEL_ORDER_NB
};

/* A channel layout is a 64-bits integer with a bit set for every channel. */

#define AV_CH_FRONT_LEFT   (1ULL << AV_CHAN_FRONT_LEFT           )
#define AV_CH_FRONT_RIGHT  (1ULL << AV_CHAN_FRONT_RIGHT          )
#define AV_CH_FRONT_CENTER (1ULL << AV_CHAN_FRONT_CENTER         )

#define AV_CH_LAYOUT_MONO   (AV_CH_FRONT_CENTER)
#define AV_CH_LAYOUT_STEREO (AV_CH_FRONT_LEFT|AV_CH_FRONT_RIGHT)

/* An AVChannelLayout holds information about the channel layout of audio data. */
typedef struct AVChannelLayout
{
	/* Channel order used in this layout. */
	enum AVChannelOrder order;

	/* Number of channels in this layout. Mandatory field. */
	int nb_channels;

	/* Details about which channels are present in this layout. */
	union
	{
		/* This member must be used for AV_CHANNEL_ORDER_NATIVE. */
		uint64_t mask;
	} u;
} AVChannelLayout;

/* Macro to define native channel layouts */
#define AV_CHANNEL_LAYOUT_MASK(nb, m) \
    { /* .order */ AV_CHANNEL_ORDER_NATIVE, \
      /* .nb_channels */  (nb), \
      /* .u.mask */ { m } }

/* Common pre-defined channel layouts */

#define AV_CHANNEL_LAYOUT_MONO   AV_CHANNEL_LAYOUT_MASK(1,  AV_CH_LAYOUT_MONO)
#define AV_CHANNEL_LAYOUT_STEREO AV_CHANNEL_LAYOUT_MASK(2,  AV_CH_LAYOUT_STEREO)

/* Get the default channel layout for a given number of channels. */
void av_channel_layout_default(AVChannelLayout* ch_layout, int nb_channels);

/* Free any allocated data in the channel layout and reset the channel count to 0. */
void av_channel_layout_uninit(AVChannelLayout* channel_layout);

/* Make a copy of a channel layout. */
int av_channel_layout_copy(AVChannelLayout* dst, const AVChannelLayout* src);

/* Get the channel with the given index in a channel layout. */
enum AVChannel av_channel_layout_channel_from_index(const AVChannelLayout* channel_layout, unsigned int idx);

/* Check whether a channel layout is valid, i.e. can possibly describe audio data. */
int av_channel_layout_check(const AVChannelLayout* channel_layout);

/* Check whether two channel layouts are semantically the same. */
int av_channel_layout_compare(const AVChannelLayout* chl, const AVChannelLayout* chl1);

#endif /* AVUTIL_CHANNEL_LAYOUT_H */
