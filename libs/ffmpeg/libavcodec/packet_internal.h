/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVCODEC_PACKET_INTERNAL_H
#define AVCODEC_PACKET_INTERNAL_H

#include <stdint.h>

#include "packet.h"

#define AVPACKET_IS_EMPTY(pkt) (!(pkt)->data)

typedef struct PacketListEntry
{
	struct PacketListEntry* next;
	AVPacket pkt;
} PacketListEntry;

typedef struct PacketList
{
	PacketListEntry* head, * tail;
} PacketList;

/* Append an AVPacket to the list. */
int avpriv_packet_list_put(PacketList* list, AVPacket* pkt,
	int (*copy)(AVPacket* dst, const AVPacket* src),
	int flags);

/* Remove the oldest AVPacket in the list and return it. */
int avpriv_packet_list_get(PacketList* list, AVPacket* pkt);

/* Wipe the list and unref all the packets in it. */
void avpriv_packet_list_free(PacketList* list);

#endif // AVCODEC_PACKET_INTERNAL_H
