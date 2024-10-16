/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVUTIL_BUFFER_INTERNAL_H
#define AVUTIL_BUFFER_INTERNAL_H

#include <stdint.h>

#include "buffer.h"

/* The buffer was av_realloc()ed, so it is reallocatable. */
#define BUFFER_FLAG_REALLOCATABLE (1 << 0)
/* The AVBuffer structure is part of a larger structure and should not be freed. */
#define BUFFER_FLAG_NO_FREE       (1 << 1)

struct AVBuffer
{
	uint8_t* data; /* data described by this buffer */
	size_t size; /* size of data in bytes */

	/* number of existing AVBufferRef instances referring to this buffer */
	intptr_t refcount;

	/* a callback for freeing the data */
	void (*free)(void* opaque, uint8_t* data);

	/* an opaque pointer, to be used by the freeing callback */
	void* opaque;

	/* A combination of AV_BUFFER_FLAG_* */
	int flags;

	/* A combination of BUFFER_FLAG_* */
	int flags_internal;
};

typedef struct BufferPoolEntry
{
	uint8_t* data;

	/*
	 * Backups of the original opaque/free of the AVBuffer corresponding to
	 * data. They will be used to free the buffer when the pool is freed.
	 */
	void* opaque;
	void (*free)(void* opaque, uint8_t* data);

	AVBufferPool* pool;
	struct BufferPoolEntry* next;

	/* An AVBuffer structure to (re)use as AVBuffer for subsequent uses of this BufferPoolEntry. */
	AVBuffer buffer;
} BufferPoolEntry;

struct AVBufferPool
{
	BufferPoolEntry* pool;

	/* This is used to track when the pool is to be freed. */
	intptr_t refcount;

	size_t size;
	void* opaque;
	AVBufferRef* (*alloc)(size_t size);
	AVBufferRef* (*alloc2)(void* opaque, size_t size);
	void         (*pool_free)(void* opaque);
};

#endif /* AVUTIL_BUFFER_INTERNAL_H */
