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

static volatile uintptr_t entropic_address = 0;
static volatile uintptr_t entropic_limit = 0;

MALLOC_NOEXPORT
uint64_t malloc_entropy[2] = {0, 0};

#define ENTROPIC_KABILLION 0x10000000 /* 256Mb */

// <rdar://problem/22277891> align 64bit ARM shift to 32MB PTE entries
#if MALLOC_TARGET_IOS && MALLOC_TARGET_64BIT
#define ENTROPIC_SHIFT 25
#else // MALLOC_TARGET_IOS && MALLOC_TARGET_64BIT
#define ENTROPIC_SHIFT SMALL_BLOCKS_ALIGN
#endif

void
mvm_aslr_init(void)
{
	// Prepare ASLR
#if __i386__ || __x86_64__ || __arm64__ || (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
#if __i386__
	uintptr_t stackbase = 0x8fe00000;
	int entropic_bits = 3;
#elif __x86_64__
	uintptr_t stackbase = USRSTACK64;
	int entropic_bits = 16;
#elif __arm64__
#if __LP64__
	uintptr_t stackbase = USRSTACK64;
	int entropic_bits = 7;
#else // __LP64__
	uintptr_t stackbase = USRSTACK;
	int entropic_bits = 3;
#endif
#else
	uintptr_t stackbase = USRSTACK;
	int entropic_bits = 3;
#endif
	// assert(((1 << entropic_bits) - 1) << SMALL_BLOCKS_ALIGN < (stackbase - MAXSSIZ - ENTROPIC_KABILLION));

	if (mvm_aslr_enabled()) {
		if (0 == entropic_address) {
			uintptr_t t = stackbase - MAXSSIZ - ((uintptr_t)(malloc_entropy[1] & ((1 << entropic_bits) - 1)) << ENTROPIC_SHIFT);
			OSAtomicCompareAndSwapLong(0, t, (volatile long *)&entropic_limit);
			OSAtomicCompareAndSwapLong(0, t - ENTROPIC_KABILLION, (volatile long *)&entropic_address);
		}
	} else {
		// zero slide when ASLR has been disabled by boot-arg. Eliminate cloaking.
		malloc_entropy[0] = 0;
		malloc_entropy[1] = 0;
	}
#else // TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#error ASLR unhandled on this platform
#endif // TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
}

void *
mvm_allocate_pages(size_t size, unsigned char align, unsigned debug_flags, int vm_page_label)
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
		malloc_zone_error(debug_flags, false, "can't allocate region\n*** mach_vm_map(size=%lu) failed (error code=%d)\n", size, kr);
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
				malloc_zone_error(debug_flags, false, "can't unmap excess guard region\n"
						"*** mach_vm_deallocate(addr=%p, size=%lu) failed (code=%d)\n",
						(void *)addr, leading, kr);
				return NULL;
			}

			kr = mach_vm_deallocate(mach_task_self(), addr + allocation_size - trailing, trailing);
			if (kr) {
				malloc_zone_error(debug_flags, false, "can't unmap excess trailing guard region\n"
						"*** mach_vm_deallocate(addr=%p, size=%lu) failed (code=%d)\n",
						(void *)(addr + allocation_size - trailing), trailing, kr);
				return NULL;
			}

			addr = alignaddr;
		} else {
			addr += vm_page_quanta_size;
		}
		mvm_protect((void *)addr, size, PROT_NONE, debug_flags);
	}
	return (void *)addr;
}

void *
mvm_allocate_pages_securely(size_t size, unsigned char align, int vm_page_label, uint32_t debug_flags)
{
	mach_vm_address_t vm_addr;
	uintptr_t addr;
	mach_vm_size_t allocation_size = round_page_quanta(size);
	mach_vm_offset_t allocation_mask = ((mach_vm_offset_t)1 << align) - 1;
	int alloc_flags = VM_FLAGS_ANYWHERE | VM_MAKE_TAG(vm_page_label);
	kern_return_t kr;

	if (debug_flags & DISABLE_ASLR) {
		return mvm_allocate_pages(size, align, 0, vm_page_label);
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
		malloc_zone_error(debug_flags, false, "can't allocate region securely\n",
				"*** mach_vm_map(size=%lu) failed (error code=%d)\n", size, kr);
		return NULL;
	}
	addr = (uintptr_t)vm_addr;

	// Don't allow allocation to rise above entropic_limit (for tidiness).
	if (addr + allocation_size > entropic_limit) { // Exhausted current range?
		uintptr_t t = entropic_address;
		uintptr_t u = t - ENTROPIC_KABILLION;

		if (u < t) { // provided we don't wrap, deallocate and retry, in the expanded entropic range
			mach_vm_deallocate(mach_task_self(), vm_addr, allocation_size);
			OSAtomicCompareAndSwapLong(t, u, (volatile long *)&entropic_address);  // Just one reduction please
			goto retry;
		}
		// fall through to use what we got
	}

	if (addr < entropic_address) { // we wrapped to find this allocation, expand the entropic range
		uintptr_t t = entropic_address;
		uintptr_t u = t - ENTROPIC_KABILLION;
		if (u < t) {
			OSAtomicCompareAndSwapLong(t, u, (volatile long *)&entropic_address);  // Just one reduction please
		}
		// fall through to use what we got
	}
	return (void *)addr;
}

void
mvm_deallocate_pages(void *addr, size_t size, unsigned debug_flags)
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
	if (kr) {
		malloc_zone_error(debug_flags, false, "Can't deallocate_pages region at %p\n", addr);
	}
}

void
mvm_protect(void *address, size_t size, unsigned protection, unsigned debug_flags)
{
	kern_return_t err;

	if (!(debug_flags & MALLOC_DONT_PROTECT_PRELUDE)) {
		err = mprotect((void *)((uintptr_t)address - vm_page_quanta_size), vm_page_quanta_size, protection);
		if (err) {
			malloc_report(ASL_LEVEL_ERR, "*** can't mvm_protect(%u) region for prelude guard page at %p\n", protection,
					(void *)((uintptr_t)address - vm_page_quanta_size));
		}
	}
	if (!(debug_flags & MALLOC_DONT_PROTECT_POSTLUDE)) {
		err = mprotect((void *)(round_page_quanta(((uintptr_t)address + size))), vm_page_quanta_size, protection);
		if (err) {
			malloc_report(ASL_LEVEL_ERR, "*** can't mvm_protect(%u) region for postlude guard page at %p\n", protection,
					(void *)((uintptr_t)address + size));
		}
	}
}

int
mvm_madvise_free(void *rack, void *r, uintptr_t pgLo, uintptr_t pgHi, uintptr_t *last, boolean_t scribble)
{
	if (pgHi > pgLo) {
		size_t len = pgHi - pgLo;

		if (scribble) {
			memset((void *)pgLo, SCRUBBLE_BYTE, len); // Scribble on MADV_FREEd memory
		}

#if TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
		if (last) {
			if (*last == pgLo) {
				return 0;
			}

			*last = pgLo;
		}
#endif // TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR

		MAGMALLOC_MADVFREEREGION(rack, r, (void *)pgLo, (int)len); // DTrace USDT Probe
		if (-1 == madvise((void *)pgLo, len, CONFIG_MADVISE_STYLE)) {
			/* -1 return: VM map entry change makes this unfit for reuse. Something evil lurks. */
#if DEBUG_MADVISE
			malloc_zone_error(NULL, false,
					"madvise_free_range madvise(..., MADV_FREE_REUSABLE) failed for %p, length=%d\n",
					(void *)pgLo, len);
#endif
			return 1;
		} else {
			MALLOC_TRACE(TRACE_madvise, (uintptr_t)r, (uintptr_t)pgLo, len, CONFIG_MADVISE_STYLE);
		}
	}
	return 0;
}
