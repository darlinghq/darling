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
 * The actual threshold boundaries between allocators. These boundaries are
 * where the next allocator will take over and they are *inclusive* of the
 * limit value.  That is, a TINY limit of X implies that an X-byte
 * allocation will come from TINY.
 *
 * The LARGE allocator cuts in at whatever the last boundary limit is. So, when
 * the medium allocator is compiled out, or not engaged, then large starts at
 * the limit of SMALL.
 */
#if MALLOC_TARGET_64BIT
#define TINY_LIMIT_THRESHOLD (1008)
#else // MALLOC_TARGET_64BIT
#define TINY_LIMIT_THRESHOLD (496)
#endif // MALLOC_TARGET_64BIT

#if MALLOC_TARGET_IOS
#define SMALL_LIMIT_THRESHOLD (15 * 1024)
#else // MALLOC_TARGET_IOS
#define SMALL_LIMIT_THRESHOLD (32 * 1024)
#endif // MALLOC_TARGET_IOS
#define MEDIUM_LIMIT_THRESHOLD (8 * 1024 * 1024)

/*
 * Tiny region size definitions; these are split into quanta of 16 bytes,
 * 64504 blocks is the magical value of how many quanta we can fit in a 1mb
 * region including the region trailer and metadata.
 */
#define SHIFT_TINY_QUANTUM 4ull
#define SHIFT_TINY_CEIL_BLOCKS 16 // ceil(log2(NUM_TINY_BLOCKS))
#define TINY_QUANTUM (1 << SHIFT_TINY_QUANTUM)
#define NUM_TINY_BLOCKS 64504
#define NUM_TINY_CEIL_BLOCKS (1 << SHIFT_TINY_CEIL_BLOCKS)
#define NUM_TINY_SLOTS (TINY_LIMIT_THRESHOLD >> SHIFT_TINY_QUANTUM)

#if MALLOC_TARGET_64BIT
#define TINY_BITMAP_RANGE_LIMIT 63
#else
#define TINY_BITMAP_RANGE_LIMIT 31
#endif

/*
 * Small region size definitions.
 *
 * We can only represent up to 1<<15 for msize; but we choose to stay
 * even below that to avoid the convention msize=0 => msize = (1<<15)
 */
#define SHIFT_SMALL_QUANTUM (SHIFT_TINY_QUANTUM + 5) // 9
#define SHIFT_SMALL_CEIL_BLOCKS 14 // ceil(log2(NUM_SMALL_BLOCKs))
#define SMALL_QUANTUM (1 << SHIFT_SMALL_QUANTUM) // 512 bytes
#define SMALL_BLOCKS_ALIGN (SHIFT_SMALL_CEIL_BLOCKS + SHIFT_SMALL_QUANTUM) // 23
#define NUM_SMALL_BLOCKS 16319
#define NUM_SMALL_CEIL_BLOCKS (1 << SHIFT_SMALL_CEIL_BLOCKS)
#define NUM_SMALL_SLOTS (SMALL_LIMIT_THRESHOLD >> SHIFT_SMALL_QUANTUM)

/*
 * Medium region size definitions.
 *
 * We can only represent up to 1<<15 for msize; but we choose to stay
 * even below that to avoid the convention msize=0 => msize = (1<<15)
 */
#define SHIFT_MEDIUM_QUANTUM (SHIFT_SMALL_QUANTUM + 6) // 15
#define SHIFT_MEDIUM_CEIL_BLOCKS 12ull // ceil(log2(NUM_MEDIUM_BLOCKS))
#define MEDIUM_QUANTUM ((uint64_t)(1 << SHIFT_MEDIUM_QUANTUM)) // 32kbytes
#define MEDIUM_BLOCKS_ALIGN (SHIFT_MEDIUM_CEIL_BLOCKS + SHIFT_MEDIUM_QUANTUM) // 27
#define NUM_MEDIUM_BLOCKS 4095
#define NUM_MEDIUM_CEIL_BLOCKS (1ull << SHIFT_MEDIUM_CEIL_BLOCKS)
#define NUM_MEDIUM_SLOTS (MEDIUM_LIMIT_THRESHOLD >> SHIFT_MEDIUM_QUANTUM)
#define MEDIUM_ACTIVATION_THRESHOLD (32ull * 1024 * 1024 * 1024)
#define MEDIUM_CONDITIONAL_MADVISE_LIMIT (2 * 1024 * 1024)
#define MEDIUM_MADVISE_SHIFT 4
#define MEDIUM_MADVISE_MIN ((3 * 1024 * 1024) / 2) // 1.5 megabytes

/*
 * When performing a realloc() that must fallback to creating a new allocation
 * and copying the previous contents to the new allocation, vm_copy is used if
 * the allocation is greater than a given size.
 *
 * This threshold must be set such that all eligible allocations would have
 * come from a page-sized, page-aligned allocator (so, medium or large).
 *
 * Note: iOS disables this threshold because the VM forces non-sharing from
 * malloc-tagged allocations.
 */
#define VM_COPY_THRESHOLD (2 * 1024 * 1024)

/*
 * <rdar://problem/6881926&27190324> Extremely old versions of Microsoft Word
 * (and, subsequently, versions of Adobe apps) required the Leopard behaviour
 * where LARGE allocations were zero-filled prior to returning them to the
 * caller.
 *
 * We've always used LARGE_THRESHOLD to denote this boundary but as we keep
 * moving it around it's better to fix it at the point it was originally.
 */
#define LEGACY_ZEROING_THRESHOLD (127 * 1024)

/*
 * Large entry cache (death row) sizes. The large cache is bounded with
 * an overall top limit size, each entry is allowed a given slice of
 * that limit.
 */

#define LARGE_CACHE_EXPANDED_THRESHOLD (32ull * 1024 * 1024 * 1024)

#if MALLOC_TARGET_64BIT
#define LARGE_ENTRY_CACHE_SIZE_HIGH 64
#define LARGE_ENTRY_SIZE_ENTRY_LIMIT_HIGH (512 * 1024 * 1024)
// lowmem config
#define LARGE_ENTRY_CACHE_SIZE_LOW 16
#define LARGE_ENTRY_SIZE_ENTRY_LIMIT_LOW (128 * 1024 * 1024)
#else // MALLOC_TARGET_64BIT
#define LARGE_ENTRY_CACHE_SIZE_HIGH 8
#define LARGE_ENTRY_SIZE_ENTRY_LIMIT_HIGH (32 * 1024 * 1024)
// lowmem config same as "highmem"
#define LARGE_ENTRY_CACHE_SIZE_LOW LARGE_ENTRY_CACHE_SIZE_HIGH
#define LARGE_ENTRY_SIZE_ENTRY_LIMIT_LOW LARGE_ENTRY_SIZE_ENTRY_LIMIT_HIGH
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
 * The magazine freelist array must be large enough to accomodate the allocation
 * granularity of the tiny, small and medium allocators. In addition, the last
 * slot in the list is special and reserved for coalesced regions bigger than
 * the overall max allocation size of the allocator.
 */
#define MAGAZINE_FREELIST_SLOTS (NUM_MEDIUM_SLOTS + 1)
#define MAGAZINE_FREELIST_BITMAP_WORDS ((MAGAZINE_FREELIST_SLOTS + 31) >> 5)

/*
 * Density threshold used in determining the level of emptiness before
 * moving regions to the recirc depot.
 */
#define DENSITY_THRESHOLD(a) \
	((a) - ((a) >> 2)) // "Emptiness" f = 0.25, so "Density" is (1 - f)*a. Generally: ((a) - ((a) >> -log2(f)))

/*
 * Minimum number of regions to retain in a recirc depot.
 */
#define DEFAULT_RECIRC_RETAINED_REGIONS 2

/* Sanity checks. */

// Tiny performs an ffsl of a uint64_t in order to determine how big an
// allocation is. Therefore, the total allocation size of small cannot exceed
// 63-bits worth of 16-byte quanta (64-bits but minus one for the start of the
// allocation itself).
MALLOC_STATIC_ASSERT((TINY_LIMIT_THRESHOLD / TINY_QUANTUM) <= TINY_BITMAP_RANGE_LIMIT,
		"TINY_LIMIT_THRESHOLD cannot exceed TINY_BITMAP_RANGE_LIMIT-bits worth of metadata");

// Check that the given threshold limits are a round multiple of their
// allocator's quantum size.
MALLOC_STATIC_ASSERT((TINY_LIMIT_THRESHOLD % TINY_QUANTUM) == 0,
		"TINY_LIMIT_THRESHOLD must be a multiple of TINY_QUANTUM");

MALLOC_STATIC_ASSERT((SMALL_LIMIT_THRESHOLD % SMALL_QUANTUM) == 0,
		"SMALL_LIMIT_THRESHOLD must be a multiple of SMALL_QUANTUM");

MALLOC_STATIC_ASSERT((MEDIUM_LIMIT_THRESHOLD % MEDIUM_QUANTUM) == 0,
		"MEDIUM_LIMIT_THRESHOLD must be a multiple of MEDIUM_QUANTUM");

// All the "slot" counts are calculated as a shift of thresholds now but
// in-case someone decides to try hand-crafted values, make sure they adhere to
// the basic expectation that slot count must account for all valid sizes of
// allocations.
MALLOC_STATIC_ASSERT(NUM_TINY_SLOTS >= TINY_LIMIT_THRESHOLD >> SHIFT_TINY_QUANTUM,
		"NUM_TINY_SLOTS must allow a free list for every valid TINY allocation");

MALLOC_STATIC_ASSERT(NUM_SMALL_SLOTS >= SMALL_LIMIT_THRESHOLD >> SHIFT_SMALL_QUANTUM,
		"NUM_SMALL_SLOTS must allow a free list for every valid SMALL allocation");

MALLOC_STATIC_ASSERT(NUM_MEDIUM_SLOTS >= MEDIUM_LIMIT_THRESHOLD >> SHIFT_MEDIUM_QUANTUM,
		"NUM_MEDIUM_SLOTS must allow a free list for every valid MEDIUM allocation");

// MAGAZINE_FREELIST_SLOTS cannot be dynamically selected by the MAX() of all
// three allocators, so it must match (at least) the maxmium slot count of the
// allocator with the largest range.
//
// Additionally, each allocator assumes that there is one additional free-list
// slot above their maximum allocation size. This allows each allocator to
// store an unordered list of maximally-sized free list entries.
MALLOC_STATIC_ASSERT(NUM_TINY_SLOTS < MAGAZINE_FREELIST_SLOTS,
		"NUM_TINY_SLOTS must be less than MAGAZINE_FREELIST_SLOTS");

MALLOC_STATIC_ASSERT(NUM_SMALL_SLOTS < MAGAZINE_FREELIST_SLOTS,
		"NUM_SMALL_SLOTS must be less than MAGAZINE_FREELIST_SLOTS");

MALLOC_STATIC_ASSERT(NUM_MEDIUM_SLOTS < MAGAZINE_FREELIST_SLOTS,
		"NUM_MEDIUM_SLOTS must be less than MAGAZINE_FREELIST_SLOTS");

MALLOC_STATIC_ASSERT(VM_COPY_THRESHOLD >= SMALL_LIMIT_THRESHOLD,
		"VM_COPY_THRESHOLD must be larger than SMALL_LIMIT_THRESHOLD");

#endif // __THRESHOLDS_H
