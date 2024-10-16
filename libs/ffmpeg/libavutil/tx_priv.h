/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVUTIL_TX_PRIV_H
#define AVUTIL_TX_PRIV_H

#include "tx.h"
#include "mem_internal.h"
#include "common.h"

#ifdef TX_FLOAT
#define TX_TAB(x) x ## _float
#define TX_NAME(x) x ## _float_c
#define TX_NAME_STR(x) x "_float_c"
#define TX_TYPE(x) AV_TX_FLOAT_ ## x
#define TX_FN_NAME(fn, suffix) ff_tx_ ## fn ## _float_ ## suffix
#define TX_FN_NAME_STR(fn, suffix) #fn "_float_" #suffix
#define MULT(x, m) ((x) * (m))
#define SCALE_TYPE float
typedef float TXSample;
typedef float TXUSample;
typedef AVComplexFloat TXComplex;
#else
typedef void TXComplex;
#endif

#define TX_DECL_FN(fn, suffix) \
    void TX_FN_NAME(fn, suffix)(AVTXContext *s, void *o, void *i, ptrdiff_t st);

#if defined(TX_FLOAT)

#define CMUL(dre, dim, are, aim, bre, bim)      \
    do {                                        \
        (dre) = (are) * (bre) - (aim) * (bim);  \
        (dim) = (are) * (bim) + (aim) * (bre);  \
    } while (0)

#define SMUL(dre, dim, are, aim, bre, bim)      \
    do {                                        \
        (dre) = (are) * (bre) - (aim) * (bim);  \
        (dim) = (are) * (bim) - (aim) * (bre);  \
    } while (0)

#define UNSCALE(x) (x)
#define RESCALE(x) (x)

#define FOLD(a, b) ((a) + (b))

#define BF(x, y, a, b)  \
    do {                \
        x = (a) - (b);  \
        y = (a) + (b);  \
    } while (0)

#endif /* TX_FLOAT */

#define CMUL3(c, a, b) CMUL((c).re, (c).im, (a).re, (a).im, (b).re, (b).im)

/* Codelet flags, used to pick codelets. Must be a superset of enum AVTXFlags,
 * but if it runs out of bits, it can be made separate. */
#define FF_TX_OUT_OF_PLACE (1ULL << 63) /* Can be OR'd with AV_TX_INPLACE             */
#define FF_TX_ALIGNED      (1ULL << 62) /* Cannot be OR'd with AV_TX_UNALIGNED        */
#define FF_TX_PRESHUFFLE   (1ULL << 61) /* Codelet expects permuted coeffs            */
#define FF_TX_INVERSE_ONLY (1ULL << 60) /* For non-orthogonal inverse-only transforms */
#define FF_TX_FORWARD_ONLY (1ULL << 59) /* For non-orthogonal forward-only transforms */
#define FF_TX_ASM_CALL     (1ULL << 58) /* For asm->asm functions only                */

typedef enum FFTXCodeletPriority
{
	FF_TX_PRIO_BASE = 0,               /* Baseline priority */
	FF_TX_PRIO_MIN = -131072, /* For naive implementations */
	FF_TX_PRIO_MAX = 32768,  /* For custom implementations/ASICs */
} FFTXCodeletPriority;

typedef enum FFTXMapDirection
{
	/* No map. Make a map up. */
	FF_TX_MAP_NONE = 0,

	/* Lookup table must be applied via dst[i] = src[lut[i]]; */
	FF_TX_MAP_GATHER,

	/* Lookup table must be applied via dst[lut[i]] = src[i]; */
	FF_TX_MAP_SCATTER,
} FFTXMapDirection;

/* Codelet options */
typedef struct FFTXCodeletOptions
{
	/* Request a specific lookup table direction. Codelets MUST put the
	 * direction in AVTXContext. If the codelet does not respect this, a
	 * conversion will be performed. */
	FFTXMapDirection map_dir;
} FFTXCodeletOptions;

/* Maximum number of factors a codelet may have. Arbitrary. */
#define TX_MAX_FACTORS 16

/* Maximum amount of subtransform functions, subtransforms and factors. Arbitrary. */
#define TX_MAX_SUB 4

/* Maximum number of returned results for ff_tx_decompose_length. Arbitrary. */
#define TX_MAX_DECOMPOSITIONS 512

typedef struct FFTXCodelet
{
	const char* name;          /* Codelet name, for debugging */
	av_tx_fn       function;   /* Codelet function, != NULL */
	enum AVTXType  type;       /* Type of codelet transform */
#define TX_TYPE_ANY INT32_MAX  /* Special type to allow all types */

	uint64_t flags;               /* A combination of AVTXFlags and codelet flags that describe its properties. */

	int factors[TX_MAX_FACTORS];  /* Length factors. MUST be coprime. */
#define TX_FACTOR_ANY -1          /* When used alone, signals that the codelet supports all factors. */

	int nb_factors;               /* Minimum number of factors that have to be a modulo of the length. Must not be 0. */

	int min_len;                  /* Minimum length of transform, must be >= 1 */
	int max_len;                  /* Maximum length of transform */
#define TX_LEN_UNLIMITED -1       /* Special length value to permit all lengths */

	int (*init)(AVTXContext* s,   /* Optional callback for current context initialization. */
		const struct FFTXCodelet* cd,
		uint64_t flags,
		FFTXCodeletOptions* opts,
		int len, int inv,
		const void* scale);

	int (*uninit)(AVTXContext* s); /* Optional callback for uninitialization. */

	int prio;                      /* < 0 = least, 0 = no pref, > 0 = prefer */
} FFTXCodelet;

struct AVTXContext
{
	/* Fields the root transform and subtransforms use or may use. */
	int len;        /* Length of the transform */
	int inv;        /* If transform is inverse */
	int* map;       /* Lookup table(s) */
	TXComplex* exp; /* Any non-pre-baked multiplication factors, or extra temporary buffer */
	TXComplex* tmp; /* Temporary buffer, if needed */

	AVTXContext* sub;        /* Subtransform context(s), if needed */
	av_tx_fn fn[TX_MAX_SUB]; /* Function(s) for the subtransforms */
	int nb_sub;              /* Number of subtransforms. */

	/* Fields mainly useul/applicable for the root transform or initialization.
	 * Fields below are not used by assembly code. */
	const FFTXCodelet* cd[TX_MAX_SUB];  /* Subtransform codelets */
	const FFTXCodelet* cd_self;         /* Codelet for the current context */
	enum AVTXType      type;            /* Type of transform */
	uint64_t           flags;           /* A combination of AVTXFlags and codelet flags used when creating */
	FFTXMapDirection   map_dir;         /* Direction of AVTXContext->map */
	float              scale_f;
	double             scale_d;
	void* opaque;          /* Free to use by implementations */
};

/* This function embeds a Ruritanian PFA input map into an existing lookup table to avoid double permutation. */
#define TX_EMBED_INPUT_PFA_MAP(map, tot_len, d1, d2)                             \
    do {                                                                         \
        int mtmp[(d1)*(d2)];                                                     \
        for (int k = 0; k < tot_len; k += (d1)*(d2)) {                           \
            memcpy(mtmp, &map[k], (d1)*(d2)*sizeof(*mtmp));                      \
            for (int m = 0; m < (d2); m++)                                       \
                for (int n = 0; n < (d1); n++)                                   \
                    map[k + m*(d1) + n] = mtmp[(m*(d1) + n*(d2)) % ((d1)*(d2))]; \
        }                                                                        \
    } while (0)

/* This function generates a Ruritanian PFA input map into s->map. */
int ff_tx_gen_pfa_input_map(AVTXContext* s, FFTXCodeletOptions* opts,
	int d1, int d2);

/* Create a subtransform in the current context with the given parameters. */
int ff_tx_init_subtx(AVTXContext* s, enum AVTXType type,
	uint64_t flags, FFTXCodeletOptions* opts,
	int len, int inv, const void* scale);

/* Clear the context by freeing all tables, maps and subtransforms. */
void ff_tx_clear_ctx(AVTXContext* s);

/* Attempt to factorize a length into 2 integers. */
int ff_tx_decompose_length(int dst[TX_MAX_DECOMPOSITIONS], enum AVTXType type, int len, int inv);

/* Generate a default map (0->len or 0, (len-1)->1 for inverse transforms) for a context. */
int ff_tx_gen_default_map(AVTXContext* s, FFTXCodeletOptions* opts);

/* Generates the PFA permutation table into AVTXContext->pfatab. */
int ff_tx_gen_compound_mapping(AVTXContext* s, FFTXCodeletOptions* opts, int inv, int n, int m);

/* Generates a standard-ish (slightly modified) Split-Radix revtab into AVTXContext->map. */
int ff_tx_gen_ptwo_revtab(AVTXContext* s, FFTXCodeletOptions* opts);

/* Generates an index into AVTXContext->inplace_idx. */
int ff_tx_gen_inplace_map(AVTXContext* s, int len);

/* This generates a parity-based revtab of length len and direction inv. */
int ff_tx_gen_split_radix_parity_revtab(AVTXContext* s, int len, int inv,
	FFTXCodeletOptions* opts,
	int basis, int dual_stride);

/* Typed init function to initialize an MDCT exptab in a context. */
int ff_tx_mdct_gen_exp_float(AVTXContext* s, int* pre_tab);

/* Lists of codelets */
extern const FFTXCodelet* const ff_tx_codelet_list_float_c[];

#endif /* AVUTIL_TX_PRIV_H */
