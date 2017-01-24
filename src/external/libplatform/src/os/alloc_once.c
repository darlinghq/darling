/*
 * Copyright (c) 2012-2013 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#include "os/internal.h"
#include "os/alloc_once_impl.h"
#include <mach/mach_init.h>
#include <mach/mach_vm.h>
#include <mach/vm_statistics.h>

#pragma mark -
#pragma mark os_alloc

typedef struct _os_alloc_heap_metadata_s {
	size_t allocated_bytes;
	void *prev;
} _os_alloc_heap_metadata_s;

#define allocation_size (2 * vm_page_size)
#define usable (allocation_size-sizeof(_os_alloc_heap_metadata_s))
static void * volatile _os_alloc_heap;

/*
 * Simple allocator that doesn't have to worry about ever freeing allocations.
 *
 * The heapptr entry of _os_alloc_once_metadata always points to the newest
 * available heap page, or NULL if this is the first allocation. The heap has a
 * small header at the top of each heap block, recording the currently
 * allocated bytes and the pointer to the previous heap block.
 *
 * Ignoring the special case where the heapptr is NULL; in which case we always
 * make a block. The allocator first atomically increments the allocated_bytes
 * counter by sz and calculates the eventual base pointer. If base+sz is
 * greater than allocation_size then we begin allocating a new page. Otherwise,
 * base is returned.
 *
 * Page allocation vm_allocates a new page of allocation_size and then attempts
 * to atomically cmpxchg that pointer with the current headptr. If successful,
 * it links the previous page to the new heap block for debugging purposes and
 * then reattempts allocation. If a thread loses the allocation race, it
 * vm_deallocates the still-clean region and reattempts the whole allocation.
 */

static inline void*
_os_alloc_alloc(void *heap, size_t sz)
{
	if (likely(heap)) {
		_os_alloc_heap_metadata_s *metadata = (_os_alloc_heap_metadata_s*)heap;
		size_t used = os_atomic_add(&metadata->allocated_bytes, sz, relaxed);
		if (likely(used <= usable)) {
			return ((char*)metadata + sizeof(_os_alloc_heap_metadata_s) +
					used - sz);
		}
	}
	/* This fall-through case is heap == NULL, or heap block is exhausted. */
	return NULL;
}

OS_NOINLINE
static void*
_os_alloc_slow(void *heap, size_t sz)
{
	void *ptr;
	do {
		/*
		 * <rdar://problem/13208498> We allocate at PAGE_SIZE or above to ensure
		 * we don't land in the zero page *if* a binary has opted not to include
		 * the __PAGEZERO load command.
		 */
		mach_vm_address_t heapblk = PAGE_SIZE;
		kern_return_t kr;
		kr = mach_vm_map(mach_task_self(), &heapblk, allocation_size,
				0, VM_FLAGS_ANYWHERE | VM_MAKE_TAG(VM_MEMORY_OS_ALLOC_ONCE),
				MEMORY_OBJECT_NULL, 0, FALSE, VM_PROT_DEFAULT, VM_PROT_ALL,
				VM_INHERIT_DEFAULT);
		if (unlikely(kr)) {
			__LIBPLATFORM_INTERNAL_CRASH__(kr, "Failed to allocate in os_alloc_once");
		}
		if (os_atomic_cmpxchg(&_os_alloc_heap, heap, (void*)heapblk, relaxed)) {
			((_os_alloc_heap_metadata_s*)heapblk)->prev = heap;
			heap = (void*)heapblk;
		} else {
			mach_vm_deallocate(mach_task_self(), heapblk, allocation_size);
			heap = _os_alloc_heap;
		}
		ptr = _os_alloc_alloc(heap, sz);
	} while (unlikely(!ptr));
	return ptr;
}

static inline void*
_os_alloc2(size_t sz)
{
	void *heap, *ptr;
	if (unlikely(!sz || sz > usable)) {
		__LIBPLATFORM_CLIENT_CRASH__(sz, "Requested allocation size is invalid");
	}
	heap = _os_alloc_heap;
	if (likely(ptr = _os_alloc_alloc(heap, sz))) {
		return ptr;
	}
	return _os_alloc_slow(heap, sz);
}

#pragma mark -
#pragma mark os_alloc_once

typedef struct _os_alloc_once_ctxt_s {
	struct _os_alloc_once_s *slot;
	size_t sz;
	os_function_t init;
} _os_alloc_once_ctxt_s;

static void
_os_alloc(void *ctxt)
{
	_os_alloc_once_ctxt_s *c = ctxt;
	c->slot->ptr = _os_alloc2((c->sz + 0xf) & ~0xfu);
	if (c->init) {
		c->init(c->slot->ptr);
	}
}

void*
_os_alloc_once(struct _os_alloc_once_s *slot, size_t sz, os_function_t init)
{
	_os_alloc_once_ctxt_s c = {
		.slot = slot,
		.sz = sz,
		.init = init,
	};
	_os_once(&slot->once, &c, _os_alloc);
	return slot->ptr;
}
