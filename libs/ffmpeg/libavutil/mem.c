/*
 * default memory allocator for libavutil
 * Copyright (c) 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* default memory allocator for libavutil */

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "mem.h"

static const intptr_t max_alloc_size = INT_MAX;

static int size_mult(size_t a, size_t b, size_t* r)
{
	size_t t;

	t = a * b;
	/* Hack inspired from glibc: don't try the division if nelem and elsize
	 * are both less than sqrt(SIZE_MAX). */
	if ((a | b) >= ((size_t)1 << (sizeof(size_t) * 4)) && a && t / a != b)
		return AVERROR(EINVAL);

	*r = t;
	return 0;
}

void* av_malloc(size_t size)
{
	void* ptr = NULL;

	if (size > (size_t)max_alloc_size)
		return NULL;

	ptr = malloc(size);
	if (!ptr && !size)
	{
		size = 1;
		ptr = av_malloc(1);
	}
	return ptr;
}

void* av_realloc(void* ptr, size_t size)
{
	void* ret;
	if (size > (size_t)max_alloc_size)
		return NULL;

	ret = realloc(ptr, size + !size);
	return ret;
}

void* av_realloc_f(void* ptr, size_t nelem, size_t elsize)
{
	size_t size;
	void* r;

	if (size_mult(elsize, nelem, &size))
	{
		av_free(ptr);
		return NULL;
	}
	r = av_realloc(ptr, size);
	if (!r)
		av_free(ptr);
	return r;
}

int av_reallocp(void* ptr, size_t size)
{
	void* val;

	if (!size)
	{
		av_freep(ptr);
		return 0;
	}

	memcpy(&val, ptr, sizeof(val));
	val = av_realloc(val, size);

	if (!val)
	{
		av_freep(ptr);
		return AVERROR(ENOMEM);
	}

	memcpy(ptr, &val, sizeof(val));
	return 0;
}

void* av_malloc_array(size_t nmemb, size_t size)
{
	size_t result;
	if (size_mult(nmemb, size, &result) < 0)
		return NULL;
	return av_malloc(result);
}

void* av_realloc_array(void* ptr, size_t nmemb, size_t size)
{
	size_t result;
	if (size_mult(nmemb, size, &result) < 0)
		return NULL;
	return av_realloc(ptr, result);
}

void av_free(void* ptr)
{
	free(ptr);
}

void av_freep(void* arg)
{
	void* val;

	memcpy(&val, arg, sizeof(val));
	memcpy(arg, &(void*){ NULL }, sizeof(val));
	av_free(val);
}

void* av_mallocz(size_t size)
{
	void* ptr = av_malloc(size);
	if (ptr)
		memset(ptr, 0, size);
	return ptr;
}

void* av_calloc(size_t nmemb, size_t size)
{
	size_t result;
	if (size_mult(nmemb, size, &result) < 0)
		return NULL;
	return av_mallocz(result);
}

void* av_memdup(const void* p, size_t size)
{
	void* ptr = NULL;
	if (p)
	{
		ptr = av_malloc(size);
		if (ptr)
			memcpy(ptr, p, size);
	}
	return ptr;
}

void* av_fast_realloc(void* ptr, unsigned int* size, size_t min_size)
{
	size_t max_size;

	if (min_size <= *size)
		return ptr;

	max_size = max_alloc_size;
	/* *size is an unsigned, so the real maximum is <= UINT_MAX. */
	max_size = FFMIN(max_size, UINT_MAX);

	if (min_size > max_size)
	{
		*size = 0;
		return NULL;
	}

	min_size = FFMIN(max_size, FFMAX(min_size + min_size / 16 + 32, min_size));

	ptr = av_realloc(ptr, min_size);
	/* we could set this to the unmodified min_size but this is safer
	 * if the user lost the ptr and uses NULL now
	 */
	if (!ptr)
		min_size = 0;

	*size = (unsigned int)min_size;

	return ptr;
}
