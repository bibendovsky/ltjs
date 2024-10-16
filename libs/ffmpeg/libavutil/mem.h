/*
 * copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Memory handling functions */

#ifndef AVUTIL_MEM_H
#define AVUTIL_MEM_H

#include <stddef.h>
#include <stdint.h>

/* Allocate a memory block with alignment suitable for all memory accesses. */
void* av_malloc(size_t size);

/*
 * Allocate a memory block with alignment suitable for all memory accesses
 * (including vectors if available on the CPU) and zero all the bytes of the
 * block.
 */
void* av_mallocz(size_t size);

/* Allocate a memory block for an array with av_malloc(). */
void* av_malloc_array(size_t nmemb, size_t size);

/* Allocate a memory block for an array with av_mallocz(). */
void* av_calloc(size_t nmemb, size_t size);

/* Allocate, reallocate, or free a block of memory. */
void* av_realloc(void* ptr, size_t size);

/* Allocate, reallocate, or free a block of memory through a pointer to a pointer. */
int av_reallocp(void* ptr, size_t size);

/* Allocate, reallocate, or free a block of memory. */
void* av_realloc_f(void* ptr, size_t nelem, size_t elsize);

/* Allocate, reallocate, or free an array. */
void* av_realloc_array(void* ptr, size_t nmemb, size_t size);

/* Reallocate the given buffer if it is not large enough, otherwise do nothing. */
void* av_fast_realloc(void* ptr, unsigned int* size, size_t min_size);

/* Free a memory block which has been allocated with a function of av_malloc() or av_realloc() family. */
void av_free(void* ptr);

/*
 * Free a memory block which has been allocated with a function of av_malloc()
 * or av_realloc() family, and set the pointer pointing to it to `NULL`.
 */
void av_freep(void* ptr);

/* Duplicate a buffer with av_malloc(). */
void* av_memdup(const void* p, size_t size);

#endif /* AVUTIL_MEM_H */
