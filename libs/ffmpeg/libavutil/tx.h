/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVUTIL_TX_H
#define AVUTIL_TX_H

#include <stdint.h>
#include <stddef.h>

typedef struct AVTXContext AVTXContext;

typedef struct AVComplexFloat
{
	float re, im;
} AVComplexFloat;

typedef struct AVComplexDouble
{
	double re, im;
} AVComplexDouble;

typedef struct AVComplexInt32
{
	int32_t re, im;
} AVComplexInt32;

enum AVTXType
{
	/*
	 * Standard complex to complex FFT with sample data type of AVComplexFloat,
	 * AVComplexDouble or AVComplexInt32, for each respective variant.
	 */
	AV_TX_FLOAT_FFT = 0,
	AV_TX_DOUBLE_FFT = 2,
	AV_TX_INT32_FFT = 4,

	/*
	 * Standard MDCT with a sample data type of float, double or int32_t,
	 * respecively. For the float and int32 variants, the scale type is
	 * 'float', while for the double variant, it's 'double'.
	 * If scale is NULL, 1.0 will be used as a default.
	 */
	AV_TX_FLOAT_MDCT = 1,
	AV_TX_DOUBLE_MDCT = 3,
	AV_TX_INT32_MDCT = 5,

	/*
	 * Real to complex and complex to real DFTs.
	 * For the float and int32 variants, the scale type is 'float', while for
	 * the double variant, it's a 'double'. If scale is NULL, 1.0 will be used
	 * as a default.
	 */
	AV_TX_FLOAT_RDFT = 6,
	AV_TX_DOUBLE_RDFT = 7,
	AV_TX_INT32_RDFT = 8,

	/*
	 * Real to real (DCT) transforms.
	 *
	 * The forward transform is a DCT-II.
	 * The inverse transform is a DCT-III.
	 */
	AV_TX_FLOAT_DCT = 9,
	AV_TX_DOUBLE_DCT = 10,
	AV_TX_INT32_DCT = 11,

	/*
	 * Discrete Cosine Transform I
	 *
	 * The forward transform is a DCT-I.
	 * The inverse transform is a DCT-I multiplied by 2/(N + 1).
	 */
	AV_TX_FLOAT_DCT_I = 12,
	AV_TX_DOUBLE_DCT_I = 13,
	AV_TX_INT32_DCT_I = 14,

	/*
	 * Discrete Sine Transform I
	 *
	 * The forward transform is a DST-I.
	 * The inverse transform is a DST-I multiplied by 2/(N + 1).
	 */
	AV_TX_FLOAT_DST_I = 15,
	AV_TX_DOUBLE_DST_I = 16,
	AV_TX_INT32_DST_I = 17,

	/* Not part of the API, do not use */
	AV_TX_NB,
};

/* Function pointer to a function to perform the transform. */
typedef void (*av_tx_fn)(AVTXContext* s, void* out, void* in, ptrdiff_t stride);

/* Flags for av_tx_init() */
enum AVTXFlags
{
	/* Allows for in-place transformations, where input == output. */
	AV_TX_INPLACE = 1ULL << 0,

	/* Relaxes alignment requirement for the in and out arrays of av_tx_fn(). */
	AV_TX_UNALIGNED = 1ULL << 1,

	/* Performs a full inverse MDCT rather than leaving out samples that can be derived through symmetry. */
	AV_TX_FULL_IMDCT = 1ULL << 2,

	/* Perform a real to half-complex RDFT. */
	AV_TX_REAL_TO_REAL = 1ULL << 3,
	AV_TX_REAL_TO_IMAGINARY = 1ULL << 4,
};

/*
 * Initialize a transform context with the given configuration
 * (i)MDCTs with an odd length are currently not supported.
 */
int av_tx_init(AVTXContext** ctx, av_tx_fn* tx, enum AVTXType type,
	int inv, int len, const void* scale, uint64_t flags);

/* Frees a context and sets *ctx to NULL, does nothing when *ctx == NULL. */
void av_tx_uninit(AVTXContext** ctx);

#endif /* AVUTIL_TX_H */
