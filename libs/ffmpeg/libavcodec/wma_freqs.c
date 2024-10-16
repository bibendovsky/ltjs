/*
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdint.h>

#include "wma_freqs.h"

const uint16_t ff_wma_critical_freqs[25] = {
	  100,   200,  300,  400,  510,  630,   770,   920,
	 1080,  1270, 1480, 1720, 2000, 2320,  2700,  3150,
	 3700,  4400, 5300, 6400, 7700, 9500, 12000, 15500,
	24500,
};
