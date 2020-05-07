/*
 * Copyright (c) 1999, 2003, 2006, 2007, 2010 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Code duplicated from Libc/gen/nanosleep.c
 */

#ifndef _ARITHMETIC_128_H_
#define _ARITHMETIC_128_H_

#include <stdint.h>

#if __LP64__

static __inline uint64_t
multi_overflow(uint64_t a, uint64_t b)
{
	__uint128_t prod;
	prod = (__uint128_t)a * (__uint128_t)b;
	return (uint64_t) (prod >> 64);
}

#else

typedef struct {
	uint64_t high;
	uint64_t low;
} uint128_data_t;

/* 128-bit addition: acc += add */
static __inline void
add128_128(uint128_data_t *acc, uint128_data_t *add)
{
	acc->high += add->high;
	acc->low += add->low;
	if (acc->low < add->low) {
		acc->high++; // carry
	}
}

/* 64x64 -> 128 bit multiplication */
static __inline void
mul64x64(uint64_t x, uint64_t y, uint128_data_t *prod)
{
	uint128_data_t add;
	/*
	 * Split the two 64-bit multiplicands into 32-bit parts:
	 * x => 2^32 * x1 + x2
	 * y => 2^32 * y1 + y2
	 */
	uint32_t x1 = (uint32_t)(x >> 32);
	uint32_t x2 = (uint32_t)x;
	uint32_t y1 = (uint32_t)(y >> 32);
	uint32_t y2 = (uint32_t)y;
	/*
	 * direct multiplication:
	 * x * y => 2^64 * (x1 * y1) + 2^32 (x1 * y2 + x2 * y1) + (x2 * y2)
	 * The first and last terms are direct assignmenet into the uint128_t
	 * structure.  Then we add the middle two terms separately, to avoid
	 * 64-bit overflow.  (We could use the Karatsuba algorithm to save
	 * one multiply, but it is harder to deal with 64-bit overflows.)
	 */
	prod->high = (uint64_t)x1 * (uint64_t)y1;
	prod->low = (uint64_t)x2 * (uint64_t)y2;
	add.low = (uint64_t)x1 * (uint64_t)y2;
	add.high = (add.low >> 32);
	add.low <<= 32;
	add128_128(prod, &add);
	add.low = (uint64_t)x2 * (uint64_t)y1;
	add.high = (add.low >> 32);
	add.low <<= 32;
	add128_128(prod, &add);
}

static __inline uint64_t
multi_overflow(uint64_t a, uint64_t b)
{
	uint128_data_t prod;
	mul64x64(a, b, &prod);
	return prod.high;
}

#endif  /* __LP64__ */
#endif  /* _ARITHMETIC_128_H_ */
