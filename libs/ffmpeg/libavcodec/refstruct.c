/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdint.h>
#include <string.h>

#include "refstruct.h"

#include "libavutil/mem.h"
#include "libavutil/mem_internal.h"

#define REFCOUNT_OFFSET FFALIGN(sizeof(RefCount), ALIGN_64)

typedef struct RefCount
{
	intptr_t  refcount;
	FFRefStructOpaque opaque;
	void (*free_cb)(FFRefStructOpaque opaque, void* obj);
	void (*free)(void* ref);
} RefCount;

static RefCount* get_refcount(void* obj)
{
	RefCount* ref = (RefCount*)((char*)obj - REFCOUNT_OFFSET);
	return ref;
}

static void* get_userdata(void* buf)
{
	return (char*)buf + REFCOUNT_OFFSET;
}

static void refcount_init(RefCount* ref, FFRefStructOpaque opaque, void (*free_cb)(FFRefStructOpaque opaque, void* obj))
{
	ref->refcount = 1;
	ref->opaque = opaque;
	ref->free_cb = free_cb;
	ref->free = av_free;
}

void* ff_refstruct_alloc_ext_c(size_t size, unsigned flags, FFRefStructOpaque opaque,
	void (*free_cb)(FFRefStructOpaque opaque, void* obj))
{
	void* buf, * obj;

	if (size > SIZE_MAX - REFCOUNT_OFFSET)
		return NULL;
	buf = av_malloc(size + REFCOUNT_OFFSET);
	if (!buf)
		return NULL;
	refcount_init(buf, opaque, free_cb);
	obj = get_userdata(buf);
	if (!(flags & FF_REFSTRUCT_FLAG_NO_ZEROING))
		memset(obj, 0, size);

	return obj;
}

void ff_refstruct_unref(void* objp)
{
	void* obj;
	RefCount* ref;

	memcpy(&obj, objp, sizeof(obj));
	if (!obj)
		return;
	memcpy(objp, &(void*){ NULL }, sizeof(obj));

	ref = get_refcount(obj);
	if ((ref->refcount--) == 1)
	{
		if (ref->free_cb)
			ref->free_cb(ref->opaque, obj);
		ref->free(ref);
	}

	return;
}
