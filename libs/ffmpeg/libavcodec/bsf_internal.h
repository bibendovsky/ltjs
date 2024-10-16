/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVCODEC_BSF_INTERNAL_H
#define AVCODEC_BSF_INTERNAL_H

#include "libavutil/log.h"

#include "bsf.h"
#include "packet.h"

typedef struct FFBitStreamFilter
{
	/* The public AVBitStreamFilter. See bsf.h for it. */
	AVBitStreamFilter p;

	int priv_data_size;
	int (*init)(AVBSFContext* ctx);
	int (*filter)(AVBSFContext* ctx, AVPacket* pkt);
	void (*close)(AVBSFContext* ctx);
	void (*flush)(AVBSFContext* ctx);
} FFBitStreamFilter;

/* Called by bitstream filters to get packet for filtering. */
int ff_bsf_get_packet_ref(AVBSFContext* ctx, AVPacket* pkt);

#endif /* AVCODEC_BSF_INTERNAL_H */
