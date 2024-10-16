/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVCODEC_REFSTRUCT_H
#define AVCODEC_REFSTRUCT_H

#include <stddef.h>

/* RefStruct is an API for creating reference-counted objects with minimal overhead. */

/**
 * This union is used for all opaque parameters in this API to spare the user
 * to cast const away in case the opaque to use is const-qualified.
 */
typedef union
{
	void* nc;
} FFRefStructOpaque;

/* If this flag is set in ff_refstruct_alloc_ext_c(), the object will not be initially zeroed. */
#define FF_REFSTRUCT_FLAG_NO_ZEROING (1 << 0)

/* Allocate a refcounted object of usable size `size` managed via the RefStruct API. */
void* ff_refstruct_alloc_ext_c(size_t size, unsigned flags, FFRefStructOpaque opaque,
	void (*free_cb)(FFRefStructOpaque opaque, void* obj));

/* A wrapper around ff_refstruct_alloc_ext_c() for the common case of a non-const qualified opaque. */
static inline
void* ff_refstruct_alloc_ext(size_t size, unsigned flags, void* opaque,
	void (*free_cb)(FFRefStructOpaque opaque, void* obj))
{
	return ff_refstruct_alloc_ext_c(size, flags, (FFRefStructOpaque){.nc = opaque}, free_cb);
}

/*
 * Decrement the reference count of the underlying object and automatically
 * free the object if there are no more references to it.
 */
void ff_refstruct_unref(void* objp);

#endif /* AVCODEC_REFSTRUCT_H */
