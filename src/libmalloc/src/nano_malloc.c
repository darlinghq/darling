/*
 * Copyright (c) 1999, 2000, 2003, 2005, 2008, 2012 Apple Inc. All rights reserved.
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

/* nano_malloc for 64bit ABI */
#if CONFIG_NANOZONE

/*********************             PROTOTYPES		***********************/

static void nano_statistics(nanozone_t *nanozone, malloc_statistics_t *stats);

/*********************	   VERY LOW LEVEL UTILITIES    ************************/
// msg prints after fmt, ...

static MALLOC_ALWAYS_INLINE unsigned int
nano_mag_index(const nanozone_t *nanozone)
{
	if (os_likely(_os_cpu_number_override == -1)) {
		return (_os_cpu_number() >> hyper_shift) % nano_common_max_magazines;
	}
	return (_os_cpu_number_override >> hyper_shift) % nano_common_max_magazines;
}

#if NANO_PREALLOCATE_BAND_VM
static boolean_t
nano_preallocate_band_vm(void)
{
	nano_blk_addr_t u;
	uintptr_t s, e;

	u.fields.nano_signature = NANOZONE_SIGNATURE;
	u.fields.nano_mag_index = 0;
	u.fields.nano_band = 0;
	u.fields.nano_slot = 0;
	u.fields.nano_offset = 0;
	s = u.addr; // start of first possible band

	u.fields.nano_mag_index = (1 << NANO_MAG_BITS) - 1;
	u.fields.nano_band = (1 << NANO_BAND_BITS) - 1;
	e = u.addr + BAND_SIZE; // end of last possible band

	return nano_common_allocate_vm_space(s, e - s);
}
#endif

/*
 * We maintain separate free lists for each (quantized) size. The literature
 * calls this the "segregated policy".
 */

static boolean_t
segregated_band_grow(nanozone_t *nanozone, nano_meta_admin_t pMeta, size_t slot_bytes, unsigned int mag_index)
{
	nano_blk_addr_t u; // the compiler holds this in a register
	uintptr_t p, s;
	size_t watermark, hiwater;

	if (0 == pMeta->slot_current_base_addr) { // First encounter?

		u.fields.nano_signature = NANOZONE_SIGNATURE;
		u.fields.nano_mag_index = mag_index;
		u.fields.nano_band = 0;
		u.fields.nano_slot = (slot_bytes >> SHIFT_NANO_QUANTUM) - 1;
		u.fields.nano_offset = 0;

		p = u.addr;
		pMeta->slot_bytes = (unsigned int)slot_bytes;
		pMeta->slot_objects = SLOT_IN_BAND_SIZE / slot_bytes;
	} else {
		p = pMeta->slot_current_base_addr + BAND_SIZE; // Growing, so stride ahead by BAND_SIZE

		u.addr = (uint64_t)p;
		if (0 == u.fields.nano_band) { // Did the band index wrap?
			return FALSE;
		}

		assert(slot_bytes == pMeta->slot_bytes);
	}
	pMeta->slot_current_base_addr = p;

	mach_vm_address_t vm_addr = p & ~((uintptr_t)(BAND_SIZE - 1)); // Address of the (2MB) band covering this (128KB) slot
	if (nanozone->band_max_mapped_baseaddr[mag_index] < vm_addr) {
#if !NANO_PREALLOCATE_BAND_VM
		// Obtain the next band to cover this slot
		kern_return_t kr = mach_vm_map(mach_task_self(), &vm_addr, BAND_SIZE, 0, VM_MAKE_TAG(VM_MEMORY_MALLOC_NANO),
				MEMORY_OBJECT_NULL, 0, FALSE, VM_PROT_DEFAULT, VM_PROT_ALL, VM_INHERIT_DEFAULT);

		void *q = (void *)vm_addr;
		if (kr || q != (void *)(p & ~((uintptr_t)(BAND_SIZE - 1)))) { // Must get exactly what we asked for
			if (!kr) {
				mach_vm_deallocate(mach_task_self(), vm_addr, BAND_SIZE);
			}
			return FALSE;
		}
#endif
		nanozone->band_max_mapped_baseaddr[mag_index] = vm_addr;
	}

	// Randomize the starting allocation from this slot (introduces 11 to 14 bits of entropy)
	if (0 == pMeta->slot_objects_mapped) { // First encounter?
		pMeta->slot_objects_skipped = (malloc_entropy[1] % (SLOT_IN_BAND_SIZE / slot_bytes));
		pMeta->slot_bump_addr = p + (pMeta->slot_objects_skipped * slot_bytes);
	} else {
		pMeta->slot_bump_addr = p;
	}

	pMeta->slot_limit_addr = p + (SLOT_IN_BAND_SIZE / slot_bytes) * slot_bytes;
	pMeta->slot_objects_mapped += (SLOT_IN_BAND_SIZE / slot_bytes);

	u.fields.nano_signature = NANOZONE_SIGNATURE;
	u.fields.nano_mag_index = mag_index;
	u.fields.nano_band = 0;
	u.fields.nano_slot = 0;
	u.fields.nano_offset = 0;
	s = u.addr; // Base for this core.

	// Set the high water mark for this CPU's entire magazine, if this resupply raised it.
	watermark = nanozone->core_mapped_size[mag_index];
	hiwater = MAX(watermark, p - s + SLOT_IN_BAND_SIZE);
	nanozone->core_mapped_size[mag_index] = hiwater;

	return TRUE;
}

static inline unsigned long
divrem(unsigned long a, unsigned int b, unsigned int *remainder)
{
	// Encapsulating the modulo and division in an in-lined function convinces the compiler
	// to issue just a single divide instruction to obtain quotient and remainder. Go figure.
	*remainder = a % b;
	return a / b;
}

static MALLOC_INLINE void *
segregated_next_block(nanozone_t *nanozone, nano_meta_admin_t pMeta, size_t slot_bytes, unsigned int mag_index)
{
	while (1) {
		uintptr_t theLimit = pMeta->slot_limit_addr; // Capture the slot limit that bounds slot_bump_addr right now
		uintptr_t b = OSAtomicAdd64Barrier(slot_bytes, (volatile int64_t *)&(pMeta->slot_bump_addr));
		b -= slot_bytes; // Atomic op returned addr of *next* free block. Subtract to get addr for *this* allocation.

		if (b < theLimit) {   // Did we stay within the bound of the present slot allocation?
			return (void *)b; // Yep, so the slot_bump_addr this thread incremented is good to go
		} else {
			if (pMeta->slot_exhausted) { // exhausted all the bands availble for this slot?
				pMeta->slot_bump_addr = theLimit;
				return 0;				 // We're toast
			} else {
				// One thread will grow the heap, others will see its been grown and retry allocation
				_malloc_lock_lock(&nanozone->band_resupply_lock[mag_index]);
				// re-check state now that we've taken the lock
				if (pMeta->slot_exhausted) {
					_malloc_lock_unlock(&nanozone->band_resupply_lock[mag_index]);
					return 0; // Toast
				} else if (b < pMeta->slot_limit_addr) {
					_malloc_lock_unlock(&nanozone->band_resupply_lock[mag_index]);
					continue; // ... the slot was successfully grown by first-taker (not us). Now try again.
				} else if (segregated_band_grow(nanozone, pMeta, slot_bytes, mag_index)) {
					_malloc_lock_unlock(&nanozone->band_resupply_lock[mag_index]);
					continue; // ... the slot has been successfully grown by us. Now try again.
				} else {
					pMeta->slot_exhausted = TRUE;
					pMeta->slot_bump_addr = theLimit;
					_malloc_lock_unlock(&nanozone->band_resupply_lock[mag_index]);
					return 0;
				}
			}
		}
	}
}

static MALLOC_INLINE size_t
segregated_size_to_fit(nanozone_t *nanozone, size_t size, size_t *pKey)
{
	size_t k, slot_bytes;

	if (0 == size) {
		size = NANO_REGIME_QUANTA_SIZE; // Historical behavior
	}
	k = (size + NANO_REGIME_QUANTA_SIZE - 1) >> SHIFT_NANO_QUANTUM; // round up and shift for number of quanta
	slot_bytes = k << SHIFT_NANO_QUANTUM;							// multiply by power of two quanta size
	*pKey = k - 1;													// Zero-based!

	return slot_bytes;
}

static MALLOC_INLINE index_t
offset_to_index(nanozone_t *nanozone, nano_meta_admin_t pMeta, uintptr_t offset)
{
	unsigned int slot_bytes = pMeta->slot_bytes;
	unsigned int slot_objects = pMeta->slot_objects; // SLOT_IN_BAND_SIZE / slot_bytes;
	unsigned int rem;
	unsigned long quo = divrem(offset, BAND_SIZE, &rem);

	assert(0 == rem % slot_bytes || pMeta->slot_exhausted);
	return (index_t)((quo * slot_objects) + (rem / slot_bytes));
}

static MALLOC_INLINE uintptr_t
index_to_offset(nanozone_t *nanozone, nano_meta_admin_t pMeta, index_t i)
{
	unsigned int slot_bytes = pMeta->slot_bytes;
	unsigned int slot_objects = pMeta->slot_objects; // SLOT_IN_BAND_SIZE / slot_bytes;
	unsigned int rem;
	unsigned long quo = divrem(i, slot_objects, &rem);

	return (quo * BAND_SIZE) + (rem * slot_bytes);
}

static kern_return_t
segregated_in_use_enumerator(task_t task,
		void *context,
		unsigned type_mask,
		nanozone_t *nanozone,
		memory_reader_t reader,
		vm_range_recorder_t recorder)
{
	unsigned int mag_index, slot_key;
	vm_range_t ptr_range;
	vm_range_t buffer[MAX_RECORDER_BUFFER];
	kern_return_t err;
	unsigned count = 0;

	for (mag_index = 0; mag_index < nano_common_max_magazines; mag_index++) {
		uintptr_t clone_magazine;  // magazine base for ourselves
		nano_blk_addr_t p;		   // slot base for remote
		uintptr_t clone_slot_base; // slot base for ourselves (tracks with "p")

		// Establish p as base address for slot 0 in remote
		p.fields.nano_signature = NANOZONE_SIGNATURE;
		p.fields.nano_mag_index = mag_index;
		p.fields.nano_band = 0;
		p.fields.nano_slot = 0;
		p.fields.nano_offset = 0;

		if (type_mask & MALLOC_PTR_IN_USE_RANGE_TYPE) {
			mach_vm_address_t vm_addr;
			mach_vm_size_t alloc_size = nanozone->core_mapped_size[mag_index];
			int alloc_flags = VM_FLAGS_ANYWHERE | VM_MAKE_TAG(VM_MEMORY_MALLOC);

			vm_addr = vm_page_size;
			kern_return_t kr = mach_vm_allocate(mach_task_self(), &vm_addr, alloc_size, alloc_flags);
			if (kr) {
				return kr;
			}
			clone_magazine = (uintptr_t)vm_addr;
			clone_slot_base = clone_magazine; // base for slot 0 in this local magazine
		} else {
			clone_slot_base = clone_magazine = 0; // and won't be used in this loop
		}

		for (slot_key = 0; slot_key < SLOT_KEY_LIMIT; p.addr += SLOT_IN_BAND_SIZE, // Advance to next slot base for remote
				clone_slot_base += SLOT_IN_BAND_SIZE,							   // Advance to next slot base for ourselves
				slot_key++) {
			nano_meta_admin_t pMeta = &(nanozone->meta_data[mag_index][slot_key]);
			size_t slot_objects_mapped = pMeta->slot_objects_mapped; // capture this volatile count

			if (0 == slot_objects_mapped) { // Nothing allocated in this magazine for this slot?
				continue;
			}

			if (type_mask & MALLOC_ADMIN_REGION_RANGE_TYPE) {
				/* do NOTHING as there is no distinct admin region */
			}

			if (type_mask & (MALLOC_PTR_REGION_RANGE_TYPE | MALLOC_ADMIN_REGION_RANGE_TYPE)) {
				nano_blk_addr_t q = p;
				uintptr_t skip_adj = index_to_offset(nanozone, pMeta, (index_t)pMeta->slot_objects_skipped);

				while (q.addr < pMeta->slot_limit_addr) {
					ptr_range.address = q.addr + skip_adj;
					ptr_range.size = SLOT_IN_BAND_SIZE - skip_adj;
					skip_adj = 0;
					recorder(task, context, MALLOC_PTR_REGION_RANGE_TYPE, &ptr_range, 1);
					q.addr += BAND_SIZE;
				}
			}

			if (type_mask & MALLOC_PTR_IN_USE_RANGE_TYPE) {
				nano_blk_addr_t q = p;
				uintptr_t slot_band, clone_slot_band_base = clone_slot_base;
				uintptr_t skip_adj = index_to_offset(nanozone, pMeta, (index_t)pMeta->slot_objects_skipped);

				// Copy the bitarray_t denoting madvise()'d pages (if any) into *this* task's address space
				bitarray_t madv_page_bitarray;
				int log_page_count;

				if (pMeta->slot_madvised_pages) {
					log_page_count = pMeta->slot_madvised_log_page_count;
					err = reader(task, (vm_address_t)(pMeta->slot_madvised_pages), bitarray_size(log_page_count),
							(void **)&madv_page_bitarray);
					if (err) {
						return err;
					}
				} else {
					madv_page_bitarray = NULL;
					log_page_count = 0;
				}

				while (q.addr < pMeta->slot_limit_addr) {
					// read slot in each remote band. Lands in some random location. Do not read
					// parts of the slot that are in madvised pages.
					if (!madv_page_bitarray) {
						// Nothing madvised yet - read everything in one go.
						size_t len = MIN(pMeta->slot_bump_addr - q.addr, SLOT_IN_BAND_SIZE) - skip_adj;
						err = reader(task, (vm_address_t)(q.addr + skip_adj), len, (void **)&slot_band);
						if (err) {
							return err;
						}

						// Place the data just read in the correct position relative to the local magazine.
						memcpy((void *)(clone_slot_band_base + skip_adj), (void *)slot_band, len);
					} else {
						// We madvised at least one page. Read only the pages that
						// have not been madvised. If bitarray_t had operations
						// like "get next bit set after a given bit" and "find
						// next unset bit after a given bit", we could do this more
						// efficiently but given that it doesn't, we have to walk
						// through each page individually. In practice this is not
						// much of an issue because this code is only used by
						// sampling tools and the additional time required is not
						// really noticeable.
						size_t len = MIN(pMeta->slot_bump_addr - q.addr, SLOT_IN_BAND_SIZE) - skip_adj;
						vm_address_t start_addr = (vm_address_t)(q.addr + skip_adj);
						vm_address_t end_addr = (vm_address_t)(start_addr + len);
						void *target_addr = (void *)(clone_slot_band_base + skip_adj);
						for (vm_address_t addr = start_addr; addr < end_addr;) {
							vm_address_t next_page_addr = trunc_page_kernel(addr + vm_kernel_page_size);
							size_t read_size = MIN(len, next_page_addr - addr);

							boolean_t madvised = false;
							nano_blk_addr_t r;
							r.addr = addr;
							index_t pgnum = ((((unsigned)r.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)r.fields.nano_offset)) >>
								vm_kernel_page_shift;
							unsigned int log_page_count = pMeta->slot_madvised_log_page_count;
							madvised = (pgnum < (1 << log_page_count)) &&
									bitarray_get(madv_page_bitarray, log_page_count, pgnum);
							if (!madvised) {
								// This is not an madvised page - grab the data.
								err = reader(task, addr, read_size, (void **)&slot_band);
								if (err) {
									return err;
								}

								// Place the data just read in the correct position relative to the local magazine.
								memcpy(target_addr, (void *)slot_band, read_size);
							} else {
								// This is an madvised page - there should be nothing in here that's
								// on the freelist, so just write garbage to the target memory.
								memset(target_addr, (char)0xee, read_size);
							}
							addr = next_page_addr;
							target_addr += read_size;
							len -= read_size;
						}
					}

					// Simultaneously advance pointers in remote and ourselves to the next band.
					q.addr += BAND_SIZE;
					clone_slot_band_base += BAND_SIZE;
					skip_adj = 0;
				}

				// Walk the slot free list and populate a bitarray_t
				int log_size = 64 - __builtin_clzl(slot_objects_mapped);
				bitarray_t slot_bitarray = bitarray_create(log_size);

				if (!slot_bitarray) {
					return errno;
				}

				chained_block_t t;
				unsigned stoploss = (unsigned)slot_objects_mapped;
				while ((t = OSAtomicDequeue(
								&(pMeta->slot_LIFO), offsetof(struct chained_block_s, next) + (clone_slot_base - p.addr)))) {
					if (0 == stoploss) {
						malloc_report(ASL_LEVEL_ERR, "Free list walk in segregated_in_use_enumerator exceeded object count.\n");
						break;
					}
					stoploss--;

					uintptr_t offset = ((uintptr_t)t - p.addr); // offset from beginning of slot, task-independent
					index_t block_index = offset_to_index(nanozone, pMeta, offset);

					if (block_index < slot_objects_mapped) {
						bitarray_set(slot_bitarray, log_size, block_index);
					}
				}
				// N.B. pMeta->slot_LIFO in *this* task is now drained (remote free list has *not* been disturbed)


				// Enumerate all the block indices issued to date, and report those not on the free list
				index_t i;
				for (i = (index_t)pMeta->slot_objects_skipped; i < slot_objects_mapped; ++i) {
					uintptr_t block_offset = index_to_offset(nanozone, pMeta, i);
					if (p.addr + block_offset >= pMeta->slot_bump_addr) {
						break;
					}

					// blocks falling on madvise()'d pages are free! So not enumerated.
					if (madv_page_bitarray) {
						nano_blk_addr_t q;
						index_t pgnum, pgnum_end;

						q.addr = p.addr + block_offset;
						pgnum = ((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >>
								vm_kernel_page_shift;
						q.addr += pMeta->slot_bytes - 1;
						pgnum_end = ((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >>
									vm_kernel_page_shift;

						if (pgnum < (1 << log_page_count)) { // bounds check for bitarray_get()'s that follow
							if (bitarray_get(madv_page_bitarray, log_page_count, pgnum) ||
									bitarray_get(madv_page_bitarray, log_page_count, pgnum_end)) {
								continue;
							}
						}
					}

					if (!bitarray_get(slot_bitarray, log_size, i)) {
						buffer[count].address = p.addr + block_offset;
						buffer[count].size = (slot_key + 1) << SHIFT_NANO_QUANTUM;
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

				free(slot_bitarray);
			}
		}
		if (clone_magazine) {
			mach_vm_address_t vm_addr = clone_magazine;
			mach_vm_size_t alloc_size = nanozone->core_mapped_size[mag_index];
			mach_vm_deallocate(mach_task_self(), vm_addr, alloc_size);
		}
	}
	return 0;
}

/******************           nanozone methods           **********************/
/*
 * These methods are called with "ptr" known to possess the nano signature (from
 * which we can additionally infer "ptr" is not NULL), and with "size" bounded to
 * the extent of the nano allocation regime -- (0, 256].
 */

static MALLOC_INLINE MALLOC_UNUSED boolean_t
_nano_block_inuse_p(nanozone_t *nanozone, const void *ptr)
{
	nano_blk_addr_t p; // happily, the compiler holds this in a register
	nano_meta_admin_t pMeta;
	chained_block_t head = NULL, tail = NULL, t;
	boolean_t inuse = TRUE;

	p.addr = (uint64_t)ptr; // place ptr on the dissecting table

	pMeta = &(nanozone->meta_data[p.fields.nano_mag_index][p.fields.nano_slot]);

	// pop elements off the free list all the while looking for ptr.
	unsigned stoploss = (unsigned)pMeta->slot_objects_mapped;
	while ((t = OSAtomicDequeue(&(pMeta->slot_LIFO), offsetof(struct chained_block_s, next)))) {
		if (0 == stoploss) {
			malloc_zone_error(nanozone->debug_flags, true,
					"Free list walk for slot %p in _nano_block_inuse_p exceeded object count.\n",
					(void *)&(pMeta->slot_LIFO));
		}
		stoploss--;

		if (NULL == head) {
			head = t;
		} else {
			tail->next = t;
		}
		tail = t;

		if (ptr == t) {
			inuse = FALSE;
			break;
		}
	}
	if (tail) {
		tail->next = NULL;
	}

	// push the free list extracted above back onto the LIFO, all at once
	if (head) {
		OSAtomicEnqueue(&(pMeta->slot_LIFO), head, (uintptr_t)tail - (uintptr_t)head + offsetof(struct chained_block_s, next));
	}

	return inuse;
}

static MALLOC_INLINE size_t
__nano_vet_and_size_inner(nanozone_t *nanozone, const void *ptr, boolean_t inner)
{
	// Extracts the size of the block in bytes. Checks for a plausible ptr.
	nano_blk_addr_t p; // the compiler holds this in a register
	nano_meta_admin_t pMeta;

	p.addr = (uint64_t)ptr; // Begin the dissection of ptr

	if (NANOZONE_SIGNATURE != p.fields.nano_signature) {
		return 0;
	}

	if (nano_common_max_magazines <= p.fields.nano_mag_index) {
		return 0;
	}

	if (!inner && p.fields.nano_offset & NANO_QUANTA_MASK) { // stray low-order bits?
		return 0;
	}

	pMeta = &(nanozone->meta_data[p.fields.nano_mag_index][p.fields.nano_slot]);
	if ((void *)(pMeta->slot_bump_addr) <= ptr) {
		return 0; // Beyond what's ever been allocated!
	}
	if (!inner && ((p.fields.nano_offset % pMeta->slot_bytes) != 0)) {
		return 0; // Not an exact multiple of the block size for this slot
	}
	return pMeta->slot_bytes;
}


static MALLOC_INLINE size_t
__nano_vet_and_size(nanozone_t *nanozone, const void *ptr)
{
	return __nano_vet_and_size_inner(nanozone, ptr, false);
}

static MALLOC_ALWAYS_INLINE boolean_t
_nano_block_has_canary_value(nanozone_t *nanozone, const void *ptr)
{
	return (((chained_block_t)ptr)->double_free_guard ^ nanozone->cookie)
			== (uintptr_t)ptr;
}

static MALLOC_ALWAYS_INLINE void
_nano_block_set_canary_value(nanozone_t *nanozone, const void *ptr)
{
	((chained_block_t)ptr)->double_free_guard =
			((uintptr_t)ptr) ^ nanozone->cookie;
}

static MALLOC_INLINE size_t
_nano_vet_and_size_of_live(nanozone_t *nanozone, const void *ptr)
{
	size_t size = __nano_vet_and_size(nanozone, ptr);

	if (0 == size) { // ptr fails sanity check?
		return 0;
	}
	
	// We have the invariant: If ptr is on a free list, then ptr->double_free_guard is the canary.
	// So if ptr->double_free_guard is NOT the canary, then ptr is not on a free list, hence is live.
	if (!_nano_block_has_canary_value(nanozone, ptr)) {
		return size; // Common case: not on a free list, hence live. Return its size.
	} else {
		// confirm that ptr is live despite ptr->double_free_guard having the canary value
		if (_nano_block_inuse_p(nanozone, ptr)) {
			return size; // live block that exhibits canary
		} else {
			return 0; // ptr wasn't live after all (likely a double free)
		}
	}
}

static MALLOC_INLINE size_t
_nano_vet_and_size_of_free(nanozone_t *nanozone, const void *ptr)
{
	size_t size = __nano_vet_and_size(nanozone, ptr);

	if (0 == size) { // ptr fails sanity check?
		return 0;
	}
	
	// ptr was just dequed from a free list, so ptr->double_free_guard must have the canary value.
	if (_nano_block_has_canary_value(nanozone, ptr)) {
		return size; // return the size of this well formed free block.
	} else {
		return 0; // Broken invariant: If ptr is on a free list, then ptr->double_free_guard is the canary. (likely use after free)
	}
}

static void *
_nano_malloc_check_clear(nanozone_t *nanozone, size_t size, boolean_t cleared_requested)
{
	MALLOC_TRACE(TRACE_nano_malloc, (uintptr_t)nanozone, size, cleared_requested, 0);

	void *ptr;
	size_t slot_key;
	size_t slot_bytes = segregated_size_to_fit(nanozone, size, &slot_key); // Note slot_key is set here
	mag_index_t mag_index = nano_mag_index(nanozone);

	nano_meta_admin_t pMeta = &(nanozone->meta_data[mag_index][slot_key]);

	ptr = OSAtomicDequeue(&(pMeta->slot_LIFO), offsetof(struct chained_block_s, next));
	if (ptr) {
		unsigned debug_flags = nanozone->debug_flags;
#if NANO_FREE_DEQUEUE_DILIGENCE
		size_t gotSize;
		nano_blk_addr_t p; // the compiler holds this in a register

		p.addr = (uint64_t)ptr; // Begin the dissection of ptr
		if (NANOZONE_SIGNATURE != p.fields.nano_signature) {
			malloc_zone_error(debug_flags, true,
					"Invalid signature for pointer %p dequeued from free list\n",
					ptr);
		}

		if (mag_index != p.fields.nano_mag_index) {
			malloc_zone_error(debug_flags, true,
					"Mismatched magazine for pointer %p dequeued from free list\n",
					ptr);
		}

		gotSize = _nano_vet_and_size_of_free(nanozone, ptr);
		if (0 == gotSize) {
			malloc_zone_error(debug_flags, true,
					"Invalid pointer %p dequeued from free list\n", ptr);
		}
		if (gotSize != slot_bytes) {
			malloc_zone_error(debug_flags, true,
					"Mismatched size for pointer %p dequeued from free list\n",
					ptr);
		}

		if (!_nano_block_has_canary_value(nanozone, ptr)) {
			malloc_zone_error(debug_flags, true,
					"Heap corruption detected, free list canary is damaged for %p\n"
					"*** Incorrect guard value: %lu\n", ptr,
					((chained_block_t)ptr)->double_free_guard);
		}

#if defined(DEBUG)
		void *next = (void *)(((chained_block_t)ptr)->next);
		if (next) {
			p.addr = (uint64_t)next; // Begin the dissection of next
			if (NANOZONE_SIGNATURE != p.fields.nano_signature) {
				malloc_zone_error(debug_flags, true,
						"Invalid next signature for pointer %p dequeued from free "
						"list, next = %p\n", ptr, "next");
			}

			if (mag_index != p.fields.nano_mag_index) {
				malloc_zone_error(debug_flags, true,
						"Mismatched next magazine for pointer %p dequeued from "
						"free list, next = %p\n", ptr, next);
			}

			gotSize = _nano_vet_and_size_of_free(nanozone, next);
			if (0 == gotSize) {
				malloc_zone_error(debug_flags, true,
						"Invalid next for pointer %p dequeued from free list, "
						"next = %p\n", ptr, next);
			}
			if (gotSize != slot_bytes) {
				malloc_zone_error(debug_flags, true,
						"Mismatched next size for pointer %p dequeued from free "
						"list, next = %p\n", ptr, next);
			}
		}
#endif /* DEBUG */
#endif /* NANO_FREE_DEQUEUE_DILIGENCE */

		((chained_block_t)ptr)->double_free_guard = 0;
		((chained_block_t)ptr)->next = NULL; // clear out next pointer to protect free list
	} else {
		ptr = segregated_next_block(nanozone, pMeta, slot_bytes, mag_index);
	}

	if (cleared_requested && ptr) {
		memset(ptr, 0, slot_bytes); // TODO: Needs a memory barrier after memset to ensure zeroes land first?
	}
	return ptr;
}

static void *
_nano_malloc_check_scribble(nanozone_t *nanozone, size_t size)
{
	void *ptr = _nano_malloc_check_clear(nanozone, size, 0);

	/*
	 * Scribble on allocated memory when requested.
	 */
	if ((nanozone->debug_flags & MALLOC_DO_SCRIBBLE) && ptr && size) {
		memset(ptr, SCRIBBLE_BYTE, _nano_vet_and_size_of_live(nanozone, ptr));
	}

	return ptr;
}

static MALLOC_INLINE size_t
_nano_size(nanozone_t *nanozone, const void *ptr)
{
	return _nano_vet_and_size_of_live(nanozone, ptr);
}

static MALLOC_INLINE size_t
_nano_good_size(nanozone_t *nanozone, size_t size)
{
	return (size <= NANO_REGIME_QUANTA_SIZE) ? NANO_REGIME_QUANTA_SIZE
											 : (((size + NANO_REGIME_QUANTA_SIZE - 1) >> SHIFT_NANO_QUANTUM) << SHIFT_NANO_QUANTUM);
}

static MALLOC_INLINE void _nano_free_trusted_size_check_scribble(nanozone_t *nanozone,
		void *ptr,
		size_t trusted_size,
		boolean_t do_scribble) MALLOC_ALWAYS_INLINE;

static MALLOC_INLINE void
_nano_free_trusted_size_check_scribble(nanozone_t *nanozone, void *ptr, size_t trusted_size, boolean_t do_scribble)
{
	if (trusted_size) {
		nano_blk_addr_t p; // happily, the compiler holds this in a register
		nano_meta_admin_t pMeta;

		if (do_scribble) {
			(void)memset(ptr, SCRABBLE_BYTE, trusted_size);
		}
		_nano_block_set_canary_value(nanozone, ptr);

		p.addr = (uint64_t)ptr; // place ptr on the dissecting table
		pMeta = &(nanozone->meta_data[p.fields.nano_mag_index][p.fields.nano_slot]);
		OSAtomicEnqueue(&(pMeta->slot_LIFO), ptr, offsetof(struct chained_block_s, next));
	} else {
		malloc_zone_error(nanozone->debug_flags, true,
				"Freeing unallocated pointer %p\n", ptr);
	}
}

static MALLOC_INLINE void _nano_free_check_scribble(nanozone_t *nanozone, void *ptr, boolean_t do_scribble) MALLOC_ALWAYS_INLINE;

static MALLOC_INLINE void
_nano_free_check_scribble(nanozone_t *nanozone, void *ptr, boolean_t do_scribble)
{
	_nano_free_trusted_size_check_scribble(nanozone, ptr, _nano_vet_and_size_of_live(nanozone, ptr), do_scribble);
}

static MALLOC_INLINE void *
_nano_realloc(nanozone_t *nanozone, void *ptr, size_t new_size)
{
	size_t old_size, new_good_size, valid_size;
	void *new_ptr;

	if (FALSE && NULL == ptr) { // ptr has our_signature so can't be NULL, but if it were Posix sez ...
		// If ptr is a null pointer, realloc() shall be equivalent to malloc() for the specified size.
		return _nano_malloc_check_scribble(nanozone, new_size);
	} else if (0 == new_size) {
		// If size is 0 and ptr is not a null pointer, the object pointed to is freed.
		_nano_free_check_scribble(nanozone, ptr, (nanozone->debug_flags & MALLOC_DO_SCRIBBLE));
		// If size is 0, either a null pointer or a unique pointer that can be successfully passed
		// to free() shall be returned.
		return _nano_malloc_check_scribble(nanozone, 1);
	}

	old_size = _nano_vet_and_size_of_live(nanozone, ptr);
	if (!old_size) {
		malloc_zone_error(nanozone->debug_flags, true,
				"pointer %p being reallocated was not allocated\n", ptr);
		return NULL;
	}

	new_good_size = _nano_good_size(nanozone, new_size);
	if (new_good_size > old_size) {
		/* Must grow. FALL THROUGH to alloc/copy/free. */
	} else if (new_good_size <= (old_size >> 1)) {
		/* Serious shrinkage (more than half). FALL THROUGH to alloc/copy/free. */
	} else {
		/* Let's hang on to what we got. */
		if (nanozone->debug_flags & MALLOC_DO_SCRIBBLE) {
			memset(ptr + new_size, SCRIBBLE_BYTE, old_size - new_size);
		}
		return ptr;
	}

	/*
	 * Allocate a new buffer and copy.
	 */
	new_ptr = _nano_malloc_check_scribble(nanozone, new_good_size);
	if (new_ptr == NULL) {
		return NULL;
	}

	valid_size = MIN(old_size, new_good_size);
	memcpy(new_ptr, ptr, valid_size);
	_nano_free_check_scribble(nanozone, ptr, (nanozone->debug_flags & MALLOC_DO_SCRIBBLE));

	return new_ptr;
}

static MALLOC_INLINE void
_nano_destroy(nanozone_t *nanozone)
{
	/* Now destroy the separate nanozone region */
	nano_common_deallocate_pages((void *)nanozone, NANOZONE_PAGED_SIZE,
			nanozone->debug_flags);
}

/******************           nanozone dispatch          **********************/

static void *
nano_malloc(nanozone_t *nanozone, size_t size)
{
	if (size <= NANO_MAX_SIZE) {
		void *p = _nano_malloc_check_clear(nanozone, size, 0);
		if (p) {
			return p;
		} else {
			/* FALLTHROUGH to helper zone */
		}
	}

	malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
	return zone->malloc(zone, size);
}

static void *
nano_forked_malloc(nanozone_t *nanozone, size_t size)
{
	malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
	return zone->malloc(zone, size);
}

static void *
nano_malloc_scribble(nanozone_t *nanozone, size_t size)
{
	if (size <= NANO_MAX_SIZE) {
		void *ptr = _nano_malloc_check_clear(nanozone, size, 0);
		if (ptr) {
			/*
			 * Scribble on allocated memory.
			 */
			if (size) {
				memset(ptr, SCRIBBLE_BYTE, _nano_vet_and_size_of_live(nanozone, ptr));
			}

			return ptr;
		} else {
			/* FALLTHROUGH to helper zone */
		}
	}
	malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
	return zone->malloc(zone, size);
}

static void *
nano_calloc(nanozone_t *nanozone, size_t num_items, size_t size)
{
	size_t total_bytes;

	if (calloc_get_size(num_items, size, 0, &total_bytes)) {
		return NULL;
	}

	if (total_bytes <= NANO_MAX_SIZE) {
		void *p = _nano_malloc_check_clear(nanozone, total_bytes, 1);
		if (p) {
			return p;
		} else {
			/* FALLTHROUGH to helper zone */
		}
	}
	malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
	return zone->calloc(zone, 1, total_bytes);
}

static void *
nano_forked_calloc(nanozone_t *nanozone, size_t num_items, size_t size)
{
	malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
	return zone->calloc(zone, num_items, size);
}

static void *
nano_valloc(nanozone_t *nanozone, size_t size)
{
	malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
	return zone->valloc(zone, size);
}

static MALLOC_INLINE void
__nano_free_definite_size(nanozone_t *nanozone, void *ptr, size_t size, boolean_t do_scribble) MALLOC_ALWAYS_INLINE;

static MALLOC_INLINE void
__nano_free_definite_size(nanozone_t *nanozone, void *ptr, size_t size, boolean_t do_scribble)
{
	nano_blk_addr_t p; // happily, the compiler holds this in a register

	p.addr = (uint64_t)ptr; // place ptr on the dissecting table
	if (NANOZONE_SIGNATURE == p.fields.nano_signature) {
		if (size == ((p.fields.nano_slot + 1) << SHIFT_NANO_QUANTUM)) { // "Trust but verify."
			_nano_free_trusted_size_check_scribble(nanozone, ptr, size, do_scribble);
			return;
		} else {
			malloc_zone_error(nanozone->debug_flags, true,
					"Freeing pointer %p whose size was misdeclared\n", ptr);
		}
	} else {
		malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
		zone->free_definite_size(zone, ptr, size);
		return;
	}
	/* NOTREACHED */
}

static void
nano_free_definite_size(nanozone_t *nanozone, void *ptr, size_t size)
{
	__nano_free_definite_size(nanozone, ptr, size, 0);
}

static void
nano_free_definite_size_scribble(nanozone_t *nanozone, void *ptr, size_t size)
{
	__nano_free_definite_size(nanozone, ptr, size, 1);
}

static MALLOC_INLINE void __nano_free(nanozone_t *nanozone, void *ptr, boolean_t do_scribble) MALLOC_ALWAYS_INLINE;

static MALLOC_INLINE void
__nano_free(nanozone_t *nanozone, void *ptr, boolean_t do_scribble)
{
	MALLOC_TRACE(TRACE_nano_free, (uintptr_t)nanozone, (uintptr_t)ptr, do_scribble, 0);

	if (!ptr) {
		return; // Protect against malloc_zone_free() passing NULL.
	}

	// <rdar://problem/26481467> exhausting a slot may result in a pointer with
	// the nanozone prefix being given to nano_free via malloc_zone_free. Calling
	// vet_and_size here, instead of in _nano_free_check_scribble means we can
	// early-out into the helper_zone if it turns out nano does not own this ptr.
	size_t sz = _nano_vet_and_size_of_live(nanozone, ptr);

	if (sz) {
		_nano_free_trusted_size_check_scribble(nanozone, ptr, sz, do_scribble);
		return;
	} else {
		malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
		zone->free(zone, ptr);
		return;
	}
	/* NOTREACHED */
}

static void
nano_free(nanozone_t *nanozone, void *ptr)
{
	__nano_free(nanozone, ptr, 0);
}

static void
nano_forked_free(nanozone_t *nanozone, void *ptr)
{
	if (!ptr) {
		return; // Protect against malloc_zone_free() passing NULL.
	}

	// <rdar://problem/26481467> exhausting a slot may result in a pointer with
	// the nanozone prefix being given to nano_free via malloc_zone_free. Calling
	// vet_and_size here, instead of in _nano_free_check_scribble means we can
	// early-out into the helper_zone if it turns out nano does not own this ptr.
	size_t sz = _nano_vet_and_size_of_live(nanozone, ptr);

	if (sz) {
		/* NOTHING. Drop it on the floor as nanozone metadata could be fouled by fork. */
		return;
	} else {
		malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
		zone->free(zone, ptr);
		return;
	}
	/* NOTREACHED */
}

static void
nano_forked_free_definite_size(nanozone_t *nanozone, void *ptr, size_t size)
{
	nano_forked_free(nanozone, ptr);
}

static void
nano_free_scribble(nanozone_t *nanozone, void *ptr)
{
	__nano_free(nanozone, ptr, 1);
}

static size_t
nano_size(nanozone_t *nanozone, const void *ptr)
{
	nano_blk_addr_t p; // happily, the compiler holds this in a register

	p.addr = (uint64_t)ptr; // place ptr on the dissecting table

	if (NANOZONE_SIGNATURE == p.fields.nano_signature) { // Our signature?
		return _nano_size(nanozone, ptr);
	} else {
		malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
		return zone->size(zone, ptr); // Not nano. Try other sizes.
	}
	/* NOTREACHED */
}

static void *
nano_realloc(nanozone_t *nanozone, void *ptr, size_t new_size)
{
	// could occur through malloc_zone_realloc() path
	if (!ptr) {
		// If ptr is a null pointer, realloc() shall be equivalent to malloc() for the specified size.
		return nano_malloc(nanozone, new_size);
	}

	size_t old_size = _nano_vet_and_size_of_live(nanozone, ptr);
	if (!old_size) {
		// not-nano pointer, hand down to helper zone
		malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
		return zone->realloc(zone, ptr, new_size);
	} else {
		if (new_size <= NANO_MAX_SIZE) {
			// nano to nano?
			void *q = _nano_realloc(nanozone, ptr, new_size);
			if (q) {
				return q;
			} else { 
				// nano exhausted
				/* FALLTHROUGH to helper zone copying case */
			}
		}

		malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
		void *new_ptr = zone->malloc(zone, new_size);

		if (new_ptr) {
			size_t valid_size = MIN(old_size, new_size);
			memcpy(new_ptr, ptr, valid_size);
			_nano_free_check_scribble(nanozone, ptr, (nanozone->debug_flags & MALLOC_DO_SCRIBBLE));
			return new_ptr;
		} else {
			/* Original ptr is left intact */
			return NULL;
		}
		/* NOTREACHED */
	}
	/* NOTREACHED */
}

static void *
nano_forked_realloc(nanozone_t *nanozone, void *ptr, size_t new_size)
{
	// could occur through malloc_zone_realloc() path
	if (!ptr) {
		// If ptr is a null pointer, realloc() shall be equivalent to malloc() for the specified size.
		return nano_forked_malloc(nanozone, new_size);
	}

	size_t old_size = _nano_vet_and_size_of_live(nanozone, ptr);
	if (!old_size) {
		// not-nano pointer, hand down to helper zone
		malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
		return zone->realloc(zone, ptr, new_size);
	} else {
		if (0 == new_size) {
			// If size is 0 and ptr is not a null pointer, the object pointed to is freed.
			// However as nanozone metadata could be fouled by fork, we'll intentionally leak it.

			// If size is 0, either a null pointer or a unique pointer that can be successfully passed
			// to free() shall be returned.
			return nano_forked_malloc(nanozone, 1);
		}

		malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
		void *new_ptr = zone->malloc(zone, new_size);

		if (new_ptr) {
			size_t valid_size = MIN(old_size, new_size);
			memcpy(new_ptr, ptr, valid_size);
			/* Original pointer is intentionally leaked as nanozone metadata could be fouled by fork. */
			return new_ptr;
		} else {
			/* Original ptr is left intact */
			return NULL;
		}
		/* NOTREACHED */
	}
	/* NOTREACHED */
}

static void
nano_destroy(nanozone_t *nanozone)
{
	malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
	zone->destroy(zone);

	_nano_destroy(nanozone);
}

static unsigned
nano_batch_malloc(nanozone_t *nanozone, size_t size, void **results, unsigned count)
{
	unsigned found = 0;

	if (size <= NANO_MAX_SIZE) {
		while (found < count) {
			void *ptr = _nano_malloc_check_clear(nanozone, size, 0);
			if (!ptr) {
				break;
			}

			*results++ = ptr;
			found++;
		}
		if (found == count) {
			return found;
		} else {
			/* FALLTHROUGH to mop-up in the helper zone */
		}
	}

	malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
	return found + zone->batch_malloc(zone, size, results, count - found);
}

static unsigned
nano_forked_batch_malloc(nanozone_t *nanozone, size_t size, void **results, unsigned count)
{
	malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
	return zone->batch_malloc(zone, size, results, count);
}

static void
nano_batch_free(nanozone_t *nanozone, void **to_be_freed, unsigned count)
{
	void *ptr;

	// frees all the pointers in to_be_freed
	// note that to_be_freed may be overwritten during the process
	if (!count) {
		return;
	}

	while (count--) {
		ptr = to_be_freed[count];
		if (ptr) {
			nano_free(nanozone, ptr);
		}
	}
}

static void
nano_forked_batch_free(nanozone_t *nanozone, void **to_be_freed, unsigned count)
{
	void *ptr;

	// frees all the pointers in to_be_freed
	// note that to_be_freed may be overwritten during the process
	if (!count) {
		return;
	}

	while (count--) {
		ptr = to_be_freed[count];
		if (ptr) {
			nano_forked_free(nanozone, ptr);
		}
	}
}

static void *
nano_memalign(nanozone_t *nanozone, size_t alignment, size_t size)
{
	malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
	return zone->memalign(zone, alignment, size);
}

static boolean_t
nano_claimed_address(nanozone_t *nanozone, void *ptr)
{
	nano_blk_addr_t p;
	p.addr = (uint64_t)ptr;
	if (NANOZONE_SIGNATURE != p.fields.nano_signature) {
		// Not a nano address - let the helper zone handle it.
		malloc_zone_t *helper_zone = nanozone->helper_zone;
		return malloc_zone_claimed_address(helper_zone, ptr);
	}
	return __nano_vet_and_size_inner(nanozone, ptr, true) != 0;
}

static boolean_t
nano_forked_claimed_address(struct _malloc_zone_t *zone, void *ptr)
{
	// This does not operate after fork - default to true to avoid
	// false negatives.
	return true;
}

static size_t
nano_try_madvise(nanozone_t *nanozone, size_t goal)
{
	unsigned int mag_index, slot_key;
	size_t bytes_toward_goal = 0;

	for (mag_index = 0; mag_index < nano_common_max_magazines; mag_index++) {
		nano_blk_addr_t p;

		// Establish p as base address for band 0, slot 0, offset 0
		p.fields.nano_signature = NANOZONE_SIGNATURE;
		p.fields.nano_mag_index = mag_index;
		p.fields.nano_band = 0;
		p.fields.nano_slot = 0;
		p.fields.nano_offset = 0;

		for (slot_key = 0; slot_key < SLOT_KEY_LIMIT; p.addr += SLOT_IN_BAND_SIZE, // Advance to next slot base
				slot_key++) {
			// malloc_report(ASL_LEVEL_WARNING,"nano_try_madvise examining slot base %p\n", p.addr);
			nano_meta_admin_t pMeta = &(nanozone->meta_data[mag_index][slot_key]);
			uintptr_t slot_bump_addr = pMeta->slot_bump_addr;		 // capture this volatile pointer
			size_t slot_objects_mapped = pMeta->slot_objects_mapped; // capture this volatile count

			if (0 == slot_objects_mapped) { // Nothing allocated in this magazine for this slot?
				continue;
			} else {
				// Walk the slot free list and populate a bitarray_t
				int log_size = 64 - __builtin_clzl(slot_objects_mapped);
				bitarray_t slot_bitarray = bitarray_create(log_size);

				unsigned int slot_bytes = pMeta->slot_bytes;
				int log_page_count = 64 - __builtin_clzl((slot_objects_mapped * slot_bytes) / vm_kernel_page_size);
				log_page_count = 1 + MAX(0, log_page_count);
				bitarray_t page_bitarray = bitarray_create(log_page_count);

				// malloc_report(ASL_LEVEL_WARNING,"slot_bitarray: %db page_bitarray: %db\n", bitarray_size(log_size),
				// bitarray_size(log_page_count));
				if (!slot_bitarray) {
					malloc_report(ASL_LEVEL_ERR, "bitarray_create(%d) in nano_try_madvise returned errno=%d.\n", log_size, errno);
					free(page_bitarray);
					return bytes_toward_goal;
				}

				if (!page_bitarray) {
					malloc_report(ASL_LEVEL_ERR, "bitarray_create(%d) in nano_try_madvise returned errno=%d.\n", log_page_count, errno);
					free(slot_bitarray);
					return bytes_toward_goal;
				}

				chained_block_t head = NULL, tail = NULL, t;
				unsigned stoploss = (unsigned)slot_objects_mapped;
				while ((t = OSAtomicDequeue(&(pMeta->slot_LIFO), offsetof(struct chained_block_s, next)))) {
					if (0 == stoploss) {
						malloc_report(ASL_LEVEL_ERR, "Free list walk in nano_try_madvise exceeded object count.\n");
						break;
					}
					stoploss--;

					uintptr_t offset = ((uintptr_t)t - p.addr); // offset from beginning of slot
					index_t block_index = offset_to_index(nanozone, pMeta, offset);

					// build a simple linked list of the free blocks we're able to obtain
					if (NULL == head) {
						head = t;
					} else {
						tail->next = t;
					}
					tail = t;

					// take note in a bitarray_t of each free block we're able to obtain (allows fast lookup below)
					if (block_index < slot_objects_mapped) {
						bitarray_set(slot_bitarray, log_size, block_index);
					}
				}
				if (tail) {
					tail->next = NULL;
				}

				if (NULL == head) {
					free(slot_bitarray);
					free(page_bitarray);
					continue;
				}

				index_t i;
				nano_blk_addr_t q;
				size_t pgnum;
				for (i = (index_t)pMeta->slot_objects_skipped; i < slot_objects_mapped; ++i) {
					uintptr_t block_offset = index_to_offset(nanozone, pMeta, i);
					if (p.addr + block_offset >= slot_bump_addr) {
						break;
					}

					if (!bitarray_get(slot_bitarray, log_size, i)) { // is block i allocated or already on an madvise'd page?

						// Mark the page(s) it resides on as live
						q.addr = p.addr + block_offset;
						pgnum = ((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >>
								vm_kernel_page_shift;
						bitarray_set(page_bitarray, log_page_count, (index_t)pgnum);

						q.addr += slot_bytes - 1;
						pgnum = ((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >>
								vm_kernel_page_shift;
						bitarray_set(page_bitarray, log_page_count, (index_t)pgnum);
					}
				}

				free(slot_bitarray);

				q.addr = p.addr + index_to_offset(nanozone, pMeta, (index_t)pMeta->slot_objects_skipped);
				index_t pgstart =
						((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >> vm_kernel_page_shift;

				q.addr = slot_bump_addr - slot_bytes;
				pgnum = ((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >> vm_kernel_page_shift;

				// malloc_report(ASL_LEVEL_WARNING,"Examining %d pages. Slot base %p.\n", pgnum - pgstart + 1, p.addr);

				if (pMeta->slot_madvised_pages) {
					if (pMeta->slot_madvised_log_page_count < log_page_count) {
						bitarray_t new_madvised_pages = bitarray_create(log_page_count);
						index_t index;
						while (bitarray_zap_first_set(pMeta->slot_madvised_pages, pMeta->slot_madvised_log_page_count, &index)) {
							bitarray_set(new_madvised_pages, log_page_count, index);
						}
						free(pMeta->slot_madvised_pages);
						pMeta->slot_madvised_pages = new_madvised_pages;
						pMeta->slot_madvised_log_page_count = log_page_count;
					}
				} else {
					pMeta->slot_madvised_pages = bitarray_create(log_page_count);
					pMeta->slot_madvised_log_page_count = log_page_count;
				}

				bitarray_t will_madvise_pages = bitarray_create(log_page_count);
				int num_advised = 0;

				for (i = pgstart; i < pgnum; ++i) {
					if ((i < (1 << log_page_count)) && // bounds check for the bitarray_get()'s that follow.
							!bitarray_get(pMeta->slot_madvised_pages, log_page_count, i) && // already madvise'd?
							!bitarray_get(page_bitarray, log_page_count, i))				// no live allocations?
					{
						num_advised++;
						bitarray_set(will_madvise_pages, log_page_count, i);
					}
				}
				free(page_bitarray);

				if (num_advised) {
					chained_block_t new_head = NULL, new_tail = NULL;
					// malloc_report(ASL_LEVEL_WARNING,"Constructing residual free list starting at %p num_advised %d\n", head,
					// num_advised);
					t = head;
					while (t) {
						q.addr = (uintptr_t)t;
						index_t pgnum_start =
								((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >>
								vm_kernel_page_shift;
						q.addr += slot_bytes - 1;
						index_t pgnum_end =
								((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >>
								vm_kernel_page_shift;

						// bounds check for the bitarray_get()'s that follow. If the pgnum is beyond the
						// capacity of the will_madvise_pages just restore the block to the free list.
						if (pgnum_start >= (1 << log_page_count)) {
							if (NULL == new_head) {
								new_head = t;
							} else {
								new_tail->next = t;
							}
							new_tail = t;
						}
						// If the block nowhere lies on an madvise()'d page restore it to the slot free list.
						else if (!bitarray_get(will_madvise_pages, log_page_count, pgnum_start) &&
								 !bitarray_get(will_madvise_pages, log_page_count, pgnum_end)) {
							if (NULL == new_head) {
								new_head = t;
							} else {
								new_tail->next = t;
							}
							new_tail = t;
						}

						t = t->next;
					}
					if (new_tail) {
						new_tail->next = NULL;
					}

					// push the free list extracted above back onto the LIFO, all at once
					if (new_head) {
						OSAtomicEnqueue(&(pMeta->slot_LIFO), new_head,
								(uintptr_t)new_tail - (uintptr_t)new_head + offsetof(struct chained_block_s, next));
					}
				} else {
					// malloc_report(ASL_LEVEL_WARNING,"Reinstating free list since no pages were madvised (%d).\n", num_advised);
					if (head) {
						OSAtomicEnqueue(&(pMeta->slot_LIFO), head,
								(uintptr_t)tail - (uintptr_t)head + offsetof(struct chained_block_s, next));
					}
				}

				for (i = pgstart; i < pgnum; ++i) {
					if ((i < (1 << log_page_count)) && bitarray_get(will_madvise_pages, log_page_count, i)) {
						q = p;
						q.fields.nano_band = (i << vm_kernel_page_shift) >> NANO_OFFSET_BITS;
						q.fields.nano_offset = (i << vm_kernel_page_shift) & ((1 << NANO_OFFSET_BITS) - 1);
						// malloc_report(ASL_LEVEL_WARNING,"Entire page non-live: %d. Slot base %p, madvising %p\n", i, p.addr,
						// q.addr);

						if (nanozone->debug_flags & MALLOC_DO_SCRIBBLE) {
							memset((void *)q.addr, SCRUBBLE_BYTE, vm_kernel_page_size);
						}

						if (-1 == madvise((void *)q.addr, vm_kernel_page_size, MADV_FREE_REUSABLE))
						{
							/* -1 return: VM map entry change makes this unfit for reuse. Something evil lurks. */
#if DEBUG_MADVISE
							nanozone_error(nanozone, 0, "madvise(..., MADV_FREE_REUSABLE) failed", (void *)cwq.addrpgLo,
									"length=%d\n", vm_page_size);
#endif
						} else {
							bytes_toward_goal += vm_kernel_page_size;
							bitarray_set(pMeta->slot_madvised_pages, log_page_count, i);
						}
					}
				}
				free(will_madvise_pages);

				if (!bitarray_first_set(pMeta->slot_madvised_pages, log_page_count)) {
					free(pMeta->slot_madvised_pages);
					pMeta->slot_madvised_pages = NULL;
					pMeta->slot_madvised_log_page_count = 0;
				}

				if (goal && bytes_toward_goal >= goal) {
					return bytes_toward_goal;
				}
			}
		}
	}
	return bytes_toward_goal;
}

static size_t
nano_pressure_relief(nanozone_t *nanozone, size_t goal)
{
	MAGMALLOC_PRESSURERELIEFBEGIN((void *)nanozone, nanozone->basic_zone.zone_name, (int)goal);
	MALLOC_TRACE(TRACE_nano_memory_pressure | DBG_FUNC_START, (uint64_t)nanozone, goal, 0, 0);

	size_t total = nano_try_madvise(nanozone, goal);

	MAGMALLOC_PRESSURERELIEFEND((void *)nanozone, nanozone->basic_zone.zone_name, (int)goal, (int)total);
	MALLOC_TRACE(TRACE_nano_memory_pressure | DBG_FUNC_END, (uint64_t)nanozone, goal, total, 0);

	return total;
}

/****************           introspection methods         *********************/

static kern_return_t
nano_ptr_in_use_enumerator(task_t task,
		void *context,
		unsigned type_mask,
		vm_address_t zone_address,
		memory_reader_t reader,
		vm_range_recorder_t recorder)
{
	nanozone_t *nanozone;
	kern_return_t err;
	struct nanozone_s zone_copy;

	if (!reader) {
		reader = nano_common_default_reader;
	}

	err = reader(task, zone_address, sizeof(nanozone_t), (void **)&nanozone);
	if (err) {
		return err;
	}
	memcpy(&zone_copy, nanozone, sizeof(zone_copy));

	err = segregated_in_use_enumerator(task, context, type_mask, &zone_copy, reader, recorder);
	return err;
}

static size_t
nano_good_size(nanozone_t *nanozone, size_t size)
{
	if (size <= NANO_MAX_SIZE) {
		return _nano_common_good_size(size);
	} else {
		malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
		return zone->introspect->good_size(zone, size);
	}
}

// TODO sanity checks
static unsigned nanozone_check_counter = 0;
static unsigned nanozone_check_start = 0;
static unsigned nanozone_check_modulo = 1;

static boolean_t
nano_check_all(nanozone_t *nanozone, const char *function)
{
	return 1;
}

static boolean_t
nanozone_check(nanozone_t *nanozone)
{
	if ((++nanozone_check_counter % 10000) == 0) {
		malloc_report(ASL_LEVEL_NOTICE, "at nanozone_check counter=%d\n", nanozone_check_counter);
	}

	if (nanozone_check_counter < nanozone_check_start) {
		return 1;
	}

	if (nanozone_check_counter % nanozone_check_modulo) {
		return 1;
	}

	return nano_check_all(nanozone, "");
}

static unsigned
count_free(nanozone_t *nanozone, nano_meta_admin_t pMeta)
{
	chained_block_t head = NULL, tail = NULL, t;
	unsigned count = 0;

	unsigned stoploss = (unsigned)pMeta->slot_objects_mapped;
	while ((t = OSAtomicDequeue(&(pMeta->slot_LIFO), offsetof(struct chained_block_s, next)))) {
		if (0 == stoploss) {
			malloc_zone_error(nanozone->debug_flags, true,
					"Free list walk in count_free exceeded object count.\n",
					(void *)&(pMeta->slot_LIFO), NULL);
		}
		stoploss--;

		if (NULL == head) {
			head = t;
		} else {
			tail->next = t;
		}
		tail = t;

		count++;
	}
	if (tail) {
		tail->next = NULL;
	}

	// push the free list extracted above back onto the LIFO, all at once
	if (head) {
		OSAtomicEnqueue(&(pMeta->slot_LIFO), head, (uintptr_t)tail - (uintptr_t)head + offsetof(struct chained_block_s, next));
	}

	return count;
}

static void
nano_print(nanozone_t *nanozone, boolean_t verbose)
{
	unsigned int mag_index, slot_key;
	malloc_statistics_t stats;

	nano_statistics(nanozone, &stats);
	malloc_report(MALLOC_REPORT_NOLOG | MALLOC_REPORT_NOPREFIX,
			"Nanozone %p: inUse=%d(%lluKB) touched=%lluKB allocated=%lluMB\n",
			nanozone, stats.blocks_in_use, (uint64_t)stats.size_in_use >> 10,
			(uint64_t)stats.max_size_in_use >> 10, (uint64_t)stats.size_allocated >> 20);

	for (mag_index = 0; mag_index < nano_common_max_magazines; mag_index++) {
		nano_blk_addr_t p;

		// Establish p as base address for band 0, slot 0, offset 0
		p.fields.nano_signature = NANOZONE_SIGNATURE;
		p.fields.nano_mag_index = mag_index;
		p.fields.nano_band = 0;
		p.fields.nano_slot = 0;
		p.fields.nano_offset = 0;

		for (slot_key = 0; slot_key < SLOT_KEY_LIMIT; p.addr += SLOT_IN_BAND_SIZE, // Advance to next slot base
				slot_key++) {
			nano_meta_admin_t pMeta = &(nanozone->meta_data[mag_index][slot_key]);
			uintptr_t slot_bump_addr = pMeta->slot_bump_addr;		 // capture this volatile pointer
			size_t slot_objects_mapped = pMeta->slot_objects_mapped; // capture this volatile count

			if (0 == slot_objects_mapped) { // Nothing allocated in this magazine for this slot?
				malloc_report(MALLOC_REPORT_NOLOG | MALLOC_REPORT_NOPREFIX, "Magazine %2d(%3d) Unrealized\n", mag_index,
						(slot_key + 1) << SHIFT_NANO_QUANTUM);
				continue;
			}

			uintptr_t offset = (0 == slot_bump_addr ? 0 : slot_bump_addr - p.addr);
			unsigned blocks_touched = offset_to_index(nanozone, pMeta, offset) - (unsigned)pMeta->slot_objects_skipped;
			unsigned blocks_now_free = count_free(nanozone, pMeta);
			unsigned blocks_in_use = blocks_touched - blocks_now_free;

			size_t size_hiwater = ((slot_key + 1) << SHIFT_NANO_QUANTUM) * blocks_touched;
			size_t size_in_use = ((slot_key + 1) << SHIFT_NANO_QUANTUM) * blocks_in_use;
			size_t size_allocated = ((offset / BAND_SIZE) + 1) * SLOT_IN_BAND_SIZE;

			malloc_report(MALLOC_REPORT_NOLOG | MALLOC_REPORT_NOPREFIX,
					"Magazine %2d(%3d) [%p, %3lluKB] \t Allocations in use=%4d \t Bytes in use=%llub \t Untouched=%lluKB\n", mag_index,
					(slot_key + 1) << SHIFT_NANO_QUANTUM, (void *)p.addr, (uint64_t)(size_allocated >> 10), blocks_in_use, (uint64_t)size_in_use,
					(uint64_t)((size_allocated - size_hiwater) >> 10));

			if (!verbose) {
				continue;
			} else {
				// Walk the slot free list and populate a bitarray_t
				int log_size = 64 - __builtin_clzl(slot_objects_mapped);
				bitarray_t slot_bitarray = bitarray_create(log_size);

				if (!slot_bitarray) {
					malloc_report(ASL_LEVEL_ERR, "bitarray_create(%d) in nano_print returned errno=%d.\n", log_size, errno);
					return;
				}

				chained_block_t head = NULL, tail = NULL, t;
				unsigned stoploss = (unsigned)slot_objects_mapped;
				while ((t = OSAtomicDequeue(&(pMeta->slot_LIFO), offsetof(struct chained_block_s, next)))) {
					if (0 == stoploss) {
						malloc_report(ASL_LEVEL_ERR, "Free list walk in nano_print exceeded object count.\n");
						break;
					}
					stoploss--;

					uintptr_t offset = ((uintptr_t)t - p.addr); // offset from beginning of slot
					index_t block_index = offset_to_index(nanozone, pMeta, offset);

					if (NULL == head) {
						head = t;
					} else {
						tail->next = t;
					}
					tail = t;

					if (block_index < slot_objects_mapped) {
						bitarray_set(slot_bitarray, log_size, block_index);
					}
				}
				if (tail) {
					tail->next = NULL;
				}

				index_t i;
				for (i = 0; i < slot_objects_mapped; ++i) {
					nano_blk_addr_t q;
					size_t pgnum;
					uintptr_t block_offset = index_to_offset(nanozone, pMeta, i);
					if (p.addr + block_offset >= slot_bump_addr) {
						break;
					}

					q.addr = p.addr + block_offset;
					pgnum = ((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >>
							vm_kernel_page_shift;

					if (i < pMeta->slot_objects_skipped) {
						malloc_report(MALLOC_REPORT_NOLOG | MALLOC_REPORT_NOPREFIX, "_");
					} else if (bitarray_get(slot_bitarray, log_size, i)) {
						malloc_report(MALLOC_REPORT_NOLOG | MALLOC_REPORT_NOPREFIX, "F");
					} else if (pMeta->slot_madvised_pages && (pgnum < (1 << pMeta->slot_madvised_log_page_count)) &&
							   bitarray_get(pMeta->slot_madvised_pages, pMeta->slot_madvised_log_page_count, (index_t)pgnum)) {
						malloc_report(MALLOC_REPORT_NOLOG | MALLOC_REPORT_NOPREFIX, "M");
					} else {
						malloc_report(MALLOC_REPORT_NOLOG | MALLOC_REPORT_NOPREFIX, ".");
					}
				}
				malloc_report(MALLOC_REPORT_NOLOG | MALLOC_REPORT_NOPREFIX, "\n");

				free(slot_bitarray);

				// push the free list extracted above back onto the LIFO, all at once
				if (head) {
					OSAtomicEnqueue(
							&(pMeta->slot_LIFO), head, (uintptr_t)tail - (uintptr_t)head + offsetof(struct chained_block_s, next));
				}
			}
		}
	}
	return;
}

static void
nano_log(malloc_zone_t *zone, void *log_address)
{
}

static void
nano_force_lock(nanozone_t *nanozone)
{
	int i;

	for (i = 0; i < nano_common_max_magazines; ++i) {
		_malloc_lock_lock(&nanozone->band_resupply_lock[i]);
	}
}

static void
nano_force_unlock(nanozone_t *nanozone)
{
	int i;

	for (i = 0; i < nano_common_max_magazines; ++i) {
		_malloc_lock_unlock(&nanozone->band_resupply_lock[i]);
	}
}

static void
nano_reinit_lock(nanozone_t *nanozone)
{
	int i;

	for (i = 0; i < nano_common_max_magazines; ++i) {
		_malloc_lock_init(&nanozone->band_resupply_lock[i]);
	}
}

static void
nano_statistics(nanozone_t *nanozone, malloc_statistics_t *stats)
{
	int i, j;

	bzero(stats, sizeof(*stats));

	for (i = 0; i < nano_common_max_magazines; ++i) {
		nano_blk_addr_t p;

		// Establish p as base address for slot 0 in this CPU magazine
		p.fields.nano_signature = NANOZONE_SIGNATURE;
		p.fields.nano_mag_index = i;
		p.fields.nano_band = 0;
		p.fields.nano_slot = 0;
		p.fields.nano_offset = 0;

		for (j = 0; j < NANO_SLOT_SIZE; p.addr += SLOT_IN_BAND_SIZE, // Advance to next slot base
				++j) {
			nano_meta_admin_t pMeta = &nanozone->meta_data[i][j];
			uintptr_t offset = pMeta->slot_bump_addr - p.addr;

			if (0 == pMeta->slot_current_base_addr) { // Nothing allocated in this magazine for this slot?
				continue;
			} else {
				unsigned blocks_touched = offset_to_index(nanozone, pMeta, offset) - (unsigned)pMeta->slot_objects_skipped;
				unsigned blocks_now_free = count_free(nanozone, pMeta);
				unsigned blocks_in_use = blocks_touched - blocks_now_free;

				size_t size_hiwater = ((j + 1) << SHIFT_NANO_QUANTUM) * blocks_touched;
				size_t size_in_use = ((j + 1) << SHIFT_NANO_QUANTUM) * blocks_in_use;
				size_t size_allocated = ((offset / BAND_SIZE) + 1) * SLOT_IN_BAND_SIZE;

				stats->blocks_in_use += blocks_in_use;

				stats->max_size_in_use += size_hiwater;
				stats->size_in_use += size_in_use;
				stats->size_allocated += size_allocated;
			}
		}
	}
}

static boolean_t
_nano_locked(nanozone_t *nanozone)
{
	int i;

	for (i = 0; i < nano_common_max_magazines; ++i) {
		if (_malloc_lock_trylock(&nanozone->band_resupply_lock[i])) {
			_malloc_lock_unlock(&nanozone->band_resupply_lock[i]);
			return TRUE;
		}
	}
	return FALSE;
}

static boolean_t
nano_locked(nanozone_t *nanozone)
{
	malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);

	return _nano_locked(nanozone) || zone->introspect->zone_locked(zone);
}

static const struct malloc_introspection_t nano_introspect = {
		(void *)nano_ptr_in_use_enumerator, (void *)nano_good_size, (void *)nanozone_check, (void *)nano_print, nano_log,
		(void *)nano_force_lock, (void *)nano_force_unlock, (void *)nano_statistics, (void *)nano_locked, NULL, NULL, NULL,
		NULL, /* Zone enumeration version 7 and forward. */
		(void *)nano_reinit_lock, // reinit_lock version 9 and foward
}; // marked as const to spare the DATA section

void
nano_forked_zone(nanozone_t *nanozone)
{
	/*
	 * Hobble the nano zone in the child of a fork prior to an exec since
	 * the state of the zone can be made inconsistent by a parent thread while the
	 * fork is underway.
	 * All new allocations will be referred to the helper zone (which is more stable.)
	 * All free()'s of existing nano objects will be leaked.
	 */

	mprotect(nanozone, sizeof(nanozone->basic_zone), PROT_READ | PROT_WRITE);

	nanozone->basic_zone.size = (void *)nano_size; /* Unchanged. */
	nanozone->basic_zone.malloc = (void *)nano_forked_malloc;
	nanozone->basic_zone.calloc = (void *)nano_forked_calloc;
	nanozone->basic_zone.valloc = (void *)nano_valloc; /* Unchanged, already always obtained from helper zone. */
	nanozone->basic_zone.free = (void *)nano_forked_free;
	nanozone->basic_zone.realloc = (void *)nano_forked_realloc;
	nanozone->basic_zone.destroy = (void *)nano_destroy; /* Unchanged. */
	nanozone->basic_zone.batch_malloc = (void *)nano_forked_batch_malloc;
	nanozone->basic_zone.batch_free = (void *)nano_forked_batch_free;
	nanozone->basic_zone.introspect = (struct malloc_introspection_t *)&nano_introspect; /* Unchanged. */
	nanozone->basic_zone.memalign = (void *)nano_memalign;								 /* Unchanged. */
	nanozone->basic_zone.free_definite_size = (void *)nano_forked_free_definite_size;
	nanozone->basic_zone.claimed_address = nano_forked_claimed_address;

	mprotect(nanozone, sizeof(nanozone->basic_zone), PROT_READ);
}

malloc_zone_t *
nano_create_zone(malloc_zone_t *helper_zone, unsigned debug_flags)
{
	nanozone_t *nanozone;
	int i, j;

	/* Note: It is important that nano_create_zone resets _malloc_engaged_nano
	 * if it is unable to enable the nanozone (and chooses not to abort). As
	 * several functions rely on _malloc_engaged_nano to determine if they
	 * should manipulate the nanozone, and these should not run if we failed
	 * to create the zone.
	 */
 	MALLOC_ASSERT(_malloc_engaged_nano == NANO_V1);

	/* get memory for the zone. */
	nanozone = nano_common_allocate_based_pages(NANOZONE_PAGED_SIZE, 0, 0, VM_MEMORY_MALLOC, 0);
	if (!nanozone) {
		_malloc_engaged_nano = NANO_NONE;
		return NULL;
	}

	/* set up the basic_zone portion of the nanozone structure */
	nanozone->basic_zone.version = 10;
	nanozone->basic_zone.size = (void *)nano_size;
	nanozone->basic_zone.malloc = (debug_flags & MALLOC_DO_SCRIBBLE) ? (void *)nano_malloc_scribble : (void *)nano_malloc;
	nanozone->basic_zone.calloc = (void *)nano_calloc;
	nanozone->basic_zone.valloc = (void *)nano_valloc;
	nanozone->basic_zone.free = (debug_flags & MALLOC_DO_SCRIBBLE) ? (void *)nano_free_scribble : (void *)nano_free;
	nanozone->basic_zone.realloc = (void *)nano_realloc;
	nanozone->basic_zone.destroy = (void *)nano_destroy;
	nanozone->basic_zone.batch_malloc = (void *)nano_batch_malloc;
	nanozone->basic_zone.batch_free = (void *)nano_batch_free;
	nanozone->basic_zone.introspect = (struct malloc_introspection_t *)&nano_introspect;
	nanozone->basic_zone.memalign = (void *)nano_memalign;
	nanozone->basic_zone.free_definite_size = (debug_flags & MALLOC_DO_SCRIBBLE) ? (void *)nano_free_definite_size_scribble
																						  : (void *)nano_free_definite_size;

	nanozone->basic_zone.pressure_relief = (void *)nano_pressure_relief;
	nanozone->basic_zone.claimed_address = (void *)nano_claimed_address;

	nanozone->basic_zone.reserved1 = 0; /* Set to zero once and for all as required by CFAllocator. */
	nanozone->basic_zone.reserved2 = 0; /* Set to zero once and for all as required by CFAllocator. */

	mprotect(nanozone, sizeof(nanozone->basic_zone), PROT_READ); /* Prevent overwriting the function pointers in basic_zone. */

	/* Nano zone does not support MALLOC_ADD_GUARD_PAGES. */
	if (debug_flags & MALLOC_ADD_GUARD_PAGES) {
		malloc_report(ASL_LEVEL_INFO, "nano zone does not support guard pages\n");
		debug_flags &= ~MALLOC_ADD_GUARD_PAGES;
	}

	/* set up the remainder of the nanozone structure */
	nanozone->debug_flags = debug_flags;

	if (phys_ncpus > sizeof(nanozone->core_mapped_size) /
			sizeof(nanozone->core_mapped_size[0])) {
		MALLOC_REPORT_FATAL_ERROR(phys_ncpus,
				"nanozone abandoned because NCPUS > max magazines.\n");
	}

	/* Initialize slot queue heads and resupply locks. */
	OSQueueHead q0 = OS_ATOMIC_QUEUE_INIT;
	for (i = 0; i < nano_common_max_magazines; ++i) {
		_malloc_lock_init(&nanozone->band_resupply_lock[i]);

		for (j = 0; j < NANO_SLOT_SIZE; ++j) {
			nanozone->meta_data[i][j].slot_LIFO = q0;
		}
	}

	/* Initialize the security token. */
	nanozone->cookie = (uintptr_t)malloc_entropy[0] & 0x0000ffffffff0000ULL; // scramble central 32bits with this cookie

	nanozone->helper_zone = helper_zone;

	return (malloc_zone_t *)nanozone;
}

void
nano_init(const char *envp[], const char *apple[],
		const char *bootargs MALLOC_UNUSED)
{
#if NANO_PREALLOCATE_BAND_VM
	// Unconditionally preallocate the VA space set aside for nano malloc to
	// reserve it in all configurations. rdar://problem/33392283
	boolean_t preallocated = nano_preallocate_band_vm();
	if (!preallocated) {
		malloc_report(ASL_LEVEL_NOTICE, "nano zone abandoned due to inability to preallocate reserved vm space.\n");
		_malloc_engaged_nano = NANO_NONE;
	}
#endif
}

// Second phase of initialization, called during _malloc_initialize(), after
// environment variables have been read and processed.
void
nano_configure()
{
	// Nothing to do.
}

#endif // CONFIG_NANOZONE

/* vim: set noet:ts=4:sw=4:cindent: */
