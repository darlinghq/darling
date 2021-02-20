/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

#if CONFIG_MEDIUM_ALLOCATOR

/*********************	MEDIUM FREE LIST UTILITIES	************************/

#pragma mark meta header helpers

static MALLOC_INLINE uint64_t
medium_sliding_madvise_granularity(magazine_t *magazine)
{
	// Use a sliding madvise granularity based on how many bytes the region
	// currently has allocated. This way we will advise at a finer granularity
	// as the region becomes more and more empty.
	// region_trailer_t *t = REGION_TRAILER_FOR_MEDIUM_REGION(region);
	if (magazine->mag_num_bytes_in_objects == 0) {
		return MEDIUM_MADVISE_MIN;
	}
	return MAX(MEDIUM_MADVISE_MIN, 1 << (64 -
			__builtin_clzl(magazine->mag_num_bytes_in_objects >> MEDIUM_MADVISE_SHIFT)));
}

static MALLOC_INLINE void
medium_madvise_header_mark_dirty(msize_t *headers, msize_t index,
		msize_t msize)
{
	// MALLOC_ASSERT(index < NUM_MEDIUM_BLOCKS);
	// MALLOC_ASSERT(index + msize <= NUM_MEDIUM_BLOCKS);
	headers[index] = msize;
	headers[index + msize - 1] = msize;
}

static MALLOC_INLINE void
medium_madvise_header_mark_clean(msize_t *headers, msize_t index,
		msize_t msize)
{
	// MALLOC_ASSERT(index < NUM_MEDIUM_BLOCKS);
	// MALLOC_ASSERT(index + msize <= NUM_MEDIUM_BLOCKS);
	headers[index] = msize | MEDIUM_IS_ADVISED;
	headers[index + msize - 1] = msize | MEDIUM_IS_ADVISED;
}

static MALLOC_INLINE void
medium_madvise_header_mark_middle(msize_t *headers, msize_t index)
{
	// MALLOC_ASSERT(index < NUM_MEDIUM_BLOCKS);
	headers[index] = 0;
}

static MALLOC_INLINE msize_t
medium_madvise_header_dirty_len(msize_t *headers, msize_t index)
{
	// MALLOC_ASSERT(index < NUM_MEDIUM_BLOCKS);
	if (headers[index] & MEDIUM_IS_ADVISED) {
		return 0;
	}
	return headers[index] & ~MEDIUM_IS_ADVISED;
}

/*
 * Mark a block as free.  Only the first quantum of a block is marked thusly,
 * the remainder are marked "middle".
 */
static MALLOC_INLINE void
medium_meta_header_set_is_free(msize_t *meta_headers, msize_t index, msize_t msize)
{
	meta_headers[index] = msize | MEDIUM_IS_FREE;
}

/*
 * Mark a block as not free, preserving its size.
 */
static MALLOC_INLINE void
medium_meta_header_set_not_free(msize_t *meta_headers, msize_t index)
{
	meta_headers[index] &= ~MEDIUM_IS_FREE;
}

/*
 * Mark a block as in use.  Only the first quantum of a block is marked thusly,
 * the remainder are marked "middle".
 */
static MALLOC_INLINE void
medium_meta_header_set_in_use(msize_t *meta_headers, msize_t index, msize_t msize)
{
	meta_headers[index] = msize;
}

/*
 * Mark a quantum as being the second or later in a block.
 */
static MALLOC_INLINE void
medium_meta_header_set_middle(msize_t *meta_headers, msize_t index)
{
	meta_headers[index] = 0;
}

static MALLOC_INLINE MALLOC_ALWAYS_INLINE
mag_index_t
medium_mag_get_thread_index(void)
{
#if CONFIG_MEDIUM_USES_HYPER_SHIFT
	if (os_likely(_os_cpu_number_override == -1)) {
		return _os_cpu_number() >> hyper_shift;
	} else {
		return _os_cpu_number_override >> hyper_shift;
	}
#else // CONFIG_MEDIUM_USES_HYPER_SHIFT
	if (os_likely(_os_cpu_number_override == -1)) {
		return _os_cpu_number();
	} else {
		return _os_cpu_number_override;
	}
#endif // CONFIG_MEDIUM_USES_HYPER_SHIFT
}

#pragma mark in-place free list

static MALLOC_INLINE void
medium_inplace_checksum_ptr(rack_t *rack, inplace_linkage_s *linkage, void *ptr)
{
	uintptr_t checksum = free_list_gen_checksum((uintptr_t)ptr ^ rack->cookie ^ (uintptr_t)rack);
	linkage->checksum = checksum;
	linkage->ptr = ptr;
}

static MALLOC_INLINE free_list_t
medium_inplace_unchecksum_ptr(rack_t *rack, inplace_linkage_s *linkage)
{
	if (linkage->checksum != (uint8_t)free_list_gen_checksum((uintptr_t)linkage->ptr ^ rack->cookie ^ (uintptr_t)rack)) {
		free_list_checksum_botch(rack, linkage, linkage->ptr);
		__builtin_trap();
	}

	return (free_list_t){ .p = linkage->ptr };
}

static MALLOC_INLINE free_list_t
medium_inplace_unchecksum_ptr_task(task_t task, memory_reader_t reader,
		print_task_printer_t printer, rack_t *rack, inplace_linkage_s *linkage)
{
	inplace_linkage_s *mapped_linkage;
	rack_t *mapped_rack;
	if (reader(task, (vm_address_t)linkage, sizeof(*linkage),
			(void **)&mapped_linkage)) {
		printer("Unable to map medium linkage pointer %p\n", linkage);
		return (free_list_t){ .p = NULL };
	}

	if (reader(task, (vm_address_t)rack,
			sizeof(struct rack_s), (void **)&mapped_rack)) {
		printer("Failed to map medium rack\n");
		return (free_list_t){ .p = NULL };
	}

	if (mapped_linkage->checksum != (uint8_t)free_list_gen_checksum(
			(uintptr_t)mapped_linkage->ptr ^ mapped_rack->cookie ^ (uintptr_t)rack)) {
		free_list_checksum_botch(rack, linkage, mapped_linkage->ptr);
		__builtin_trap();
	}

	return (free_list_t){ .p = mapped_linkage->ptr };
}

static MALLOC_INLINE free_list_t
medium_inplace_free_entry_get_previous(rack_t *rack, medium_inplace_free_entry_t ptr)
{
	return medium_inplace_unchecksum_ptr(rack, &ptr->previous);
}

static MALLOC_INLINE void
medium_inplace_free_entry_set_previous(rack_t *rack, medium_inplace_free_entry_t entry, free_list_t previous)
{
	medium_inplace_checksum_ptr(rack, &entry->previous, previous.p);
}

static MALLOC_INLINE free_list_t
medium_inplace_free_entry_get_next(rack_t *rack, medium_inplace_free_entry_t ptr)
{
	return medium_inplace_unchecksum_ptr(rack, &ptr->next);
}

static MALLOC_INLINE free_list_t
medium_inplace_free_entry_get_next_task(task_t task, memory_reader_t reader,
		print_task_printer_t printer, rack_t *rack,
		medium_inplace_free_entry_t ptr)
{
	return medium_inplace_unchecksum_ptr_task(task, reader, printer, rack,
			&ptr->next);
}

static MALLOC_INLINE void
medium_inplace_free_entry_set_next(rack_t *rack, medium_inplace_free_entry_t entry, free_list_t next)
{
	medium_inplace_checksum_ptr(rack, &entry->next, next.p);
}

#pragma mark OOB free list

// Returns true if the address and size of the free list entry would result
// in the free entry being the only data on a given page.
static MALLOC_INLINE boolean_t
medium_needs_oob_free_entry(void *ptr, msize_t msize)
{
	return ((trunc_page_quanta((uintptr_t)ptr) == (uintptr_t)ptr) &&
			(MEDIUM_BYTES_FOR_MSIZE(msize) >= vm_kernel_page_size));
}

// Returns true if the address given lies within the region's OOB free
// list entries, rather than a free_list_t in the region's heap space.
static MALLOC_INLINE boolean_t
medium_is_oob_free_entry(free_list_t ptr)
{
	medium_region_t region = MEDIUM_REGION_FOR_PTR(ptr.p);
	return (((uintptr_t)ptr.p >= (uintptr_t)&region->medium_oob_free_entries[0]) &&
			((uintptr_t)ptr.p < (uintptr_t)&region->medium_oob_free_entries[MEDIUM_OOB_COUNT]));
}

static MALLOC_INLINE void
medium_oob_free_entry_set_previous(oob_free_entry_t oobe, free_list_t previous)
{
	// MALLOC_ASSERT(medium_is_oob_free_entry(previous) || !((uintptr_t)previous.p & 0x1fffffff));
	oobe->prev = (uintptr_t)previous.p;
}

static MALLOC_INLINE free_list_t
medium_oob_free_entry_get_previous(oob_free_entry_t oobe)
{
	return (free_list_t){ .p = (void *)oobe->prev };
}

static MALLOC_INLINE void
medium_oob_free_entry_set_next(oob_free_entry_t oobe, free_list_t next)
{
	// MALLOC_ASSERT(medium_is_oob_free_entry(next) || !((uintptr_t)next.p & 0x1fffffff));
	oobe->next = (uintptr_t)next.p;
}

static MALLOC_INLINE free_list_t
medium_oob_free_entry_get_next(oob_free_entry_t oobe)
{
	return (free_list_t){ .p = (void *)oobe->next };
}

static MALLOC_INLINE free_list_t
medium_oob_free_entry_get_next_task(task_t task, memory_reader_t reader,
		print_task_printer_t printer, oob_free_entry_t oobe)
{
	oob_free_entry_t mapped_oobe;
	if (reader(task, (vm_address_t)oobe, sizeof(*oobe),
			(void **)&mapped_oobe)) {
		printer("Failed to map medium oobe pointer\n");
		return (free_list_t){ .p = NULL };
	}
	return (free_list_t){ .p = (void *)mapped_oobe->next };
}

static MALLOC_INLINE void *
medium_oob_free_entry_get_ptr(oob_free_entry_t oobe)
{
	medium_region_t region = MEDIUM_REGION_FOR_PTR(oobe);
	uint16_t block = oobe->ptr & ~MEDIUM_IS_OOB;
	return (void *)((uintptr_t)region + (block << SHIFT_MEDIUM_QUANTUM));
}

static MALLOC_INLINE void *
medium_oob_free_entry_get_ptr_task(task_t task, memory_reader_t reader,
		print_task_printer_t printer, oob_free_entry_t oobe)
{
	// We need to map the oob_free_entry_t to read the pointer value.
	oob_free_entry_t mapped_oobe;
	if (reader(task, (vm_address_t)oobe, sizeof(*oobe),
			(void **)&mapped_oobe)) {
		printer("Failed to map medium oobe pointer\n");
		return NULL;
	}

	if (!(mapped_oobe->ptr & MEDIUM_IS_OOB)) {
		return NULL;
	}

	// The rest of this code works with target process addresses and returns an
	// address in the target process.
	medium_region_t region = MEDIUM_REGION_FOR_PTR(oobe);
	uint16_t block = mapped_oobe->ptr & ~MEDIUM_IS_OOB;
	return (void *)((uintptr_t)region + (block << SHIFT_MEDIUM_QUANTUM));
}

static MALLOC_INLINE void
medium_oob_free_entry_set_ptr(oob_free_entry_t oobe, void *ptr)
{
	oobe->ptr = MEDIUM_IS_OOB | (MEDIUM_OFFSET_FOR_PTR(ptr) >> SHIFT_MEDIUM_QUANTUM);
}

static MALLOC_INLINE void
medium_oob_free_entry_set_free(oob_free_entry_t oobe)
{
	oobe->prev = ~0;
	oobe->next = ~0;
	oobe->ptr = 0;
}

// Finds the first unused OOB free list entry in the pointer's region.
// Returns NULL if all of the OOB entries are used.
static MALLOC_INLINE oob_free_entry_t
medium_oob_free_find_empty(void *ptr, msize_t msize)
{
	medium_region_t region = MEDIUM_REGION_FOR_PTR(ptr);

	// There are 61 of these entries at the end of a medium region.
	// If this changes, then a linear search through the list may
	// become an unsuitable choice.
	for (int i=0; i < MEDIUM_OOB_COUNT; i++) {
		if (region->medium_oob_free_entries[i].ptr == 0) {
			return &region->medium_oob_free_entries[i];
		}
	}

#if DEBUG_MALLOC
	malloc_report(ASL_LEVEL_INFO, "used all slots of OOB entries\n");
#endif
	return NULL;
}

static MALLOC_INLINE oob_free_entry_t
medium_oob_free_find_ptr(void *ptr, msize_t msize)
{
	medium_region_t region = MEDIUM_REGION_FOR_PTR(ptr);

	// There are 61 of these entries at the end of a medium region.
	// If this changes, then a linear search through the list may
	// become an unsuitable choice.
	for (int i=0; i < MEDIUM_OOB_COUNT; i++) {
		oob_free_entry_t oob = &region->medium_oob_free_entries[i];
		if (medium_oob_free_entry_get_ptr(oob) == ptr &&
				oob->ptr & MEDIUM_IS_OOB) {
			return &region->medium_oob_free_entries[i];
		}
	}

	return NULL;
}

#pragma mark generic free list

static MALLOC_INLINE void
medium_free_list_set_previous(rack_t *rack, free_list_t entry, free_list_t previous)
{
	if (medium_is_oob_free_entry(entry)) {
		medium_oob_free_entry_set_previous(entry.oob, previous);
	} else {
		medium_inplace_free_entry_set_previous(rack, entry.medium_inplace, previous);
	}
}

static MALLOC_INLINE free_list_t
medium_free_list_get_previous(rack_t *rack, free_list_t ptr)
{
	MALLOC_ASSERT(ptr.p);
	if (medium_is_oob_free_entry(ptr)) {
		return medium_oob_free_entry_get_previous(ptr.oob);
	} else {
		return medium_inplace_free_entry_get_previous(rack, ptr.medium_inplace);
	}
}

static MALLOC_INLINE void
medium_free_list_set_next(rack_t *rack, free_list_t entry, free_list_t next)
{
	if (medium_is_oob_free_entry(entry)) {
		medium_oob_free_entry_set_next(entry.oob, next);
	} else {
		medium_inplace_free_entry_set_next(rack, entry.medium_inplace, next);
	}
}

static MALLOC_INLINE free_list_t
medium_free_list_get_next(rack_t *rack, free_list_t ptr)
{
	MALLOC_ASSERT(ptr.p);
	if (medium_is_oob_free_entry(ptr)) {
		return medium_oob_free_entry_get_next(ptr.oob);
	} else {
		return medium_inplace_free_entry_get_next(rack, ptr.medium_inplace);
	}
}

static MALLOC_INLINE free_list_t
medium_free_list_get_next_task(task_t task, memory_reader_t reader,
		print_task_printer_t printer, rack_t *rack, free_list_t ptr)
{
	MALLOC_ASSERT(ptr.p);
	if (medium_is_oob_free_entry(ptr)) {
		return medium_oob_free_entry_get_next_task(task, reader, printer, ptr.oob);
	} else {
		return medium_inplace_free_entry_get_next_task(task, reader, printer,
				rack, ptr.medium_inplace);
	}
}

static MALLOC_INLINE void *
medium_free_list_get_ptr(rack_t *rack, free_list_t ptr)
{
	if (!ptr.p) {
		return NULL;
	} else if (medium_is_oob_free_entry(ptr)) {
		return medium_oob_free_entry_get_ptr(ptr.oob);
	} else {
		return (void *)ptr.p;
	}
}

static MALLOC_INLINE void *
medium_free_list_get_ptr_task(task_t task, memory_reader_t reader,
		print_task_printer_t printer, free_list_t ptr)
{
	if (!ptr.p) {
		return NULL;
	} else if (medium_is_oob_free_entry(ptr)) {
		return medium_oob_free_entry_get_ptr_task(task, reader, printer, ptr.oob);
	} else {
		return (void *)ptr.p;
	}
}

// Returns a free_list_t that is either inline or not based on the
// pointer and msize.
static MALLOC_INLINE free_list_t
medium_free_list_from_ptr(rack_t *rack, void *ptr, msize_t msize)
{
	MALLOC_ASSERT(msize);

	// The default is to put the free_list_t in the memory that
	// the pointer leads to.
	free_list_t entry;
	entry.p = ptr;

	// If the pointer is page aligned, and the msize is greater
	// than a whole page, then we try and put the entry in
	// the out-of-band area instead.
	if (medium_needs_oob_free_entry(ptr, msize)) {
		oob_free_entry_t oobe = medium_oob_free_find_empty(ptr, msize);
		if (oobe) {
			medium_oob_free_entry_set_ptr(oobe, ptr);
			entry.oob = oobe;
		}
	}

	return entry;
}

static MALLOC_INLINE void
medium_free_mark_free(rack_t *rack, free_list_t entry, msize_t msize)
{
	// Marks both the start and end block of a free-list entry as free.
	void *ptr = medium_free_list_get_ptr(rack, entry);
	msize_t *meta_headers = MEDIUM_META_HEADER_FOR_PTR(ptr);
	uintptr_t start_index = MEDIUM_META_INDEX_FOR_PTR(ptr);
	uintptr_t end_index = MEDIUM_META_INDEX_FOR_PTR(ptr + MEDIUM_BYTES_FOR_MSIZE(msize) - 1);
	MALLOC_ASSERT(start_index <= end_index);

	medium_meta_header_set_is_free(meta_headers, start_index, msize);
	medium_meta_header_set_is_free(meta_headers, end_index, msize);
}

static MALLOC_INLINE void
medium_free_mark_middle(rack_t *rack, free_list_t entry, msize_t msize)
{
	// Marks both the start and end block of a free-list entry as "middle" (unfree).
	void *ptr = medium_free_list_get_ptr(rack, entry);
	msize_t *meta_headers = MEDIUM_META_HEADER_FOR_PTR(ptr);
	uintptr_t start_index = MEDIUM_META_INDEX_FOR_PTR(ptr);
	uintptr_t end_index = MEDIUM_META_INDEX_FOR_PTR(ptr + MEDIUM_BYTES_FOR_MSIZE(msize) - 1);
	MALLOC_ASSERT(start_index <= end_index);
	MALLOC_ASSERT((meta_headers[start_index] & ~MEDIUM_IS_FREE) == msize);

	medium_meta_header_set_middle(meta_headers, start_index);
	medium_meta_header_set_middle(meta_headers, end_index);
}

static MALLOC_INLINE void
medium_free_mark_unfree(rack_t *rack, free_list_t entry, msize_t msize)
{
	// Marks both the start and end block of a free-list entry as not free.
	void *ptr = medium_free_list_get_ptr(rack, entry);
	msize_t *meta_headers = MEDIUM_META_HEADER_FOR_PTR(ptr);
	uintptr_t start_index = MEDIUM_META_INDEX_FOR_PTR(ptr);
	uintptr_t end_index = MEDIUM_META_INDEX_FOR_PTR(ptr +
			MEDIUM_BYTES_FOR_MSIZE(msize) - 1);
	MALLOC_ASSERT(start_index <= end_index);

	medium_meta_header_set_not_free(meta_headers, start_index);
	medium_meta_header_set_not_free(meta_headers, end_index);
}

static MALLOC_INLINE unsigned int
medium_free_list_count(task_t task, memory_reader_t reader,
		print_task_printer_t printer, rack_t *rack, free_list_t ptr)
{
	unsigned int count = 0;
	while (ptr.p) {
		count++;
		ptr = medium_free_list_get_next_task(task, reader, printer, rack, ptr);
	}
	return count;
}

/*
 * Adds an item to the proper free list, and also marks the meta-header of the
 * block properly.
 * Assumes szone has been locked
 */
static free_list_t
medium_free_list_add_ptr(rack_t *rack, magazine_t *medium_mag_ptr, void *ptr, msize_t msize)
{
	grain_t slot = MEDIUM_FREE_SLOT_FOR_MSIZE(rack, msize);
	free_list_t free_head = medium_mag_ptr->mag_free_list[slot];

	// This will either return the free_list_t for the current pointer, or attempt
	// to reserve an OOB entry for us.
	free_list_t free_ptr = medium_free_list_from_ptr(rack, ptr, msize);

#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_report(ASL_LEVEL_INFO, "in %s, ptr=%p, msize=%d\n", __FUNCTION__, ptr, msize);
	}
	if (((uintptr_t)ptr) & (MEDIUM_QUANTUM - 1)) {
		malloc_zone_error(rack->debug_flags, true, "medium_free_list_add_ptr: Unaligned ptr %p\n", ptr);
	}
	if (!msize) {
		malloc_zone_error(rack->debug_flags, true, "medium_free_list_add_ptr: msize=0, %p\n", ptr);
	}
#endif

	medium_free_list_set_previous(rack, free_ptr, (free_list_t){ .p = NULL });
	medium_free_list_set_next(rack, free_ptr, free_head);

	// Set the start and end blocks of the meta header as "free". Marking the last block
	// allows coalescing the regions when we free adjacent regions.
	medium_free_mark_free(rack, free_ptr, msize);

	if (medium_free_list_get_ptr(rack, free_head)) {
#if DEBUG_MALLOC
		if (medium_free_list_get_previous(szone, free_head)) {
			malloc_zone_error(rack->debug_flags, true, "medium_free_list_add_ptr: Internal invariant broken (free_head->previous != NULL)\n"
						"ptr=%p slot=%d free_head=%p previous=%p\n", ptr, slot, (void *)free_head, free_head->previous.p);
		}
		if (!MEDIUM_PTR_IS_FREE(medium_free_list_get_ptr(szone, free_head))) {
			malloc_zone_error(rack->debug_flags, true, "medium_free_list_add_ptr: Internal invariant broken (free_head is not a free pointer)\n"
						"ptr=%p slot=%d free_head=%p\n", ptr, slot, (void *)medium_free_list_get_ptr(szone, free_head));
		}
#endif
		medium_free_list_set_previous(rack, free_head, free_ptr);
	} else {
		BITMAPN_SET(medium_mag_ptr->mag_bitmap, slot);
	}

	medium_mag_ptr->mag_free_list[slot] = free_ptr;
	return free_ptr;
}

/*
 * Removes the item pointed to by ptr in the proper free list.
 * Assumes szone has been locked
 */
static void
medium_free_list_remove_ptr_no_clear(rack_t *rack, magazine_t *medium_mag_ptr, free_list_t entry, msize_t msize)
{
	grain_t slot = MEDIUM_FREE_SLOT_FOR_MSIZE(rack, msize);
	free_list_t next, previous;

#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_report(ASL_LEVEL_INFO, "In %s, ptr=%p, msize=%d\n", __FUNCTION__, ptr, msize);
	}
#endif

	previous = medium_free_list_get_previous(rack, entry);
	next = medium_free_list_get_next(rack, entry);

	if (!medium_free_list_get_ptr(rack, previous)) {
		// The block to remove is the head of the free list
#if DEBUG_MALLOC
		if (medium_mag_ptr->mag_free_list[slot] != ptr) {
			malloc_zone_error(rack->debug_flags, true,
					"medium_free_list_remove_ptr_no_clear: Internal invariant broken (medium_mag_ptr->mag_free_list[slot])\n"
					"ptr=%p slot=%d msize=%d medium_mag_ptr->mag_free_list[slot]=%p\n", ptr, slot, msize,
					(void *)medium_mag_ptr->mag_free_list[slot]);
			return;
		}
#endif
		medium_mag_ptr->mag_free_list[slot] = next;
		if (!medium_free_list_get_ptr(rack, next)) {
			BITMAPN_CLR(medium_mag_ptr->mag_bitmap, slot);
		}
	} else {
		// Check that the next pointer of "previous" points to "entry".
		free_list_t prev_next = medium_free_list_get_next(rack, previous);
		if (medium_free_list_get_ptr(rack, prev_next) != medium_free_list_get_ptr(rack, entry)) {
			malloc_zone_error(rack->debug_flags, true,
					"medium_free_list_remove_ptr_no_clear: Internal invariant broken (next ptr of prev) for %p, prev_next=%p\n",
					medium_free_list_get_ptr(rack, entry), medium_free_list_get_ptr(rack, prev_next));
			__builtin_unreachable(); // Always crashes in malloc_zone_error().
		}
		medium_free_list_set_next(rack, previous, next);
	}

	if (medium_free_list_get_ptr(rack, next)) {
		// Check that the previous pointer of "next" points to "entry".
		free_list_t next_prev = medium_free_list_get_previous(rack, next);
		if (medium_free_list_get_ptr(rack, next_prev) != medium_free_list_get_ptr(rack, entry)) {
			malloc_zone_error(rack->debug_flags, true,
					"medium_free_list_remove_ptr_no_clear: Internal invariant broken (prev ptr of next) for %p, next_prev=%p\n",
					medium_free_list_get_ptr(rack, entry), medium_free_list_get_ptr(rack, next_prev));
			__builtin_unreachable(); // Always crashes in malloc_zone_error().
		}
		medium_free_list_set_previous(rack, next, previous);
	}

	if (medium_is_oob_free_entry(entry)) {
		medium_oob_free_entry_set_free(entry.oob);
	}
}

static void
medium_free_list_remove_ptr(rack_t *rack, magazine_t *medium_mag_ptr, free_list_t entry, msize_t msize)
{
	// In the general case we want to ensure we marked these entries as "middle"
	// while we are in this function. However, when we're moving free list entries
	// from/to the recirc depot we rely on the metadata bits being intact to
	// reconstruct the free list. In that case we have to be able to skip this
	// metadata manipulation.
	medium_free_mark_middle(rack, entry, msize);
	medium_free_list_remove_ptr_no_clear(rack, medium_mag_ptr, entry, msize);
}

// Find a free list entry by its pointer address. This should only really be used
// by medium_finalize_region, or similar, where the free_list_t entry of a known
// pointer is desired. Otherwise it is cheaper to always pull off the free lists.
static free_list_t
medium_free_list_find_by_ptr(rack_t *rack, magazine_t *medium_mag_ptr, void *ptr, msize_t msize)
{
	if (*MEDIUM_METADATA_FOR_PTR(ptr) == (MEDIUM_IS_FREE | msize)) {
		// If the block is marked free, and of size `msize`, then we first must check
		// if the alignment+size is such that we could have use an OOB-entry.
		if (medium_needs_oob_free_entry(ptr, msize)) {
			// Scan the OOB entries looking for this address.
			medium_region_t region = MEDIUM_REGION_FOR_PTR(ptr);
			for (int i=0; i<MEDIUM_OOB_COUNT; i++) {
				if (!region->medium_oob_free_entries[i].ptr) {
					continue;
				}
				if (medium_oob_free_entry_get_ptr(&region->medium_oob_free_entries[i]) == ptr) {
					return (free_list_t){ .oob = &region->medium_oob_free_entries[i] };
				}
			}
		}

		// Otherwise, the freed pointer will be in place.
		return (free_list_t){ .p = ptr };
	}

	malloc_zone_error(rack->debug_flags, true,
			"medium_free_list_find_by_ptr: ptr is not free (ptr metadata !MEDIUM_IS_FREE), "
			"ptr=%p msize=%d metadata=0x%x\n", ptr, msize, *MEDIUM_METADATA_FOR_PTR(ptr));
	__builtin_trap();
}

void
medium_finalize_region(rack_t *rack, magazine_t *medium_mag_ptr)
{
	void *last_block, *previous_block;
	msize_t last_msize, previous_msize, last_index;
	free_list_t previous;

	// It is possible that the block prior to the last block in the region has
	// been free'd, but was not coalesced with the free bytes at the end of the
	// block, since we treat the bytes at the end of the region as "in use" in
	// the meta headers. Attempt to coalesce the last block with the previous
	// block, so we don't violate the "no consecutive free blocks" invariant.
	//
	// FIXME: If we could calculate the previous medium free size in the same
	//        manner as tiny_previous_preceding_free, it would eliminate the
	//        index & previous msize checks, which are a guard against reading
	//        bogus data out of in-use or written-on-freed memory.
	//
	// FIXME: Need to investigate how much work would be required to increase
	//        'mag_bytes_free_at_end' when freeing the preceding block, rather
	//        than performing this workaround.
	//
	if (medium_mag_ptr->mag_bytes_free_at_end) {
		last_block = MEDIUM_REGION_END(medium_mag_ptr->mag_last_region) - medium_mag_ptr->mag_bytes_free_at_end;
		last_msize = MEDIUM_MSIZE_FOR_BYTES(medium_mag_ptr->mag_bytes_free_at_end);

		last_index = MEDIUM_META_INDEX_FOR_PTR(last_block);
		previous_msize = MEDIUM_PREVIOUS_MSIZE(last_block);

		if (last_index && (previous_msize <= last_index)) {
			previous_block = (void *)((uintptr_t)last_block - MEDIUM_BYTES_FOR_MSIZE(previous_msize));

			if (MEDIUM_PTR_IS_FREE(previous_block)) {
				previous = medium_free_list_find_by_ptr(rack, medium_mag_ptr, previous_block, previous_msize);
				medium_free_list_remove_ptr(rack, medium_mag_ptr, previous, previous_msize);
				last_block = previous_block;
				last_msize += previous_msize;
			}
		}

		// splice last_block into the free list
		medium_free_list_add_ptr(rack, medium_mag_ptr, last_block, last_msize);
		medium_mag_ptr->mag_bytes_free_at_end = 0;
	}

#if CONFIG_ASLR_INTERNAL
	free_list_t next;

	if (medium_mag_ptr->mag_bytes_free_at_start) {
		last_block = MEDIUM_REGION_ADDRESS(medium_mag_ptr->mag_last_region);
		last_msize = MEDIUM_MSIZE_FOR_BYTES(medium_mag_ptr->mag_bytes_free_at_start);

		void *next_block = (void *)((uintptr_t)last_block + medium_mag_ptr->mag_bytes_free_at_start);
		if (MEDIUM_PTR_IS_FREE(next_block)) {
			msize_t next_msize = MEDIUM_PTR_SIZE(next_block);
			next = medium_free_list_find_by_ptr(rack, medium_mag_ptr, next_block, next_msize);
			medium_free_list_remove_ptr(rack, medium_mag_ptr, next, next_msize);
			last_msize += next_msize;
		}

		// splice last_block into the free list
		medium_free_list_add_ptr(rack, medium_mag_ptr, last_block, last_msize);
		medium_mag_ptr->mag_bytes_free_at_start = 0;
	}
#endif

	// TODO: Will we ever need to coalesce the blocks at the beginning and end when we finalize?
	medium_mag_ptr->mag_last_region = NULL;
}

int
medium_free_detach_region(rack_t *rack, magazine_t *medium_mag_ptr, region_t r)
{
	unsigned char *ptr = MEDIUM_REGION_ADDRESS(r);
	msize_t *meta_headers = MEDIUM_META_HEADER_FOR_PTR(ptr);
	uintptr_t start = (uintptr_t)MEDIUM_REGION_ADDRESS(r);
	uintptr_t current = start;
	uintptr_t limit = (uintptr_t)MEDIUM_REGION_END(r);
	int total_alloc = 0;

	while (current < limit) {
		unsigned index = MEDIUM_META_INDEX_FOR_PTR(current);
		msize_t msize_and_free = meta_headers[index];
		boolean_t is_free = msize_and_free & MEDIUM_IS_FREE;
		msize_t msize = msize_and_free & ~MEDIUM_IS_FREE;

		if (!msize) {
#if DEBUG_MALLOC
			boolean_t is_free = msize_and_free & MEDIUM_IS_FREE;
			malloc_report(ASL_LEVEL_ERR, "*** medium_free_detach_region error with %p: msize=%d is_free=%d\n", (void *)current, msize, is_free);
#endif
			break;
		}

		if (is_free) {
			free_list_t entry = medium_free_list_find_by_ptr(rack, medium_mag_ptr, (void *)current, msize);
			medium_free_list_remove_ptr_no_clear(rack, medium_mag_ptr, entry, msize);
		} else {
			total_alloc++;
		}
		current += MEDIUM_BYTES_FOR_MSIZE(msize);
	}
	return total_alloc;
}

size_t
medium_free_reattach_region(rack_t *rack, magazine_t *medium_mag_ptr, region_t r)
{
	unsigned char *ptr = MEDIUM_REGION_ADDRESS(r);
	msize_t *meta_headers = MEDIUM_META_HEADER_FOR_PTR(ptr);
	uintptr_t start = (uintptr_t)MEDIUM_REGION_ADDRESS(r);
	uintptr_t current = start;
	uintptr_t limit = (uintptr_t)MEDIUM_REGION_END(r);
	size_t total_alloc = 0;

	while (current < limit) {
		unsigned index = MEDIUM_META_INDEX_FOR_PTR(current);
		msize_t msize_and_free = meta_headers[index];
		boolean_t is_free = msize_and_free & MEDIUM_IS_FREE;
		msize_t msize = msize_and_free & ~MEDIUM_IS_FREE;

		if (!msize) {
#if DEBUG_MALLOC
			malloc_report(ASL_LEVEL_ERR, "*** medium_free_reattach_region error with %p: msize=%d is_free=%d\n", (void *)current, msize, is_free);
#endif
			break;
		}
		if (is_free) {
			medium_free_list_add_ptr(rack, medium_mag_ptr, (void *)current, msize);
		} else {
			total_alloc += MEDIUM_BYTES_FOR_MSIZE(msize);
		}
		current += MEDIUM_BYTES_FOR_MSIZE(msize);
	}
	return total_alloc;
}

typedef struct _medium_advisory_s {
	struct _medium_advisory_s *next;
	size_t size;
} medium_advisory_s, *medium_advisory_t;

void
medium_free_scan_madvise_free(rack_t *rack, magazine_t *depot_ptr, region_t r)
{
	uintptr_t start = (uintptr_t)MEDIUM_REGION_ADDRESS(r);
	uintptr_t current = start;
	uintptr_t limit = (uintptr_t)MEDIUM_REGION_END(r);
	msize_t *meta_headers = MEDIUM_META_HEADER_FOR_PTR(start);
	msize_t *madv_headers = MEDIUM_MADVISE_HEADER_FOR_PTR(start);

	medium_advisory_t advisories = NULL;

	// Scan the metadata identifying blocks which span one or more pages. Mark
	// the pages MADV_FREE taking care to preserve free list management data.
	while (current < limit) {
		unsigned index = MEDIUM_META_INDEX_FOR_PTR(current);

		msize_t alloc_msize = meta_headers[index] & ~MEDIUM_IS_FREE;
		bool alloc_is_free = meta_headers[index] & MEDIUM_IS_FREE;

		if (alloc_is_free && alloc_msize == MEDIUM_MAX_MSIZE) {
			// The first allocation is both free and covers the entire of the
			// zone.
#if DEBUG_MALLOC
			// first block is all free
			malloc_report(ASL_LEVEL_ERR, "*** medium_free_scan_madvise_free "
					"first block is all free! %p: msize=%d is_free=%d\n",
					(void *)current, alloc_msize, alloc_is_free);
#endif
			uintptr_t pgLo = round_page_kernel(start + sizeof(medium_inplace_free_entry_s) +
					sizeof(msize_t));
			uintptr_t pgHi = trunc_page_kernel(start - sizeof(msize_t) +
					(NUM_MEDIUM_BLOCKS << SHIFT_MEDIUM_QUANTUM));

			if (pgLo < pgHi) {
				medium_advisory_t mat = (medium_advisory_t)pgLo;
				mat->next = advisories;
				mat->size = pgHi - pgLo;
			}
			break;
		}
		if (!alloc_msize) {
#if DEBUG_MALLOC
			malloc_report(ASL_LEVEL_ERR, "*** medium_free_scan_madvise_free "
					"error with %p: msize=%d is_free=%d\n", (void *)current,
					msize, is_free);
#endif
			break;
		}
		if (alloc_is_free) {
			msize_t advise_msize = madv_headers[index]
				& ~MEDIUM_IS_ADVISED;
			bool run_is_advised = madv_headers[index]
				& MEDIUM_IS_ADVISED;

			if (advise_msize != alloc_msize || !run_is_advised) {
				uintptr_t pgLo = round_page_kernel(current + 
						sizeof(medium_inplace_free_entry_s) + sizeof(msize_t));
				uintptr_t pgHi = trunc_page_kernel(current - sizeof(msize_t) +
						MEDIUM_BYTES_FOR_MSIZE(alloc_msize));

				if (pgLo < pgHi) {
					medium_advisory_t mat = (medium_advisory_t)pgLo;
					mat->next = advisories;
					mat->size = pgHi - pgLo;
				}

				memset(&madv_headers[index], 0, sizeof(uint16_t) * alloc_msize);
				medium_madvise_header_mark_clean(madv_headers, index,
						alloc_msize);
			}
		}
		current += MEDIUM_BYTES_FOR_MSIZE(alloc_msize);
	}

	if (advisories) {
		OSAtomicIncrement32Barrier(
				&(REGION_TRAILER_FOR_MEDIUM_REGION(r)->pinned_to_depot));
		SZONE_MAGAZINE_PTR_UNLOCK(depot_ptr);
		while (advisories) {
			uintptr_t addr = (uintptr_t)advisories;
			size_t size = advisories->size;
			advisories = advisories->next;

			mvm_madvise_free(rack, r, addr, addr + size, NULL,
					rack->debug_flags & MALLOC_DO_SCRIBBLE);
		}
		SZONE_MAGAZINE_PTR_LOCK(depot_ptr);
		OSAtomicDecrement32Barrier(
				&(REGION_TRAILER_FOR_MEDIUM_REGION(r)->pinned_to_depot));
	}
}

static region_t
medium_find_msize_region(rack_t *rack, magazine_t *medium_mag_ptr, mag_index_t mag_index, msize_t msize)
{
	void *ptr;
	grain_t slot = MEDIUM_FREE_SLOT_FOR_MSIZE(rack, msize);
	free_list_t *free_list = medium_mag_ptr->mag_free_list;
	free_list_t *the_slot = free_list + slot;
	free_list_t *limit;
	unsigned bitmap;

	// Assumes we've locked the magazine
	CHECK_MAGAZINE_PTR_LOCKED(szone, medium_mag_ptr, __PRETTY_FUNCTION__);

	// Look for an exact match by checking the freelist for this msize.
	ptr = medium_free_list_get_ptr(rack, *the_slot);
	if (ptr) {
		return MEDIUM_REGION_FOR_PTR(ptr);
	}

	// Mask off the bits representing slots holding free blocks smaller than
	// the size we need.
	//
	// BITMAPN_CTZ implementation
	unsigned idx = slot >> 5;
	bitmap = 0;
	unsigned mask = ~((1 << (slot & 31)) - 1);
	for (; idx < MEDIUM_FREELIST_BITMAP_WORDS(rack); ++idx) {
		bitmap = medium_mag_ptr->mag_bitmap[idx] & mask;
		if (bitmap != 0) {
			break;
		}
		mask = ~0U;
	}
	// Check for fallthrough: No bits set in bitmap
	if ((bitmap == 0) && (idx == MEDIUM_FREELIST_BITMAP_WORDS(rack))) {
		return NULL;
	}

	// Start looking at the first set bit, plus 32 bits for every word of
	// zeroes or entries that were too medium.
	slot = BITMAP32_CTZ((&bitmap)) + (idx * 32);

	limit = free_list + MEDIUM_FREE_SLOT_COUNT(rack) - 1;
	free_list += slot;

	if (free_list < limit) {
		ptr = medium_free_list_get_ptr(rack, *free_list);
		if (ptr) {
			return MEDIUM_REGION_FOR_PTR(ptr);
		} else {
			/* Shouldn't happen. Fall through to look at last slot. */
#if DEBUG_MALLOC
			malloc_report(ASL_LEVEL_ERR, "in medium_malloc_from_free_list(), mag_bitmap out of sync, slot=%d\n", slot);
#endif
		}
	}

	// We are now looking at the last slot, which contains blocks equal to, or
	// due to coalescing of free blocks, larger than (num_medium_slots - 1) * (medium quantum size).
	ptr = medium_free_list_get_ptr(rack, *limit);
	if (ptr) {
		return MEDIUM_REGION_FOR_PTR(ptr);
	}

	return NULL;
}

static boolean_t
medium_get_region_from_depot(rack_t *rack, magazine_t *medium_mag_ptr, mag_index_t mag_index, msize_t msize)
{
	magazine_t *depot_ptr = &(rack->magazines[DEPOT_MAGAZINE_INDEX]);

	/* FIXME: Would Uniprocessor benefit from recirc and MADV_FREE? */
	if (rack->num_magazines == 1) { // Uniprocessor, single magazine, so no recirculation necessary
		return 0;
	}

#if DEBUG_MALLOC
	if (DEPOT_MAGAZINE_INDEX == mag_index) {
		malloc_zone_error(rack->debug_flags, true, "medium_get_region_from_depot called for magazine index -1\n", NULL, NULL);
		return 0;
	}
#endif

	SZONE_MAGAZINE_PTR_LOCK(depot_ptr);

	// Appropriate a Depot'd region that can satisfy requested msize.
	region_trailer_t *node;
	region_t sparse_region;

	while (1) {
		sparse_region = medium_find_msize_region(rack, depot_ptr, DEPOT_MAGAZINE_INDEX, msize);
		if (NULL == sparse_region) { // Depot empty?
			SZONE_MAGAZINE_PTR_UNLOCK(depot_ptr);
			return 0;
		}

		node = REGION_TRAILER_FOR_MEDIUM_REGION(sparse_region);
		if (0 >= node->pinned_to_depot) {
			break;
		}

		SZONE_MAGAZINE_PTR_UNLOCK(depot_ptr);
		yield();
		SZONE_MAGAZINE_PTR_LOCK(depot_ptr);
	}

	// disconnect node from Depot
	recirc_list_extract(rack, depot_ptr, node);

	// Iterate the region pulling its free entries off the (locked) Depot's free list
	int objects_in_use = medium_free_detach_region(rack, depot_ptr, sparse_region);

	// Transfer ownership of the region
	MAGAZINE_INDEX_FOR_MEDIUM_REGION(sparse_region) = mag_index;
	node->pinned_to_depot = 0;

	// Iterate the region putting its free entries on its new (locked) magazine's free list
	size_t bytes_inplay = medium_free_reattach_region(rack, medium_mag_ptr, sparse_region);

	depot_ptr->mag_num_bytes_in_objects -= bytes_inplay;
	depot_ptr->num_bytes_in_magazine -= MEDIUM_REGION_PAYLOAD_BYTES;
	depot_ptr->mag_num_objects -= objects_in_use;

	medium_mag_ptr->mag_num_bytes_in_objects += bytes_inplay;
	medium_mag_ptr->num_bytes_in_magazine += MEDIUM_REGION_PAYLOAD_BYTES;
	medium_mag_ptr->mag_num_objects += objects_in_use;

	// connect to magazine as first node
	recirc_list_splice_first(rack, medium_mag_ptr, node);

	SZONE_MAGAZINE_PTR_UNLOCK(depot_ptr);

	MAGMALLOC_DEPOTREGION(MEDIUM_SZONE_FROM_RACK(rack), (int)mag_index, (void *)sparse_region, MEDIUM_REGION_SIZE,
						  (int)BYTES_USED_FOR_MEDIUM_REGION(sparse_region)); // DTrace USDT Probe

	return 1;
}

#if CONFIG_MADVISE_PRESSURE_RELIEF
void
medium_madvise_pressure_relief(rack_t *rack)
{
	mag_index_t mag_index;
	magazine_t *medium_depot_ptr = &rack->magazines[DEPOT_MAGAZINE_INDEX];

	for (mag_index = 0; mag_index < rack->num_magazines; mag_index++) {
		size_t index;
		for (index = 0; index < rack->region_generation->num_regions_allocated; ++index) {
			SZONE_LOCK(MEDIUM_SZONE_FROM_RACK(rack));

			region_t medium = rack->region_generation->hashed_regions[index];
			if (!medium || medium == HASHRING_REGION_DEALLOCATED) {
				SZONE_UNLOCK(MEDIUM_SZONE_FROM_RACK(rack));
				continue;
			}

			magazine_t *mag_ptr = mag_lock_zine_for_region_trailer(rack->magazines,
					REGION_TRAILER_FOR_MEDIUM_REGION(medium),
					MAGAZINE_INDEX_FOR_MEDIUM_REGION(medium));

			SZONE_UNLOCK(MEDIUM_SZONE_FROM_RACK(rack));

			/* Ordering is important here, the magazine of a region may potentially change
			 * during mag_lock_zine_for_region_trailer, so src_mag_index must be taken
			 * after we've obtained the lock.
			 */
			mag_index_t src_mag_index = MAGAZINE_INDEX_FOR_MEDIUM_REGION(medium);

			/* We can (and must) ignore magazines that are already in the recirc depot. */
			if (src_mag_index == DEPOT_MAGAZINE_INDEX) {
				SZONE_MAGAZINE_PTR_UNLOCK(mag_ptr);
				continue;
			}

			if (REGION_TRAILER_FOR_MEDIUM_REGION(medium)->pinned_to_depot > 0) {
				SZONE_MAGAZINE_PTR_UNLOCK(mag_ptr);
				continue;
			}

			if (medium == mag_ptr->mag_last_region && (mag_ptr->mag_bytes_free_at_end || mag_ptr->mag_bytes_free_at_start)) {
				medium_finalize_region(rack, mag_ptr);
			}

			/* Because this region is currently in use, we can't safely madvise it while
			 * it's attached to the magazine. For this operation we have to remove it from
			 * the current mag, attach it to the depot and then madvise.
			 */

			recirc_list_extract(rack, mag_ptr, REGION_TRAILER_FOR_MEDIUM_REGION(medium));
			int objects_in_use = medium_free_detach_region(rack, mag_ptr, medium);

			SZONE_MAGAZINE_PTR_LOCK(medium_depot_ptr);
			MAGAZINE_INDEX_FOR_MEDIUM_REGION(medium) = DEPOT_MAGAZINE_INDEX;
			REGION_TRAILER_FOR_MEDIUM_REGION(medium)->pinned_to_depot = 0;

			size_t bytes_inplay = medium_free_reattach_region(rack, medium_depot_ptr, medium);

			/* Fix up the metadata of the target magazine while the region is in the depot. */
			mag_ptr->mag_num_bytes_in_objects -= bytes_inplay;
			mag_ptr->num_bytes_in_magazine -= MEDIUM_REGION_PAYLOAD_BYTES;
			mag_ptr->mag_num_objects -= objects_in_use;

			/* Now we can drop the magazine lock of the source mag. */
			SZONE_MAGAZINE_PTR_UNLOCK(mag_ptr);

			medium_depot_ptr->mag_num_bytes_in_objects += bytes_inplay;
			medium_depot_ptr->num_bytes_in_magazine += MEDIUM_REGION_PAYLOAD_BYTES;
			medium_depot_ptr->mag_num_objects -= objects_in_use;

			recirc_list_splice_last(rack, medium_depot_ptr, REGION_TRAILER_FOR_MEDIUM_REGION(medium));

			/* Actually do the scan, done holding the depot lock, the call will drop the lock
			 * around the actual madvise syscalls.
			 */
			medium_free_scan_madvise_free(rack, medium_depot_ptr, medium);

			/* Now the region is in the recirc depot, the next allocations to require more
			 * blocks will come along and take one of these regions back out of the depot.
			 * As OS X madvise's reuse on an per-region basis, we leave as many of these
			 * regions in the depot as possible after memory pressure.
			 */
			SZONE_MAGAZINE_PTR_UNLOCK(medium_depot_ptr);
		}
	}
}
#endif // CONFIG_MADVISE_PRESSURE_RELIEF

static MALLOC_INLINE void
medium_madvise_free_range_conditional_no_lock(rack_t *rack, magazine_t *mag_ptr,
		size_t trigger_level, region_t region, free_list_t *fl, msize_t flmsz,
		void *ptr, size_t sz)
{
	region_trailer_t *node = REGION_TRAILER_FOR_MEDIUM_REGION(region);
	msize_t *madvh = MEDIUM_MADVISE_HEADER_FOR_PTR(ptr);

	msize_t trigger_msize = trigger_level >> SHIFT_MEDIUM_QUANTUM;

	size_t free_header_size = sizeof(medium_inplace_free_entry_s) + sizeof(msize_t);
	size_t free_trailer_size = sizeof(msize_t);
	// We have to avoid the free_list header at the start of the allocation
	// unless the entry is out in the OOB area, in which case, we can madvise
	// the whole region.
	if (medium_is_oob_free_entry(*fl)) {
		free_header_size = 0;
		free_trailer_size = 0;
	}

	uintptr_t rangep = (uintptr_t)medium_free_list_get_ptr(rack, *fl);
	msize_t range_idx = MEDIUM_META_INDEX_FOR_PTR(rangep);
	msize_t range_msz = flmsz;

	msize_t src_idx = MEDIUM_META_INDEX_FOR_PTR(ptr);
	msize_t src_msz = MEDIUM_MSIZE_FOR_BYTES(sz);
	msize_t src_end_idx = src_idx + src_msz - 1;
	medium_madvise_header_mark_middle(madvh, src_idx);
	medium_madvise_header_mark_middle(madvh, src_idx + src_msz - 1);

	msize_t left_end_idx = src_idx - 1;
	msize_t left_msz = src_idx - range_idx;
	msize_t right_start_idx = src_idx + src_msz;
	msize_t right_end_idx = range_idx + range_msz - 1;
	msize_t right_msz = right_end_idx - right_start_idx + 1;

	msize_t dirty_msz = src_msz;

	size_t vote_force = 0;
	msize_t left_dirty_msz = 0;
	if (range_idx < src_idx) {
		// Peek back one block and see if the range directly in front of this
		// one had any blocks that had not been madvised.
		left_dirty_msz = medium_madvise_header_dirty_len(madvh, left_end_idx);
		if (left_dirty_msz) {
			dirty_msz += left_dirty_msz;
		} else if (src_idx - range_idx > src_msz) {
			// The left-hand region was actually clean, so only choose to
			// madvise if the center region is enclosured by clean data.
			vote_force++;
		}
		medium_madvise_header_mark_middle(madvh, range_idx);
		medium_madvise_header_mark_middle(madvh, left_end_idx);
	}

	msize_t right_dirty_msz = 0;
	if (right_end_idx < src_end_idx) {
		// Same as above, if we had trailing data coalesced with this entry
		// and that was not madvised, consider it, too.
		right_dirty_msz = medium_madvise_header_dirty_len(madvh, right_start_idx);
		if (right_dirty_msz) {
			dirty_msz += right_dirty_msz;
		} else if (right_end_idx - right_start_idx > src_msz) {
			vote_force++;
		}
		medium_madvise_header_mark_middle(madvh, right_start_idx);
		medium_madvise_header_mark_middle(madvh, right_end_idx);
	}

	// We absolutely can't madvise lower the the free-list entry pointer plus
	// the header size. When the entry is OOB, there's no header or footer to
	// store in memory.
	uintptr_t safe_start_ptr = round_page_kernel(rangep + free_header_size);
	uintptr_t safe_end_ptr = trunc_page_kernel(rangep +
			MEDIUM_BYTES_FOR_MSIZE(range_msz) - free_trailer_size);

	// If the target region is madvisable, then madvise whatever we can but
	// bound it by the safe_start/end pointers to make sure we don't clobber
	// the free-list.
	if ((vote_force == 2) || (dirty_msz >= trigger_msize)) {
		uintptr_t lo = MAX(MEDIUM_PTR_FOR_META_INDEX(region, range_idx),
				safe_start_ptr);
		uintptr_t hi = MIN(MEDIUM_PTR_FOR_META_INDEX(region, range_idx) +
				MEDIUM_BYTES_FOR_MSIZE(range_msz), safe_end_ptr);

		// The page that contains the freelist entry needs to be marked as not
		// having been madvised.
		if (range_idx < MEDIUM_META_INDEX_FOR_PTR(safe_start_ptr)) {
			medium_madvise_header_mark_dirty(madvh, range_idx,
					MEDIUM_META_INDEX_FOR_PTR(safe_start_ptr) - range_idx);
		}
		if (range_idx + range_msz > MEDIUM_META_INDEX_FOR_PTR(safe_end_ptr)) {
			medium_madvise_header_mark_dirty(madvh,
					MEDIUM_META_INDEX_FOR_PTR(safe_end_ptr) + 1, range_idx + 
					range_msz - MEDIUM_META_INDEX_FOR_PTR(safe_end_ptr));
		}

		medium_madvise_header_mark_clean(madvh,
				MEDIUM_META_INDEX_FOR_PTR(lo),
				MEDIUM_META_INDEX_FOR_PTR(hi) - MEDIUM_META_INDEX_FOR_PTR(lo));

		// Mark the whole region as off-limits for the allocator while we drop
		// the lock and go to the kernel.
		medium_free_mark_unfree(rack, *fl, flmsz);
		medium_free_list_remove_ptr_no_clear(rack, mag_ptr, *fl, flmsz);
		OSAtomicIncrement32Barrier(&node->pinned_to_depot);
		SZONE_MAGAZINE_PTR_UNLOCK(mag_ptr);

		mvm_madvise_free(rack, region, lo, hi,
				&rack->last_madvise, rack->debug_flags & MALLOC_DO_SCRIBBLE);

		SZONE_MAGAZINE_PTR_LOCK(mag_ptr);
		OSAtomicDecrement32Barrier(&node->pinned_to_depot);
		*fl = medium_free_list_add_ptr(rack, mag_ptr, (void *)rangep, flmsz);
	} else {
		// We chose not to madvise, we need to re-mark the region as dirty
		// for when we come back to it later.
		if (left_dirty_msz < left_msz) {
			medium_madvise_header_mark_clean(madvh, range_idx,
					left_msz - left_dirty_msz);
		}
		if (right_dirty_msz < right_msz) {
			medium_madvise_header_mark_clean(madvh, right_start_idx +
					right_dirty_msz, right_msz - right_dirty_msz);
		}
		medium_madvise_header_mark_dirty(madvh, src_idx - left_dirty_msz,
				src_msz + right_dirty_msz + left_dirty_msz);
	}
}

#if CONFIG_AGGRESSIVE_MADVISE || CONFIG_RECIRC_DEPOT
static MALLOC_INLINE void
medium_madvise_free_range_no_lock(rack_t *rack,
								 magazine_t *medium_mag_ptr,
								 size_t trigger_level,
								 region_t region,
								 free_list_t freee,
								 msize_t fmsize,
								 void *headptr,
								 size_t headsize)
{
	void *ptr = medium_free_list_get_ptr(rack, freee);
	region_trailer_t *node = REGION_TRAILER_FOR_MEDIUM_REGION(region);

	// Lock on medium_magazines[mag_index] is already held here.
	// Calculate the first page in the coalesced block that would be safe to mark MADV_FREE
	size_t free_header_size = sizeof(medium_inplace_free_entry_s) + sizeof(msize_t);

	// If the free_list_t entry is out-of-line then we don't need to reserve any space
	// at the start of the region.
	if (medium_is_oob_free_entry(freee)) {
		free_header_size = 0;
	}

	uintptr_t safe_ptr = (uintptr_t)ptr + free_header_size;
	uintptr_t round_safe = round_page_kernel(safe_ptr);

	// Calculate the last page in the coalesced block that would be safe to mark MADV_FREE
	uintptr_t safe_extent = (uintptr_t)ptr + MEDIUM_BYTES_FOR_MSIZE(fmsize);
	uintptr_t trunc_extent = trunc_page_kernel(safe_extent);

	// If the region spans more than the madvise trigger level, then mark the
	// pages as advise-freed.
	if (round_safe + trigger_level < trunc_extent) {
		// Extend the freed block by the free region header and tail sizes to include pages
		// we may have coalesced that no longer host free region tails and headers.
		// This may extend over in-use ranges, but the MIN/MAX clamping below will fix that up.
		uintptr_t lo = trunc_page_kernel((uintptr_t)headptr);
		uintptr_t hi = round_page_kernel((uintptr_t)headptr + headsize + free_header_size);

		uintptr_t free_lo = MAX(round_safe, lo);
		uintptr_t free_hi = MIN(trunc_extent, hi);

		if (free_lo < free_hi) {
			// Before unlocking, ensure that the metadata for the freed region
			// makes it look not free but includes the length. This ensures that
			// any code that inspects the metadata while we are unlocked sees
			// a valid state and will not try to use or coalesce freed memory
			// into it.
			medium_free_mark_unfree(rack, freee, fmsize);
			medium_free_list_remove_ptr_no_clear(rack, medium_mag_ptr, freee, fmsize);
			OSAtomicIncrement32Barrier(&(node->pinned_to_depot));
			SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
			mvm_madvise_free(rack, region, free_lo, free_hi, &rack->last_madvise, rack->debug_flags & MALLOC_DO_SCRIBBLE);
			SZONE_MAGAZINE_PTR_LOCK(medium_mag_ptr);
			OSAtomicDecrement32Barrier(&(node->pinned_to_depot));
			medium_free_list_add_ptr(rack, medium_mag_ptr, ptr, fmsize);
		}
	}
}
#endif // CONFIG_AGGRESSIVE_MADVISE || CONFIG_RECIRC_DEPOT

#if CONFIG_RECIRC_DEPOT
static region_t
medium_free_try_depot_unmap_no_lock(rack_t *rack, magazine_t *depot_ptr, region_trailer_t *node)
{
	if (0 < node->bytes_used || 0 < node->pinned_to_depot || depot_ptr->recirculation_entries < recirc_retained_regions) {
		return NULL;
	}

	// disconnect first node from Depot
	recirc_list_extract(rack, depot_ptr, node);

	// Iterate the region pulling its free entries off the (locked) Depot's free list
	region_t sparse_region = MEDIUM_REGION_FOR_PTR(node);
	int objects_in_use = medium_free_detach_region(rack, depot_ptr, sparse_region);

	if (0 == objects_in_use) {
		// Invalidate the hash table entry for this region with HASHRING_REGION_DEALLOCATED.
		// Using HASHRING_REGION_DEALLOCATED preserves the collision chain, using HASHRING_OPEN_ENTRY (0) would not.
		rgnhdl_t pSlot = hash_lookup_region_no_lock(rack->region_generation->hashed_regions,
													rack->region_generation->num_regions_allocated,
													rack->region_generation->num_regions_allocated_shift,
													sparse_region);
		if (NULL == pSlot) {
			malloc_zone_error(rack->debug_flags, true, "medium_free_try_depot_unmap_no_lock hash lookup failed: %p\n", sparse_region);
			return NULL;
		}
		*pSlot = HASHRING_REGION_DEALLOCATED;
		depot_ptr->num_bytes_in_magazine -= MEDIUM_REGION_PAYLOAD_BYTES;
		// Atomically increment num_regions_dealloc
#ifdef __LP64___
		OSAtomicIncrement64(&rack->num_regions_dealloc);
#else
		OSAtomicIncrement32((int32_t *)&rack->num_regions_dealloc);
#endif

		// Caller will transfer ownership of the region back to the OS with no locks held
		MAGMALLOC_DEALLOCREGION(MEDIUM_SZONE_FROM_RACK(rack), (void *)sparse_region, (int)MEDIUM_REGION_SIZE); // DTrace USDT Probe
		return sparse_region;

	} else {
		malloc_zone_error(rack->debug_flags, true, "medium_free_try_depot_unmap_no_lock objects_in_use not zero: %d\n", objects_in_use);
		return NULL;
	}
}

static boolean_t
medium_free_do_recirc_to_depot(rack_t *rack, magazine_t *medium_mag_ptr, mag_index_t mag_index)
{
	// The entire magazine crossed the "emptiness threshold". Transfer a region
	// from this magazine to the Depot. Choose a region that itself has crossed the emptiness threshold (i.e
	// is at least fraction "f" empty.) Such a region will be marked "suitable" on the recirculation list.
	region_trailer_t *node = medium_mag_ptr->firstNode;

	while (node && (!node->recirc_suitable || node->pinned_to_depot)) {
		node = node->next;
	}

	if (NULL == node) {
#if DEBUG_MALLOC
		malloc_report(ASL_LEVEL_ERR, "*** medium_free_do_recirc_to_depot end of list\n");
#endif
		return TRUE; // Caller must SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
	}

	region_t sparse_region = MEDIUM_REGION_FOR_PTR(node);

	// Deal with unclaimed memory -- mag_bytes_free_at_end or mag_bytes_free_at start
	if (sparse_region == medium_mag_ptr->mag_last_region &&
		(medium_mag_ptr->mag_bytes_free_at_end || medium_mag_ptr->mag_bytes_free_at_start)) {
		medium_finalize_region(rack, medium_mag_ptr);
	}

	// disconnect "suitable" node from magazine
	recirc_list_extract(rack, medium_mag_ptr, node);

	// Iterate the region pulling its free entries off its (locked) magazine's free list
	int objects_in_use = medium_free_detach_region(rack, medium_mag_ptr, sparse_region);
	magazine_t *depot_ptr = &(rack->magazines[DEPOT_MAGAZINE_INDEX]);

	// hand over the region to the (locked) Depot
	SZONE_MAGAZINE_PTR_LOCK(depot_ptr);
	// this will cause medium_free_list_add_ptr called by medium_free_reattach_region to use
	// the depot as its target magazine, rather than magazine formerly associated with sparse_region
	MAGAZINE_INDEX_FOR_MEDIUM_REGION(sparse_region) = DEPOT_MAGAZINE_INDEX;
	node->pinned_to_depot = 0;

	// Iterate the region putting its free entries on Depot's free list
	size_t bytes_inplay = medium_free_reattach_region(rack, depot_ptr, sparse_region);

	medium_mag_ptr->mag_num_bytes_in_objects -= bytes_inplay;
	medium_mag_ptr->num_bytes_in_magazine -= MEDIUM_REGION_PAYLOAD_BYTES;
	medium_mag_ptr->mag_num_objects -= objects_in_use;

	SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr); // Unlock the originating magazine

	depot_ptr->mag_num_bytes_in_objects += bytes_inplay;
	depot_ptr->num_bytes_in_magazine += MEDIUM_REGION_PAYLOAD_BYTES;
	depot_ptr->mag_num_objects += objects_in_use;

	// connect to Depot as last node
	recirc_list_splice_last(rack, depot_ptr, node);

	MAGMALLOC_RECIRCREGION(MEDIUM_SZONE_FROM_RACK(rack), (int)mag_index, (void *)sparse_region, MEDIUM_REGION_SIZE,
						   (int)BYTES_USED_FOR_MEDIUM_REGION(sparse_region)); // DTrace USDT Probe

#if !CONFIG_AGGRESSIVE_MADVISE
	// Mark free'd dirty pages with MADV_FREE to reduce memory pressure
	medium_free_scan_madvise_free(rack, depot_ptr, sparse_region);
#endif

	// If the region is entirely empty vm_deallocate() it outside the depot lock
	region_t r_dealloc = medium_free_try_depot_unmap_no_lock(rack, depot_ptr, node);
	SZONE_MAGAZINE_PTR_UNLOCK(depot_ptr);
	if (r_dealloc) {
		mvm_deallocate_pages(r_dealloc, MEDIUM_REGION_SIZE, 0);
	}
	return FALSE; // Caller need not unlock the originating magazine
}

static MALLOC_INLINE boolean_t
medium_free_try_recirc_to_depot(rack_t *rack,
							   magazine_t *medium_mag_ptr,
							   mag_index_t mag_index,
							   region_t region,
							   free_list_t freee,
							   msize_t msize,
							   void *headptr,
							   size_t headsize)
{
	region_trailer_t *node = REGION_TRAILER_FOR_MEDIUM_REGION(region);
	size_t bytes_used = node->bytes_used;

	/* FIXME: Would Uniprocessor benefit from recirc and MADV_FREE? */
	if (rack->num_magazines == 1) { // Uniprocessor, single magazine, so no recirculation necessary
		/* NOTHING */
		return TRUE; // Caller must do SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr)
	} else if (DEPOT_MAGAZINE_INDEX != mag_index) {
		// Emptiness discriminant
		if (bytes_used < DENSITY_THRESHOLD(MEDIUM_REGION_PAYLOAD_BYTES)) {
			/* Region has crossed threshold from density to sparsity. Mark it "suitable" on the
			 * recirculation candidates list. */
			node->recirc_suitable = TRUE;
		} else {
			/* After this free, we've found the region is still dense, so it must have been even more so before
			 * the free. That implies the region is already correctly marked. Do nothing. */
		}

		// Has the entire magazine crossed the "emptiness threshold"? If so, transfer a region
		// from this magazine to the Depot. Choose a region that itself has crossed the emptiness threshold (i.e
		// is at least fraction "f" empty.) Such a region will be marked "suitable" on the recirculation list.

		size_t a = medium_mag_ptr->num_bytes_in_magazine;	// Total bytes allocated to this magazine
		size_t u = medium_mag_ptr->mag_num_bytes_in_objects; // In use (malloc'd) from this magaqzine

		if (a - u > ((3 * MEDIUM_REGION_PAYLOAD_BYTES) / 2) && u < DENSITY_THRESHOLD(a)) {
			return medium_free_do_recirc_to_depot(rack, medium_mag_ptr, mag_index);
		}

	} else {
#if !CONFIG_AGGRESSIVE_MADVISE
		// We are free'ing into the depot, so madvise as we do so unless we were madvising every incoming
		// allocation anyway.
		medium_madvise_free_range_no_lock(rack, medium_mag_ptr,
				vm_kernel_page_size, region, freee, msize, headptr, headsize);
#endif

		if (0 < bytes_used || 0 < node->pinned_to_depot) {
			/* Depot'd region is still live. Leave it in place on the Depot's recirculation list
			 * so as to avoid thrashing between the Depot's free list and a magazines's free list
			 * with detach_region/reattach_region */
		} else {
			/* Depot'd region is just now empty. Consider return to OS. */
			region_t r_dealloc = medium_free_try_depot_unmap_no_lock(rack, medium_mag_ptr, node);
			SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
			if (r_dealloc) {
				mvm_deallocate_pages(r_dealloc, MEDIUM_REGION_SIZE, 0);
			}
			return FALSE; // Caller need not unlock
		}
	}
	return TRUE; // Caller must do SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr)
}
#endif // CONFIG_RECIRC_DEPOT

static MALLOC_INLINE boolean_t
medium_free_no_lock(rack_t *rack, magazine_t *medium_mag_ptr, mag_index_t mag_index, region_t region, void *ptr, msize_t msize)
{
	msize_t *meta_headers = MEDIUM_META_HEADER_FOR_PTR(ptr);
	unsigned index = MEDIUM_META_INDEX_FOR_PTR(ptr);
	size_t original_size = MEDIUM_BYTES_FOR_MSIZE(msize);
	unsigned char *next_block = ((unsigned char *)ptr + original_size);
	msize_t next_index = index + msize;

	MALLOC_TRACE(TRACE_medium_free, (uintptr_t)rack, (uintptr_t)medium_mag_ptr, (uintptr_t)ptr, MEDIUM_BYTES_FOR_MSIZE(msize));

#if CONFIG_AGGRESSIVE_MADVISE || CONFIG_RECIRC_DEPOT
	void *original_ptr = ptr;
#endif

#if DEBUG_MALLOC
	if (!msize) {
		malloc_zone_error(rack->debug_flags, true, "trying to free medium block "
				"that is too small in medium_free_no_lock(), ptr=%p, msize=%d\n",
				ptr, msize);
	}
	if (medium_madvise_header_dirty_len(MEDIUM_MADVISE_HEADER_FOR_PTR(ptr), index) == 0) {
		malloc_zone_error(rack->debug_flags, true, "incoming freed block is not "
				"marked as dirty in madvise metadata, ptr=%p, msize=%d\n",
				ptr, msize);
	}
#endif

	// We try to coalesce this block with the preceeding one
	if (index > 0 && (meta_headers[index - 1] & MEDIUM_IS_FREE)) {
		msize_t previous_msize = meta_headers[index - 1] & ~MEDIUM_IS_FREE;
		grain_t previous_index = index - previous_msize;

		// Check if the metadata for the start of the block is also free.
		if (meta_headers[previous_index] == (previous_msize | MEDIUM_IS_FREE)) {
			void *previous_ptr = (void *)((uintptr_t)ptr - MEDIUM_BYTES_FOR_MSIZE(previous_msize));
			free_list_t previous = medium_free_list_find_by_ptr(rack, medium_mag_ptr, previous_ptr, previous_msize);
			medium_free_list_remove_ptr(rack, medium_mag_ptr, previous, previous_msize);
			ptr = previous_ptr;
			medium_meta_header_set_middle(meta_headers, index); // This block is now a middle block.
			msize += previous_msize;
			index -= previous_msize;
		} else {
			_os_set_crash_log_message("medium free list metadata inconsistency (headers[previous] != previous size)");
			__builtin_trap();
		}
	}

	// Try to coalesce with this block with the next block
	if ((next_block < MEDIUM_REGION_END(region)) && (meta_headers[next_index] & MEDIUM_IS_FREE)) {
		msize_t next_msize = meta_headers[next_index] & ~MEDIUM_IS_FREE;
		free_list_t next = medium_free_list_find_by_ptr(rack, medium_mag_ptr, next_block, next_msize);
		medium_free_list_remove_ptr(rack, medium_mag_ptr, next, next_msize);
		msize += next_msize;
	}

	if (rack->debug_flags & MALLOC_DO_SCRIBBLE) {
		if (!msize) {
			malloc_zone_error(rack->debug_flags, true, "incorrect size information for %p - block header was damaged\n", ptr);
		} else {
			memset(ptr, SCRABBLE_BYTE, MEDIUM_BYTES_FOR_MSIZE(msize));
		}
	}

	free_list_t freee = medium_free_list_add_ptr(rack, medium_mag_ptr, ptr, msize);

	// use original_size and not msize to avoid double counting the coalesced blocks
	medium_mag_ptr->mag_num_bytes_in_objects -= original_size;
	medium_mag_ptr->mag_num_objects--;

	// Update this region's bytes in use count
	region_trailer_t *node = REGION_TRAILER_FOR_MEDIUM_REGION(region);
	size_t bytes_used = node->bytes_used - original_size;
	node->bytes_used = (unsigned int)bytes_used;

	// Always attempt to madvise free regions that exceed the conditional
	// madvise limit size.
	medium_madvise_free_range_conditional_no_lock(rack, medium_mag_ptr,
			medium_sliding_madvise_granularity(medium_mag_ptr), region, &freee,
			msize, original_ptr, original_size);

	// Caller must do SZONE_MAGAZINE_PTR_UNLOCK(tiny_mag_ptr) if this function
	// returns TRUE.
	boolean_t needs_unlock = TRUE;

#if CONFIG_RECIRC_DEPOT
	needs_unlock = medium_free_try_recirc_to_depot(rack, medium_mag_ptr, mag_index, region, freee, msize, original_ptr, original_size);
#endif
	return needs_unlock;
}

// Allocates from the last region or a freshly allocated region
static void *
medium_malloc_from_region_no_lock(rack_t *rack,
								 magazine_t *medium_mag_ptr,
								 mag_index_t mag_index,
								 msize_t msize,
								 void *aligned_address)
{
	void *ptr;

	// Before anything we transform the mag_bytes_free_at_end or mag_bytes_free_at_start - if any - to a regular free block
	/* FIXME: last_block needs to be coalesced with previous entry if free, <rdar://5462322> */
	if (medium_mag_ptr->mag_bytes_free_at_end || medium_mag_ptr->mag_bytes_free_at_start) {
		medium_finalize_region(rack, medium_mag_ptr);
	}

	// Tag the region at "aligned_address" as belonging to us,
	// and so put it under the protection of the magazine lock we are holding.
	// Do this before advertising "aligned_address" on the hash ring(!)
	MAGAZINE_INDEX_FOR_MEDIUM_REGION(aligned_address) = mag_index;

	// Insert the new region into the hash ring
	rack_region_insert(rack, (region_t)aligned_address);

	medium_mag_ptr->mag_last_region = aligned_address;
	BYTES_USED_FOR_MEDIUM_REGION(aligned_address) = MEDIUM_BYTES_FOR_MSIZE(msize);

#if CONFIG_ASLR_INTERNAL
	int offset_msize = malloc_entropy[1] & MEDIUM_ENTROPY_MASK;
#if DEBUG_MALLOC
	if (getenv("MallocASLRForce")) {
		offset_msize = strtol(getenv("MallocASLRForce"), NULL, 0) & MEDIUM_ENTROPY_MASK;
	}
	if (getenv("MallocASLRPrint")) {
		malloc_report(ASL_LEVEL_INFO, "Region: %p offset: %d\n", aligned_address, offset_msize);
	}
#endif
#else
	int offset_msize = 0;
#endif
	ptr = (void *)((uintptr_t)aligned_address +
			MEDIUM_BYTES_FOR_MSIZE(offset_msize));
	medium_meta_header_set_in_use(MEDIUM_META_HEADER_FOR_PTR(ptr),
			offset_msize, msize);
	medium_madvise_header_mark_dirty(MEDIUM_MADVISE_HEADER_FOR_PTR(ptr),
			offset_msize, msize);
	medium_mag_ptr->mag_num_objects++;
	medium_mag_ptr->mag_num_bytes_in_objects += MEDIUM_BYTES_FOR_MSIZE(msize);
	medium_mag_ptr->num_bytes_in_magazine += MEDIUM_REGION_PAYLOAD_BYTES;

	// add a big free block at the end, mark as already advised clean
	// because we haven't touched it.
	msize_t trailing_offset = offset_msize + msize;
	msize_t trailing_msize = NUM_MEDIUM_BLOCKS - msize - offset_msize;
	medium_meta_header_set_in_use(MEDIUM_META_HEADER_FOR_PTR(ptr),
			trailing_offset, trailing_msize);
	medium_madvise_header_mark_clean(MEDIUM_MADVISE_HEADER_FOR_PTR(ptr),
			trailing_offset, trailing_msize);
	medium_mag_ptr->mag_bytes_free_at_end = 
		MEDIUM_BYTES_FOR_MSIZE(NUM_MEDIUM_BLOCKS - msize - offset_msize);

#if CONFIG_ASLR_INTERNAL
	// add a big free block at the start
	medium_mag_ptr->mag_bytes_free_at_start = MEDIUM_BYTES_FOR_MSIZE(offset_msize);
	if (offset_msize) {
		medium_meta_header_set_in_use(MEDIUM_META_HEADER_FOR_PTR(ptr), 0,
				offset_msize);
		medium_madvise_header_mark_clean(MEDIUM_MADVISE_HEADER_FOR_PTR(ptr), 0,
				offset_msize);
	}
#else
	medium_mag_ptr->mag_bytes_free_at_start = 0;
#endif

	// connect to magazine as last node
	recirc_list_splice_last(rack, medium_mag_ptr,
			REGION_TRAILER_FOR_MEDIUM_REGION(aligned_address));

	return ptr;
}

void *
medium_memalign(szone_t *szone, size_t alignment, size_t size, size_t span)
{
	msize_t mspan = MEDIUM_MSIZE_FOR_BYTES(span + MEDIUM_QUANTUM - 1);
	void *p = medium_malloc_should_clear(&szone->medium_rack, mspan, 0);

	if (NULL == p) {
		return NULL;
	}

	size_t offset = ((uintptr_t)p) & (alignment - 1);	// p % alignment
	size_t pad = (0 == offset) ? 0 : alignment - offset; // p + pad achieves desired alignment

	msize_t msize = MEDIUM_MSIZE_FOR_BYTES(size + MEDIUM_QUANTUM - 1);
	msize_t mpad = MEDIUM_MSIZE_FOR_BYTES(pad + MEDIUM_QUANTUM - 1);
	msize_t mwaste = mspan - msize - mpad; // excess blocks

	if (mpad > 0) {
		void *q = (void *)(((uintptr_t)p) + pad);

		// Mark q as block header and in-use, thus creating two blocks.
		magazine_t *medium_mag_ptr = mag_lock_zine_for_region_trailer(
				szone->medium_rack.magazines,
				REGION_TRAILER_FOR_MEDIUM_REGION(MEDIUM_REGION_FOR_PTR(p)),
				MAGAZINE_INDEX_FOR_MEDIUM_REGION(MEDIUM_REGION_FOR_PTR(p)));
		medium_meta_header_set_in_use(MEDIUM_META_HEADER_FOR_PTR(p), MEDIUM_META_INDEX_FOR_PTR(p), mpad);
		medium_madvise_header_mark_dirty(MEDIUM_MADVISE_HEADER_FOR_PTR(p), MEDIUM_META_INDEX_FOR_PTR(p), mpad);
		medium_meta_header_set_in_use(MEDIUM_META_HEADER_FOR_PTR(q), MEDIUM_META_INDEX_FOR_PTR(q), msize + mwaste);
		medium_madvise_header_mark_dirty(MEDIUM_MADVISE_HEADER_FOR_PTR(q), MEDIUM_META_INDEX_FOR_PTR(q), msize + mwaste);
		medium_mag_ptr->mag_num_objects++;
		SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);

		// Give up mpad blocks beginning at p to the medium free list
		free_medium(&szone->medium_rack, p, MEDIUM_REGION_FOR_PTR(p), MEDIUM_BYTES_FOR_MSIZE(mpad));

		p = q; // advance p to the desired alignment
	}
	if (mwaste > 0) {
		void *q = (void *)(((uintptr_t)p) + MEDIUM_BYTES_FOR_MSIZE(msize));
		// Mark q as block header and in-use, thus creating two blocks.
		magazine_t *medium_mag_ptr = mag_lock_zine_for_region_trailer(szone->medium_rack.magazines,
				REGION_TRAILER_FOR_MEDIUM_REGION(MEDIUM_REGION_FOR_PTR(p)),
				MAGAZINE_INDEX_FOR_MEDIUM_REGION(MEDIUM_REGION_FOR_PTR(p)));
		medium_meta_header_set_in_use(MEDIUM_META_HEADER_FOR_PTR(p), MEDIUM_META_INDEX_FOR_PTR(p), msize);
		medium_madvise_header_mark_dirty(MEDIUM_MADVISE_HEADER_FOR_PTR(p), MEDIUM_META_INDEX_FOR_PTR(p), msize);
		medium_meta_header_set_in_use(MEDIUM_META_HEADER_FOR_PTR(q), MEDIUM_META_INDEX_FOR_PTR(q), mwaste);
		medium_madvise_header_mark_dirty(MEDIUM_MADVISE_HEADER_FOR_PTR(q), MEDIUM_META_INDEX_FOR_PTR(q), mwaste);
		medium_mag_ptr->mag_num_objects++;
		SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);

		// Give up mwaste blocks beginning at q to the medium free list
		free_medium(&szone->medium_rack, q, MEDIUM_REGION_FOR_PTR(q), MEDIUM_BYTES_FOR_MSIZE(mwaste));
	}

	return p; // p has the desired size and alignment, and can later be free()'d
}

boolean_t
medium_claimed_address(rack_t *rack, void *ptr)
{
	region_t r = medium_region_for_ptr_no_lock(rack, ptr);
	return r && ptr < (void *)MEDIUM_REGION_END(r);
}

void *
medium_try_shrink_in_place(rack_t *rack, void *ptr, size_t old_size, size_t new_good_size)
{
	msize_t new_msize = MEDIUM_MSIZE_FOR_BYTES(new_good_size);
	msize_t mshrinkage = MEDIUM_MSIZE_FOR_BYTES(old_size) - new_msize;

	if (mshrinkage) {
		void *q = (void *)((uintptr_t)ptr + MEDIUM_BYTES_FOR_MSIZE(new_msize));
		magazine_t *medium_mag_ptr = mag_lock_zine_for_region_trailer(rack->magazines,
				REGION_TRAILER_FOR_MEDIUM_REGION(MEDIUM_REGION_FOR_PTR(ptr)),
				MAGAZINE_INDEX_FOR_MEDIUM_REGION(MEDIUM_REGION_FOR_PTR(ptr)));

		// Mark q as block header and in-use, thus creating two blocks.
		medium_meta_header_set_in_use(MEDIUM_META_HEADER_FOR_PTR(ptr), MEDIUM_META_INDEX_FOR_PTR(ptr), new_msize);
		medium_madvise_header_mark_dirty(MEDIUM_MADVISE_HEADER_FOR_PTR(ptr), MEDIUM_META_INDEX_FOR_PTR(ptr), new_msize);
		medium_meta_header_set_in_use(MEDIUM_META_HEADER_FOR_PTR(q), MEDIUM_META_INDEX_FOR_PTR(q), mshrinkage);
		medium_madvise_header_mark_dirty(MEDIUM_MADVISE_HEADER_FOR_PTR(q), MEDIUM_META_INDEX_FOR_PTR(q), mshrinkage);
		medium_mag_ptr->mag_num_objects++;

		SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
		free_medium(rack, q, MEDIUM_REGION_FOR_PTR(q), 0);
	}

	return ptr;
}

boolean_t
medium_try_realloc_in_place(rack_t *rack, void *ptr, size_t old_size, size_t new_size)
{
	// returns 1 on success
	msize_t *meta_headers = MEDIUM_META_HEADER_FOR_PTR(ptr);
	msize_t *madv_headers = MEDIUM_MADVISE_HEADER_FOR_PTR(ptr);
	unsigned index;
	msize_t old_msize, new_msize;
	unsigned next_index;
	void *next_block;
	msize_t next_msize_and_free;
	boolean_t is_free;
	msize_t next_msize, leftover_msize;
	void *leftover;

	index = MEDIUM_META_INDEX_FOR_PTR(ptr);
	old_msize = MEDIUM_MSIZE_FOR_BYTES(old_size);
	new_msize = MEDIUM_MSIZE_FOR_BYTES(new_size + MEDIUM_QUANTUM - 1);
	next_index = index + old_msize;

	if (next_index >= NUM_MEDIUM_BLOCKS) {
		return 0;
	}
	next_block = (char *)ptr + old_size;

#if DEBUG_MALLOC
	if ((uintptr_t)next_block & (MEDIUM_QUANTUM - 1)) {
		malloc_zone_error(rack->debug_flags, true, "internal invariant broken in realloc(next_block) for %p\n", next_block);
	}
	if (meta_headers[index] != old_msize) {
		malloc_report(ASL_LEVEL_ERR, "*** medium_try_realloc_in_place incorrect old %d %d\n", meta_headers[index], old_msize);
	}
#endif

	magazine_t *medium_mag_ptr = mag_lock_zine_for_region_trailer(rack->magazines,
			REGION_TRAILER_FOR_MEDIUM_REGION(MEDIUM_REGION_FOR_PTR(ptr)),
			MAGAZINE_INDEX_FOR_MEDIUM_REGION(MEDIUM_REGION_FOR_PTR(ptr)));
	if (DEPOT_MAGAZINE_INDEX == MAGAZINE_INDEX_FOR_MEDIUM_REGION(MEDIUM_REGION_FOR_PTR(ptr))) {
		SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
		return 0;
	}

	msize_t coalesced_msize = new_msize - old_msize;
#if CONFIG_MEDIUM_CACHE
	void *last_free_ptr = medium_mag_ptr->mag_last_free;
	msize_t last_free_msize = medium_mag_ptr->mag_last_free_msize;
	if (last_free_ptr == next_block && old_msize + last_free_msize >= new_msize) {
		/*
		 * There is a block in mag_last_free and it's immediately after
		 * this block and it's large enough. We can use some or all of it.
		 */
		leftover_msize = last_free_msize - coalesced_msize;
		if (leftover_msize) {
			medium_mag_ptr->mag_last_free_msize -= coalesced_msize;
			medium_mag_ptr->mag_last_free += new_size - old_size;
			// The block in mag_last_free is still marked as header and in-use, so copy that
			// state to the block that remains. The state for the block that we're going to
			// use is adjusted by the medium_meta_header_set_middle() call below.
			medium_meta_header_set_in_use(meta_headers, index + new_msize, leftover_msize);

			if (madv_headers[index] & MEDIUM_IS_ADVISED) {
				medium_madvise_header_mark_clean(madv_headers, index + new_msize, leftover_msize);
			} else {
				medium_madvise_header_mark_dirty(madv_headers, index + new_msize, leftover_msize);
			}
		} else {
			// Using the whole block.
			medium_mag_ptr->mag_last_free = NULL;
			medium_mag_ptr->mag_last_free_msize = 0;
			medium_mag_ptr->mag_last_free_rgn = NULL;
		}
		medium_meta_header_set_in_use(meta_headers, index, new_msize);
		medium_madvise_header_mark_dirty(madv_headers, index, new_msize);
		medium_meta_header_set_middle(meta_headers, next_index);
		medium_madvise_header_mark_middle(madv_headers, next_index);
	} else {
#endif // CONFIG_MEDIUM_CACHE
		/*
		 * Try to expand into unused space immediately after this block.
		 */
		msize_t unused_msize = MEDIUM_MSIZE_FOR_BYTES(medium_mag_ptr->mag_bytes_free_at_end);
		void *unused_start = MEDIUM_REGION_END(MEDIUM_REGION_FOR_PTR(ptr)) - medium_mag_ptr->mag_bytes_free_at_end;
		if (medium_mag_ptr->mag_last_region == MEDIUM_REGION_FOR_PTR(ptr)
				&& coalesced_msize < unused_msize && unused_start == ptr + old_size) {
			// Extend the in-use for this block to the new size
			medium_meta_header_set_in_use(meta_headers, index, new_msize);
			medium_madvise_header_mark_dirty(madv_headers, index, new_msize);

			// Clear the in-use size for the start of the area we extended into
			medium_meta_header_set_middle(meta_headers, next_index);
			medium_madvise_header_mark_middle(madv_headers, next_index);

			// Reduce mag_bytes_free_at_end and update its in-use size.
			medium_mag_ptr->mag_bytes_free_at_end -= MEDIUM_BYTES_FOR_MSIZE(coalesced_msize);
			medium_meta_header_set_in_use(meta_headers, index + new_msize,
					MEDIUM_MSIZE_FOR_BYTES(medium_mag_ptr->mag_bytes_free_at_end));
			medium_madvise_header_mark_clean(madv_headers, index + new_msize,
					MEDIUM_MSIZE_FOR_BYTES(medium_mag_ptr->mag_bytes_free_at_end));
		} else {
			/*
			 * Look for a free block immediately afterwards.  If it's large enough, we can consume (part of)
			 * it.
			 */
			next_msize_and_free = meta_headers[next_index];
			is_free = next_msize_and_free & MEDIUM_IS_FREE;
			if (!is_free) {
				SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
				return 0; // next_block is in use;
			}

			next_msize = next_msize_and_free & ~MEDIUM_IS_FREE;
			if (old_msize + next_msize < new_msize) {
				SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
				return 0; // even with next block, not enough
			}

			// The following block is big enough; pull it from its freelist and chop off enough to satisfy
			// our needs.
			free_list_t freee = medium_free_list_find_by_ptr(rack, medium_mag_ptr, next_block, next_msize);
			medium_free_list_remove_ptr(rack, medium_mag_ptr, freee, next_msize);
			medium_meta_header_set_middle(meta_headers, next_index);
			medium_madvise_header_mark_middle(madv_headers, next_index);
			leftover_msize = old_msize + next_msize - new_msize;
			if (leftover_msize) {
				/* there's some left, so put the remainder back */
				leftover = (unsigned char *)ptr + MEDIUM_BYTES_FOR_MSIZE(new_msize);
				medium_free_list_add_ptr(rack, medium_mag_ptr, leftover, leftover_msize);
			}
			medium_meta_header_set_in_use(meta_headers, index, new_msize);
			medium_madvise_header_mark_dirty(madv_headers, index, new_msize);
		}
#if CONFIG_MEDIUM_CACHE
	}
#endif // CONFIG_MEDIUM_CACHE
#if DEBUG_MALLOC
	if (MEDIUM_BYTES_FOR_MSIZE(new_msize) > szone->large_threshold) {
		malloc_report(ASL_LEVEL_ERR, "*** realloc in place for %p exceeded msize=%d\n", new_msize);
	}

	if (LOG(szone, ptr)) {
		malloc_report(ASL_LEVEL_INFO, "in medium_try_realloc_in_place(), ptr=%p, msize=%d\n", ptr, *MEDIUM_METADATA_FOR_PTR(ptr));
	}
#endif
	medium_mag_ptr->mag_num_bytes_in_objects += MEDIUM_BYTES_FOR_MSIZE(new_msize - old_msize);

	// Update this region's bytes in use count
	region_trailer_t *node = REGION_TRAILER_FOR_MEDIUM_REGION(MEDIUM_REGION_FOR_PTR(ptr));
	size_t bytes_used = node->bytes_used + MEDIUM_BYTES_FOR_MSIZE(new_msize - old_msize);
	node->bytes_used = (unsigned int)bytes_used;

	// Emptiness discriminant
	if (bytes_used < DENSITY_THRESHOLD(MEDIUM_REGION_PAYLOAD_BYTES)) {
		/* After this reallocation the region is still sparse, so it must have been even more so before
		 * the reallocation. That implies the region is already correctly marked. Do nothing. */
	} else {
		/* Region has crossed threshold from sparsity to density. Mark it not "suitable" on the
		 * recirculation candidates list. */
		node->recirc_suitable = FALSE;
	}

	SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
	CHECK(szone, __PRETTY_FUNCTION__);
	return 1;
}

static char *medium_check_fail_msg = "check: incorrect medium region ";

#define MEDIUM_CHECK_FAIL(fmt, ...) \
	malloc_zone_check_fail(medium_check_fail_msg, \
			"%ld, counter=%d\n" fmt,  region_index, counter, __VA_ARGS__);

boolean_t
medium_check_region(rack_t *rack, region_t region, size_t region_index,
		unsigned counter)
{
	unsigned char *ptr = MEDIUM_REGION_ADDRESS(region);
	msize_t *meta_headers = MEDIUM_META_HEADER_FOR_PTR(ptr);
	unsigned char *region_end = MEDIUM_REGION_END(region);
	msize_t prev_free = 0;
	unsigned index;
	msize_t msize_and_free;
	msize_t msize;
	free_list_t free_head, previous, next;
	msize_t *follower;
	mag_index_t mag_index = MAGAZINE_INDEX_FOR_MEDIUM_REGION(MEDIUM_REGION_FOR_PTR(ptr));
	magazine_t *medium_mag_ptr = &(rack->magazines[mag_index]);

	// Assumes locked
	CHECK_MAGAZINE_PTR_LOCKED(szone, medium_mag_ptr, __PRETTY_FUNCTION__);

	if (region == medium_mag_ptr->mag_last_region) {
		ptr += medium_mag_ptr->mag_bytes_free_at_start;
		region_end -= medium_mag_ptr->mag_bytes_free_at_end;
	}

	while (ptr < region_end) {
		index = MEDIUM_META_INDEX_FOR_PTR(ptr);
		msize_and_free = meta_headers[index];
		if (!(msize_and_free & MEDIUM_IS_FREE)) {
			// block is in use
			msize = msize_and_free;
			if (!msize) {
				MEDIUM_CHECK_FAIL("*** invariant broken: null msize ptr=%p num_medium_regions=%d end=%p\n", ptr,
							  (int)rack->num_regions, region_end);
				return 0;
			}
#if !CONFIG_RELAXED_INVARIANT_CHECKS
			if (MEDIUM_BYTES_FOR_MSIZE(msize) > szone->large_threshold) {
				MEDIUM_CHECK_FAIL("*** invariant broken for %p this medium msize=%d - size is too large\n", ptr, msize_and_free);
				return 0;
			}
#endif // CONFIG_RELAXED_INVARIANT_CHECKS
			ptr += MEDIUM_BYTES_FOR_MSIZE(msize);
			prev_free = 0;
		} else {
			// free pointer
			msize = msize_and_free & ~MEDIUM_IS_FREE;
			free_head = (free_list_t){ .p = ptr };
			follower = (msize_t *)FOLLOWING_MEDIUM_PTR(ptr, msize);
			if (!msize) {
				MEDIUM_CHECK_FAIL("*** invariant broken for free block %p this msize=%d\n", ptr, msize);
				return 0;
			}
#if !CONFIG_RELAXED_INVARIANT_CHECKS
			if (prev_free) {
				MEDIUM_CHECK_FAIL("*** invariant broken for %p (2 free in a row)\n", ptr);
				return 0;
			}
#endif

			// <rdar://problem/24680189> check for possible OOB entry if needed
			if (medium_needs_oob_free_entry(ptr, msize)) {
				oob_free_entry_t oob = medium_oob_free_find_ptr(ptr, msize);
				if (oob) {
					free_head.oob = oob;
				}
			}

			previous = medium_free_list_get_previous(rack, free_head);
			next = medium_free_list_get_next(rack, free_head);
			if (previous.p && !MEDIUM_PTR_IS_FREE(medium_free_list_get_ptr(rack, previous))) {
				MEDIUM_CHECK_FAIL("*** invariant broken for %p (previous %p is not a free pointer)\n", ptr, medium_free_list_get_ptr(rack, previous));
				return 0;
			}
			if (next.p && !MEDIUM_PTR_IS_FREE(medium_free_list_get_ptr(rack, next))) {
				MEDIUM_CHECK_FAIL("*** invariant broken for %p (next %p is not a free pointer)\n", ptr, medium_free_list_get_ptr(rack, next));
				return 0;
			}
			if (MEDIUM_PREVIOUS_MSIZE(follower) != msize) {
				MEDIUM_CHECK_FAIL("*** invariant broken for medium free %p followed by %p in region [%p-%p] "
						"(end marker incorrect) should be %d; in fact %d\n",
						ptr, follower, MEDIUM_REGION_ADDRESS(region), region_end, msize, MEDIUM_PREVIOUS_MSIZE(follower));
				return 0;
			}
			ptr = (unsigned char *)follower;
			prev_free = MEDIUM_IS_FREE;
		}
	}
	return 1;
}

kern_return_t
medium_in_use_enumerator(task_t task,
						void *context,
						unsigned type_mask,
						szone_t *szone,
						memory_reader_t reader,
						vm_range_recorder_t recorder)
{
	size_t num_regions;
	size_t index;
	region_t *regions;
	vm_range_t buffer[MAX_RECORDER_BUFFER];
	unsigned count = 0;
	kern_return_t err;
	region_t region;
	vm_range_t range;
	vm_range_t admin_range;
	vm_range_t ptr_range;
	unsigned char *mapped_region;
	msize_t *block_header;
	unsigned block_index;
	unsigned block_limit;
	msize_t msize_and_free;
	msize_t msize;
	magazine_t *medium_mag_base = NULL;

	region_hash_generation_t *srg_ptr;
	err = reader(task, (vm_address_t)szone->medium_rack.region_generation, sizeof(region_hash_generation_t), (void **)&srg_ptr);
	if (err) {
		return err;
	}

	num_regions = srg_ptr->num_regions_allocated;
	err = reader(task, (vm_address_t)srg_ptr->hashed_regions, sizeof(region_t) * num_regions, (void **)&regions);
	if (err) {
		return err;
	}

	if (type_mask & MALLOC_PTR_IN_USE_RANGE_TYPE) {
		// Map in all active magazines. Do this outside the iteration over regions.
		err = reader(task, (vm_address_t)(szone->medium_rack.magazines), szone->medium_rack.num_magazines * sizeof(magazine_t),
					 (void **)&medium_mag_base);
		if (err) {
			return err;
		}
	}

	for (index = 0; index < num_regions; ++index) {
		region = regions[index];
		if (HASHRING_OPEN_ENTRY != region && HASHRING_REGION_DEALLOCATED != region) {
			range.address = (vm_address_t)MEDIUM_REGION_ADDRESS(region);
			range.size = MEDIUM_REGION_SIZE;
			if (type_mask & MALLOC_ADMIN_REGION_RANGE_TYPE) {
				admin_range.address = range.address + MEDIUM_METADATA_START;
				admin_range.size = MEDIUM_METADATA_SIZE;
				recorder(task, context, MALLOC_ADMIN_REGION_RANGE_TYPE, &admin_range, 1);
			}
			if (type_mask & (MALLOC_PTR_REGION_RANGE_TYPE | MALLOC_ADMIN_REGION_RANGE_TYPE)) {
				ptr_range.address = range.address;
				ptr_range.size = NUM_MEDIUM_BLOCKS * MEDIUM_QUANTUM;
				recorder(task, context, MALLOC_PTR_REGION_RANGE_TYPE, &ptr_range, 1);
			}
			if (type_mask & MALLOC_PTR_IN_USE_RANGE_TYPE) {
				err = reader(task, range.address, range.size, (void **)&mapped_region);
				if (err) {
					return err;
				}

				mag_index_t mag_index = MAGAZINE_INDEX_FOR_MEDIUM_REGION(mapped_region);
				magazine_t *medium_mag_ptr = medium_mag_base + mag_index;

				int cached_free_blocks = 0;
#if CONFIG_MEDIUM_CACHE
				// Each magazine could have a pointer to a cached free block from
				// this region. Count the regions that have such a pointer.
				for (mag_index = 0; mag_index < szone->medium_rack.num_magazines; mag_index++) {
					if ((void *)range.address == (medium_mag_base + mag_index)->mag_last_free_rgn) {
						cached_free_blocks++;
					}
				}
#endif // CONFIG_MEDIUM_CACHE

				block_header = (msize_t *)(mapped_region + MEDIUM_METADATA_START + sizeof(region_trailer_t));
				block_index = 0;
				block_limit = NUM_MEDIUM_BLOCKS;
				if (region == medium_mag_ptr->mag_last_region) {
					block_index += MEDIUM_MSIZE_FOR_BYTES(medium_mag_ptr->mag_bytes_free_at_start);
					block_limit -= MEDIUM_MSIZE_FOR_BYTES(medium_mag_ptr->mag_bytes_free_at_end);
				}
				for (;block_index < block_limit; block_index += msize) {
					msize_and_free = block_header[block_index];
					msize = msize_and_free & ~MEDIUM_IS_FREE;
					if (!msize) {
						return KERN_FAILURE; // Somethings amiss. Avoid looping at this block_index.
					}
					if (!(msize_and_free & MEDIUM_IS_FREE)) {
						vm_address_t ptr = range.address + MEDIUM_BYTES_FOR_MSIZE(block_index);
#if CONFIG_MEDIUM_CACHE
						// If there are still magazines that have cached free
						// blocks in this region, check whether this is one of
						// them and don't return the block pointer if it is.
						boolean_t block_cached = false;
						if (cached_free_blocks) {
							for (mag_index = 0; mag_index < szone->medium_rack.num_magazines; mag_index++) {
								if ((void *)ptr == (medium_mag_base + mag_index)->mag_last_free) {
									block_cached = true;
									cached_free_blocks--;
									break;
								}
							}
						}
						if (block_cached) {
							continue;
						}
#endif // CONFIG_MEDIUM_CACHE
						// Block in use
						buffer[count].address = ptr;
						buffer[count].size = MEDIUM_BYTES_FOR_MSIZE(msize);
						count++;
						if (count >= MAX_RECORDER_BUFFER) {
							recorder(task, context, MALLOC_PTR_IN_USE_RANGE_TYPE, buffer, count);
							count = 0;
						}
					}
				}
				if (count) {
					recorder(task, context, MALLOC_PTR_IN_USE_RANGE_TYPE, buffer, count);
					count = 0;
				}
			}
		}
	}
	return 0;
}

static void *
medium_malloc_from_free_list(rack_t *rack, magazine_t *medium_mag_ptr, mag_index_t mag_index, msize_t msize)
{
	msize_t this_msize;
	bool was_madvised;
	grain_t slot = MEDIUM_FREE_SLOT_FOR_MSIZE(rack, msize);
	free_list_t *free_list = medium_mag_ptr->mag_free_list;
	free_list_t *the_slot = free_list + slot;
	free_list_t *limit;
	unsigned bitmap;
	msize_t leftover_msize;
	void *leftover_ptr;
	void *ptr;

	// Assumes we've locked the region
	CHECK_MAGAZINE_PTR_LOCKED(szone, medium_mag_ptr, __PRETTY_FUNCTION__);

	// Look for an exact match by checking the freelist for this msize.
	if (medium_free_list_get_ptr(rack, *the_slot)) {
		ptr = medium_free_list_get_ptr(rack, *the_slot);
		this_msize = msize;
		medium_free_list_remove_ptr(rack, medium_mag_ptr, *the_slot, msize);
		goto return_medium_alloc;
	}

	// Mask off the bits representing slots holding free blocks smaller than
	// the size we need.  If there are no larger free blocks, try allocating
	// from the free space at the end of the medium region.
	//
	// BITMAPN_CTZ implementation
	unsigned idx = slot >> 5;
	bitmap = 0;
	unsigned mask = ~((1 << (slot & 31)) - 1);
	for (; idx < MEDIUM_FREELIST_BITMAP_WORDS(rack); ++idx) {
		bitmap = medium_mag_ptr->mag_bitmap[idx] & mask;
		if (bitmap != 0) {
			break;
		}
		mask = ~0U;
	}
	// Check for fallthrough: No bits set in bitmap
	if ((bitmap == 0) && (idx == MEDIUM_FREELIST_BITMAP_WORDS(rack))) {
		goto try_medium_from_end;
	}

	// Start looking at the first set bit, plus 32 bits for every word of
	// zeroes or entries that were too medium.
	slot = BITMAP32_CTZ((&bitmap)) + (idx * 32);

	// FIXME: Explain use of - 1 here, last slot has special meaning
	limit = free_list + MEDIUM_FREE_SLOT_COUNT(rack) - 1;
	free_list += slot;

	// Attempt to pull off the free_list slot that we now think is full.
	if ((ptr = medium_free_list_get_ptr(rack, *free_list))) {
		this_msize = MEDIUM_PTR_SIZE(ptr);
		was_madvised = (medium_madvise_header_dirty_len(
				MEDIUM_MADVISE_HEADER_FOR_PTR(ptr), this_msize) == 0);
		medium_free_list_remove_ptr(rack, medium_mag_ptr, *free_list, this_msize);
		goto add_leftover_and_proceed;
	}

#if DEBUG_MALLOC
	malloc_report(ASL_LEVEL_ERR, "in medium_malloc_from_free_list(), mag_bitmap out of sync, slot=%d\n", slot);
#endif

try_medium_from_end:
	// Let's see if we can use medium_mag_ptr->mag_bytes_free_at_end
	if (medium_mag_ptr->mag_bytes_free_at_end >= MEDIUM_BYTES_FOR_MSIZE(msize)) {
		ptr = MEDIUM_REGION_END(medium_mag_ptr->mag_last_region) -
			medium_mag_ptr->mag_bytes_free_at_end;
		medium_mag_ptr->mag_bytes_free_at_end -= MEDIUM_BYTES_FOR_MSIZE(msize);
		if (medium_mag_ptr->mag_bytes_free_at_end) {
			// let's mark this block as in use to serve as boundary
			medium_meta_header_set_in_use(MEDIUM_META_HEADER_FOR_PTR(ptr),
					MEDIUM_META_INDEX_FOR_PTR((unsigned char *)ptr + MEDIUM_BYTES_FOR_MSIZE(msize)),
					MEDIUM_MSIZE_FOR_BYTES(medium_mag_ptr->mag_bytes_free_at_end));
			medium_madvise_header_mark_clean(MEDIUM_MADVISE_HEADER_FOR_PTR(ptr),
					MEDIUM_META_INDEX_FOR_PTR((unsigned char *)ptr + MEDIUM_BYTES_FOR_MSIZE(msize)),
					MEDIUM_MSIZE_FOR_BYTES(medium_mag_ptr->mag_bytes_free_at_end));
		}
		this_msize = msize;
		goto return_medium_alloc;
	}
#if CONFIG_ASLR_INTERNAL
	// Try from start if nothing left at end
	if (medium_mag_ptr->mag_bytes_free_at_start >= MEDIUM_BYTES_FOR_MSIZE(msize)) {
		ptr = MEDIUM_REGION_ADDRESS(medium_mag_ptr->mag_last_region) +
				medium_mag_ptr->mag_bytes_free_at_start -
				MEDIUM_BYTES_FOR_MSIZE(msize);
		medium_mag_ptr->mag_bytes_free_at_start -= MEDIUM_BYTES_FOR_MSIZE(msize);
		if (medium_mag_ptr->mag_bytes_free_at_start) {
			// let's mark this block as in use to serve as boundary
			medium_meta_header_set_in_use(MEDIUM_META_HEADER_FOR_PTR(ptr), 0,
					MEDIUM_MSIZE_FOR_BYTES(medium_mag_ptr->mag_bytes_free_at_start));
			medium_madvise_header_mark_clean(MEDIUM_MADVISE_HEADER_FOR_PTR(ptr), 0,
					MEDIUM_MSIZE_FOR_BYTES(medium_mag_ptr->mag_bytes_free_at_start));
		}
		this_msize = msize;
		goto return_medium_alloc;
	}
#endif
	return NULL;

add_leftover_and_proceed:
	if (this_msize > msize) {
		leftover_msize = this_msize - msize;
		leftover_ptr = (unsigned char *)ptr + MEDIUM_BYTES_FOR_MSIZE(msize);
#if DEBUG_MALLOC
		if (LOG(szone, ptr)) {
			malloc_report(ASL_LEVEL_INFO, "in medium_malloc_from_free_list(), adding leftover ptr=%p, this_msize=%d\n", ptr, this_msize);
		}
#endif
		medium_free_list_add_ptr(rack, medium_mag_ptr, leftover_ptr, leftover_msize);
		if (was_madvised) {
			medium_madvise_header_mark_clean(
					MEDIUM_MADVISE_HEADER_FOR_PTR(leftover_ptr),
					MEDIUM_META_INDEX_FOR_PTR(leftover_ptr), leftover_msize);
		} else {
			medium_madvise_header_mark_dirty(
					MEDIUM_MADVISE_HEADER_FOR_PTR(leftover_ptr),
					MEDIUM_META_INDEX_FOR_PTR(leftover_ptr), leftover_msize);
		}
		this_msize = msize;
	}

return_medium_alloc:
	medium_mag_ptr->mag_num_objects++;
	medium_mag_ptr->mag_num_bytes_in_objects += MEDIUM_BYTES_FOR_MSIZE(this_msize);

	// Update this region's bytes in use count
	region_trailer_t *node = REGION_TRAILER_FOR_MEDIUM_REGION(MEDIUM_REGION_FOR_PTR(ptr));
	size_t bytes_used = node->bytes_used + MEDIUM_BYTES_FOR_MSIZE(this_msize);
	node->bytes_used = (unsigned int)bytes_used;

	// Emptiness discriminant
	if (bytes_used < DENSITY_THRESHOLD(MEDIUM_REGION_PAYLOAD_BYTES)) {
		/* After this allocation the region is still sparse, so it must have been even more so before
		 * the allocation. That implies the region is already correctly marked. Do nothing. */
	} else {
		/* Region has crossed threshold from sparsity to density. Mark in not "suitable" on the
		 * recirculation candidates list. */
		node->recirc_suitable = FALSE;
	}
#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_report(ASL_LEVEL_INFO, "in medium_malloc_from_free_list(), ptr=%p, this_msize=%d, msize=%d\n", ptr, this_msize, msize);
	}
#endif
	medium_meta_header_set_in_use(MEDIUM_META_HEADER_FOR_PTR(ptr),
			MEDIUM_META_INDEX_FOR_PTR(ptr), this_msize);
	medium_madvise_header_mark_dirty(MEDIUM_MADVISE_HEADER_FOR_PTR(ptr),
			MEDIUM_META_INDEX_FOR_PTR(ptr), this_msize);
	return ptr;
}

void *
medium_malloc_should_clear(rack_t *rack, msize_t msize, boolean_t cleared_requested)
{
	void *ptr;
	mag_index_t mag_index = medium_mag_get_thread_index() % rack->num_magazines;
	magazine_t *medium_mag_ptr = &(rack->magazines[mag_index]);

	MALLOC_TRACE(TRACE_medium_malloc, (uintptr_t)rack, MEDIUM_BYTES_FOR_MSIZE(msize), (uintptr_t)medium_mag_ptr, cleared_requested);

	SZONE_MAGAZINE_PTR_LOCK(medium_mag_ptr);

#if CONFIG_MEDIUM_CACHE
	ptr = medium_mag_ptr->mag_last_free;

	if (medium_mag_ptr->mag_last_free_msize == msize) {
		// we have a winner
		medium_mag_ptr->mag_last_free = NULL;
		medium_mag_ptr->mag_last_free_msize = 0;
		medium_mag_ptr->mag_last_free_rgn = NULL;
		SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
		CHECK(szone, __PRETTY_FUNCTION__);
		if (cleared_requested) {
			memset(ptr, 0, MEDIUM_BYTES_FOR_MSIZE(msize));
		}
		return ptr;
	}
#endif /* CONFIG_MEDIUM_CACHE */

	while (1) {
		ptr = medium_malloc_from_free_list(rack, medium_mag_ptr, mag_index, msize);
		if (ptr) {
			SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
			CHECK(szone, __PRETTY_FUNCTION__);
			if (cleared_requested) {
				memset(ptr, 0, MEDIUM_BYTES_FOR_MSIZE(msize));
			}
			return ptr;
		}

		if (medium_get_region_from_depot(rack, medium_mag_ptr, mag_index, msize)) {
			ptr = medium_malloc_from_free_list(rack, medium_mag_ptr, mag_index, msize);
			if (ptr) {
				SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
				CHECK(szone, __PRETTY_FUNCTION__);
				if (cleared_requested) {
					memset(ptr, 0, MEDIUM_BYTES_FOR_MSIZE(msize));
				}
				return ptr;
			}
		}

		// The magazine is exhausted. A new region (heap) must be allocated to satisfy this call to malloc().
		// The allocation, an mmap() system call, will be performed outside the magazine spin locks by the first
		// thread that suffers the exhaustion. That thread sets "alloc_underway" and enters a critical section.
		// Threads arriving here later are excluded from the critical section, yield the CPU, and then retry the
		// allocation. After some time the magazine is resupplied, the original thread leaves with its allocation,
		// and retry-ing threads succeed in the code just above.
		if (!medium_mag_ptr->alloc_underway) {
			void *fresh_region;

			// time to create a new region (do this outside the magazine lock)
			medium_mag_ptr->alloc_underway = TRUE;
			OSMemoryBarrier();
			SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
			fresh_region = mvm_allocate_pages_securely(MEDIUM_REGION_SIZE,
					MEDIUM_BLOCKS_ALIGN, VM_MEMORY_MALLOC_MEDIUM, 
					rack->debug_flags);
			SZONE_MAGAZINE_PTR_LOCK(medium_mag_ptr);

			// DTrace USDT Probe
			MAGMALLOC_ALLOCREGION(MEDIUM_SZONE_FROM_RACK(rack), (int)mag_index,
					fresh_region, MEDIUM_REGION_SIZE);

			if (!fresh_region) { // out of memory!
				medium_mag_ptr->alloc_underway = FALSE;
				OSMemoryBarrier();
				SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
				return NULL;
			}

			ptr = medium_malloc_from_region_no_lock(rack, medium_mag_ptr,
					mag_index, msize, fresh_region);

			// we don't clear because this freshly allocated space is pristine
			medium_mag_ptr->alloc_underway = FALSE;
			OSMemoryBarrier();
			SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
			CHECK(szone, __PRETTY_FUNCTION__);
			return ptr;
		} else {
			SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
			yield();
			SZONE_MAGAZINE_PTR_LOCK(medium_mag_ptr);
		}
	}
	/* NOTREACHED */
}

size_t
medium_size(rack_t *rack, const void *ptr)
{
	if (medium_region_for_ptr_no_lock(rack, ptr)) {
		if (MEDIUM_META_INDEX_FOR_PTR(ptr) >= NUM_MEDIUM_BLOCKS) {
			return 0;
		}
		msize_t msize_and_free = *MEDIUM_METADATA_FOR_PTR(ptr);
		if (msize_and_free & MEDIUM_IS_FREE) {
			return 0;
		}
#if CONFIG_MEDIUM_CACHE
		{
			mag_index_t mag_index = MAGAZINE_INDEX_FOR_MEDIUM_REGION(MEDIUM_REGION_FOR_PTR(ptr));
			if (DEPOT_MAGAZINE_INDEX != mag_index) {
				magazine_t *medium_mag_ptr = &(rack->magazines[mag_index]);

				if (ptr == medium_mag_ptr->mag_last_free) {
					return 0;
				}
			} else {
				for (mag_index = 0; mag_index < rack->num_magazines; mag_index++) {
					magazine_t *medium_mag_ptr = &(rack->magazines[mag_index]);

					if (ptr == medium_mag_ptr->mag_last_free) {
						return 0;
					}
				}
			}
		}
#endif
		return MEDIUM_BYTES_FOR_MSIZE(msize_and_free);
	}

	return 0;
}

static MALLOC_NOINLINE void
free_medium_botch(rack_t *rack, void *ptr)
{
	mag_index_t mag_index = MAGAZINE_INDEX_FOR_MEDIUM_REGION(MEDIUM_REGION_FOR_PTR(ptr));
	magazine_t *medium_mag_ptr = &(rack->magazines[mag_index]);
	SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
	malloc_zone_error(rack->debug_flags, true, "double free for ptr %p\n", ptr);
}

void
free_medium(rack_t *rack, void *ptr, region_t medium_region, size_t known_size)
{
	msize_t msize;
	mag_index_t mag_index = MAGAZINE_INDEX_FOR_MEDIUM_REGION(MEDIUM_REGION_FOR_PTR(ptr));
	magazine_t *medium_mag_ptr = &(rack->magazines[mag_index]);

	// ptr is known to be in medium_region
	if (known_size) {
		msize = MEDIUM_MSIZE_FOR_BYTES(known_size + MEDIUM_QUANTUM - 1);
	} else {
		msize = MEDIUM_PTR_SIZE(ptr);
		if (MEDIUM_PTR_IS_FREE(ptr)) {
			free_medium_botch(rack, ptr);
			return;
		}
	}

	SZONE_MAGAZINE_PTR_LOCK(medium_mag_ptr);

#if CONFIG_MEDIUM_CACHE
	// Depot does not participate in CONFIG_MEDIUM_CACHE since it can't be directly malloc()'d
	if (DEPOT_MAGAZINE_INDEX != mag_index) {
		void *ptr2 = medium_mag_ptr->mag_last_free; // Might be NULL
		msize_t msize2 = medium_mag_ptr->mag_last_free_msize;
		region_t rgn2 = medium_mag_ptr->mag_last_free_rgn;

		/* check that we don't already have this pointer in the cache */
		if (ptr == ptr2) {
			free_medium_botch(rack, ptr);
			return;
		}

		if ((rack->debug_flags & MALLOC_DO_SCRIBBLE) && msize) {
			memset(ptr, SCRABBLE_BYTE, MEDIUM_BYTES_FOR_MSIZE(msize));
		}

		uint64_t madv_window = medium_sliding_madvise_granularity(medium_mag_ptr);
		if (MEDIUM_BYTES_FOR_MSIZE(msize) > madv_window) {
			uintptr_t lo = round_page_kernel((uintptr_t)ptr);
			uintptr_t hi = trunc_page_kernel((uintptr_t)ptr + MEDIUM_BYTES_FOR_MSIZE(msize));
			SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
			mvm_madvise_free(rack, medium_region, lo, hi, NULL, false);
			SZONE_MAGAZINE_PTR_LOCK(medium_mag_ptr);

			medium_madvise_header_mark_clean(MEDIUM_MADVISE_HEADER_FOR_PTR(ptr),
					MEDIUM_META_INDEX_FOR_PTR(ptr), msize);

			ptr2 = medium_mag_ptr->mag_last_free; // Might be NULL
			msize2 = medium_mag_ptr->mag_last_free_msize;
			rgn2 = medium_mag_ptr->mag_last_free_rgn;
		}


		medium_mag_ptr->mag_last_free = ptr;
		medium_mag_ptr->mag_last_free_msize = msize;
		medium_mag_ptr->mag_last_free_rgn = medium_region;

		if (!ptr2) {
			SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
			CHECK(szone, __PRETTY_FUNCTION__);
			return;
		}

		msize = msize2;
		ptr = ptr2;
		medium_region = rgn2;
	}
#endif /* CONFIG_MEDIUM_CACHE */

	// Now in the time it took to acquire the lock, the region may have migrated
	// from one magazine to another. I.e. trailer->mag_index is volatile.
	// In which case the magazine lock we obtained (namely magazines[mag_index].mag_lock)
	// is stale. If so, keep on tryin' ...
	region_trailer_t *trailer = REGION_TRAILER_FOR_MEDIUM_REGION(medium_region);
	mag_index_t refreshed_index;

	while (mag_index != (refreshed_index = trailer->mag_index)) { // Note assignment

		SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);

		mag_index = refreshed_index;
		medium_mag_ptr = &(rack->magazines[mag_index]);
		SZONE_MAGAZINE_PTR_LOCK(medium_mag_ptr);
	}

	if (medium_free_no_lock(rack, medium_mag_ptr, mag_index, medium_region, ptr, msize)) {
		SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
	}

	CHECK(szone, __PRETTY_FUNCTION__);
}

void
print_medium_free_list(task_t task, memory_reader_t reader,
		print_task_printer_t printer, rack_t *rack)
{
	free_list_t ptr;
	_SIMPLE_STRING b = _simple_salloc();
	mag_index_t mag_index;

	if (b) {
		rack_t *mapped_rack;
		magazine_t *mapped_magazines;
		if (reader(task, (vm_address_t)rack, sizeof(struct rack_s),
				(void **)&mapped_rack)) {
			printer("Failed to map medium rack\n");
			return;
		}
		if (reader(task, (vm_address_t)mapped_rack->magazines,
				mapped_rack->num_magazines * sizeof(magazine_t),
				(void **)&mapped_magazines)) {
			printer("Failed to map medium rack magazines\n");
			return;
		}

		_simple_sappend(b, "medium free sizes:\n");
		grain_t free_slots = MEDIUM_FREE_SLOT_COUNT(mapped_rack);
		for (mag_index = -1; mag_index < mapped_rack->num_magazines;
				mag_index++) {
			grain_t slot = 0;
			if (mag_index == -1) {
				_simple_sprintf(b, "\tRecirc depot: ");
			} else {
				_simple_sprintf(b, "\tMagazine %d: ", mag_index);
			}
			while (slot < free_slots) {
				ptr = mapped_magazines[mag_index].mag_free_list[slot];
				if (medium_free_list_get_ptr_task(task, reader, printer, ptr)) {
					_simple_sprintf(b, "%s%y[%lld]; ", (slot == free_slots - 1) ?
							">=" : "", (slot + 1) * MEDIUM_QUANTUM,
							medium_free_list_count(task, reader, printer,
									rack, ptr));
				}
				slot++;
			}
			_simple_sappend(b, "\n");
		}
		printer("%s\n", _simple_string(b));
		_simple_sfree(b);
	}
}

void
print_medium_region(task_t task, memory_reader_t reader,
		print_task_printer_t printer, szone_t *szone, int level,
		region_t region, size_t bytes_at_start, size_t bytes_at_end)
{
	unsigned counts[1024];
	unsigned in_use = 0;
	uintptr_t start = (uintptr_t)MEDIUM_REGION_ADDRESS(region);
	uintptr_t current = start + bytes_at_start;
	uintptr_t limit = (uintptr_t)MEDIUM_REGION_END(region) - bytes_at_end;
	uintptr_t mapped_start;
	msize_t msize_and_free;
	msize_t msize;
	unsigned ci;
	_SIMPLE_STRING b;
	uintptr_t pgTot = 0;
	uintptr_t advTot = 0;

	if (reader(task, (vm_address_t)start, MEDIUM_REGION_SIZE,
			(void **)&mapped_start)) {
		printer("Failed to map small region at %p\n", start);
		return;
	}
	off_t start_offset = mapped_start - start;
	region_t mapped_region = (region_t)mapped_start;

	if (region == HASHRING_REGION_DEALLOCATED) {
		if ((b = _simple_salloc()) != NULL) {
			_simple_sprintf(b, "Medium region [unknown address] was returned to the OS\n");
			printer("%s\n", _simple_string(b));
			_simple_sfree(b);
		}
		return;
	}

	memset(counts, 0, sizeof(counts));
	while (current < limit) {
		msize_and_free = *(uintptr_t *)((char *)MEDIUM_METADATA_FOR_PTR(current)
				+ start_offset);
		msize = msize_and_free & ~MEDIUM_IS_FREE;
		if (!msize) {
			printer("*** error with %p: msize=%d, free: %x\n", (void *)current,
					(unsigned)msize, msize_and_free & MEDIUM_IS_FREE);
			break;
		}
		if (!(msize_and_free & MEDIUM_IS_FREE)) {
			// block in use
			if (msize < 1024) {
				counts[msize]++;
			}
			in_use++;
		} else {
			uintptr_t pgLo = round_page_quanta(current + 
					sizeof(medium_inplace_free_entry_s) + sizeof(msize_t));
			uintptr_t pgHi = trunc_page_quanta(current + 
					MEDIUM_BYTES_FOR_MSIZE(msize) - sizeof(msize_t));

			if (pgLo < pgHi) {
				pgTot += (pgHi - pgLo);
			}

			uintptr_t curAdv = current;
			uintptr_t limAdv = current + MEDIUM_BYTES_FOR_MSIZE(msize);
			while (curAdv < limAdv) {
				msize_t adv = *(MEDIUM_MADVISE_HEADER_FOR_PTR(curAdv) + 
					MEDIUM_META_INDEX_FOR_PTR(curAdv));
				uintptr_t bytes = MEDIUM_BYTES_FOR_MSIZE(adv & ~MEDIUM_IS_ADVISED);
				if (adv & MEDIUM_IS_ADVISED) {
					advTot += bytes;
				}
				curAdv += bytes;
			}
		}
		current += MEDIUM_BYTES_FOR_MSIZE(msize);
	}
	if ((b = _simple_salloc()) != NULL) {
		mag_index_t mag_index = MAGAZINE_INDEX_FOR_MEDIUM_REGION(mapped_region);
		_simple_sprintf(b, "Medium region [%p-%p, %y] \t", (void *)start,
				MEDIUM_REGION_END(region), (int)MEDIUM_REGION_SIZE);
		if (mag_index == DEPOT_MAGAZINE_INDEX) {
			_simple_sprintf(b, "Recirc depot \t");
		} else {
			_simple_sprintf(b, "Magazine=%d \t", mag_index);
		}
		_simple_sprintf(b, "Allocations in use=%d \t Bytes in use=%ly (%d%%) \t",
				in_use, BYTES_USED_FOR_MEDIUM_REGION(region),
				(int)(100.0F * BYTES_USED_FOR_MEDIUM_REGION(mapped_region))/MEDIUM_REGION_SIZE);
		if (bytes_at_end || bytes_at_start) {
			_simple_sprintf(b, "Untouched=%ly ", bytes_at_end + bytes_at_start);
		}
		_simple_sprintf(b, "Advised=%ly ", advTot);
#if CONFIG_RECIRC_DEPOT
		_simple_sprintf(b, medium_region_below_recirc_threshold(mapped_region) ?
				"\tEmpty enough to be moved to recirc depot" :
				"\tNot empty enough to be moved to recirc depot");
#endif // CONFIG_RECIRC_DEPOT
		_simple_sprintf(b, "Dirty=%ly ", MEDIUM_REGION_PAYLOAD_BYTES - 
				bytes_at_start - bytes_at_end -
				BYTES_USED_FOR_MEDIUM_REGION(mapped_region) - advTot);
		if (level >= MALLOC_VERBOSE_PRINT_LEVEL && in_use) {
			_simple_sappend(b, "\n\tSizes in use: ");
			for (ci = 0; ci < 1024; ci++) {
				if (counts[ci]) {
					_simple_sprintf(b, "%y[%d] ", MEDIUM_BYTES_FOR_MSIZE(ci),
							counts[ci]);
				}
			}
		}
		printer("%s\n", _simple_string(b));
		_simple_sfree(b);
	}
}

void
print_medium_region_vis(szone_t *szone, region_t region)
{
	_SIMPLE_STRING b;

	if (region == HASHRING_REGION_DEALLOCATED) {
		if ((b = _simple_salloc()) != NULL) {
			_simple_sprintf(b, "Medium region [unknown address] was returned "
					"to the kernel\n");
			malloc_report(MALLOC_REPORT_NOLOG | MALLOC_REPORT_NOPREFIX,
					"%s\n", _simple_string(b));
			_simple_sfree(b);
		}
		return;
	}

	msize_t *metah = MEDIUM_META_HEADER_FOR_PTR(region);
	msize_t *madvh = MEDIUM_MADVISE_HEADER_FOR_PTR(region);

	mag_index_t mag_idx = MAGAZINE_INDEX_FOR_MEDIUM_REGION(region);
	magazine_t *mag = &szone->medium_rack.magazines[mag_idx];

	msize_t cur_msz = 0;
	bool is_free = false;
	msize_t cur_adv_msz = 0;
	bool is_advised = false;

	if ((b = _simple_salloc()) == NULL) {
		abort();
	}

	malloc_report(MALLOC_REPORT_NOLOG | MALLOC_REPORT_NOPREFIX,
			"Medium region [%p-%p, %y, %y]\n", (void *)region,
			MEDIUM_REGION_END(region), (int)MEDIUM_REGION_SIZE,
			((medium_region_t)region)->trailer.bytes_used);

	for (size_t x = 0; x < NUM_MEDIUM_BLOCKS; x++) {
		bool is_last_rgn = mag->mag_last_region == region;
		bool is_in_free_trailer = is_last_rgn && ((x << SHIFT_MEDIUM_QUANTUM) >
				(MEDIUM_REGION_SIZE - mag->mag_bytes_free_at_end));
		bool is_blk_start = false;
		bool is_blk_end = cur_msz == 1;
		bool is_adv_boundary = false;

		if (cur_msz == 0) {
			cur_msz = metah[x] & ~MEDIUM_IS_FREE;
			is_free = metah[x] & MEDIUM_IS_FREE;
			is_blk_start = true;
		}
		if (cur_adv_msz == 0 && madvh[x]) {
			cur_adv_msz = madvh[x] & ~MEDIUM_IS_FREE;
			is_advised = madvh[x] & MEDIUM_IS_FREE;
			is_adv_boundary = true;
		}
		if (cur_adv_msz > 0) {
			cur_adv_msz--;
		}

		const char *sigil = "?";
		if (is_in_free_trailer) {
			sigil = "_";
		} else if (cur_msz > 0) {
			if (is_free) {
				if (is_advised) { sigil = "~";  }
				else if (is_blk_start && cur_msz == 1) { sigil = "."; }
				else if (is_blk_start) { sigil = "<"; }
				else if (is_blk_end) { sigil = ">"; }
				else if (is_adv_boundary) { sigil = "!"; }
				else { sigil = "."; }
			} else {
				if (is_blk_start && cur_msz == 1) { sigil = "#"; }
				else if (is_blk_start) { sigil = "["; }
				else if (is_blk_end) { sigil = "]"; }
				else { sigil = "#"; }
			}
			cur_msz--;
		}

		if (x > 0 && (x % 128) == 0) {
			malloc_report(MALLOC_REPORT_NOLOG | MALLOC_REPORT_NOPREFIX, "%s\n",
					_simple_string(b));
			_simple_sfree(b);
			if ((b = _simple_salloc()) == NULL) {
				abort();
			}
			if (is_in_free_trailer) {
				break;
			}
		}

		_simple_sappend(b, sigil);
	}
	_simple_sfree(b);
}

static char *medium_freelist_fail_msg = "check: medium free list incorrect";

#define MEDIUM_FREELIST_FAIL(fmt, ...) \
	malloc_zone_check_fail(medium_freelist_fail_msg, \
			" (slot=%u), counter=%d\n" fmt,  slot, counter, __VA_ARGS__);

boolean_t
medium_free_list_check(rack_t *rack, grain_t slot, unsigned counter)
{
	mag_index_t mag_index;

	for (mag_index = -1; mag_index < rack->num_magazines; mag_index++) {
		magazine_t *medium_mag_ptr = &(rack->magazines[mag_index]);
		SZONE_MAGAZINE_PTR_LOCK(medium_mag_ptr);

		unsigned count = 0;
		free_list_t current = rack->magazines[mag_index].mag_free_list[slot];
		free_list_t previous = (free_list_t){ .p = NULL };
		msize_t msize_and_free;
		void *ptr = NULL;

		while ((ptr = medium_free_list_get_ptr(rack, current))) {
			msize_and_free = *MEDIUM_METADATA_FOR_PTR(ptr);
			if (!(msize_and_free & MEDIUM_IS_FREE)) {
				MEDIUM_FREELIST_FAIL("*** in-use ptr in free list slot=%u count=%d ptr=%p\n", slot, count, ptr);
				SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
				return 0;
			}
			if (((uintptr_t)ptr) & (MEDIUM_QUANTUM - 1)) {
				MEDIUM_FREELIST_FAIL("*** unaligned ptr in free list slot=%u count=%d ptr=%p\n", slot, count, ptr);
				SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
				return 0;
			}
			if (!medium_region_for_ptr_no_lock(rack, ptr)) {
				MEDIUM_FREELIST_FAIL("*** ptr not in szone slot=%d count=%d ptr=%p\n", slot, count, ptr);
				SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
				return 0;
			}
			if (medium_free_list_get_previous(rack, current).p != previous.p) {
				MEDIUM_FREELIST_FAIL("*** previous incorrectly set slot=%u count=%d ptr=%p\n", slot, count, ptr);
				SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
				return 0;
			}
			previous = current;
			current = medium_free_list_get_next(rack, current);
			count++;
		}

		SZONE_MAGAZINE_PTR_UNLOCK(medium_mag_ptr);
	}
	return 1;
}

#endif // CONFIG_MEDIUM_ALLOCATOR
