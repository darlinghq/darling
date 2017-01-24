/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#ifndef __THRESHOLDS_H
#define __THRESHOLDS_H

/*
 * Tiny region size definitions; these are split into quanta of 16 bytes, 
 * 64520 blocks is the magical value of how many quanta we can fit in a 1mb
 * region including the region trailer and metadata.
 */
#define SHIFT_TINY_QUANTUM 4
#define SHIFT_TINY_CEIL_BLOCKS 16 // ceil(log2(NUM_TINY_BLOCKS))
#define TINY_QUANTUM (1 << SHIFT_TINY_QUANTUM)
#define NUM_TINY_BLOCKS 64520
#define NUM_TINY_CEIL_BLOCKS (1 << SHIFT_TINY_CEIL_BLOCKS)

/* 
 * Small region size defintions.
 *
 * We can only represent up to 1<<15 for msize; but we choose to stay
 * even below that to avoid the convention msize=0 => msize = (1<<15)
 */
#define SHIFT_SMALL_QUANTUM (SHIFT_TINY_QUANTUM + 5) // 9
#define SMALL_QUANTUM (1 << SHIFT_SMALL_QUANTUM)	 // 512 bytes
#define SHIFT_SMALL_CEIL_BLOCKS 14 // ceil(log2(NUM_SMALL_BLOCKs))
#define NUM_SMALL_BLOCKS 16319
#define NUM_SMALL_CEIL_BLOCKS (1 << SHIFT_SMALL_CEIL_BLOCKS)
#define SMALL_BLOCKS_ALIGN (SHIFT_SMALL_CEIL_BLOCKS + SHIFT_SMALL_QUANTUM) // 23

/*
 * The number of slots in the free-list for small blocks.  To avoid going to
 * vm system as often on large memory machines, increase the number of free list
 * spots above some amount of RAM installed in the system.
 */
#define NUM_SMALL_SLOTS 32
#define NUM_SMALL_SLOTS_LARGEMEM 256
#define SMALL_BITMAP_WORDS 8

#if MALLOC_TARGET_64BIT
#define NUM_TINY_SLOTS 64 // number of slots for free-lists
#else // MALLOC_TARGET_64BIT
#define NUM_TINY_SLOTS 32 // number of slots for free-lists
#endif // MALLOC_TARGET_64BIT

/* 
 * The threshold above which we start allocating from the small
 * magazines. Computed from the largest allocation we can make
 * in the tiny region (currently 1008 bytes on 64-bit, and 
 * 496 bytes on 32-bit).
 */
#define SMALL_THRESHOLD ((NUM_TINY_SLOTS - 1) * TINY_QUANTUM)

/*
 * The threshold above which we start allocating from the large
 * "region" (ie. direct vm_allocates). The LARGEMEM size is used
 * on systems that have more than 1GB RAM.
 */
#define LARGE_THRESHOLD (15 * 1024)
#define LARGE_THRESHOLD_LARGEMEM (127 * 1024)

/*
 * When all memory is touched after a copy, vm_copy() is always a lose
 * But if the memory is only read, vm_copy() wins over memmove() at 3 or 4 pages
 * (on a G3/300MHz)
 *
 * This must be larger than LARGE_THRESHOLD
 */
#define VM_COPY_THRESHOLD (40 * 1024)
#define VM_COPY_THRESHOLD_LARGEMEM (128 * 1024)

/*
 * Large entry cache (death row) sizes. The large cache is bounded with
 * an overall top limit size, each entry is allowed a given slice of
 * that limit.
 */
#if MALLOC_TARGET_64BIT
#define LARGE_ENTRY_CACHE_SIZE 16
#define LARGE_CACHE_SIZE_LIMIT ((vm_size_t)0x80000000) /* 2Gb */
#define LARGE_CACHE_SIZE_ENTRY_LIMIT (LARGE_CACHE_SIZE_LIMIT / LARGE_ENTRY_CACHE_SIZE)
#else // MALLOC_TARGET_64BIT
#define LARGE_ENTRY_CACHE_SIZE 8
#define LARGE_CACHE_SIZE_LIMIT ((vm_size_t)0x02000000) /* 32Mb */
#define LARGE_CACHE_SIZE_ENTRY_LIMIT (LARGE_CACHE_SIZE_LIMIT / LARGE_ENTRY_CACHE_SIZE)
#endif // MALLOC_TARGET_64BIT

/*
 * Large entry cache (death row) "flotsam" limits. Until the large cache
 * contains at least "high" bytes, the cache is not cleaned under memory
 * pressure. After that, memory pressure notifications cause cache cleaning
 * until the large cache drops below the "low" limit.
 */
#define SZONE_FLOTSAM_THRESHOLD_LOW (1024 * 512)
#define SZONE_FLOTSAM_THRESHOLD_HIGH (1024 * 1024)

/*
 * Density threshold used in determining the level of emptiness before
 * moving regions to the recirc depot.
 */
#define DENSITY_THRESHOLD(a) \
	((a) - ((a) >> 2)) // "Emptiness" f = 0.25, so "Density" is (1 - f)*a. Generally: ((a) - ((a) >> -log2(f)))

/* Sanity checks. */

#if (LARGE_THRESHOLD > NUM_SMALL_SLOTS * SMALL_QUANTUM)
#error LARGE_THRESHOLD should always be less than NUM_SMALL_SLOTS * SMALL_QUANTUM
#endif

#if (LARGE_THRESHOLD_LARGEMEM > NUM_SMALL_SLOTS_LARGEMEM * SMALL_QUANTUM)
#error LARGE_THRESHOLD_LARGEMEM should always be less than NUM_SMALL_SLOTS * SMALL_QUANTUM
#endif

#endif // __THRESHOLDS_H
