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


#include "internal.h"

//
// purgeable zones have their own "large" allocation pool, but share "tiny" and "large"
// heaps with a helper_zone identified in the call to create_purgeable_zone()
//
static size_t
purgeable_size(szone_t *szone, const void *ptr)
{
	// Only claim our large allocations, leave the shared tiny/small for the helper zone to claim.
	return szone_size_try_large(szone, ptr);
}

static void *
purgeable_malloc(szone_t *szone, size_t size)
{
	if (size <= szone->large_threshold) {
		return szone_malloc(szone->helper_zone, size);
	} else {
		return szone_malloc(szone, size);
	}
}

static void *
purgeable_calloc(szone_t *szone, size_t num_items, size_t size)
{
	size_t total_bytes = num_items * size;

	// Check for overflow of integer multiplication
	if (num_items > 1) {
#if __LP64__ /* size_t is uint64_t */
		if ((num_items | size) & 0xffffffff00000000ul) {
			// num_items or size equals or exceeds sqrt(2^64) == 2^32, appeal to wider arithmetic
			__uint128_t product = ((__uint128_t)num_items) * ((__uint128_t)size);
			if ((uint64_t)(product >> 64)) { // compiles to test on upper register of register pair
				return NULL;
			}
		}
#else /* size_t is uint32_t */
		if ((num_items | size) & 0xffff0000ul) {
			// num_items or size equals or exceeds sqrt(2^32) == 2^16, appeal to wider arithmetic
			uint64_t product = ((uint64_t)num_items) * ((uint64_t)size);
			if ((uint32_t)(product >> 32)) { // compiles to test on upper register of register pair
				return NULL;
			}
		}
#endif
	}

	if (total_bytes <= szone->large_threshold) {
		return szone_calloc(szone->helper_zone, 1, total_bytes);
	} else {
		return szone_calloc(szone, 1, total_bytes);
	}
}

static void *
purgeable_valloc(szone_t *szone, size_t size)
{
	if (size <= szone->large_threshold) {
		return szone_valloc(szone->helper_zone, size);
	} else {
		return szone_valloc(szone, size);
	}
}

static void
purgeable_free(szone_t *szone, void *ptr)
{
	large_entry_t *entry;

	SZONE_LOCK(szone);
	entry = large_entry_for_pointer_no_lock(szone, ptr);
	SZONE_UNLOCK(szone);
	if (entry) {
		return free_large(szone, ptr);
	} else {
		return szone_free(szone->helper_zone, ptr);
	}
}

static void
purgeable_free_definite_size(szone_t *szone, void *ptr, size_t size)
{
	if (size <= szone->large_threshold) {
		return szone_free_definite_size(szone->helper_zone, ptr, size);
	} else {
		return szone_free_definite_size(szone, ptr, size);
	}
}

static void *
purgeable_realloc(szone_t *szone, void *ptr, size_t new_size)
{
	size_t old_size;

	if (NULL == ptr) {
		// If ptr is a null pointer, realloc() shall be equivalent to malloc() for the specified size.
		return purgeable_malloc(szone, new_size);
	} else if (0 == new_size) {
		// If size is 0 and ptr is not a null pointer, the object pointed to is freed.
		purgeable_free(szone, ptr);
		// If size is 0, either a null pointer or a unique pointer that can be successfully passed
		// to free() shall be returned.
		return purgeable_malloc(szone, 1);
	}

	old_size = purgeable_size(szone, ptr); // Now ptr can be safely size()'d
	if (!old_size) {
		old_size = szone_size(szone->helper_zone, ptr);
	}

	if (!old_size) {
		szone_error(szone, 1, "pointer being reallocated was not allocated", ptr, NULL);
		return NULL;
	}

	// Distinguish 4 cases: {oldsize, newsize} x { <= , > large_threshold }
	// and deal with the allocation crossing from the purgeable zone to the helper zone and vice versa.
	if (old_size <= szone->large_threshold) {
		if (new_size <= szone->large_threshold) {
			return szone_realloc(szone->helper_zone, ptr, new_size);
		} else {
			// allocation crosses from helper to purgeable zone
			void *new_ptr = purgeable_malloc(szone, new_size);
			if (new_ptr) {
				memcpy(new_ptr, ptr, old_size);
				szone_free_definite_size(szone->helper_zone, ptr, old_size);
			}
			return new_ptr; // in state VM_PURGABLE_NONVOLATILE
		}
	} else {
		if (new_size <= szone->large_threshold) {
			// allocation crosses from purgeable to helper zone
			void *new_ptr = szone_malloc(szone->helper_zone, new_size);
			if (new_ptr) {
				memcpy(new_ptr, ptr, new_size);
				purgeable_free_definite_size(szone, ptr, old_size);
			}
			return new_ptr;
		} else {
			void *new_ptr = purgeable_malloc(szone, new_size);
			if (new_ptr) {
				memcpy(new_ptr, ptr, MIN(old_size, new_size));
				purgeable_free_definite_size(szone, ptr, old_size);
			}
			return new_ptr; // in state VM_PURGABLE_NONVOLATILE
		}
	}
	/* NOTREACHED */
}

static void
purgeable_destroy(szone_t *szone)
{
	/* destroy large entries */
	size_t index = szone->num_large_entries;
	large_entry_t *large;
	vm_range_t range_to_deallocate;

	while (index--) {
		large = szone->large_entries + index;
		if (large->address) {
			// we deallocate_pages, including guard pages
			deallocate_pages(szone, (void *)(large->address), large->size, szone->debug_flags);
		}
	}
	large_entries_free_no_lock(szone, szone->large_entries, szone->num_large_entries, &range_to_deallocate);
	if (range_to_deallocate.size) {
		deallocate_pages(szone, (void *)range_to_deallocate.address, (size_t)range_to_deallocate.size, 0);
	}

	/* Now destroy the separate szone region */
	deallocate_pages(szone, (void *)szone, SZONE_PAGED_SIZE, 0);
}

static unsigned
purgeable_batch_malloc(szone_t *szone, size_t size, void **results, unsigned count)
{
	return szone_batch_malloc(szone->helper_zone, size, results, count);
}

static void
purgeable_batch_free(szone_t *szone, void **to_be_freed, unsigned count)
{
	return szone_batch_free(szone->helper_zone, to_be_freed, count);
}

static void *
purgeable_memalign(szone_t *szone, size_t alignment, size_t size)
{
	if (size <= szone->large_threshold) {
		return szone_memalign(szone->helper_zone, alignment, size);
	} else {
		return szone_memalign(szone, alignment, size);
	}
}

static kern_return_t
purgeable_ptr_in_use_enumerator(task_t task,
								void *context,
								unsigned type_mask,
								vm_address_t zone_address,
								memory_reader_t reader,
								vm_range_recorder_t recorder)
{
	szone_t *szone;
	kern_return_t err;

	if (!reader) {
		reader = _szone_default_reader;
	}

	err = reader(task, zone_address, sizeof(szone_t), (void **)&szone);
	if (err) {
		return err;
	}

	err = large_in_use_enumerator(
								  task, context, type_mask, (vm_address_t)szone->large_entries, szone->num_large_entries, reader, recorder);
	return err;
}

static size_t
purgeable_good_size(szone_t *szone, size_t size)
{
	if (size <= szone->large_threshold) {
		return szone_good_size(szone->helper_zone, size);
	} else {
		return szone_good_size(szone, size);
	}
}

static boolean_t
purgeable_check(szone_t *szone)
{
	return 1;
}

static void
purgeable_print(szone_t *szone, boolean_t verbose)
{
	_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX, "Scalable zone %p: inUse=%u(%y) flags=%d\n", szone,
				   szone->num_large_objects_in_use, szone->num_bytes_in_large_objects, szone->debug_flags);
}

static void
purgeable_log(malloc_zone_t *zone, void *log_address)
{
	szone_t *szone = (szone_t *)zone;

	szone->log_address = log_address;
}

static void
purgeable_force_lock(szone_t *szone)
{
	SZONE_LOCK(szone);
}

static void
purgeable_force_unlock(szone_t *szone)
{
	SZONE_UNLOCK(szone);
}

static void
purgeable_reinit_lock(szone_t *szone)
{
	SZONE_REINIT_LOCK(szone);
}

static void
purgeable_statistics(szone_t *szone, malloc_statistics_t *stats)
{
	stats->blocks_in_use = szone->num_large_objects_in_use;
	stats->size_in_use = stats->max_size_in_use = stats->size_allocated = szone->num_bytes_in_large_objects;
}

static boolean_t
purgeable_locked(szone_t *szone)
{
	int tookLock;

	tookLock = SZONE_TRY_LOCK(szone);
	if (tookLock == 0) {
		return 1;
	}
	SZONE_UNLOCK(szone);
	return 0;
}

static size_t
purgeable_pressure_relief(szone_t *szone, size_t goal)
{
	return szone_pressure_relief(szone, goal) + szone_pressure_relief(szone->helper_zone, goal);
}

static const struct malloc_introspection_t purgeable_introspect = {
	(void *)purgeable_ptr_in_use_enumerator, (void *)purgeable_good_size, (void *)purgeable_check, (void *)purgeable_print,
	purgeable_log, (void *)purgeable_force_lock, (void *)purgeable_force_unlock, (void *)purgeable_statistics,
	(void *)purgeable_locked, NULL, NULL, NULL, NULL, /* Zone enumeration version 7 and forward. */
	(void *)purgeable_reinit_lock, // reinit_lock version 9 and foward
}; // marked as const to spare the DATA section

malloc_zone_t *
create_purgeable_zone(size_t initial_size, malloc_zone_t *malloc_default_zone, unsigned debug_flags)
{
	szone_t *szone;
	uint64_t hw_memsize = 0;

	/* get memory for the zone. */
	szone = allocate_pages(NULL, SZONE_PAGED_SIZE, 0, 0, VM_MEMORY_MALLOC);
	if (!szone) {
		return NULL;
	}

	/* set up the szone structure */
#if 0
#warning LOG enabled
	szone->log_address = ~0;
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) || defined(__arm64__)
	hw_memsize = *(uint64_t *)(uintptr_t)_COMM_PAGE_MEMORY_SIZE;
#else
	size_t uint64_t_size = sizeof(hw_memsize);

	sysctlbyname("hw.memsize", &hw_memsize, &uint64_t_size, 0, 0);
#endif

	szone->trg[0].nextgen = &(szone->trg[1]);
	szone->trg[1].nextgen = &(szone->trg[0]);
	szone->tiny_region_generation = &(szone->trg[0]);

	szone->tiny_region_generation->hashed_regions = szone->initial_tiny_regions;
	szone->tiny_region_generation->num_regions_allocated = INITIAL_NUM_REGIONS;
	szone->tiny_region_generation->num_regions_allocated_shift = INITIAL_NUM_REGIONS_SHIFT;

	szone->srg[0].nextgen = &(szone->srg[1]);
	szone->srg[1].nextgen = &(szone->srg[0]);
	szone->small_region_generation = &(szone->srg[0]);

	szone->small_region_generation->hashed_regions = szone->initial_small_regions;
	szone->small_region_generation->num_regions_allocated = INITIAL_NUM_REGIONS;
	szone->small_region_generation->num_regions_allocated_shift = INITIAL_NUM_REGIONS_SHIFT;

	/* Purgeable zone does not participate in the adaptive "largemem" sizing. */
	szone->is_largemem = 0;
	szone->large_threshold = LARGE_THRESHOLD;
	szone->vm_copy_threshold = VM_COPY_THRESHOLD;

#if CONFIG_LARGE_CACHE
	szone->large_entry_cache_reserve_limit =
	hw_memsize >> 10; // madvise(..., MADV_REUSABLE) death-row arrivals above this threshold [~0.1%]

	/* <rdar://problem/6610904> Reset protection when returning a previous large allocation? */
	int32_t libSystemVersion = NSVersionOfLinkTimeLibrary("System");
	if ((-1 != libSystemVersion) && ((libSystemVersion >> 16) < 112) /* CFSystemVersionSnowLeopard */) {
		szone->large_legacy_reset_mprotect = TRUE;
	} else {
		szone->large_legacy_reset_mprotect = FALSE;
	}
#endif

	szone->basic_zone.version = 9;
	szone->basic_zone.size = (void *)purgeable_size;
	szone->basic_zone.malloc = (void *)purgeable_malloc;
	szone->basic_zone.calloc = (void *)purgeable_calloc;
	szone->basic_zone.valloc = (void *)purgeable_valloc;
	szone->basic_zone.free = (void *)purgeable_free;
	szone->basic_zone.realloc = (void *)purgeable_realloc;
	szone->basic_zone.destroy = (void *)purgeable_destroy;
	szone->basic_zone.batch_malloc = (void *)purgeable_batch_malloc;
	szone->basic_zone.batch_free = (void *)purgeable_batch_free;
	szone->basic_zone.introspect = (struct malloc_introspection_t *)&purgeable_introspect;
	szone->basic_zone.memalign = (void *)purgeable_memalign;
	szone->basic_zone.free_definite_size = (void *)purgeable_free_definite_size;
	szone->basic_zone.pressure_relief = (void *)purgeable_pressure_relief;

	szone->basic_zone.reserved1 = 0;					   /* Set to zero once and for all as required by CFAllocator. */
	szone->basic_zone.reserved2 = 0;					   /* Set to zero once and for all as required by CFAllocator. */
	mprotect(szone, sizeof(szone->basic_zone), PROT_READ); /* Prevent overwriting the function pointers in basic_zone. */

	szone->debug_flags = debug_flags | MALLOC_PURGEABLE;

	/* Purgeable zone does not support MALLOC_ADD_GUARD_PAGES. */
	if (szone->debug_flags & MALLOC_ADD_GUARD_PAGES) {
		_malloc_printf(ASL_LEVEL_INFO, "purgeable zone does not support guard pages\n");
		szone->debug_flags &= ~MALLOC_ADD_GUARD_PAGES;
	}

	_malloc_lock_init(&szone->large_szone_lock);

	szone->helper_zone = (struct szone_s *)malloc_default_zone;

	CHECK(szone, __PRETTY_FUNCTION__);
	return (malloc_zone_t *)szone;
}
