/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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

#ifndef __MAGAZINE_RACK_H
#define __MAGAZINE_RACK_H

/*******************************************************************************
 * Definitions for region hash
 ******************************************************************************/

typedef void *region_t;
typedef region_t *rgnhdl_t; /* A pointer into hashed_regions array. */

#define INITIAL_NUM_REGIONS_SHIFT 6							 // log2(INITIAL_NUM_REGIONS)
#define INITIAL_NUM_REGIONS (1 << INITIAL_NUM_REGIONS_SHIFT) // Must be a power of 2!
#define HASHRING_OPEN_ENTRY ((region_t)0)					 // Initial value and sentinel marking end of collision chain
#define HASHRING_REGION_DEALLOCATED ((region_t)-1)			 // Region at this slot reclaimed by OS
#define HASH_BLOCKS_ALIGN TINY_BLOCKS_ALIGN					 // MIN( TINY_BLOCKS_ALIGN, SMALL_BLOCKS_ALIGN, ... )

typedef struct region_hash_generation {
	size_t num_regions_allocated;
	size_t num_regions_allocated_shift; // log2(num_regions_allocated)
	region_t *hashed_regions;			// hashed by location
	struct region_hash_generation *nextgen;
} region_hash_generation_t;

OS_ENUM(rack_type, uint32_t,
	RACK_TYPE_NONE = 0,
	RACK_TYPE_TINY,
	RACK_TYPE_SMALL,
	RACK_TYPE_MEDIUM,
);

/*******************************************************************************
 * Per-allocator collection of regions and magazines
 ******************************************************************************/

typedef struct rack_s {
	/* Regions for tiny objects */
	_malloc_lock_s region_lock MALLOC_CACHE_ALIGN;

	rack_type_t type;
	size_t num_regions;
	size_t num_regions_dealloc;
	region_hash_generation_t *region_generation;
	region_hash_generation_t rg[2];
	region_t initial_regions[INITIAL_NUM_REGIONS];

	int num_magazines;
	unsigned num_magazines_mask;
	int num_magazines_mask_shift;
	uint32_t debug_flags;

	// array of per-processor magazines
	magazine_t *magazines;

	uintptr_t cookie;
	uintptr_t last_madvise;
} rack_t;


MALLOC_NOEXPORT
void
rack_init(rack_t *rack, rack_type_t type, uint32_t num_magazines, uint32_t debug_flags);

MALLOC_NOEXPORT
void
rack_destroy_regions(rack_t *rack, size_t region_size);

MALLOC_NOEXPORT
void
rack_destroy(rack_t *rack);

MALLOC_NOEXPORT
void
rack_region_insert(rack_t *rack, region_t region);

#endif // __MAGAZINE_RACK_H
