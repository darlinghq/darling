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

#include "internal.h"

void
rack_init(rack_t *rack, rack_type_t type, uint32_t num_magazines, uint32_t debug_flags)
{
	rack->type = type;
	rack->rg[0].nextgen = &rack->rg[1];
	rack->rg[1].nextgen = &rack->rg[0];
	rack->region_generation = &rack->rg[0];

	rack->region_generation->hashed_regions = rack->initial_regions;
	rack->region_generation->num_regions_allocated = INITIAL_NUM_REGIONS;
	rack->region_generation->num_regions_allocated_shift = INITIAL_NUM_REGIONS_SHIFT;

	memset(rack->initial_regions, '\0', sizeof(region_t) * INITIAL_NUM_REGIONS);

	rack->cookie = (uintptr_t)malloc_entropy[0];

	if (type == RACK_TYPE_SMALL) {
		// Flip the cookie for SMALL regions so that tiny and small free list
		// entries will trap when used if used in opposing region types.
		rack->cookie = ~rack->cookie;
	}

	rack->debug_flags = debug_flags;
	rack->num_magazines = num_magazines;
	rack->num_regions = 0;
	rack->num_regions_dealloc = 0;
	rack->magazines = NULL;

	if (num_magazines > 0) {
		// num_magazines + 1, the [-1] index will become the depot magazine
		size_t magsize = round_page_quanta(sizeof(magazine_t) * (num_magazines + 1));
		magazine_t *magazines = mvm_allocate_pages(magsize, 0, MALLOC_ADD_GUARD_PAGES, VM_MEMORY_MALLOC);
		if (!magazines) {
			MALLOC_REPORT_FATAL_ERROR(0, "unable to allocate magazine array");
		}

		rack->magazines = &magazines[1];
		rack->num_magazines_mask_shift = 0;

		// The magazines are indexed in [0 .. (num_magazines - 1)]
		// Find the smallest power of 2 that exceeds (num_magazines - 1)
		int i = 1;
		while (i <= (num_magazines - 1)) {
			rack->num_magazines_mask_shift++;
			i <<= 1;
		}

		// Reduce i by 1 to obtain a mask covering [0 .. (num_tiny_magazines - 1)]
		rack->num_magazines_mask = i - 1;
		rack->last_madvise = 0;

		_malloc_lock_init(&rack->region_lock);
		_malloc_lock_init(&rack->magazines[DEPOT_MAGAZINE_INDEX].magazine_lock);

		for (int i=0; i < rack->num_magazines; i++) {
			_malloc_lock_init(&rack->magazines[i].magazine_lock);
		}
	}
}

void
rack_destroy_regions(rack_t *rack, size_t region_size)
{
	/* destroy regions attached to this rack */
	for (int i=0; i < rack->region_generation->num_regions_allocated; i++) {
		if ((rack->region_generation->hashed_regions[i] != HASHRING_OPEN_ENTRY) &&
			(rack->region_generation->hashed_regions[i] != HASHRING_REGION_DEALLOCATED))
		{
			mvm_deallocate_pages(rack->region_generation->hashed_regions[i], region_size, 0);
			rack->region_generation->hashed_regions[i] = HASHRING_REGION_DEALLOCATED;
		}
	}
}

void
rack_destroy(rack_t *rack)
{
	/* if the rack has additional regions, then deallocate them */
	if (rack->region_generation->hashed_regions != rack->initial_regions) {
		size_t size = round_page_quanta(rack->region_generation->num_regions_allocated * sizeof(region_t));
		mvm_deallocate_pages(rack->region_generation->hashed_regions, size, 0);
	}

	if (rack->num_magazines > 0) {
		size_t size = round_page_quanta(sizeof(magazine_t) * (rack->num_magazines + 1));
		mvm_deallocate_pages(&rack->magazines[-1], size, MALLOC_ADD_GUARD_PAGES);
		rack->magazines = NULL;
	}
}

void
rack_region_insert(rack_t *rack, region_t region)
{
	// Here find the only place in rackland that (infrequently) takes the tiny_regions_lock.
	// Only one thread at a time should be permitted to assess the density of the hash
	// ring and adjust if needed.
	// Only one thread at a time should be permitted to insert its new region on
	// the hash ring.
	// It is safe for all other threads to read the hash ring (hashed_regions) and
	// the associated sizes (num_regions_allocated and num_tiny_regions).

	_malloc_lock_lock(&rack->region_lock);

	// Check to see if the hash ring of tiny regions needs to grow.  Try to
	// avoid the hash ring becoming too dense.
	if (rack->region_generation->num_regions_allocated < (2 * rack->num_regions)) {
		region_t *new_regions;
		size_t new_size;
		size_t new_shift = rack->region_generation->num_regions_allocated_shift; // In/Out parameter
		new_regions = hash_regions_grow_no_lock(rack->region_generation->hashed_regions,
												rack->region_generation->num_regions_allocated, &new_shift, &new_size);
		// Do not deallocate the current hashed_regions allocation since someone may
		// be iterating it.  Instead, just leak it.

		// Prepare to advance to the "next generation" of the hash ring.
		rack->region_generation->nextgen->hashed_regions = new_regions;
		rack->region_generation->nextgen->num_regions_allocated = new_size;
		rack->region_generation->nextgen->num_regions_allocated_shift = new_shift;

		// Throw the switch to atomically advance to the next generation.
		rack->region_generation = rack->region_generation->nextgen;
		// Ensure everyone sees the advance.
		OSMemoryBarrier();
	}

	// Insert the new region into the hash ring, and update malloc statistics
	hash_region_insert_no_lock(rack->region_generation->hashed_regions,
							   rack->region_generation->num_regions_allocated,
							   rack->region_generation->num_regions_allocated_shift,
							   region);

	rack->num_regions++;
	_malloc_lock_unlock(&rack->region_lock);
}
