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

#if DEBUG_MALLOC
static void
large_debug_print(szone_t *szone)
{
	unsigned index;
	large_entry_t *range;
	_SIMPLE_STRING b = _simple_salloc();

	if (b) {
		for (index = 0, range = szone->large_entries; index < szone->num_large_entries; index++, range++) {
			if (range->address) {
				_simple_sprintf(b, "%d: %p(%y);  ", index, range->address, range->size);
			}
		}

		_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX, "%s\n", _simple_string(b));
		_simple_sfree(b);
	}
}
#endif

/*
 * Scan the hash ring looking for an entry for the given pointer.
 */
large_entry_t *
large_entry_for_pointer_no_lock(szone_t *szone, const void *ptr)
{
	// result only valid with lock held
	unsigned num_large_entries = szone->num_large_entries;
	unsigned hash_index;
	unsigned index;
	large_entry_t *range;

	if (!num_large_entries) {
		return NULL;
	}

	hash_index = ((uintptr_t)ptr >> vm_page_quanta_shift) % num_large_entries;
	index = hash_index;

	do {
		range = szone->large_entries + index;
		if (range->address == (vm_address_t)ptr) {
			return range;
		}
		if (0 == range->address) {
			return NULL; // end of chain
		}
		index++;
		if (index == num_large_entries) {
			index = 0;
		}
	} while (index != hash_index);

	return NULL;
}

static void
large_entry_insert_no_lock(szone_t *szone, large_entry_t range)
{
	unsigned num_large_entries = szone->num_large_entries;
	unsigned hash_index = (((uintptr_t)(range.address)) >> vm_page_quanta_shift) % num_large_entries;
	unsigned index = hash_index;
	large_entry_t *entry;

	// assert(szone->num_large_objects_in_use < szone->num_large_entries); /* must be called with room to spare */

	do {
		entry = szone->large_entries + index;
		if (0 == entry->address) {
			*entry = range;
			return; // end of chain
		}
		index++;
		if (index == num_large_entries) {
			index = 0;
		}
	} while (index != hash_index);

	// assert(0); /* must not fallthrough! */
}

// FIXME: can't we simply swap the (now empty) entry with the last entry on the collision chain for this hash slot?
static MALLOC_INLINE void
large_entries_rehash_after_entry_no_lock(szone_t *szone, large_entry_t *entry)
{
	unsigned num_large_entries = szone->num_large_entries;
	uintptr_t hash_index = entry - szone->large_entries;
	uintptr_t index = hash_index;
	large_entry_t range;

	// assert(entry->address == 0) /* caller must have cleared *entry */

	do {
		index++;
		if (index == num_large_entries) {
			index = 0;
		}
		range = szone->large_entries[index];
		if (0 == range.address) {
			return;
		}
		szone->large_entries[index].address = (vm_address_t)0;
		szone->large_entries[index].size = 0;
		szone->large_entries[index].did_madvise_reusable = FALSE;
		large_entry_insert_no_lock(szone, range); // this will reinsert in the
		// proper place
	} while (index != hash_index);

	// assert(0); /* since entry->address == 0, must not fallthrough! */
}

// FIXME: num should probably be a size_t, since you can theoretically allocate
// more than 2^32-1 large_threshold objects in 64 bit.
static MALLOC_INLINE large_entry_t *
large_entries_alloc_no_lock(szone_t *szone, unsigned num)
{
	size_t size = num * sizeof(large_entry_t);

	// Note that we allocate memory (via a system call) under a spin lock
	// That is certainly evil, however it's very rare in the lifetime of a process
	// The alternative would slow down the normal case
	return allocate_pages(szone, round_page_quanta(size), 0, 0, VM_MEMORY_MALLOC_LARGE);
}

void
large_entries_free_no_lock(szone_t *szone, large_entry_t *entries, unsigned num, vm_range_t *range_to_deallocate)
{
	size_t size = num * sizeof(large_entry_t);

	range_to_deallocate->address = (vm_address_t)entries;
	range_to_deallocate->size = round_page_quanta(size);
}

static large_entry_t *
large_entries_grow_no_lock(szone_t *szone, vm_range_t *range_to_deallocate)
{
	// sets range_to_deallocate
	unsigned old_num_entries = szone->num_large_entries;
	large_entry_t *old_entries = szone->large_entries;
	// always an odd number for good hashing
	unsigned new_num_entries =
	(old_num_entries) ? old_num_entries * 2 + 1 : (unsigned)((vm_page_quanta_size / sizeof(large_entry_t)) - 1);
	large_entry_t *new_entries = large_entries_alloc_no_lock(szone, new_num_entries);
	unsigned index = old_num_entries;
	large_entry_t oldRange;

	// if the allocation of new entries failed, bail
	if (new_entries == NULL) {
		return NULL;
	}

	szone->num_large_entries = new_num_entries;
	szone->large_entries = new_entries;

	/* rehash entries into the new list */
	while (index--) {
		oldRange = old_entries[index];
		if (oldRange.address) {
			large_entry_insert_no_lock(szone, oldRange);
		}
	}

	if (old_entries) {
		large_entries_free_no_lock(szone, old_entries, old_num_entries, range_to_deallocate);
	} else {
		range_to_deallocate->address = (vm_address_t)0;
		range_to_deallocate->size = 0;
	}

	return new_entries;
}

// frees the specific entry in the size table
// returns a range to truly deallocate
static vm_range_t
large_entry_free_no_lock(szone_t *szone, large_entry_t *entry)
{
	vm_range_t range;

	MALLOC_TRACE(TRACE_large_free, (uintptr_t)szone, (uintptr_t)entry->address, entry->size, 0);

	range.address = entry->address;
	range.size = entry->size;

	if (szone->debug_flags & MALLOC_ADD_GUARD_PAGES) {
		protect((void *)range.address, range.size, PROT_READ | PROT_WRITE, szone->debug_flags);
		range.address -= vm_page_quanta_size;
		range.size += 2 * vm_page_quanta_size;
	}

	entry->address = 0;
	entry->size = 0;
	entry->did_madvise_reusable = FALSE;
	large_entries_rehash_after_entry_no_lock(szone, entry);

#if DEBUG_MALLOC
	if (large_entry_for_pointer_no_lock(szone, (void *)range.address)) {
		malloc_printf("*** freed entry %p still in use; num_large_entries=%d\n", range.address, szone->num_large_entries);
		large_debug_print(szone);
		szone_sleep();
	}
#endif
	return range;
}

kern_return_t
large_in_use_enumerator(task_t task,
						void *context,
						unsigned type_mask,
						vm_address_t large_entries_address,
						unsigned num_entries,
						memory_reader_t reader,
						vm_range_recorder_t recorder)
{
	unsigned index = 0;
	vm_range_t buffer[MAX_RECORDER_BUFFER];
	unsigned count = 0;
	large_entry_t *entries;
	kern_return_t err;
	vm_range_t range;
	large_entry_t entry;

	err = reader(task, large_entries_address, sizeof(large_entry_t) * num_entries, (void **)&entries);
	if (err) {
		return err;
	}

	index = num_entries;
	if (type_mask & MALLOC_ADMIN_REGION_RANGE_TYPE) {
		range.address = large_entries_address;
		range.size = round_page_quanta(num_entries * sizeof(large_entry_t));
		recorder(task, context, MALLOC_ADMIN_REGION_RANGE_TYPE, &range, 1);
	}
	if (type_mask & (MALLOC_PTR_IN_USE_RANGE_TYPE | MALLOC_PTR_REGION_RANGE_TYPE)) {
		while (index--) {
			entry = entries[index];
			if (entry.address) {
				range.address = entry.address;
				range.size = entry.size;
				buffer[count++] = range;
				if (count >= MAX_RECORDER_BUFFER) {
					recorder(task, context, MALLOC_PTR_IN_USE_RANGE_TYPE | MALLOC_PTR_REGION_RANGE_TYPE, buffer, count);
					count = 0;
				}
			}
		}
	}
	if (count) {
		recorder(task, context, MALLOC_PTR_IN_USE_RANGE_TYPE | MALLOC_PTR_REGION_RANGE_TYPE, buffer, count);
	}
	return 0;
}

void *
large_malloc(szone_t *szone, size_t num_kernel_pages, unsigned char alignment, boolean_t cleared_requested)
{
	void *addr;
	vm_range_t range_to_deallocate;
	size_t size;
	large_entry_t large_entry;

	MALLOC_TRACE(TRACE_large_malloc, (uintptr_t)szone, num_kernel_pages, alignment, cleared_requested);

	if (!num_kernel_pages) {
		num_kernel_pages = 1; // minimal allocation size for this szone
	}
	size = (size_t)num_kernel_pages << vm_page_quanta_shift;
	range_to_deallocate.size = 0;
	range_to_deallocate.address = 0;

#if CONFIG_LARGE_CACHE
	if (size < LARGE_CACHE_SIZE_ENTRY_LIMIT) { // Look for a large_entry_t on the death-row cache?
		SZONE_LOCK(szone);

		int i, best = -1, idx = szone->large_entry_cache_newest, stop_idx = szone->large_entry_cache_oldest;
		size_t best_size = SIZE_T_MAX;

		while (1) { // Scan large_entry_cache for best fit, starting with most recent entry
			size_t this_size = szone->large_entry_cache[idx].size;
			addr = (void *)szone->large_entry_cache[idx].address;

			if (0 == alignment || 0 == (((uintptr_t)addr) & (((uintptr_t)1 << alignment) - 1))) {
				if (size == this_size) { // size match!
					best = idx;
					best_size = this_size;
					break;
				}

				if (size <= this_size && this_size < best_size) { // improved fit?
					best = idx;
					best_size = this_size;
				}
			}

			if (idx == stop_idx) { // exhausted live ring?
				break;
			}

			if (idx) {
				idx--; // bump idx down
			} else {
				idx = LARGE_ENTRY_CACHE_SIZE - 1; // wrap idx
			}
		}

		if (best > -1 && (best_size - size) < size) { // limit fragmentation to 50%
			addr = (void *)szone->large_entry_cache[best].address;
			boolean_t was_madvised_reusable = szone->large_entry_cache[best].did_madvise_reusable;

			// Compact live ring to fill entry now vacated at large_entry_cache[best]
			// while preserving time-order
			if (szone->large_entry_cache_oldest < szone->large_entry_cache_newest) {
				// Ring hasn't wrapped. Fill in from right.
				for (i = best; i < szone->large_entry_cache_newest; ++i) {
					szone->large_entry_cache[i] = szone->large_entry_cache[i + 1];
				}

				szone->large_entry_cache_newest--; // Pull in right endpoint.

			} else if (szone->large_entry_cache_newest < szone->large_entry_cache_oldest) {
				// Ring has wrapped. Arrange to fill in from the contiguous side.
				if (best <= szone->large_entry_cache_newest) {
					// Fill from right.
					for (i = best; i < szone->large_entry_cache_newest; ++i) {
						szone->large_entry_cache[i] = szone->large_entry_cache[i + 1];
					}

					if (0 < szone->large_entry_cache_newest) {
						szone->large_entry_cache_newest--;
					} else {
						szone->large_entry_cache_newest = LARGE_ENTRY_CACHE_SIZE - 1;
					}
				} else {
					// Fill from left.
					for (i = best; i > szone->large_entry_cache_oldest; --i) {
						szone->large_entry_cache[i] = szone->large_entry_cache[i - 1];
					}

					if (szone->large_entry_cache_oldest < LARGE_ENTRY_CACHE_SIZE - 1) {
						szone->large_entry_cache_oldest++;
					} else {
						szone->large_entry_cache_oldest = 0;
					}
				}

			} else {
				// By trichotomy, large_entry_cache_newest == large_entry_cache_oldest.
				// That implies best == large_entry_cache_newest == large_entry_cache_oldest
				// and the ring is now empty.
				szone->large_entry_cache[best].address = 0;
				szone->large_entry_cache[best].size = 0;
				szone->large_entry_cache[best].did_madvise_reusable = FALSE;
			}

			if ((szone->num_large_objects_in_use + 1) * 4 > szone->num_large_entries) {
				// density of hash table too high; grow table
				// we do that under lock to avoid a race
				large_entry_t *entries = large_entries_grow_no_lock(szone, &range_to_deallocate);
				if (entries == NULL) {
					SZONE_UNLOCK(szone);
					return NULL;
				}
			}

			large_entry.address = (vm_address_t)addr;
			large_entry.size = best_size;
			large_entry.did_madvise_reusable = FALSE;
			large_entry_insert_no_lock(szone, large_entry);

			szone->num_large_objects_in_use++;
			szone->num_bytes_in_large_objects += best_size;
			if (!was_madvised_reusable) {
				szone->large_entry_cache_reserve_bytes -= best_size;
			}

			szone->large_entry_cache_bytes -= best_size;

			if (szone->flotsam_enabled && szone->large_entry_cache_bytes < SZONE_FLOTSAM_THRESHOLD_LOW) {
				szone->flotsam_enabled = FALSE;
			}

			SZONE_UNLOCK(szone);

			if (range_to_deallocate.size) {
				// we deallocate outside the lock
				deallocate_pages(szone, (void *)range_to_deallocate.address, range_to_deallocate.size, 0);
			}

			// Perform the madvise() outside the lock.
			// Typically the madvise() is successful and we'll quickly return from this routine.
			// In the unusual case of failure, reacquire the lock to unwind.
#if TARGET_OS_EMBEDDED
			// Ok to do this madvise on embedded because we won't call MADV_FREE_REUSABLE on a large
			// cache block twice without MADV_FREE_REUSE in between.
#endif
			if (was_madvised_reusable && -1 == madvise(addr, size, MADV_FREE_REUSE)) {
				/* -1 return: VM map entry change makes this unfit for reuse. */
#if DEBUG_MADVISE
				szone_error(szone, 0, "large_malloc madvise(..., MADV_FREE_REUSE) failed", addr, "length=%d\n", size);
#endif

				SZONE_LOCK(szone);
				szone->num_large_objects_in_use--;
				szone->num_bytes_in_large_objects -= large_entry.size;

				// Re-acquire "entry" after interval just above where we let go the lock.
				large_entry_t *entry = large_entry_for_pointer_no_lock(szone, addr);
				if (NULL == entry) {
					szone_error(szone, 1, "entry for pointer being discarded from death-row vanished", addr, NULL);
					SZONE_UNLOCK(szone);
				} else {
					range_to_deallocate = large_entry_free_no_lock(szone, entry);
					SZONE_UNLOCK(szone);

					if (range_to_deallocate.size) {
						// we deallocate outside the lock
						deallocate_pages(szone, (void *)range_to_deallocate.address, range_to_deallocate.size, 0);
					}
				}
				/* Fall through to allocate_pages() afresh. */
			} else {
				if (cleared_requested) {
					memset(addr, 0, size);
				}

				return addr;
			}
		} else {
			SZONE_UNLOCK(szone);
		}
	}

	range_to_deallocate.size = 0;
	range_to_deallocate.address = 0;
#endif /* CONFIG_LARGE_CACHE */

	addr = allocate_pages(szone, size, alignment, szone->debug_flags, VM_MEMORY_MALLOC_LARGE);
	if (addr == NULL) {
		return NULL;
	}

	SZONE_LOCK(szone);
	if ((szone->num_large_objects_in_use + 1) * 4 > szone->num_large_entries) {
		// density of hash table too high; grow table
		// we do that under lock to avoid a race
		large_entry_t *entries = large_entries_grow_no_lock(szone, &range_to_deallocate);
		if (entries == NULL) {
			SZONE_UNLOCK(szone);
			return NULL;
		}
	}

	large_entry.address = (vm_address_t)addr;
	large_entry.size = size;
	large_entry.did_madvise_reusable = FALSE;
	large_entry_insert_no_lock(szone, large_entry);

	szone->num_large_objects_in_use++;
	szone->num_bytes_in_large_objects += size;
	SZONE_UNLOCK(szone);

	if (range_to_deallocate.size) {
		// we deallocate outside the lock
		deallocate_pages(szone, (void *)range_to_deallocate.address, range_to_deallocate.size, 0);
	}
	return addr;
}

void
free_large(szone_t *szone, void *ptr)
{
	// We have established ptr is page-aligned and neither tiny nor small
	large_entry_t *entry;
	vm_range_t vm_range_to_deallocate;

	SZONE_LOCK(szone);
	entry = large_entry_for_pointer_no_lock(szone, ptr);
	if (entry) {
#if CONFIG_LARGE_CACHE
		if (entry->size < LARGE_CACHE_SIZE_ENTRY_LIMIT &&
			-1 != madvise((void *)(entry->address), entry->size,
						  MADV_CAN_REUSE)) { // Put the large_entry_t on the death-row cache?
				int idx = szone->large_entry_cache_newest, stop_idx = szone->large_entry_cache_oldest;
				large_entry_t this_entry = *entry; // Make a local copy, "entry" is volatile when lock is let go.
				boolean_t reusable = TRUE;
				boolean_t should_madvise =
				szone->large_entry_cache_reserve_bytes + this_entry.size > szone->large_entry_cache_reserve_limit;

				// Already freed?
				// [Note that repeated entries in death-row risk vending the same entry subsequently
				// to two different malloc() calls. By checking here the (illegal) double free
				// is accommodated, matching the behavior of the previous implementation.]
				while (1) { // Scan large_entry_cache starting with most recent entry
					if (szone->large_entry_cache[idx].address == entry->address) {
						szone_error(szone, 1, "pointer being freed already on death-row", ptr, NULL);
						SZONE_UNLOCK(szone);
						return;
					}

					if (idx == stop_idx) { // exhausted live ring?
						break;
					}

					if (idx) {
						idx--; // bump idx down
					} else {
						idx = LARGE_ENTRY_CACHE_SIZE - 1; // wrap idx
					}
				}

				SZONE_UNLOCK(szone);

				if (szone->debug_flags & MALLOC_PURGEABLE) { // Are we a purgable zone?
					int state = VM_PURGABLE_NONVOLATILE;			  // restore to default condition

					if (KERN_SUCCESS != vm_purgable_control(mach_task_self(), this_entry.address, VM_PURGABLE_SET_STATE, &state)) {
						malloc_printf("*** can't vm_purgable_control(..., VM_PURGABLE_SET_STATE) for large freed block at %p\n",
									  this_entry.address);
						reusable = FALSE;
					}
				}

				if (szone->large_legacy_reset_mprotect) { // Linked for Leopard?
					// Accomodate Leopard apps that (illegally) mprotect() their own guard pages on large malloc'd allocations
					int err = mprotect((void *)(this_entry.address), this_entry.size, PROT_READ | PROT_WRITE);
					if (err) {
						malloc_printf("*** can't reset protection for large freed block at %p\n", this_entry.address);
						reusable = FALSE;
					}
				}

				// madvise(..., MADV_REUSABLE) death-row arrivals if hoarding would exceed large_entry_cache_reserve_limit
				if (should_madvise) {
					// Issue madvise to avoid paging out the dirtied free()'d pages in "entry"
					MAGMALLOC_MADVFREEREGION(
											 (void *)szone, (void *)0, (void *)(this_entry.address), this_entry.size); // DTrace USDT Probe

#if TARGET_OS_EMBEDDED
					// Ok to do this madvise on embedded because we won't call MADV_FREE_REUSABLE on a large
					// cache block twice without MADV_FREE_REUSE in between.
#endif
					if (-1 == madvise((void *)(this_entry.address), this_entry.size, MADV_FREE_REUSABLE)) {
						/* -1 return: VM map entry change makes this unfit for reuse. */
#if DEBUG_MADVISE
						szone_error(szone, 0, "free_large madvise(..., MADV_FREE_REUSABLE) failed", (void *)this_entry.address,
									"length=%d\n", this_entry.size);
#endif
						reusable = FALSE;
					}
				}

				SZONE_LOCK(szone);

				// Re-acquire "entry" after interval just above where we let go the lock.
				entry = large_entry_for_pointer_no_lock(szone, ptr);
				if (NULL == entry) {
					szone_error(szone, 1, "entry for pointer being freed from death-row vanished", ptr, NULL);
					SZONE_UNLOCK(szone);
					return;
				}

				// Add "entry" to death-row ring
				if (reusable) {
					int idx = szone->large_entry_cache_newest; // Most recently occupied
					vm_address_t addr;
					size_t adjsize;

					if (szone->large_entry_cache_newest == szone->large_entry_cache_oldest &&
						0 == szone->large_entry_cache[idx].address) {
						// Ring is empty, idx is good as it stands
						addr = 0;
						adjsize = 0;
					} else {
						// Extend the queue to the "right" by bumping up large_entry_cache_newest
						if (idx == LARGE_ENTRY_CACHE_SIZE - 1) {
							idx = 0; // Wrap index
						} else {
							idx++; // Bump index
						}
						if (idx == szone->large_entry_cache_oldest) { // Fully occupied
							// Drop this entry from the cache and deallocate the VM
							addr = szone->large_entry_cache[idx].address;
							adjsize = szone->large_entry_cache[idx].size;
							szone->large_entry_cache_bytes -= adjsize;
							if (!szone->large_entry_cache[idx].did_madvise_reusable) {
								szone->large_entry_cache_reserve_bytes -= adjsize;
							}
						} else {
							// Using an unoccupied cache slot
							addr = 0;
							adjsize = 0;
						}
					}

					if ((szone->debug_flags & MALLOC_DO_SCRIBBLE)) {
						memset((void *)(entry->address), should_madvise ? SCRUBBLE_BYTE : SCRABBLE_BYTE, entry->size);
					}

					entry->did_madvise_reusable = should_madvise; // Was madvise()'d above?
					if (!should_madvise) {						  // Entered on death-row without madvise() => up the hoard total
						szone->large_entry_cache_reserve_bytes += entry->size;
					}

					szone->large_entry_cache_bytes += entry->size;

					if (!szone->flotsam_enabled && szone->large_entry_cache_bytes > SZONE_FLOTSAM_THRESHOLD_HIGH) {
						szone->flotsam_enabled = TRUE;
					}

					szone->large_entry_cache[idx] = *entry;
					szone->large_entry_cache_newest = idx;

					szone->num_large_objects_in_use--;
					szone->num_bytes_in_large_objects -= entry->size;

					(void)large_entry_free_no_lock(szone, entry);

					if (0 == addr) {
						SZONE_UNLOCK(szone);
						return;
					}

					// Fall through to drop large_entry_cache_oldest from the cache,
					// and then deallocate its pages.

					// Trim the queue on the "left" by bumping up large_entry_cache_oldest
					if (szone->large_entry_cache_oldest == LARGE_ENTRY_CACHE_SIZE - 1) {
						szone->large_entry_cache_oldest = 0;
					} else {
						szone->large_entry_cache_oldest++;
					}

					// we deallocate_pages, including guard pages, outside the lock
					SZONE_UNLOCK(szone);
					deallocate_pages(szone, (void *)addr, (size_t)adjsize, 0);
					return;
				} else {
					/* fall through to discard an allocation that is not reusable */
				}
			}
#endif /* CONFIG_LARGE_CACHE */

		szone->num_large_objects_in_use--;
		szone->num_bytes_in_large_objects -= entry->size;

		vm_range_to_deallocate = large_entry_free_no_lock(szone, entry);
	} else {
#if DEBUG_MALLOC
		large_debug_print(szone);
#endif
		szone_error(szone, 1, "pointer being freed was not allocated", ptr, NULL);
		SZONE_UNLOCK(szone);
		return;
	}
	SZONE_UNLOCK(szone); // we release the lock asap
	CHECK(szone, __PRETTY_FUNCTION__);

	// we deallocate_pages, including guard pages, outside the lock
	if (vm_range_to_deallocate.address) {
#if DEBUG_MALLOC
		// FIXME: large_entry_for_pointer_no_lock() needs the lock held ...
		if (large_entry_for_pointer_no_lock(szone, (void *)vm_range_to_deallocate.address)) {
			malloc_printf("*** invariant broken: %p still in use num_large_entries=%d\n", vm_range_to_deallocate.address,
						  szone->num_large_entries);
			large_debug_print(szone);
			szone_sleep();
		}
#endif
		deallocate_pages(szone, (void *)vm_range_to_deallocate.address, (size_t)vm_range_to_deallocate.size, 0);
	}
}

void *
large_try_shrink_in_place(szone_t *szone, void *ptr, size_t old_size, size_t new_good_size)
{
	size_t shrinkage = old_size - new_good_size;

	if (shrinkage) {
		SZONE_LOCK(szone);
		/* contract existing large entry */
		large_entry_t *large_entry = large_entry_for_pointer_no_lock(szone, ptr);
		if (!large_entry) {
			szone_error(szone, 1, "large entry reallocated is not properly in table", ptr, NULL);
			SZONE_UNLOCK(szone);
			return ptr;
		}

		large_entry->address = (vm_address_t)ptr;
		large_entry->size = new_good_size;
		szone->num_bytes_in_large_objects -= shrinkage;
		SZONE_UNLOCK(szone); // we release the lock asap

		deallocate_pages(szone, (void *)((uintptr_t)ptr + new_good_size), shrinkage, 0);
	}
	return ptr;
}

int
large_try_realloc_in_place(szone_t *szone, void *ptr, size_t old_size, size_t new_size)
{
	vm_address_t addr = (vm_address_t)ptr + old_size;
	large_entry_t *large_entry;
	kern_return_t err;

	SZONE_LOCK(szone);
	large_entry = large_entry_for_pointer_no_lock(szone, (void *)addr);
	SZONE_UNLOCK(szone);

	if (large_entry) { // check if "addr = ptr + old_size" is already spoken for
		return 0;	  // large pointer already exists in table - extension is not going to work
	}

	new_size = round_page_quanta(new_size);
	/*
	 * Ask for allocation at a specific address, and mark as realloc
	 * to request coalescing with previous realloc'ed extensions.
	 */
	err = vm_allocate(mach_task_self(), &addr, new_size - old_size, VM_MAKE_TAG(VM_MEMORY_REALLOC));
	if (err != KERN_SUCCESS) {
		return 0;
	}

	SZONE_LOCK(szone);
	/* extend existing large entry */
	large_entry = large_entry_for_pointer_no_lock(szone, ptr);
	if (!large_entry) {
		szone_error(szone, 1, "large entry reallocated is not properly in table", ptr, NULL);
		SZONE_UNLOCK(szone);
		return 0; // Bail, leaking "addr"
	}
	
	large_entry->address = (vm_address_t)ptr;
	large_entry->size = new_size;
	szone->num_bytes_in_large_objects += new_size - old_size;
	SZONE_UNLOCK(szone); // we release the lock asap
	
	return 1;
}
