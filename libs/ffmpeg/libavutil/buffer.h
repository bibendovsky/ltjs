/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* refcounted data buffer API */

#ifndef AVUTIL_BUFFER_H
#define AVUTIL_BUFFER_H

#include <stddef.h>
#include <stdint.h>

/* A reference counted buffer type. */
typedef struct AVBuffer AVBuffer;

/* A reference to a data buffer. */
typedef struct AVBufferRef
{
	AVBuffer* buffer;

	/* The data buffer. */
	uint8_t* data;
	/* Size of data in bytes. */
	size_t size;
} AVBufferRef;

/* Allocate an AVBuffer of the given size using av_malloc(). */
AVBufferRef* av_buffer_alloc(size_t size);

/* Same as av_buffer_alloc(), except the returned buffer will be initialized to zero. */
AVBufferRef* av_buffer_allocz(size_t size);

/* Create an AVBuffer from an existing array. */
AVBufferRef* av_buffer_create(uint8_t* data, size_t size, void (*free)(void* opaque, uint8_t* data), void* opaque, int flags);

/* Default free callback, which calls av_free() on the buffer data. */
void av_buffer_default_free(void* opaque, uint8_t* data);

/* Create a new reference to an AVBuffer. */
AVBufferRef* av_buffer_ref(const AVBufferRef* buf);

/* Free a given reference and automatically free the buffer if there are no more references to it. */
void av_buffer_unref(AVBufferRef** buf);

/*
 * @return 1 if the caller may write to the data referred to by buf (which is
 * true if and only if buf is the only reference to the underlying AVBuffer).
 * Return 0 otherwise.
 */
int av_buffer_is_writable(const AVBufferRef* buf);

/* Reallocate a given buffer. */
int av_buffer_realloc(AVBufferRef** buf, size_t size);

/* Ensure dst refers to the same data as src. */
int av_buffer_replace(AVBufferRef** dst, const AVBufferRef* src);

/* The buffer pool. */
typedef struct AVBufferPool AVBufferPool;

/* Allocate and initialize a buffer pool. */
AVBufferPool* av_buffer_pool_init(size_t size, AVBufferRef* (*alloc)(size_t size));

/* Mark the pool as being available for freeing. */
void av_buffer_pool_uninit(AVBufferPool** pool);

/* Allocate a new AVBuffer, reusing an old buffer from the pool when available. */
AVBufferRef* av_buffer_pool_get(AVBufferPool* pool);

#endif /* AVUTIL_BUFFER_H */
