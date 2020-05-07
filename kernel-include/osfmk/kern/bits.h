/*
 * Copyright (c) 2015-2016 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 *
 * Bit manipulation functions
 */

#ifndef __BITS_H__
#define __BITS_H__

#include <kern/assert.h>
#include <kern/kalloc.h>
#include <stdbool.h>
#include <stdint.h>

typedef unsigned int                    uint;

#define BIT(b)                          (1ULL << (b))

#define mask(width)                     (width >= 64 ? -1 : (BIT(width) - 1))
#define extract(x, shift, width)        ((((uint64_t)(x)) >> (shift)) & mask(width))
#define bits(x, hi, lo)                 extract((x), (lo), (hi) - (lo) + 1)

#define bit_set(x, b)                   ((x) |= BIT(b))
#define bit_clear(x, b)                 ((x) &= ~BIT(b))
#define bit_test(x, b)                  ((bool)((x) & BIT(b)))

inline static uint64_t
bit_ror64(uint64_t bitmap, uint n)
{
#if defined(__arm64__)
	uint64_t result;
	uint64_t _n = (uint64_t)n;
	asm volatile ("ror %0, %1, %2" : "=r" (result) : "r" (bitmap), "r" (_n));
	return result;
#else
	n = n & 63;
	return (bitmap >> n) | (bitmap << (64 - n));
#endif
}

inline static uint64_t
bit_rol64(uint64_t bitmap, uint n)
{
#if defined(__arm64__)
	return bit_ror64(bitmap, 64U - n);
#else
	n = n & 63;
	return (bitmap << n) | (bitmap >> (64 - n));
#endif
}

/* Non-atomically clear the bit and returns whether the bit value was changed */
inline static bool
bit_clear_if_set(uint64_t bitmap, int bit)
{
	bool bit_is_set = bit_test(bitmap, bit);
	bit_clear(bitmap, bit);
	return bit_is_set;
}

/* Non-atomically set the bit and returns whether the bit value was changed */
inline static bool
bit_set_if_clear(uint64_t bitmap, int bit)
{
	bool bit_is_set = bit_test(bitmap, bit);
	bit_set(bitmap, bit);
	return !bit_is_set;
}

/* Returns the most significant '1' bit, or -1 if all zeros */
inline static int
bit_first(uint64_t bitmap)
{
#if defined(__arm64__)
	int64_t result;
	asm volatile ("clz %0, %1" : "=r" (result) : "r" (bitmap));
	return 63 - (int)result;
#else
	return (bitmap == 0) ? -1 : 63 - __builtin_clzll(bitmap);
#endif
}


inline static int
__bit_next(uint64_t bitmap, int previous_bit)
{
	uint64_t mask = previous_bit ? mask(previous_bit) : ~0ULL;

	return bit_first(bitmap & mask);
}

/* Returns the most significant '1' bit that is less significant than previous_bit,
 * or -1 if no such bit exists.
 */
inline static int
bit_next(uint64_t bitmap, int previous_bit)
{
	if (previous_bit == 0) {
		return -1;
	} else {
		return __bit_next(bitmap, previous_bit);
	}
}

/* Returns the least significant '1' bit, or -1 if all zeros */
inline static int
lsb_first(uint64_t bitmap)
{
	return __builtin_ffsll(bitmap) - 1;
}

/* Returns the least significant '1' bit that is more significant than previous_bit,
 * or -1 if no such bit exists.
 * previous_bit may be -1, in which case this is equivalent to lsb_first()
 */
inline static int
lsb_next(uint64_t bitmap, int previous_bit)
{
	uint64_t mask = mask(previous_bit + 1);

	return lsb_first(bitmap & ~mask);
}

inline static int
bit_count(uint64_t x)
{
	return __builtin_popcountll(x);
}

/* Return the highest power of 2 that is <= n, or -1 if n == 0 */
inline static int
bit_floor(uint64_t n)
{
	return bit_first(n);
}

/* Return the lowest power of 2 that is >= n, or -1 if n == 0 */
inline static int
bit_ceiling(uint64_t n)
{
	if (n == 0) {
		return -1;
	}
	return bit_first(n - 1) + 1;
}

/* If n is a power of 2, bit_log2(n) == bit_floor(n) == bit_ceiling(n) */
#define bit_log2(n)             bit_floor((uint64_t)(n))

typedef uint64_t                bitmap_t;


inline static bool
atomic_bit_set(_Atomic bitmap_t *map, int n, int mem_order)
{
	bitmap_t prev;
	prev = __c11_atomic_fetch_or(map, BIT(n), mem_order);
	return bit_test(prev, n);
}

inline static bool
atomic_bit_clear(_Atomic bitmap_t *map, int n, int mem_order)
{
	bitmap_t prev;
	prev = __c11_atomic_fetch_and(map, ~BIT(n), mem_order);
	return bit_test(prev, n);
}


#define BITMAP_LEN(n)   (((uint)(n) + 63) >> 6)         /* Round to 64bit bitmap_t */
#define BITMAP_SIZE(n)  (size_t)(BITMAP_LEN(n) << 3)            /* Round to 64bit bitmap_t, then convert to bytes */
#define bitmap_bit(n)   bits(n, 5, 0)
#define bitmap_index(n) bits(n, 63, 6)

inline static bitmap_t *
bitmap_zero(bitmap_t *map, uint nbits)
{
	return (bitmap_t *)memset((void *)map, 0, BITMAP_SIZE(nbits));
}

inline static bitmap_t *
bitmap_full(bitmap_t *map, uint nbits)
{
	return (bitmap_t *)memset((void *)map, ~0, BITMAP_SIZE(nbits));
}

inline static bitmap_t *
bitmap_alloc(uint nbits)
{
	assert(nbits > 0);
	bitmap_t *map = (bitmap_t *)kalloc(BITMAP_SIZE(nbits));
	if (map) {
		bitmap_zero(map, nbits);
	}
	return map;
}

inline static void
bitmap_free(bitmap_t *map, uint nbits)
{
	assert(nbits > 0);
	kfree(map, BITMAP_SIZE(nbits));
}

inline static void
bitmap_set(bitmap_t *map, uint n)
{
	bit_set(map[bitmap_index(n)], bitmap_bit(n));
}

inline static void
bitmap_clear(bitmap_t *map, uint n)
{
	bit_clear(map[bitmap_index(n)], bitmap_bit(n));
}

inline static bool
atomic_bitmap_set(_Atomic bitmap_t *map, uint n, int mem_order)
{
	return atomic_bit_set(&map[bitmap_index(n)], bitmap_bit(n), mem_order);
}

inline static bool
atomic_bitmap_clear(_Atomic bitmap_t *map, uint n, int mem_order)
{
	return atomic_bit_clear(&map[bitmap_index(n)], bitmap_bit(n), mem_order);
}

inline static bool
bitmap_test(bitmap_t *map, uint n)
{
	return bit_test(map[bitmap_index(n)], bitmap_bit(n));
}

inline static int
bitmap_first(bitmap_t *map, uint nbits)
{
	for (int i = (int)bitmap_index(nbits - 1); i >= 0; i--) {
		if (map[i] == 0) {
			continue;
		}
		return (i << 6) + bit_first(map[i]);
	}

	return -1;
}

inline static int
bitmap_and_not_mask_first(bitmap_t *map, bitmap_t *mask, uint nbits)
{
	for (int i = (int)bitmap_index(nbits - 1); i >= 0; i--) {
		if ((map[i] & ~mask[i]) == 0) {
			continue;
		}
		return (i << 6) + bit_first(map[i] & ~mask[i]);
	}

	return -1;
}

inline static int
bitmap_lsb_first(bitmap_t *map, uint nbits)
{
	for (uint i = 0; i <= bitmap_index(nbits - 1); i++) {
		if (map[i] == 0) {
			continue;
		}
		return (int)((i << 6) + (uint32_t)lsb_first(map[i]));
	}

	return -1;
}

inline static int
bitmap_next(bitmap_t *map, uint prev)
{
	if (prev == 0) {
		return -1;
	}

	int64_t i = bitmap_index(prev - 1);
	int res = __bit_next(map[i], bits(prev, 5, 0));
	if (res >= 0) {
		return (int)(res + (i << 6));
	}

	for (i = i - 1; i >= 0; i--) {
		if (map[i] == 0) {
			continue;
		}
		return (int)((i << 6) + bit_first(map[i]));
	}

	return -1;
}

inline static int
bitmap_lsb_next(bitmap_t *map, uint nbits, uint prev)
{
	if ((prev + 1) >= nbits) {
		return -1;
	}

	uint64_t i = bitmap_index(prev + 1);
	uint b = bits((prev + 1), 5, 0) - 1;
	int32_t res = lsb_next((uint64_t)map[i], (int)b);
	if (res >= 0) {
		return (int)((uint64_t)res + (i << 6));
	}

	for (i = i + 1; i <= bitmap_index(nbits - 1); i++) {
		if (map[i] == 0) {
			continue;
		}
		return (int)((i << 6) + (uint64_t)lsb_first(map[i]));
	}

	return -1;
}

#endif
