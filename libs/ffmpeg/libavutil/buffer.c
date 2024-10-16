/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdint.h>
#include <string.h>

#include "avassert.h"
#include "buffer_internal.h"
#include "common.h"
#include "mem.h"

static AVBufferRef* buffer_create(AVBuffer* buf, uint8_t* data, size_t size,
	void (*free)(void* opaque, uint8_t* data),
	void* opaque, int flags)
{
	AVBufferRef* ref = NULL;

	buf->data = data;
	buf->size = size;
	buf->free = free ? free : av_buffer_default_free;
	buf->opaque = opaque;

	buf->refcount = 1;

	buf->flags = flags;

	ref = av_mallocz(sizeof(*ref));
	if (!ref)
		return NULL;

	ref->buffer = buf;
	ref->data = data;
	ref->size = size;

	return ref;
}

AVBufferRef* av_buffer_create(uint8_t* data, size_t size,
	void (*free)(void* opaque, uint8_t* data),
	void* opaque, int flags)
{
	AVBufferRef* ret;
	AVBuffer* buf = av_mallocz(sizeof(*buf));
	if (!buf)
		return NULL;

	ret = buffer_create(buf, data, size, free, opaque, flags);
	if (!ret)
	{
		av_free(buf);
		return NULL;
	}
	return ret;
}

void av_buffer_default_free(void* opaque, uint8_t* data)
{
	(void)opaque;

	av_free(data);
}

AVBufferRef* av_buffer_alloc(size_t size)
{
	AVBufferRef* ret = NULL;
	uint8_t* data = NULL;

	data = av_malloc(size);
	if (!data)
		return NULL;

	ret = av_buffer_create(data, size, av_buffer_default_free, NULL, 0);
	if (!ret)
		av_freep(&data);

	return ret;
}

AVBufferRef* av_buffer_allocz(size_t size)
{
	AVBufferRef* ret = av_buffer_alloc(size);
	if (!ret)
		return NULL;

	memset(ret->data, 0, size);
	return ret;
}

AVBufferRef* av_buffer_ref(const AVBufferRef* buf)
{
	AVBufferRef* ret = av_mallocz(sizeof(*ret));

	if (!ret)
		return NULL;

	*ret = *buf;
	buf->buffer->refcount++;
	return ret;
}

static void buffer_replace(AVBufferRef** dst, AVBufferRef** src)
{
	AVBuffer* b;

	b = (*dst)->buffer;

	if (src)
	{
		**dst = **src;
		av_freep(src);
	}
	else
		av_freep(dst);

	if ((b->refcount--) == 1)
	{
		/* b->free below might already free the structure containing *b,
		 * so we have to read the flag now to avoid use-after-free. */
		int free_avbuffer = !(b->flags_internal & BUFFER_FLAG_NO_FREE);
		b->free(b->opaque, b->data);
		if (free_avbuffer)
			av_free(b);
	}
}

void av_buffer_unref(AVBufferRef** buf)
{
	if (!buf || !*buf)
		return;

	buffer_replace(buf, NULL);
}

int av_buffer_is_writable(const AVBufferRef* buf)
{
	return buf->buffer->refcount == 1;
}

int av_buffer_realloc(AVBufferRef** pbuf, size_t size)
{
	AVBufferRef* buf = *pbuf;
	uint8_t* tmp;
	int ret;

	if (!buf)
	{
		/* allocate a new buffer with av_realloc(), so it will be reallocatable later */
		uint8_t* data = av_realloc(NULL, size);
		if (!data)
			return AVERROR(ENOMEM);

		buf = av_buffer_create(data, size, av_buffer_default_free, NULL, 0);
		if (!buf)
		{
			av_freep(&data);
			return AVERROR(ENOMEM);
		}

		buf->buffer->flags_internal |= BUFFER_FLAG_REALLOCATABLE;
		*pbuf = buf;

		return 0;
	}
	else if (buf->size == size)
		return 0;

	if (!(buf->buffer->flags_internal & BUFFER_FLAG_REALLOCATABLE) ||
		!av_buffer_is_writable(buf) || buf->data != buf->buffer->data)
	{
		/* cannot realloc, allocate a new reallocable buffer and copy data */
		AVBufferRef* new = NULL;

		ret = av_buffer_realloc(&new, size);
		if (ret < 0)
			return ret;

		memcpy(new->data, buf->data, FFMIN(size, buf->size));

		buffer_replace(pbuf, &new);
		return 0;
	}

	tmp = av_realloc(buf->buffer->data, size);
	if (!tmp)
		return AVERROR(ENOMEM);

	buf->buffer->data = buf->data = tmp;
	buf->buffer->size = buf->size = size;
	return 0;
}

int av_buffer_replace(AVBufferRef** pdst, const AVBufferRef* src)
{
	AVBufferRef* dst = *pdst;
	AVBufferRef* tmp;

	if (!src)
	{
		av_buffer_unref(pdst);
		return 0;
	}

	if (dst && dst->buffer == src->buffer)
	{
		/* make sure the data pointers match */
		dst->data = src->data;
		dst->size = src->size;
		return 0;
	}

	tmp = av_buffer_ref(src);
	if (!tmp)
		return AVERROR(ENOMEM);

	av_buffer_unref(pdst);
	*pdst = tmp;
	return 0;
}

AVBufferPool* av_buffer_pool_init(size_t size, AVBufferRef* (*alloc)(size_t size))
{
	AVBufferPool* pool = av_mallocz(sizeof(*pool));
	if (!pool)
		return NULL;

	pool->size = size;
	pool->alloc = alloc ? alloc : av_buffer_alloc;

	pool->refcount = 1;

	return pool;
}

static void buffer_pool_flush(AVBufferPool* pool)
{
	while (pool->pool)
	{
		BufferPoolEntry* buf = pool->pool;
		pool->pool = buf->next;

		buf->free(buf->opaque, buf->data);
		av_freep(&buf);
	}
}

/* This function gets called when the pool has been uninited and all the buffers returned to it. */
static void buffer_pool_free(AVBufferPool* pool)
{
	buffer_pool_flush(pool);

	if (pool->pool_free)
		pool->pool_free(pool->opaque);

	av_freep(&pool);
}

void av_buffer_pool_uninit(AVBufferPool** ppool)
{
	AVBufferPool* pool;

	if (!ppool || !*ppool)
		return;
	pool = *ppool;
	*ppool = NULL;

	buffer_pool_flush(pool);

	if ((pool->refcount--) == 1)
		buffer_pool_free(pool);
}

static void pool_release_buffer(void* opaque, uint8_t* data)
{
	(void)data;

	BufferPoolEntry* buf = opaque;
	AVBufferPool* pool = buf->pool;

	buf->next = pool->pool;
	pool->pool = buf;

	if ((pool->refcount--) == 1)
		buffer_pool_free(pool);
}

/* allocate a new buffer and override its free() callback so that
 * it is returned to the pool on free */
static AVBufferRef* pool_alloc_buffer(AVBufferPool* pool)
{
	BufferPoolEntry* buf;
	AVBufferRef* ret;

	av_assert0(pool->alloc || pool->alloc2);

	ret = pool->alloc2 ? pool->alloc2(pool->opaque, pool->size) :
		pool->alloc(pool->size);
	if (!ret)
		return NULL;

	buf = av_mallocz(sizeof(*buf));
	if (!buf)
	{
		av_buffer_unref(&ret);
		return NULL;
	}

	buf->data = ret->buffer->data;
	buf->opaque = ret->buffer->opaque;
	buf->free = ret->buffer->free;
	buf->pool = pool;

	ret->buffer->opaque = buf;
	ret->buffer->free = pool_release_buffer;

	return ret;
}

AVBufferRef* av_buffer_pool_get(AVBufferPool* pool)
{
	AVBufferRef* ret;
	BufferPoolEntry* buf;

	buf = pool->pool;
	if (buf)
	{
		memset(&buf->buffer, 0, sizeof(buf->buffer));
		ret = buffer_create(&buf->buffer, buf->data, pool->size, pool_release_buffer, buf, 0);
		if (ret)
		{
			pool->pool = buf->next;
			buf->next = NULL;
			buf->buffer.flags_internal |= BUFFER_FLAG_NO_FREE;
		}
	}
	else
	{
		ret = pool_alloc_buffer(pool);
	}

	if (ret)
		pool->refcount++;

	return ret;
}
