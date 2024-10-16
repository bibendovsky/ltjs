/*
 * copyright (c) 2012 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AVUTIL_QSORT_H
#define AVUTIL_QSORT_H

#include "macros.h"

/*
 * Quicksort
 * This sort is fast, and fully inplace but not stable and it is possible
 * to construct input that requires O(n^2) time but this is very unlikely to
 * happen with non constructed input.
 */
#define AV_QSORT(p, num, type, cmp) do {\
    void *stack[64][2];\
    int sp= 1;\
    stack[0][0] = p;\
    stack[0][1] = (p)+(num)-1;\
    while(sp){\
        type *start= stack[--sp][0];\
        type *end  = stack[  sp][1];\
        while(start < end){\
            if(start < end-1) {\
                int checksort=0;\
                type *right = end-2;\
                type *left  = start+1;\
                type *mid = start + ((end-start)>>1);\
                if(cmp(start, end) > 0) {\
                    if(cmp(  end, mid) > 0) FFSWAP(type, *start, *mid);\
                    else                    FFSWAP(type, *start, *end);\
                }else{\
                    if(cmp(start, mid) > 0) FFSWAP(type, *start, *mid);\
                    else checksort= 1;\
                }\
                if(cmp(mid, end) > 0){ \
                    FFSWAP(type, *mid, *end);\
                    checksort=0;\
                }\
                if(start == end-2) break;\
                FFSWAP(type, end[-1], *mid);\
                while(left <= right){\
                    while(left<=right && cmp(left, end-1) < 0)\
                        left++;\
                    while(left<=right && cmp(right, end-1) > 0)\
                        right--;\
                    if(left <= right){\
                        FFSWAP(type, *left, *right);\
                        left++;\
                        right--;\
                    }\
                }\
                FFSWAP(type, end[-1], *left);\
                if(checksort && (mid == left-1 || mid == left)){\
                    mid= start;\
                    while(mid<end && cmp(mid, mid+1) <= 0)\
                        mid++;\
                    if(mid==end)\
                        break;\
                }\
                if(end-left < left-start){\
                    stack[sp  ][0]= start;\
                    stack[sp++][1]= right;\
                    start = left+1;\
                }else{\
                    stack[sp  ][0]= left+1;\
                    stack[sp++][1]= end;\
                    end = right;\
                }\
            }else{\
                if(cmp(start, end) > 0)\
                    FFSWAP(type, *start, *end);\
                break;\
            }\
        }\
    }\
} while (0)

#endif /* AVUTIL_QSORT_H */
