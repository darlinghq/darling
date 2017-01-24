/*
 * Copyright (c) 1999, 2006, 2008 Apple Inc. All rights reserved.
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

/* Author: Bertrand Serlet, August 1999 */

/*
 * Multithread enhancements for "tiny" allocations introduced February 2008.
 * These are in the spirit of "Hoard". See:
 * Berger, E.D.; McKinley, K.S.; Blumofe, R.D.; Wilson, P.R. (2000).
 * "Hoard: a scalable memory allocator for multithreaded applications".
 * ACM SIGPLAN Notices 35 (11): 117-128. Berger2000.
 * <http://portal.acm.org/citation.cfm?id=356989.357000>
 * Retrieved on 2008-02-22.
 */

#include "internal.h"

static volatile uintptr_t entropic_address = 0;
static volatile uintptr_t entropic_limit = 0;
#define ENTROPIC_KABILLION 0x10000000 /* 256Mb */

// <rdar://problem/22277891> align 64bit ARM shift to 32MB PTE entries
#if MALLOC_TARGET_IOS && MALLOC_TARGET_64BIT
#define ENTROPIC_SHIFT 25
#else // MALLOC_TARGET_IOS && MALLOC_TARGET_64BIT
#define ENTROPIC_SHIFT SMALL_BLOCKS_ALIGN
#endif

#if DEBUG_MALLOC
#define LOG(szone, ptr) (szone->log_address && (((uintptr_t)szone->log_address == -1) || (szone->log_address == (void *)(ptr))))
#else
#define LOG(szone, ptr) 0
#endif

/*********************	VERY LOW LEVEL UTILITIES  ************************/

static void
szone_sleep(void)
{
	if (getenv("MallocErrorStop")) {
		_malloc_printf(ASL_LEVEL_NOTICE, "*** sending SIGSTOP to help debug\n");
		kill(getpid(), SIGSTOP);
	} else if (getenv("MallocErrorSleep")) {
		_malloc_printf(ASL_LEVEL_NOTICE, "*** sleeping to help debug\n");
		sleep(3600); // to help debug
	}
}

// msg prints after fmt, ...
void
szone_error(szone_t *szone, int is_corruption, const char *msg, const void *ptr, const char *fmt, ...)
{
	va_list ap;
	_SIMPLE_STRING b = _simple_salloc();

	if (b) {
		if (fmt) {
			va_start(ap, fmt);
			_simple_vsprintf(b, fmt, ap);
			va_end(ap);
		}
		if (ptr) {
			_simple_sprintf(b, "*** error for object %p: %s\n", ptr, msg);
		} else {
			_simple_sprintf(b, "*** error: %s\n", msg);
		}
		malloc_printf("%s*** set a breakpoint in malloc_error_break to debug\n", _simple_string(b));
	} else {
		/*
		 * Should only get here if vm_allocate() can't get a single page of
		 * memory, implying _simple_asl_log() would also fail.  So we just
		 * print to the file descriptor.
		 */
		if (fmt) {
			va_start(ap, fmt);
			_malloc_vprintf(MALLOC_PRINTF_NOLOG, fmt, ap);
			va_end(ap);
		}
		if (ptr) {
			_malloc_printf(MALLOC_PRINTF_NOLOG, "*** error for object %p: %s\n", ptr, msg);
		} else {
			_malloc_printf(MALLOC_PRINTF_NOLOG, "*** error: %s\n", msg);
		}
		_malloc_printf(MALLOC_PRINTF_NOLOG, "*** set a breakpoint in malloc_error_break to debug\n");
	}
	malloc_error_break();
#if DEBUG_MALLOC
	szone_print(szone, 1);
#endif
	szone_sleep();
	// Call abort() if this is a memory corruption error and the abort on
	// corruption flag is set, or if any error should abort.
	if ((is_corruption && (szone->debug_flags & MALLOC_ABORT_ON_CORRUPTION)) ||
			(szone->debug_flags & MALLOC_ABORT_ON_ERROR)) {
		_os_set_crash_log_message_dynamic(b ? _simple_string(b) : msg);
		abort();
	} else if (b) {
		_simple_sfree(b);
	}
}

void
protect(void *address, size_t size, unsigned protection, unsigned debug_flags)
{
	kern_return_t err;

	if (!(debug_flags & MALLOC_DONT_PROTECT_PRELUDE)) {
		err = mprotect((void *)((uintptr_t)address - vm_page_quanta_size), vm_page_quanta_size, protection);
		if (err) {
			malloc_printf("*** can't protect(%p) region for prelude guard page at %p\n", protection,
					(uintptr_t)address - vm_page_quanta_size);
		}
	}
	if (!(debug_flags & MALLOC_DONT_PROTECT_POSTLUDE)) {
		err = mprotect((void *)(round_page_quanta(((uintptr_t)address + size))), vm_page_quanta_size, protection);
		if (err) {
			malloc_printf("*** can't protect(%p) region for postlude guard page at %p\n", protection, (uintptr_t)address + size);
		}
	}
}

void *
allocate_pages(szone_t *szone, size_t size, unsigned char align, unsigned debug_flags, int vm_page_label)
{
	boolean_t add_guard_pages = debug_flags & MALLOC_ADD_GUARD_PAGES;
	boolean_t purgeable = debug_flags & MALLOC_PURGEABLE;
	mach_vm_address_t vm_addr;
	uintptr_t addr;
	mach_vm_size_t allocation_size = round_page_quanta(size);
	mach_vm_offset_t allocation_mask = ((mach_vm_offset_t)1 << align) - 1;
	int alloc_flags = VM_FLAGS_ANYWHERE | VM_MAKE_TAG(vm_page_label);
	kern_return_t kr;

	if (!allocation_size) {
		allocation_size = vm_page_quanta_size;
	}
	if (add_guard_pages) {
		if (align > vm_page_quanta_shift) {
			/* <rdar://problem/16601499> alignment greater than pagesize needs more work */
			allocation_size += (1 << align) + vm_page_quanta_size;
		} else {
			allocation_size += 2 * vm_page_quanta_size;
		}
	}

	if (purgeable) {
		alloc_flags |= VM_FLAGS_PURGABLE;
	}
	if (allocation_size < size) { // size_t arithmetic wrapped!
		return NULL;
	}

	vm_addr = vm_page_quanta_size;
	kr = mach_vm_map(mach_task_self(), &vm_addr, allocation_size, allocation_mask, alloc_flags, MEMORY_OBJECT_NULL, 0, FALSE,
			VM_PROT_DEFAULT, VM_PROT_ALL, VM_INHERIT_DEFAULT);
	if (kr) {
		szone_error(szone, 0, "can't allocate region", NULL, "*** mach_vm_map(size=%lu) failed (error code=%d)\n", size, kr);
		return NULL;
	}
	addr = (uintptr_t)vm_addr;

	if (add_guard_pages) {
		if (align > vm_page_quanta_shift) {
			/* <rdar://problem/16601499> calculate the first address inside the alignment padding
			 * where we can place the guard page and still be aligned.
			 *
			 * |-----------------------------------------------------------|
			 * |leading|gp|                  alloc                  |gp| t |
			 * |-----------------------------------------------------------|
			 */
			uintptr_t alignaddr = ((addr + vm_page_quanta_size) + (1 << align) - 1) & ~((1 << align) - 1);
			size_t leading = alignaddr - addr - vm_page_quanta_size;
			size_t trailing = (1 << align) - vm_page_quanta_size - leading;

			/* Unmap the excess area. */
			kr = mach_vm_deallocate(mach_task_self(), addr, leading);
			if (kr) {
				szone_error(szone, 0, "can't unmap excess guard region", NULL,
						"*** mach_vm_deallocate(addr=%p, size=%lu) failed (code=%d)", (void *)addr, leading, kr);
				return NULL;
			}

			kr = mach_vm_deallocate(mach_task_self(), addr + allocation_size - trailing, trailing);
			if (kr) {
				szone_error(szone, 0, "can't unmap excess trailing guard region", NULL,
						"*** mach_vm_deallocate(addr=%p, size=%lu) failed (code=%d)", (void *)(addr + allocation_size - trailing),
						trailing, kr);
				return NULL;
			}

			addr = alignaddr;
		} else {
			addr += vm_page_quanta_size;
		}
		protect((void *)addr, size, PROT_NONE, debug_flags);
	}
	return (void *)addr;
}

void *
allocate_pages_securely(szone_t *szone, size_t size, unsigned char align, int vm_page_label)
{
	mach_vm_address_t vm_addr;
	uintptr_t addr;
	mach_vm_size_t allocation_size = round_page_quanta(size);
	mach_vm_offset_t allocation_mask = ((mach_vm_offset_t)1 << align) - 1;
	int alloc_flags = VM_FLAGS_ANYWHERE | VM_MAKE_TAG(vm_page_label);
	kern_return_t kr;

	if (szone->debug_flags & DISABLE_ASLR) {
		return allocate_pages(szone, size, align, 0, vm_page_label);
	}

	if (!allocation_size) {
		allocation_size = vm_page_quanta_size;
	}
	if (allocation_size < size) { // size_t arithmetic wrapped!
		return NULL;
	}

retry:
	vm_addr = entropic_address;
	kr = mach_vm_map(mach_task_self(), &vm_addr, allocation_size, allocation_mask, alloc_flags, MEMORY_OBJECT_NULL, 0, FALSE,
			VM_PROT_DEFAULT, VM_PROT_ALL, VM_INHERIT_DEFAULT);
	if (kr == KERN_NO_SPACE) {
		vm_addr = vm_page_quanta_size;
		kr = mach_vm_map(mach_task_self(), &vm_addr, allocation_size, allocation_mask, alloc_flags, MEMORY_OBJECT_NULL, 0, FALSE,
				VM_PROT_DEFAULT, VM_PROT_ALL, VM_INHERIT_DEFAULT);
	}
	if (kr) {
		szone_error(
				szone, 0, "can't allocate region securely", NULL, "*** mach_vm_map(size=%lu) failed (error code=%d)\n", size, kr);
		return NULL;
	}
	addr = (uintptr_t)vm_addr;

	// Don't allow allocation to rise above entropic_limit (for tidiness).
	if (addr + allocation_size > entropic_limit) { // Exhausted current range?
		uintptr_t t = entropic_address;
		uintptr_t u = t - ENTROPIC_KABILLION;

		if (u < t) { // provided we don't wrap, deallocate and retry, in the expanded entropic range
			mach_vm_deallocate(mach_task_self(), vm_addr, allocation_size);
			(void)__sync_bool_compare_and_swap(&entropic_address, t, u); // Just one reduction please
			goto retry;
		}
		// fall through to use what we got
	}

	if (addr < entropic_address) { // we wrapped to find this allocation, expand the entropic range
		uintptr_t t = entropic_address;
		uintptr_t u = t - ENTROPIC_KABILLION;
		if (u < t) {
			(void)__sync_bool_compare_and_swap(&entropic_address, t, u); // Just one reduction please
		}
		// fall through to use what we got
	}
	return (void *)addr;
}

void
deallocate_pages(szone_t *szone, void *addr, size_t size, unsigned debug_flags)
{
	boolean_t add_guard_pages = debug_flags & MALLOC_ADD_GUARD_PAGES;
	mach_vm_address_t vm_addr = (mach_vm_address_t)addr;
	mach_vm_size_t allocation_size = size;
	kern_return_t kr;

	if (add_guard_pages) {
		vm_addr -= vm_page_quanta_size;
		allocation_size += 2 * vm_page_quanta_size;
	}
	kr = mach_vm_deallocate(mach_task_self(), vm_addr, allocation_size);
	if (kr && szone) {
		szone_error(szone, 0, "Can't deallocate_pages region", addr, NULL);
	}
}

int
madvise_free_range(szone_t *szone, region_t r, uintptr_t pgLo, uintptr_t pgHi, uintptr_t *last)
{
	if (pgHi > pgLo) {
		size_t len = pgHi - pgLo;

		if (szone->debug_flags & MALLOC_DO_SCRIBBLE) {
			memset((void *)pgLo, SCRUBBLE_BYTE, len); // Scribble on MADV_FREEd memory
		}

#if TARGET_OS_EMBEDDED
		if (last) {
			if (*last == pgLo) {
				return 0;
			}

			*last = pgLo;
		}
#endif

		MAGMALLOC_MADVFREEREGION((void *)szone, (void *)r, (void *)pgLo, (int)len); // DTrace USDT Probe
		if (-1 == madvise((void *)pgLo, len, CONFIG_MADVISE_STYLE)) {
/* -1 return: VM map entry change makes this unfit for reuse. Something evil lurks. */
#if DEBUG_MADVISE
			szone_error(szone, 0, "madvise_free_range madvise(..., MADV_FREE_REUSABLE) failed", (void *)pgLo, "length=%d\n", len);
#endif
		}
	}
	return 0;
}

int
madvise_reuse_range(szone_t *szone, region_t r, uintptr_t pgLo, uintptr_t phHi)
{
	if (phHi > pgLo) {
		size_t len = phHi - pgLo;

		if (madvise((void *)pgLo, len, MADV_FREE_REUSE) == -1) {
/* -1 return: VM map entry change makes this unfit for reuse. Something evil lurks. */
#if DEBUG_MADVISE
			szone_error(szone, 0, "madvise_reuse_range madvise(..., MADV_FREE_REUSE) failed", sparse_region, "length=%d\n",
					TINY_REGION_PAYLOAD_BYTES);
#endif
			return 1;
		}
	}
	return 0;
}

/*********************	Zone call backs	************************/
/*
 * Mark these MALLOC_NOINLINE to avoid bloating the purgeable zone call backs
 */
void
szone_free(szone_t *szone, void *ptr)
{
	region_t tiny_region;
	region_t small_region;

#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_printf("in szone_free with %p\n", ptr);
	}
#endif
	if (!ptr) {
		return;
	}
	/*
	 * Try to free to a tiny region.
	 */
	if ((uintptr_t)ptr & (TINY_QUANTUM - 1)) {
		szone_error(szone, 1, "Non-aligned pointer being freed", ptr, NULL);
		return;
	}
	if ((tiny_region = tiny_region_for_ptr_no_lock(szone, ptr)) != NULL) {
		if (TINY_INDEX_FOR_PTR(ptr) >= NUM_TINY_BLOCKS) {
			szone_error(szone, 1, "Pointer to metadata being freed", ptr, NULL);
			return;
		}
		free_tiny(szone, ptr, tiny_region, 0);
		return;
	}

	/*
	 * Try to free to a small region.
	 */
	if ((uintptr_t)ptr & (SMALL_QUANTUM - 1)) {
		szone_error(szone, 1, "Non-aligned pointer being freed (2)", ptr, NULL);
		return;
	}
	if ((small_region = small_region_for_ptr_no_lock(szone, ptr)) != NULL) {
		if (SMALL_META_INDEX_FOR_PTR(ptr) >= NUM_SMALL_BLOCKS) {
			szone_error(szone, 1, "Pointer to metadata being freed (2)", ptr, NULL);
			return;
		}
		free_small(szone, ptr, small_region, 0);
		return;
	}

	/* check that it's a legal large allocation */
	if ((uintptr_t)ptr & (vm_page_quanta_size - 1)) {
		szone_error(szone, 1, "non-page-aligned, non-allocated pointer being freed", ptr, NULL);
		return;
	}
	free_large(szone, ptr);
}

void
szone_free_definite_size(szone_t *szone, void *ptr, size_t size)
{
#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_printf("in szone_free_definite_size with %p\n", ptr);
	}

	if (0 == size) {
		szone_error(szone, 1, "pointer of size zero being freed", ptr, NULL);
		return;
	}

#endif
	if (!ptr) {
		return;
	}

	/*
	 * Try to free to a tiny region.
	 */
	if ((uintptr_t)ptr & (TINY_QUANTUM - 1)) {
		szone_error(szone, 1, "Non-aligned pointer being freed", ptr, NULL);
		return;
	}
	if (size <= SMALL_THRESHOLD) {
		if (TINY_INDEX_FOR_PTR(ptr) >= NUM_TINY_BLOCKS) {
			szone_error(szone, 1, "Pointer to metadata being freed", ptr, NULL);
			return;
		}
		free_tiny(szone, ptr, TINY_REGION_FOR_PTR(ptr), size);
		return;
	}

	/*
	 * Try to free to a small region.
	 */
	if ((uintptr_t)ptr & (SMALL_QUANTUM - 1)) {
		szone_error(szone, 1, "Non-aligned pointer being freed (2)", ptr, NULL);
		return;
	}
	if (size <= szone->large_threshold) {
		if (SMALL_META_INDEX_FOR_PTR(ptr) >= NUM_SMALL_BLOCKS) {
			szone_error(szone, 1, "Pointer to metadata being freed (2)", ptr, NULL);
			return;
		}
		free_small(szone, ptr, SMALL_REGION_FOR_PTR(ptr), size);
		return;
	}

	/* check that it's a legal large allocation */
	if ((uintptr_t)ptr & (vm_page_quanta_size - 1)) {
		szone_error(szone, 1, "non-page-aligned, non-allocated pointer being freed", ptr, NULL);
		return;
	}
	free_large(szone, ptr);
}

MALLOC_NOINLINE void *
szone_malloc_should_clear(szone_t *szone, size_t size, boolean_t cleared_requested)
{
	void *ptr;
	msize_t msize;

	if (size <= SMALL_THRESHOLD) {
		// tiny size: <1024 bytes (64-bit), <512 bytes (32-bit)
		// think tiny
		msize = TINY_MSIZE_FOR_BYTES(size + TINY_QUANTUM - 1);
		if (!msize) {
			msize = 1;
		}
		ptr = tiny_malloc_should_clear(szone, msize, cleared_requested);
	} else if (size <= szone->large_threshold) {
		// small size: <15k (<1GB machines), <127k (>1GB machines)
		// think small
		msize = SMALL_MSIZE_FOR_BYTES(size + SMALL_QUANTUM - 1);
		if (!msize) {
			msize = 1;
		}
		ptr = small_malloc_should_clear(szone, msize, cleared_requested);
	} else {
		// large: all other allocations
		size_t num_kernel_pages = round_page_quanta(size) >> vm_page_quanta_shift;
		if (num_kernel_pages == 0) { /* Overflowed */
			ptr = 0;
		} else {
			ptr = large_malloc(szone, num_kernel_pages, 0, cleared_requested);
		}
	}
#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_printf("szone_malloc returned %p\n", ptr);
	}
#endif
	/*
	 * If requested, scribble on allocated memory.
	 */
	if ((szone->debug_flags & MALLOC_DO_SCRIBBLE) && ptr && !cleared_requested && size) {
		memset(ptr, SCRIBBLE_BYTE, szone_size(szone, ptr));
	}

	return ptr;
}

void *
szone_malloc(szone_t *szone, size_t size)
{
	return szone_malloc_should_clear(szone, size, 0);
}

void *
szone_calloc(szone_t *szone, size_t num_items, size_t size)
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

	return szone_malloc_should_clear(szone, total_bytes, 1);
}

void *
szone_valloc(szone_t *szone, size_t size)
{
	void *ptr;

	if (size <= szone->large_threshold) {
		ptr = szone_memalign(szone, vm_page_quanta_size, size);
	} else {
		size_t num_kernel_pages;

		num_kernel_pages = round_page_quanta(size) >> vm_page_quanta_shift;
		ptr = large_malloc(szone, num_kernel_pages, 0, 0);
	}

#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_printf("szone_valloc returned %p\n", ptr);
	}
#endif
	return ptr;
}

/* Isolate PIC-base load here. */
size_t
szone_size_try_large(szone_t *szone, const void *ptr)
{
	size_t size = 0;
	large_entry_t *entry;

	SZONE_LOCK(szone);
	entry = large_entry_for_pointer_no_lock(szone, ptr);
	if (entry) {
		size = entry->size;
	}
	SZONE_UNLOCK(szone);
#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_printf("szone_size for %p returned %d\n", ptr, (unsigned)size);
	}
#endif
	return size;
}

size_t
szone_size(szone_t *szone, const void *ptr)
{
	boolean_t is_free;
	msize_t msize, msize_and_free;

	if (!ptr) {
		return 0;
	}
#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_printf("in szone_size for %p (szone=%p)\n", ptr, szone);
	}
#endif

	/*
	 * Look for it in a tiny region.
	 */
	if ((uintptr_t)ptr & (TINY_QUANTUM - 1)) {
		return 0;
	}
	if (tiny_region_for_ptr_no_lock(szone, ptr)) {
		if (TINY_INDEX_FOR_PTR(ptr) >= NUM_TINY_BLOCKS) {
			return 0;
		}
		msize = get_tiny_meta_header(ptr, &is_free);
		if (is_free) {
			return 0;
		}
#if CONFIG_TINY_CACHE
		{
			mag_index_t mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(TINY_REGION_FOR_PTR(ptr));
			if (DEPOT_MAGAZINE_INDEX != mag_index) {
				magazine_t *tiny_mag_ptr = &(szone->tiny_magazines[mag_index]);

				if (msize < TINY_QUANTUM && ptr == (void *)((uintptr_t)(tiny_mag_ptr->mag_last_free) & ~(TINY_QUANTUM - 1))) {
					return 0;
				}
			} else {
				for (mag_index = 0; mag_index < szone->num_tiny_magazines; mag_index++) {
					magazine_t *tiny_mag_ptr = &(szone->tiny_magazines[mag_index]);

					if (msize < TINY_QUANTUM && ptr == (void *)((uintptr_t)(tiny_mag_ptr->mag_last_free) & ~(TINY_QUANTUM - 1))) {
						return 0;
					}
				}
			}
		}
#endif
		return TINY_BYTES_FOR_MSIZE(msize);
	}

	/*
	 * Look for it in a small region.
	 */
	if ((uintptr_t)ptr & (SMALL_QUANTUM - 1)) {
		return 0;
	}
	if (small_region_for_ptr_no_lock(szone, ptr)) {
		if (SMALL_META_INDEX_FOR_PTR(ptr) >= NUM_SMALL_BLOCKS) {
			return 0;
		}
		msize_and_free = *SMALL_METADATA_FOR_PTR(ptr);
		if (msize_and_free & SMALL_IS_FREE) {
			return 0;
		}
#if CONFIG_SMALL_CACHE
		{
			mag_index_t mag_index = MAGAZINE_INDEX_FOR_SMALL_REGION(SMALL_REGION_FOR_PTR(ptr));
			if (DEPOT_MAGAZINE_INDEX != mag_index) {
				magazine_t *small_mag_ptr = &(szone->small_magazines[mag_index]);

				if (ptr == (void *)((uintptr_t)(small_mag_ptr->mag_last_free) & ~(SMALL_QUANTUM - 1))) {
					return 0;
				}
			} else {
				for (mag_index = 0; mag_index < szone->num_small_magazines; mag_index++) {
					magazine_t *small_mag_ptr = &(szone->small_magazines[mag_index]);

					if (ptr == (void *)((uintptr_t)(small_mag_ptr->mag_last_free) & ~(SMALL_QUANTUM - 1))) {
						return 0;
					}
				}
			}
		}
#endif
		return SMALL_BYTES_FOR_MSIZE(msize_and_free);
	}

	/*
	 * If not page-aligned, it cannot have come from a large allocation.
	 */
	if ((uintptr_t)ptr & (vm_page_quanta_size - 1)) {
		return 0;
	}

	/*
	 * Look for it in a large entry.
	 */
	return szone_size_try_large(szone, ptr);
}

void *
szone_realloc(szone_t *szone, void *ptr, size_t new_size)
{
	size_t old_size, new_good_size, valid_size;
	void *new_ptr;

#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_printf("in szone_realloc for %p, %d\n", ptr, (unsigned)new_size);
	}
#endif
	if (NULL == ptr) {
		// If ptr is a null pointer, realloc() shall be equivalent to malloc() for the specified size.
		return szone_malloc(szone, new_size);
	} else if (0 == new_size) {
		// If size is 0 and ptr is not a null pointer, the object pointed to is freed.
		szone_free(szone, ptr);
		// If size is 0, either a null pointer or a unique pointer that can be successfully passed
		// to free() shall be returned.
		return szone_malloc(szone, 1);
	}

	old_size = szone_size(szone, ptr);
	if (!old_size) {
		szone_error(szone, 1, "pointer being reallocated was not allocated", ptr, NULL);
		return NULL;
	}

	new_good_size = szone_good_size(szone, new_size);
	if (new_good_size == old_size) { // Existing allocation is best fit evar?
		return ptr;
	}

	/*
	 * If the new size suits the tiny allocator and the pointer being resized
	 * belongs to a tiny region, try to reallocate in-place.
	 */
	if (new_good_size <= SMALL_THRESHOLD) {
		if (old_size <= SMALL_THRESHOLD) {
			if (new_good_size <= (old_size >> 1)) {
				/*
				 * Serious shrinkage (more than half). free() the excess.
				 */
				return tiny_try_shrink_in_place(szone, ptr, old_size, new_good_size);
			} else if (new_good_size <= old_size) {
				/*
				 * new_good_size smaller than old_size but not by much (less than half).
				 * Avoid thrashing at the expense of some wasted storage.
				 */
				if (szone->debug_flags & MALLOC_DO_SCRIBBLE) {
					memset(ptr + new_size, SCRIBBLE_BYTE, old_size - new_size);
				}
				return ptr;
			} else if (tiny_try_realloc_in_place(szone, ptr, old_size, new_good_size)) { // try to grow the allocation
				if (szone->debug_flags & MALLOC_DO_SCRIBBLE) {
					memset(ptr + old_size, SCRIBBLE_BYTE, new_good_size - old_size);
				}
				return ptr;
			}
		}

		/*
		 * Else if the new size suits the small allocator and the pointer being resized
		 * belongs to a small region, and we're not protecting the small allocations
		 * try to reallocate in-place.
		 */
	} else if (new_good_size <= szone->large_threshold) {
		if (SMALL_THRESHOLD < old_size && old_size <= szone->large_threshold) {
			if (new_good_size <= (old_size >> 1)) {
				return small_try_shrink_in_place(szone, ptr, old_size, new_good_size);
			} else if (new_good_size <= old_size) {
				if (szone->debug_flags & MALLOC_DO_SCRIBBLE) {
					memset(ptr + new_size, SCRIBBLE_BYTE, old_size - new_size);
				}
				return ptr;
			} else if (small_try_realloc_in_place(szone, ptr, old_size, new_good_size)) {
				if (szone->debug_flags & MALLOC_DO_SCRIBBLE) {
					memset(ptr + old_size, SCRIBBLE_BYTE, new_good_size - old_size);
				}
				return ptr;
			}
		}
		/*
		 * Else if the allocation's a large allocation, try to reallocate in-place there.
		 */
	} else if (!(szone->debug_flags & MALLOC_PURGEABLE) && // purgeable needs fresh allocation
			   (old_size > szone->large_threshold) && (new_good_size > szone->large_threshold)) {
		if (new_good_size <= (old_size >> 1)) {
			return large_try_shrink_in_place(szone, ptr, old_size, new_good_size);
		} else if (new_good_size <= old_size) {
			if (szone->debug_flags & MALLOC_DO_SCRIBBLE) {
				memset(ptr + new_size, SCRIBBLE_BYTE, old_size - new_size);
			}
			return ptr;
		} else if (large_try_realloc_in_place(szone, ptr, old_size, new_good_size)) {
			if (szone->debug_flags & MALLOC_DO_SCRIBBLE) {
				memset(ptr + old_size, SCRIBBLE_BYTE, new_good_size - old_size);
			}
			return ptr;
		}
	}

	/*
	 * Can't reallocate in place for whatever reason; allocate a new buffer and copy.
	 */
	if (new_good_size <= (old_size >> 1)) {
		/* Serious shrinkage (more than half). FALL THROUGH to alloc/copy/free. */
	} else if (new_good_size <= old_size) {
		if (szone->debug_flags & MALLOC_DO_SCRIBBLE) {
			memset(ptr + new_size, SCRIBBLE_BYTE, old_size - new_size);
		}
		return ptr;
	}

	new_ptr = szone_malloc(szone, new_size);
	if (new_ptr == NULL) {
		return NULL;
	}

	/*
	 * If the allocation's large enough, try to copy using VM.  If that fails, or
	 * if it's too small, just copy by hand.
	 */
	valid_size = MIN(old_size, new_size);
	if ((valid_size < szone->vm_copy_threshold) ||
			vm_copy(mach_task_self(), (vm_address_t)ptr, valid_size, (vm_address_t)new_ptr)) {
		memcpy(new_ptr, ptr, valid_size);
	}
	szone_free(szone, ptr);

#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_printf("szone_realloc returned %p for %d\n", new_ptr, (unsigned)new_size);
	}
#endif
	return new_ptr;
}

void *
szone_memalign(szone_t *szone, size_t alignment, size_t size)
{
	if (size == 0) {
		size = 1; // Ensures we'll return an aligned free()-able pointer
	}
	if ((size + alignment) < size) { // size_t arithmetic wrapped!
		return NULL;
	}

	// alignment is gauranteed a power of 2 at least as large as sizeof(void *), hence non-zero.
	// Since size + alignment didn't wrap, 0 <= size + alignment - 1 < size + alignment
	size_t span = size + alignment - 1;

	if (alignment <= TINY_QUANTUM) {
		return szone_malloc(szone, size); // Trivially satisfied by tiny, small, or large

	} else if (span <= SMALL_THRESHOLD) {
		return tiny_memalign(szone, alignment, size, span);

	} else if (SMALL_THRESHOLD < size && alignment <= SMALL_QUANTUM) {
		return szone_malloc(szone, size); // Trivially satisfied by small or large

	} else if (span <= szone->large_threshold) {
		return small_memalign(szone, alignment, size, span);

	} else if (szone->large_threshold < size && alignment <= vm_page_quanta_size) {
		return szone_malloc(szone, size); // Trivially satisfied by large

	} else {
		// ensure block allocated by large does not have a small-possible size
		size_t num_kernel_pages = round_page_quanta(MAX(szone->large_threshold + 1, size)) >> vm_page_quanta_shift;
		void *p;

		if (num_kernel_pages == 0) { /* Overflowed */
			p = NULL;
		} else {
			p = large_malloc(szone, num_kernel_pages, MAX(vm_page_quanta_shift, __builtin_ctz((unsigned)alignment)), 0);
		}

		return p;
	}
	/* NOTREACHED */
}

// given a size, returns the number of pointers allocated capable of holding
// that size, up to the limit specified by the 'count' argument.  These pointers
// are stored in the 'results' array, which must be allocated by the caller.
// may return zero, since this function is only a best attempt at allocating
// the pointers.  clients should be prepared to call malloc for any additional
// blocks they need.
unsigned
szone_batch_malloc(szone_t *szone, size_t size, void **results, unsigned count)
{
	msize_t msize = TINY_MSIZE_FOR_BYTES(size + TINY_QUANTUM - 1);
	unsigned found = 0;
	mag_index_t mag_index = mag_get_thread_index(szone);
	magazine_t *tiny_mag_ptr = &(szone->tiny_magazines[mag_index]);

	// only bother implementing this for tiny
	if (size > SMALL_THRESHOLD) {
		return 0;
	}
	// make sure to return objects at least one quantum in size
	if (!msize) {
		msize = 1;
	}

	CHECK(szone, __PRETTY_FUNCTION__);

	// We must lock the zone now, since tiny_malloc_from_free_list assumes that
	// the caller has done so.
	SZONE_MAGAZINE_PTR_LOCK(szone, tiny_mag_ptr);

	// with the zone locked, allocate objects from the free list until all
	// sufficiently large objects have been exhausted, or we have met our quota
	// of objects to allocate.
	while (found < count) {
		void *ptr = tiny_malloc_from_free_list(szone, tiny_mag_ptr, mag_index, msize);
		if (!ptr) {
			break;
		}

		*results++ = ptr;
		found++;
	}
	SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
	return found;
}

/* Try caching the tiny_region and checking if the next ptr hits there. */
void
szone_batch_free(szone_t *szone, void **to_be_freed, unsigned count)
{
	unsigned cc = 0;
	void *ptr;
	region_t tiny_region = NULL;
	boolean_t is_free;
	msize_t msize;
	magazine_t *tiny_mag_ptr = NULL;
	mag_index_t mag_index = -1;

	// frees all the pointers in to_be_freed
	// note that to_be_freed may be overwritten during the process
	if (!count) {
		return;
	}

	CHECK(szone, __PRETTY_FUNCTION__);
	while (cc < count) {
		ptr = to_be_freed[cc];
		if (ptr) {
			if (NULL == tiny_region || tiny_region != TINY_REGION_FOR_PTR(ptr)) { // region same as last iteration?
				if (tiny_mag_ptr) {												  // non-NULL iff magazine lock taken
					SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
					tiny_mag_ptr = NULL;
				}

				tiny_region = tiny_region_for_ptr_no_lock(szone, ptr);

				if (tiny_region) {
					tiny_mag_ptr = mag_lock_zine_for_region_trailer(szone, szone->tiny_magazines,
							REGION_TRAILER_FOR_TINY_REGION(tiny_region), MAGAZINE_INDEX_FOR_TINY_REGION(tiny_region));
					mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(tiny_region);
				}
			}
			if (tiny_region) {
				// this is a tiny pointer
				if (TINY_INDEX_FOR_PTR(ptr) >= NUM_TINY_BLOCKS) {
					break; // pointer to metadata; let the standard free deal with it
				}
				msize = get_tiny_meta_header(ptr, &is_free);
				if (is_free) {
					break; // a double free; let the standard free deal with it
				}
				if (!tiny_free_no_lock(szone, tiny_mag_ptr, mag_index, tiny_region, ptr, msize)) {
					// Arrange to re-acquire magazine lock
					tiny_mag_ptr = NULL;
					tiny_region = NULL;
				}
				to_be_freed[cc] = NULL;
			} else {
				// No region in this zone claims ptr; let the standard free deal with it
				break;
			}
		}
		cc++;
	}

	if (tiny_mag_ptr) {
		SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
		tiny_mag_ptr = NULL;
	}

	CHECK(szone, __PRETTY_FUNCTION__);
	while (count--) {
		ptr = to_be_freed[count];
		if (ptr) {
			szone_free(szone, ptr);
		}
	}
}

// FIXME: Suppose one of the locks is held?
static void
szone_destroy(szone_t *szone)
{
	size_t index;
	large_entry_t *large;
	vm_range_t range_to_deallocate;

#if CONFIG_LARGE_CACHE
	SZONE_LOCK(szone);

	/* disable any memory pressure responder */
	szone->flotsam_enabled = FALSE;

	// stack allocated copy of the death-row cache
	int idx = szone->large_entry_cache_oldest, idx_max = szone->large_entry_cache_newest;
	large_entry_t local_entry_cache[LARGE_ENTRY_CACHE_SIZE];

	memcpy((void *)local_entry_cache, (void *)szone->large_entry_cache, sizeof(local_entry_cache));

	szone->large_entry_cache_oldest = szone->large_entry_cache_newest = 0;
	szone->large_entry_cache[0].address = 0x0;
	szone->large_entry_cache[0].size = 0;
	szone->large_entry_cache_bytes = 0;
	szone->large_entry_cache_reserve_bytes = 0;

	SZONE_UNLOCK(szone);

	// deallocate the death-row cache outside the zone lock
	while (idx != idx_max) {
		deallocate_pages(szone, (void *)local_entry_cache[idx].address, local_entry_cache[idx].size, 0);
		if (++idx == LARGE_ENTRY_CACHE_SIZE) {
			idx = 0;
		}
	}
	if (0 != local_entry_cache[idx].address && 0 != local_entry_cache[idx].size) {
		deallocate_pages(szone, (void *)local_entry_cache[idx].address, local_entry_cache[idx].size, 0);
	}
#endif

	/* destroy large entries */
	index = szone->num_large_entries;
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

	/* destroy tiny regions */
	for (index = 0; index < szone->tiny_region_generation->num_regions_allocated; ++index) {
		if ((HASHRING_OPEN_ENTRY != szone->tiny_region_generation->hashed_regions[index]) &&
				(HASHRING_REGION_DEALLOCATED != szone->tiny_region_generation->hashed_regions[index])) {
			deallocate_pages(szone, szone->tiny_region_generation->hashed_regions[index], TINY_REGION_SIZE, 0);
		}
	}

	/* destroy small regions */
	for (index = 0; index < szone->small_region_generation->num_regions_allocated; ++index) {
		if ((HASHRING_OPEN_ENTRY != szone->small_region_generation->hashed_regions[index]) &&
				(HASHRING_REGION_DEALLOCATED != szone->small_region_generation->hashed_regions[index])) {
			deallocate_pages(szone, szone->small_region_generation->hashed_regions[index], SMALL_REGION_SIZE, 0);
		}
	}

	/* destroy region hash rings, if any */
	if (szone->tiny_region_generation->hashed_regions != szone->initial_tiny_regions) {
		size_t size = round_page_quanta(szone->tiny_region_generation->num_regions_allocated * sizeof(region_t));
		deallocate_pages(szone, szone->tiny_region_generation->hashed_regions, size, 0);
	}
	if (szone->small_region_generation->hashed_regions != szone->initial_small_regions) {
		size_t size = round_page_quanta(szone->small_region_generation->num_regions_allocated * sizeof(region_t));
		deallocate_pages(szone, szone->small_region_generation->hashed_regions, size, 0);
	}

	/* Now destroy the separate szone region */
	deallocate_pages(szone, (void *)&(szone->tiny_magazines[-1]), TINY_MAGAZINE_PAGED_SIZE, MALLOC_ADD_GUARD_PAGES);
	deallocate_pages(szone, (void *)&(szone->small_magazines[-1]), SMALL_MAGAZINE_PAGED_SIZE, MALLOC_ADD_GUARD_PAGES);
	deallocate_pages(szone, (void *)szone, SZONE_PAGED_SIZE, 0);
}

size_t
szone_good_size(szone_t *szone, size_t size)
{
	msize_t msize;

	// Find a good size for this tiny allocation.
	if (size <= SMALL_THRESHOLD) {
		msize = TINY_MSIZE_FOR_BYTES(size + TINY_QUANTUM - 1);
		if (!msize) {
			msize = 1;
		}
		return TINY_BYTES_FOR_MSIZE(msize);
	}

	// Find a good size for this small allocation.
	if (size <= szone->large_threshold) {
		msize = SMALL_MSIZE_FOR_BYTES(size + SMALL_QUANTUM - 1);
		if (!msize) {
			msize = 1;
		}
		return SMALL_BYTES_FOR_MSIZE(msize);
	}

	// Check for integer overflow on the size, since unlike the two cases above,
	// there is no upper bound on allocation size at this point.
	if (size > round_page_quanta(size)) {
		return (size_t)(-1LL);
	}

#if DEBUG_MALLOC
	// It is not acceptable to see a size of zero here, since that means we
	// failed to catch a request for zero bytes in the tiny check, or the size
	// overflowed to zero during some arithmetic.
	if (size == 0) {
		malloc_printf("szone_good_size() invariant broken %y\n", size);
	}
#endif
	return round_page_quanta(size);
}

unsigned szone_check_counter = 0;
unsigned szone_check_start = 0;
unsigned szone_check_modulo = 1;

static MALLOC_NOINLINE boolean_t
szone_check_all(szone_t *szone, const char *function)
{
	size_t index;

	/* check tiny regions - chould check region count */
	for (index = 0; index < szone->tiny_region_generation->num_regions_allocated; ++index) {
		region_t tiny = szone->tiny_region_generation->hashed_regions[index];

		if (HASHRING_REGION_DEALLOCATED == tiny) {
			continue;
		}

		if (tiny) {
			magazine_t *tiny_mag_ptr = mag_lock_zine_for_region_trailer(
					szone, szone->tiny_magazines, REGION_TRAILER_FOR_TINY_REGION(tiny), MAGAZINE_INDEX_FOR_TINY_REGION(tiny));

			if (!tiny_check_region(szone, tiny)) {
				SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
				szone->debug_flags &= ~CHECK_REGIONS;
				szone_error(szone, 1, "check: tiny region incorrect", NULL,
						"*** tiny region %ld incorrect szone_check_all(%s) counter=%d\n", index, function, szone_check_counter);
				return 0;
			}
			SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_mag_ptr);
		}
	}
	/* check tiny free lists */
	for (index = 0; index < NUM_TINY_SLOTS; ++index) {
		if (!tiny_free_list_check(szone, (grain_t)index)) {
			szone->debug_flags &= ~CHECK_REGIONS;
			szone_error(szone, 1, "check: tiny free list incorrect", NULL,
					"*** tiny free list incorrect (slot=%ld) szone_check_all(%s) counter=%d\n", index, function,
					szone_check_counter);
			return 0;
		}
	}

	/* check small regions - could check region count */
	for (index = 0; index < szone->small_region_generation->num_regions_allocated; ++index) {
		region_t small = szone->small_region_generation->hashed_regions[index];

		if (HASHRING_REGION_DEALLOCATED == small) {
			continue;
		}

		if (small) {
			magazine_t *small_mag_ptr = mag_lock_zine_for_region_trailer(
					szone, szone->small_magazines, REGION_TRAILER_FOR_SMALL_REGION(small), MAGAZINE_INDEX_FOR_SMALL_REGION(small));

			if (!small_check_region(szone, small)) {
				SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
				szone->debug_flags &= ~CHECK_REGIONS;
				szone_error(szone, 1, "check: small region incorrect", NULL,
						"*** small region %ld incorrect szone_check_all(%s) counter=%d\n", index, function, szone_check_counter);
				return 0;
			}
			SZONE_MAGAZINE_PTR_UNLOCK(szone, small_mag_ptr);
		}
	}
	/* check small free lists */
	for (index = 0; index < szone->num_small_slots; ++index) {
		if (!small_free_list_check(szone, (grain_t)index)) {
			szone->debug_flags &= ~CHECK_REGIONS;
			szone_error(szone, 1, "check: small free list incorrect", NULL,
					"*** small free list incorrect (slot=%ld) szone_check_all(%s) counter=%d\n", index, function,
					szone_check_counter);
			return 0;
		}
	}

	return 1;
}

static boolean_t
szone_check(szone_t *szone)
{
	if ((++szone_check_counter % 10000) == 0) {
		_malloc_printf(ASL_LEVEL_NOTICE, "at szone_check counter=%d\n", szone_check_counter);
	}

	if (szone_check_counter < szone_check_start) {
		return 1;
	}

	if (szone_check_counter % szone_check_modulo) {
		return 1;
	}

	return szone_check_all(szone, "");
}

static kern_return_t
szone_ptr_in_use_enumerator(task_t task,
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

	err = tiny_in_use_enumerator(task, context, type_mask, szone, reader, recorder);
	if (err) {
		return err;
	}

	err = small_in_use_enumerator(task, context, type_mask, szone, reader, recorder);
	if (err) {
		return err;
	}

	err = large_in_use_enumerator(
			task, context, type_mask, (vm_address_t)szone->large_entries, szone->num_large_entries, reader, recorder);
	return err;
}

// Following method is deprecated:  use scalable_zone_statistics instead
void
scalable_zone_info(malloc_zone_t *zone, unsigned *info_to_fill, unsigned count)
{
	szone_t *szone = (void *)zone;
	unsigned info[13];

	// We do not lock to facilitate debug

	size_t s = 0;
	unsigned t = 0;
	size_t u = 0;
	mag_index_t mag_index;

	for (mag_index = -1; mag_index < szone->num_tiny_magazines; mag_index++) {
		s += szone->tiny_magazines[mag_index].mag_bytes_free_at_start;
		s += szone->tiny_magazines[mag_index].mag_bytes_free_at_end;
		t += szone->tiny_magazines[mag_index].mag_num_objects;
		u += szone->tiny_magazines[mag_index].mag_num_bytes_in_objects;
	}

	info[4] = (unsigned)t;
	info[5] = (unsigned)u;

	for (t = 0, u = 0, mag_index = -1; mag_index < szone->num_small_magazines; mag_index++) {
		s += szone->small_magazines[mag_index].mag_bytes_free_at_start;
		s += szone->small_magazines[mag_index].mag_bytes_free_at_end;
		t += szone->small_magazines[mag_index].mag_num_objects;
		u += szone->small_magazines[mag_index].mag_num_bytes_in_objects;
	}

	info[6] = (unsigned)t;
	info[7] = (unsigned)u;

	info[8] = (unsigned)szone->num_large_objects_in_use;
	info[9] = (unsigned)szone->num_bytes_in_large_objects;

	info[10] = 0; // DEPRECATED szone->num_huge_entries;
	info[11] = 0; // DEPRECATED szone->num_bytes_in_huge_objects;

	info[12] = szone->debug_flags;

	info[0] = info[4] + info[6] + info[8] + info[10];
	info[1] = info[5] + info[7] + info[9] + info[11];

	info[3] = (unsigned)(szone->num_tiny_regions - szone->num_tiny_regions_dealloc) * TINY_REGION_SIZE +
			  (unsigned)(szone->num_small_regions - szone->num_small_regions_dealloc) * SMALL_REGION_SIZE + info[9] + info[11];

	info[2] = info[3] - (unsigned)s;
	memcpy(info_to_fill, info, sizeof(unsigned) * count);
}

// FIXME: consistent picture requires locking!
static MALLOC_NOINLINE void
szone_print(szone_t *szone, boolean_t verbose)
{
	unsigned info[13];
	size_t index;
	region_t region;

	scalable_zone_info((void *)szone, info, 13);
	_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX,
			"Scalable zone %p: inUse=%u(%y) touched=%y allocated=%y flags=%d\n", szone, info[0], info[1], info[2], info[3],
			info[12]);
	_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX, "\ttiny=%u(%y) small=%u(%y) large=%u(%y) huge=%u(%y)\n", info[4],
			info[5], info[6], info[7], info[8], info[9], info[10], info[11]);
	// tiny
	_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX, "%lu tiny regions:\n", szone->num_tiny_regions);
	if (szone->num_tiny_regions_dealloc) {
		_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX, "[%lu tiny regions have been vm_deallocate'd]\n",
				szone->num_tiny_regions_dealloc);
	}
	for (index = 0; index < szone->tiny_region_generation->num_regions_allocated; ++index) {
		region = szone->tiny_region_generation->hashed_regions[index];
		if (HASHRING_OPEN_ENTRY != region && HASHRING_REGION_DEALLOCATED != region) {
			mag_index_t mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(region);
			print_tiny_region(verbose, region, (region == szone->tiny_magazines[mag_index].mag_last_region)
													   ? szone->tiny_magazines[mag_index].mag_bytes_free_at_start
													   : 0,
					(region == szone->tiny_magazines[mag_index].mag_last_region)
							? szone->tiny_magazines[mag_index].mag_bytes_free_at_end
							: 0);
		}
	}
	if (verbose) {
		print_tiny_free_list(szone);
	}
	// small
	_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX, "%lu small regions:\n", szone->num_small_regions);
	if (szone->num_small_regions_dealloc) {
		_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX, "[%lu small regions have been vm_deallocate'd]\n",
				szone->num_small_regions_dealloc);
	}
	for (index = 0; index < szone->small_region_generation->num_regions_allocated; ++index) {
		region = szone->small_region_generation->hashed_regions[index];
		if (HASHRING_OPEN_ENTRY != region && HASHRING_REGION_DEALLOCATED != region) {
			mag_index_t mag_index = MAGAZINE_INDEX_FOR_SMALL_REGION(region);
			print_small_region(szone, verbose, region, (region == szone->small_magazines[mag_index].mag_last_region)
															   ? szone->small_magazines[mag_index].mag_bytes_free_at_start
															   : 0,
					(region == szone->small_magazines[mag_index].mag_last_region)
							? szone->small_magazines[mag_index].mag_bytes_free_at_end
							: 0);
		}
	}
	if (verbose) {
		print_small_free_list(szone);
	}
}

static void
szone_log(malloc_zone_t *zone, void *log_address)
{
	szone_t *szone = (szone_t *)zone;

	szone->log_address = log_address;
}

// <rdar://problem/18001324>
// When forcing the lock on the entire zone, make sure we are out of the critical section in each magazine
static MALLOC_INLINE void
szone_force_lock_magazine(szone_t *szone, magazine_t *mag)
{
	while (1) {
		SZONE_MAGAZINE_PTR_LOCK(szone, mag);
		if (!mag->alloc_underway) {
			return;
		}

		SZONE_MAGAZINE_PTR_UNLOCK(szone, mag);
		yield();
	}
}

static void
szone_force_lock(szone_t *szone)
{
	mag_index_t i;

	for (i = 0; i < szone->num_tiny_magazines; ++i) {
		szone_force_lock_magazine(szone, &szone->tiny_magazines[i]);
	}
	szone_force_lock_magazine(szone, &szone->tiny_magazines[DEPOT_MAGAZINE_INDEX]);

	for (i = 0; i < szone->num_small_magazines; ++i) {
		szone_force_lock_magazine(szone, &szone->small_magazines[i]);
	}
	szone_force_lock_magazine(szone, &szone->small_magazines[DEPOT_MAGAZINE_INDEX]);

	SZONE_LOCK(szone);
}

static void
szone_force_unlock(szone_t *szone)
{
	mag_index_t i;

	SZONE_UNLOCK(szone);

	for (i = -1; i < szone->num_small_magazines; ++i) {
		SZONE_MAGAZINE_PTR_UNLOCK(szone, (&(szone->small_magazines[i])));
	}

	for (i = -1; i < szone->num_tiny_magazines; ++i) {
		SZONE_MAGAZINE_PTR_UNLOCK(szone, (&(szone->tiny_magazines[i])));
	}
}

static void
szone_reinit_lock(szone_t *szone)
{
	mag_index_t i;

	SZONE_REINIT_LOCK(szone);

	for (i = -1; i < szone->num_small_magazines; ++i) {
		SZONE_MAGAZINE_PTR_REINIT_LOCK(szone, (&(szone->small_magazines[i])));
	}

	for (i = -1; i < szone->num_tiny_magazines; ++i) {
		SZONE_MAGAZINE_PTR_REINIT_LOCK(szone, (&(szone->tiny_magazines[i])));
	}
}

static boolean_t
szone_locked(szone_t *szone)
{
	mag_index_t i;
	int tookLock;

	tookLock = SZONE_TRY_LOCK(szone);
	if (tookLock == 0) {
		return 1;
	}
	SZONE_UNLOCK(szone);

	for (i = -1; i < szone->num_small_magazines; ++i) {
		tookLock = SZONE_MAGAZINE_PTR_TRY_LOCK(szone, (&(szone->small_magazines[i])));
		if (tookLock == 0) {
			return 1;
		}
		SZONE_MAGAZINE_PTR_UNLOCK(szone, (&(szone->small_magazines[i])));
	}

	for (i = -1; i < szone->num_tiny_magazines; ++i) {
		tookLock = SZONE_MAGAZINE_PTR_TRY_LOCK(szone, (&(szone->tiny_magazines[i])));
		if (tookLock == 0) {
			return 1;
		}
		SZONE_MAGAZINE_PTR_UNLOCK(szone, (&(szone->tiny_magazines[i])));
	}
	return 0;
}

size_t
szone_pressure_relief(szone_t *szone, size_t goal)
{
	size_t total = 0;

#if CONFIG_MADVISE_PRESSURE_RELIEF
	mag_index_t mag_index;

	magazine_t *tiny_depot_ptr = (&szone->tiny_magazines[DEPOT_MAGAZINE_INDEX]);
	magazine_t *small_depot_ptr = (&szone->small_magazines[DEPOT_MAGAZINE_INDEX]);

	for (mag_index = 0; mag_index < szone->num_tiny_magazines; mag_index++) {
		size_t index;
		for (index = 0; index < szone->tiny_region_generation->num_regions_allocated; ++index) {
			SZONE_LOCK(szone);

			region_t tiny = szone->tiny_region_generation->hashed_regions[index];
			if (!tiny || tiny == HASHRING_REGION_DEALLOCATED) {
				SZONE_UNLOCK(szone);
				continue;
			}

			magazine_t *mag_ptr = mag_lock_zine_for_region_trailer(
					szone, szone->tiny_magazines, REGION_TRAILER_FOR_TINY_REGION(tiny), MAGAZINE_INDEX_FOR_TINY_REGION(tiny));
			SZONE_UNLOCK(szone);

			/* Ordering is important here, the magazine of a region may potentially change
			 * during mag_lock_zine_for_region_trailer, so src_mag_index must be taken
			 * after we've obtained the lock.
			 */
			mag_index_t src_mag_index = MAGAZINE_INDEX_FOR_TINY_REGION(tiny);

			/* We can (and must) ignore magazines that are already in the recirc depot. */
			if (src_mag_index == DEPOT_MAGAZINE_INDEX) {
				SZONE_MAGAZINE_PTR_UNLOCK(szone, mag_ptr);
				continue;
			}

			if (tiny == mag_ptr->mag_last_region && (mag_ptr->mag_bytes_free_at_end || mag_ptr->mag_bytes_free_at_start)) {
				tiny_finalize_region(szone, mag_ptr);
			}

			/* Because this region is currently in use, we can't safely madvise it while
			 * it's attached to the magazine. For this operation we have to remove it from
			 * the current mag, attach it to the depot and then madvise.
			 */

			recirc_list_extract(szone, mag_ptr, REGION_TRAILER_FOR_TINY_REGION(tiny));
			int objects_in_use = tiny_free_detach_region(szone, mag_ptr, tiny);

			SZONE_MAGAZINE_PTR_LOCK(szone, tiny_depot_ptr);
			MAGAZINE_INDEX_FOR_TINY_REGION(tiny) = DEPOT_MAGAZINE_INDEX;
			REGION_TRAILER_FOR_TINY_REGION(tiny)->pinned_to_depot = 0;

			size_t bytes_inplay = tiny_free_reattach_region(szone, tiny_depot_ptr, tiny);

			/* Fix up the metadata of the target magazine while the region is in the depot. */
			mag_ptr->mag_num_bytes_in_objects -= bytes_inplay;
			mag_ptr->num_bytes_in_magazine -= TINY_REGION_PAYLOAD_BYTES;
			mag_ptr->mag_num_objects -= objects_in_use;

			/* Now we can drop the magazine lock of the source mag. */
			SZONE_MAGAZINE_PTR_UNLOCK(szone, mag_ptr);

			tiny_depot_ptr->mag_num_bytes_in_objects += bytes_inplay;
			tiny_depot_ptr->num_bytes_in_magazine += TINY_REGION_PAYLOAD_BYTES;
			tiny_depot_ptr->mag_num_objects -= objects_in_use;

			recirc_list_splice_last(szone, tiny_depot_ptr, REGION_TRAILER_FOR_TINY_REGION(tiny));

			/* Actually do the scan, done holding the depot lock, the call will drop the lock
			 * around the actual madvise syscalls.
			 */
			tiny_free_scan_madvise_free(szone, tiny_depot_ptr, tiny);

			/* Now the region is in the recirc depot, the next allocations to require more
			 * blocks will come along and take one of these regions back out of the depot.
			 * As OS X madvise's reuse on an per-region basis, we leave as many of these
			 * regions in the depot as possible after memory pressure.
			 */
			SZONE_MAGAZINE_PTR_UNLOCK(szone, tiny_depot_ptr);
		}
	}

	for (mag_index = 0; mag_index < szone->num_small_magazines; mag_index++) {
		size_t index;
		for (index = 0; index < szone->small_region_generation->num_regions_allocated; ++index) {
			SZONE_LOCK(szone);

			region_t small = szone->small_region_generation->hashed_regions[index];
			if (!small || small == HASHRING_REGION_DEALLOCATED) {
				SZONE_UNLOCK(szone);
				continue;
			}

			magazine_t *mag_ptr = mag_lock_zine_for_region_trailer(
					szone, szone->small_magazines, REGION_TRAILER_FOR_SMALL_REGION(small), MAGAZINE_INDEX_FOR_SMALL_REGION(small));
			SZONE_UNLOCK(szone);

			/* Ordering is important here, the magazine of a region may potentially change
			 * during mag_lock_zine_for_region_trailer, so src_mag_index must be taken
			 * after we've obtained the lock.
			 */
			mag_index_t src_mag_index = MAGAZINE_INDEX_FOR_SMALL_REGION(small);

			/* We can (and must) ignore magazines that are already in the recirc depot. */
			if (src_mag_index == DEPOT_MAGAZINE_INDEX) {
				SZONE_MAGAZINE_PTR_UNLOCK(szone, mag_ptr);
				continue;
			}

			if (small == mag_ptr->mag_last_region && (mag_ptr->mag_bytes_free_at_end || mag_ptr->mag_bytes_free_at_start)) {
				small_finalize_region(szone, mag_ptr);
			}

			/* Because this region is currently in use, we can't safely madvise it while
			 * it's attached to the magazine. For this operation we have to remove it from
			 * the current mag, attach it to the depot and then madvise.
			 */

			recirc_list_extract(szone, mag_ptr, REGION_TRAILER_FOR_SMALL_REGION(small));
			int objects_in_use = small_free_detach_region(szone, mag_ptr, small);

			SZONE_MAGAZINE_PTR_LOCK(szone, small_depot_ptr);
			MAGAZINE_INDEX_FOR_SMALL_REGION(small) = DEPOT_MAGAZINE_INDEX;
			REGION_TRAILER_FOR_SMALL_REGION(small)->pinned_to_depot = 0;

			size_t bytes_inplay = small_free_reattach_region(szone, small_depot_ptr, small);

			/* Fix up the metadata of the target magazine while the region is in the depot. */
			mag_ptr->mag_num_bytes_in_objects -= bytes_inplay;
			mag_ptr->num_bytes_in_magazine -= SMALL_REGION_PAYLOAD_BYTES;
			mag_ptr->mag_num_objects -= objects_in_use;

			/* Now we can drop the magazine lock of the source mag. */
			SZONE_MAGAZINE_PTR_UNLOCK(szone, mag_ptr);

			small_depot_ptr->mag_num_bytes_in_objects += bytes_inplay;
			small_depot_ptr->num_bytes_in_magazine += SMALL_REGION_PAYLOAD_BYTES;
			small_depot_ptr->mag_num_objects -= objects_in_use;

			recirc_list_splice_last(szone, small_depot_ptr, REGION_TRAILER_FOR_SMALL_REGION(small));

			/* Actually do the scan, done holding the depot lock, the call will drop the lock
			 * around the actual madvise syscalls.
			 */
			small_free_scan_madvise_free(szone, small_depot_ptr, small);

			/* Now the region is in the recirc depot, the next allocations to require more
			 * blocks will come along and take one of these regions back out of the depot.
			 * As OS X madvise's reuse on an per-region basis, we leave as many of these
			 * regions in the depot as possible after memory pressure.
			 */
			SZONE_MAGAZINE_PTR_UNLOCK(szone, small_depot_ptr);
		}
	}
#endif

#if CONFIG_LARGE_CACHE
	if (szone->flotsam_enabled) {
		SZONE_LOCK(szone);

		// stack allocated copy of the death-row cache
		int idx = szone->large_entry_cache_oldest, idx_max = szone->large_entry_cache_newest;
		large_entry_t local_entry_cache[LARGE_ENTRY_CACHE_SIZE];

		memcpy((void *)local_entry_cache, (void *)szone->large_entry_cache, sizeof(local_entry_cache));

		szone->large_entry_cache_oldest = szone->large_entry_cache_newest = 0;
		szone->large_entry_cache[0].address = 0x0;
		szone->large_entry_cache[0].size = 0;
		szone->large_entry_cache_bytes = 0;
		szone->large_entry_cache_reserve_bytes = 0;

		szone->flotsam_enabled = FALSE;

		SZONE_UNLOCK(szone);

		// deallocate the death-row cache outside the zone lock
		size_t total = 0;
		while (idx != idx_max) {
			deallocate_pages(szone, (void *)local_entry_cache[idx].address, local_entry_cache[idx].size, 0);
			total += local_entry_cache[idx].size;
			if (++idx == LARGE_ENTRY_CACHE_SIZE) {
				idx = 0;
			}
		}
		if (0 != local_entry_cache[idx].address && 0 != local_entry_cache[idx].size) {
			deallocate_pages(szone, (void *)local_entry_cache[idx].address, local_entry_cache[idx].size, 0);
			total += local_entry_cache[idx].size;
		}
	}
#endif

	MAGMALLOC_PRESSURERELIEF((void *)szone, (int)goal, (int)total); // DTrace USDT Probe
	return total;
}

boolean_t
scalable_zone_statistics(malloc_zone_t *zone, malloc_statistics_t *stats, unsigned subzone)
{
	szone_t *szone = (szone_t *)zone;

	switch (subzone) {
	case 0: {
		size_t s = 0;
		unsigned t = 0;
		size_t u = 0;
		mag_index_t mag_index;

		for (mag_index = -1; mag_index < szone->num_tiny_magazines; mag_index++) {
			s += szone->tiny_magazines[mag_index].mag_bytes_free_at_start;
			s += szone->tiny_magazines[mag_index].mag_bytes_free_at_end;
			t += szone->tiny_magazines[mag_index].mag_num_objects;
			u += szone->tiny_magazines[mag_index].mag_num_bytes_in_objects;
		}

		stats->blocks_in_use = t;
		stats->size_in_use = u;
		stats->size_allocated = (szone->num_tiny_regions - szone->num_tiny_regions_dealloc) * TINY_REGION_SIZE;
		stats->max_size_in_use = stats->size_allocated - s;
		return 1;
	}
	case 1: {
		size_t s = 0;
		unsigned t = 0;
		size_t u = 0;
		mag_index_t mag_index;

		for (mag_index = -1; mag_index < szone->num_small_magazines; mag_index++) {
			s += szone->small_magazines[mag_index].mag_bytes_free_at_start;
			s += szone->small_magazines[mag_index].mag_bytes_free_at_end;
			t += szone->small_magazines[mag_index].mag_num_objects;
			u += szone->small_magazines[mag_index].mag_num_bytes_in_objects;
		}

		stats->blocks_in_use = t;
		stats->size_in_use = u;
		stats->size_allocated = (szone->num_small_regions - szone->num_small_regions_dealloc) * SMALL_REGION_SIZE;
		stats->max_size_in_use = stats->size_allocated - s;
		return 1;
	}
	case 2:
		stats->blocks_in_use = szone->num_large_objects_in_use;
		stats->size_in_use = szone->num_bytes_in_large_objects;
		stats->max_size_in_use = stats->size_allocated = stats->size_in_use;
		return 1;
	case 3:
		stats->blocks_in_use = 0; // DEPRECATED szone->num_huge_entries;
		stats->size_in_use = 0;   // DEPRECATED szone->num_bytes_in_huge_objects;
		stats->max_size_in_use = stats->size_allocated = 0;
		return 1;
	}
	return 0;
}

static void
szone_statistics(szone_t *szone, malloc_statistics_t *stats)
{
	size_t large;

	size_t s = 0;
	unsigned t = 0;
	size_t u = 0;
	mag_index_t mag_index;

	for (mag_index = -1; mag_index < szone->num_tiny_magazines; mag_index++) {
		s += szone->tiny_magazines[mag_index].mag_bytes_free_at_start;
		s += szone->tiny_magazines[mag_index].mag_bytes_free_at_end;
		t += szone->tiny_magazines[mag_index].mag_num_objects;
		u += szone->tiny_magazines[mag_index].mag_num_bytes_in_objects;
	}

	for (mag_index = -1; mag_index < szone->num_small_magazines; mag_index++) {
		s += szone->small_magazines[mag_index].mag_bytes_free_at_start;
		s += szone->small_magazines[mag_index].mag_bytes_free_at_end;
		t += szone->small_magazines[mag_index].mag_num_objects;
		u += szone->small_magazines[mag_index].mag_num_bytes_in_objects;
	}

	large = szone->num_bytes_in_large_objects + 0; // DEPRECATED szone->num_bytes_in_huge_objects;

	stats->blocks_in_use = t + szone->num_large_objects_in_use + 0; // DEPRECATED szone->num_huge_entries;
	stats->size_in_use = u + large;
	stats->max_size_in_use = stats->size_allocated =
			(szone->num_tiny_regions - szone->num_tiny_regions_dealloc) * TINY_REGION_SIZE +
			(szone->num_small_regions - szone->num_small_regions_dealloc) * SMALL_REGION_SIZE + large;
	// Now we account for the untouched areas
	stats->max_size_in_use -= s;
}

const struct malloc_introspection_t szone_introspect = {
		(void *)szone_ptr_in_use_enumerator, (void *)szone_good_size, (void *)szone_check, (void *)szone_print, szone_log,
		(void *)szone_force_lock, (void *)szone_force_unlock, (void *)szone_statistics, (void *)szone_locked, NULL, NULL, NULL,
		NULL, /* Zone enumeration version 7 and forward. */
		(void *)szone_reinit_lock, // reinit_lock version 9 and foward
}; // marked as const to spare the DATA section

szone_t *
create_scalable_szone(size_t initial_size, unsigned debug_flags)
{
	szone_t *szone;

#if defined(__i386__) || defined(__x86_64__)
	if (_COMM_PAGE_VERSION_REQD > (*((uint16_t *)_COMM_PAGE_VERSION))) {
		MALLOC_PRINTF_FATAL_ERROR((*((uint16_t *)_COMM_PAGE_VERSION)), "comm page version mismatch");
	}
#endif

	/* get memory for the zone. */
	szone = allocate_pages(NULL, SZONE_PAGED_SIZE, 0, 0, VM_MEMORY_MALLOC);
	if (!szone) {
		return NULL;
	}

/* set up the szone structure */
#if 0
#warning CHECK_REGIONS enabled
	debug_flags |= CHECK_REGIONS;
#endif
#if 0
#warning LOG enabled
	szone->log_address = ~0;
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

#if CONFIG_SMALL_CUTTOFF_127KB 
	szone->is_largemem = 1;
	szone->num_small_slots = NUM_SMALL_SLOTS_LARGEMEM;
	szone->large_threshold = LARGE_THRESHOLD_LARGEMEM;
	szone->vm_copy_threshold = VM_COPY_THRESHOLD_LARGEMEM;
#else // CONFIG_SMALL_CUTTOFF_127KB
	szone->is_largemem = 0;
	szone->num_small_slots = NUM_SMALL_SLOTS;
	szone->large_threshold = LARGE_THRESHOLD;
	szone->vm_copy_threshold = VM_COPY_THRESHOLD;
#endif // CONFIG_SMALL_CUTTOFF_127KB

#if CONFIG_LARGE_CACHE
	uint64_t hw_memsize = 0;

#if CONFIG_HAS_COMMPAGE_MEMSIZE
	hw_memsize = *(uint64_t *)(uintptr_t)_COMM_PAGE_MEMORY_SIZE;
#else
	size_t uint64_t_size = sizeof(hw_memsize);
	// hw_memsize was always 0 if sysctlbyname failed, so preserve that behaviour
	(void)sysctlbyname("hw.memsize", &hw_memsize, &uint64_t_size, 0, 0);
#endif

	// madvise(..., MADV_REUSABLE) death-row arrivals above this threshold [~0.1%]
	szone->large_entry_cache_reserve_limit = hw_memsize >> 10; 

	/* <rdar://problem/6610904> Reset protection when returning a previous large allocation? */
	int32_t libSystemVersion = NSVersionOfLinkTimeLibrary("System");
	if ((-1 != libSystemVersion) && ((libSystemVersion >> 16) < 112) /* CFSystemVersionSnowLeopard */) {
		szone->large_legacy_reset_mprotect = TRUE;
	} else {
		szone->large_legacy_reset_mprotect = FALSE;
	}
#endif

// Prepare ASLR
#if __i386__ || __x86_64__ || __arm64__ || TARGET_OS_EMBEDDED
#if __i386__
	uintptr_t stackbase = 0x8fe00000;
	int entropic_bits = 3;
#elif __x86_64__
	uintptr_t stackbase = USRSTACK64;
	int entropic_bits = 16;
#elif __arm64__
	uintptr_t stackbase = USRSTACK64;
	int entropic_bits = 7;
#else
	uintptr_t stackbase = USRSTACK;
	int entropic_bits = 3;
#endif

	// assert(((1 << entropic_bits) - 1) << SMALL_BLOCKS_ALIGN < (stackbase - MAXSSIZ - ENTROPIC_KABILLION));

	if (0 != _dyld_get_image_slide((const struct mach_header *)_NSGetMachExecuteHeader())) {
		if (0 == entropic_address) {
			uintptr_t t = stackbase - MAXSSIZ - ((uintptr_t)(malloc_entropy[1] & ((1 << entropic_bits) - 1)) << ENTROPIC_SHIFT);
			(void)__sync_bool_compare_and_swap(&entropic_limit, 0, t);						  // Just one initialization please
			(void)__sync_bool_compare_and_swap(&entropic_address, 0, t - ENTROPIC_KABILLION); // Just one initialization please
		}
		debug_flags &= ~DISABLE_ASLR;
	} else {
		// zero slide when ASLR has been disabled by boot-arg. Eliminate cloaking.
		malloc_entropy[0] = 0;
		malloc_entropy[1] = 0;
		debug_flags |= DISABLE_ASLR;
	}

#else
	malloc_entropy[0] = 0;
	malloc_entropy[1] = 0;
	debug_flags |= DISABLE_ASLR;
#endif

	// Initialize the security token.
	szone->cookie = (uintptr_t)malloc_entropy[0];

	szone->basic_zone.version = 9;
	szone->basic_zone.size = (void *)szone_size;
	szone->basic_zone.malloc = (void *)szone_malloc;
	szone->basic_zone.calloc = (void *)szone_calloc;
	szone->basic_zone.valloc = (void *)szone_valloc;
	szone->basic_zone.free = (void *)szone_free;
	szone->basic_zone.realloc = (void *)szone_realloc;
	szone->basic_zone.destroy = (void *)szone_destroy;
	szone->basic_zone.batch_malloc = (void *)szone_batch_malloc;
	szone->basic_zone.batch_free = (void *)szone_batch_free;
	szone->basic_zone.introspect = (struct malloc_introspection_t *)&szone_introspect;
	szone->basic_zone.memalign = (void *)szone_memalign;
	szone->basic_zone.free_definite_size = (void *)szone_free_definite_size;
	szone->basic_zone.pressure_relief = (void *)szone_pressure_relief;

	szone->basic_zone.reserved1 = 0;					   /* Set to zero once and for all as required by CFAllocator. */
	szone->basic_zone.reserved2 = 0;					   /* Set to zero once and for all as required by CFAllocator. */
	mprotect(szone, sizeof(szone->basic_zone), PROT_READ); /* Prevent overwriting the function pointers in basic_zone. */

	szone->debug_flags = debug_flags;
	_malloc_lock_init(&szone->large_szone_lock);

	szone->cpu_id_key = -1UL; // Unused.

// Query the number of configured processors.
// Uniprocessor case gets just one tiny and one small magazine (whose index is zero). This gives
// the same behavior as the original scalable malloc. MP gets per-CPU magazines
// that scale (way) better.
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) || defined(__arm64__)
	int nproc = *(uint8_t *)(uintptr_t)_COMM_PAGE_NCPUS;
#else
	int nproc = sysconf(_SC_NPROCESSORS_CONF);
#endif
	szone->num_tiny_magazines = (nproc > 1) ? MIN(nproc, TINY_MAX_MAGAZINES) : 1;

	// FIXME vm_allocate() based on number of configured CPUs
	magazine_t *tiny_magazines =
			allocate_pages(NULL, TINY_MAGAZINE_PAGED_SIZE, 0, MALLOC_ADD_GUARD_PAGES, VM_MEMORY_MALLOC);
	if (NULL == tiny_magazines) {
		return NULL;
	}

	szone->tiny_magazines = &(tiny_magazines[1]); // szone->tiny_magazines[-1] is the Depot

	// The magazines are indexed in [0 .. (num_tiny_magazines - 1)]
	// Find the smallest power of 2 that exceeds (num_tiny_magazines - 1)
	szone->num_tiny_magazines_mask_shift = 0;
	int i = 1;
	while (i <= (szone->num_tiny_magazines - 1)) {
		szone->num_tiny_magazines_mask_shift++;
		i <<= 1;
	}

	// Now if i <= TINY_MAX_MAGAZINES we'll never access tiny_magazines[] out of bounds.
	if (i > TINY_MAX_MAGAZINES) {
		MALLOC_PRINTF_FATAL_ERROR(i, "magazine mask exceeds allocated magazines");
	}

	// Reduce i by 1 to obtain a mask covering [0 .. (num_tiny_magazines - 1)]
	szone->num_tiny_magazines_mask = i - 1; // A mask used for hashing to a magazine index (and a safety aid)
	szone->last_tiny_advise = 0;

	// Init the tiny_magazine locks
	_malloc_lock_init(&szone->tiny_regions_lock);
	_malloc_lock_init(&szone->tiny_magazines[DEPOT_MAGAZINE_INDEX].magazine_lock);
	for (i = 0; i < szone->num_tiny_magazines; ++i) {
		_malloc_lock_init(&szone->tiny_magazines[i].magazine_lock);
	}

	szone->num_small_magazines = (nproc > 1) ? MIN(nproc, SMALL_MAX_MAGAZINES) : 1;

	// FIXME vm_allocate() based on number of configured CPUs
	magazine_t *small_magazines =
			allocate_pages(NULL, SMALL_MAGAZINE_PAGED_SIZE, 0, MALLOC_ADD_GUARD_PAGES, VM_MEMORY_MALLOC);
	if (NULL == small_magazines) {
		return NULL;
	}

	szone->small_magazines = &(small_magazines[1]); // szone->small_magazines[-1] is the Depot

	// The magazines are indexed in [0 .. (num_small_magazines - 1)]
	// Find the smallest power of 2 that exceeds (num_small_magazines - 1)
	szone->num_small_magazines_mask_shift = 0;
	while (i <= (szone->num_small_magazines - 1)) {
		szone->num_small_magazines_mask_shift++;
		i <<= 1;
	}

	// Now if i <= SMALL_MAX_MAGAZINES we'll never access small_magazines[] out of bounds.
	if (i > SMALL_MAX_MAGAZINES) {
		MALLOC_PRINTF_FATAL_ERROR(i, "magazine mask exceeds allocated magazines");
	}

	// Reduce i by 1 to obtain a mask covering [0 .. (num_small_magazines - 1)]
	szone->num_small_magazines_mask = i - 1; // A mask used for hashing to a magazine index (and a safety aid)
	szone->last_small_advise = 0;

	// Init the small_magazine locks
	_malloc_lock_init(&szone->small_regions_lock);
	_malloc_lock_init(&szone->small_magazines[DEPOT_MAGAZINE_INDEX].magazine_lock);
	for (i = 0; i < szone->num_small_magazines; ++i) {
		_malloc_lock_init(&szone->small_magazines[i].magazine_lock);
	}

	CHECK(szone, __PRETTY_FUNCTION__);
	return szone;
}

malloc_zone_t *
create_scalable_zone(size_t initial_size, unsigned debug_flags) {
	return (malloc_zone_t *) create_scalable_szone(initial_size, debug_flags);
}

/* vim: set noet:ts=4:sw=4:cindent: */
