/*
 * Bitstream filters public API
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVCODEC_BSF_H
#define AVCODEC_BSF_H

#include "libavutil/dict.h"
#include "libavutil/log.h"
#include "libavutil/rational.h"

#include "codec_id.h"
#include "codec_par.h"
#include "packet.h"

/* The bitstream filter state. */
typedef struct AVBSFContext
{
	/* The bitstream filter this context is an instance of. */
	const struct AVBitStreamFilter* filter;

	/* Opaque filter-specific private data. */
	void* priv_data;

	/* Parameters of the input stream. */
	AVCodecParameters* par_in;

	/* Parameters of the output stream. */
	AVCodecParameters* par_out;

	/* The timebase used for the timestamps of the input packets. */
	AVRational time_base_in;

	/* The timebase used for the timestamps of the output packets. */
	AVRational time_base_out;
} AVBSFContext;

typedef struct AVBitStreamFilter
{
	const char* name;

	/* A list of codec ids supported by the filter, terminated by AV_CODEC_ID_NONE. */
	const enum AVCodecID* codec_ids;
} AVBitStreamFilter;

/* Return a bitstream filter with the specified name or NULL if no such bitstream filter exists. */
const AVBitStreamFilter* av_bsf_get_by_name(const char* name);

/* Iterate over all registered bitstream filters. */
const AVBitStreamFilter* av_bsf_iterate(void** opaque);

/* Allocate a context for a given bitstream filter. */
int av_bsf_alloc(const AVBitStreamFilter* filter, AVBSFContext** ctx);

/* Prepare the filter for use, after all the parameters and options have been set. */
int av_bsf_init(AVBSFContext* ctx);

/* Submit a packet for filtering. */
int av_bsf_send_packet(AVBSFContext* ctx, AVPacket* pkt);

/* Retrieve a filtered packet. */
int av_bsf_receive_packet(AVBSFContext* ctx, AVPacket* pkt);

/* Reset the internal bitstream filter state. Should be called e.g. when seeking. */
void av_bsf_flush(AVBSFContext* ctx);

/* Free a bitstream filter context and everything associated with it. */
void av_bsf_free(AVBSFContext** ctx);

/*
 * Parse string describing list of bitstream filters and create single
 * AVBSFContext describing the whole chain of bitstream filters.
 */
int av_bsf_list_parse_str(const char* str, AVBSFContext** bsf);

/* Get null/pass-through bitstream filter. */
int av_bsf_get_null_filter(AVBSFContext** bsf);

#endif // AVCODEC_BSF_H
