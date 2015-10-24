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

#ifdef __LP64__ /* nano_malloc for 64bit ABI */
#define NDEBUG 1
#define NANO_FREE_DEQUEUE_DILIGENCE 1 /* Check for corrupt free list */

#include <_simple.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <TargetConditionals.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/param.h>

#include <mach/mach.h>
#include <mach/mach_vm.h>

#include <libkern/OSAtomic.h>
#include <mach-o/dyld_priv.h>	/* for _dyld_get_image_slide() */
#include <crt_externs.h>	/* for _NSGetMachExecuteHeader() */

#include <os/tsd.h>

#if defined(__x86_64__)
#define __APPLE_API_PRIVATE
#include <machine/cpu_capabilities.h>
#define _COMM_PAGE_VERSION_REQD 9
#undef __APPLE_API_PRIVATE
#else
Unknown Architecture
#endif

#include "scalable_malloc.h"
#include "malloc_internal.h"
#include "malloc_printf.h"

#include <CrashReporterClient.h>

#include "bitarray.h"

#ifndef VM_MEMORY_MALLOC_NANO /* Until osfmk/mach/vm_statistics.h is updated in xnu */
#define VM_MEMORY_MALLOC_NANO 11
#endif

extern uint64_t malloc_entropy[2];
/*********************	DEFINITIONS	************************/

#define INLINE	__inline__
#define ALWAYSINLINE __attribute__((always_inline))
#define NOINLINE __attribute__((noinline))

#if defined(__x86_64__)
#define CACHE_LINE	64
#define CACHE_ALIGN __attribute__ ((aligned (64) ))
#else
#define CACHE_ALIGN /* TBD for other platforms */
#endif

#define NANO_MAG_INDEX(nz)		(_os_cpu_number() >> nz->hyper_shift)

#define SCRIBBLE_BYTE			0xaa /* allocated scribble */
#define SCRABBLE_BYTE			0x55 /* free()'d scribble */
#define SCRUBBLE_BYTE			0xdd /* madvise(..., MADV_FREE) scriblle */

#define MAX_RECORDER_BUFFER		256

/*************          nanozone address field layout        ******************/

#if defined(__x86_64)
#define NANO_SIGNATURE_BITS		20
#define NANOZONE_SIGNATURE		0x00006ULL			// 0x00006nnnnnnnnnnn the address range devoted to us.
#define NANO_MAG_BITS			5
#define NANO_BAND_BITS			18
#define NANO_SLOT_BITS			4
#define NANO_OFFSET_BITS		17

#else
#error Unknown Architecture
#endif

#if defined(__BIG_ENDIAN__)
struct nano_blk_addr_s {
	uint64_t
nano_signature:NANO_SIGNATURE_BITS,	// 0x00006nnnnnnnnnnn the address range devoted to us.
nano_mag_index:NANO_MAG_BITS,		// the core that allocated this block
nano_band:NANO_BAND_BITS,
nano_slot:NANO_SLOT_BITS,		// bucket of homogenous quanta-multiple blocks
nano_offset:NANO_OFFSET_BITS;		// locates the block
};
#else
// least significant bits declared first
struct nano_blk_addr_s {
	uint64_t
nano_offset:NANO_OFFSET_BITS,		// locates the block
nano_slot:NANO_SLOT_BITS,		// bucket of homogenous quanta-multiple blocks
nano_band:NANO_BAND_BITS,
nano_mag_index:NANO_MAG_BITS,		// the core that allocated this block
nano_signature:NANO_SIGNATURE_BITS;	// 0x00006nnnnnnnnnnn the address range devoted to us.
};
#endif

typedef union  {
	uint64_t			addr;
	struct nano_blk_addr_s	fields;
} nano_blk_addr_t;

/* Are we using the nano allocator? Set by the initializer. */
__attribute__((visibility("hidden")))
boolean_t _malloc_engaged_nano;

#define NANO_MAX_SIZE			256 /* Buckets sized {16, 32, 48, 64, 80, 96, 112, ...} */
#define SHIFT_NANO_QUANTUM		4
#define NANO_REGIME_QUANTA_SIZE		(1 << SHIFT_NANO_QUANTUM)	// 16
#define NANO_QUANTA_MASK		0xFULL				// NANO_REGIME_QUANTA_SIZE - 1

#define SLOT_IN_BAND_SIZE 	(1 << NANO_OFFSET_BITS)
#define SLOT_KEY_LIMIT 		(1 << NANO_SLOT_BITS) /* Must track nano_slot width */
#define BAND_SIZE 		(1 << (NANO_SLOT_BITS + NANO_OFFSET_BITS)) /*  == Number of bytes covered by a page table entry */
#define NANO_MAG_SIZE 		(1 << NANO_MAG_BITS)
#define NANO_SLOT_SIZE 		(1 << NANO_SLOT_BITS)

/****************************** zone itself ***********************************/

/*
 * Note that objects whose adddress are held in pointers here must be pursued
 * individually in the nano_in_use_enumeration() routines.
 */

typedef struct chained_block_s {
	uintptr_t			double_free_guard;
	struct chained_block_s	*next;
} *chained_block_t;

typedef struct nano_meta_s {
	OSQueueHead			slot_LIFO CACHE_ALIGN;
    unsigned int		slot_madvised_log_page_count;
	volatile uintptr_t		slot_current_base_addr;
	volatile uintptr_t		slot_limit_addr;
	volatile size_t		slot_objects_mapped;
	volatile size_t		slot_objects_skipped;
	bitarray_t			slot_madvised_pages;
    volatile uintptr_t		slot_bump_addr CACHE_ALIGN; // position on cache line distinct from that of slot_LIFO
    volatile boolean_t		slot_exhausted;
	unsigned int		slot_bytes;
	unsigned int		slot_objects;
} *nano_meta_admin_t;

typedef struct nanozone_s {				// vm_allocate()'d, so page-aligned to begin with.
	malloc_zone_t		basic_zone;		// first page will be given read-only protection
	uint8_t			pad[PAGE_MAX_SIZE - sizeof(malloc_zone_t)];

	// remainder of structure is R/W (contains no function pointers)
	// page-aligned
	struct nano_meta_s		meta_data[NANO_MAG_SIZE][NANO_SLOT_SIZE]; // max: NANO_MAG_SIZE cores x NANO_SLOT_SIZE slots for nano blocks {16 .. 256}
	_malloc_lock_s			band_resupply_lock[NANO_MAG_SIZE];
    uintptr_t           band_max_mapped_baseaddr[NANO_MAG_SIZE];
	size_t			core_mapped_size[NANO_MAG_SIZE];

	unsigned			debug_flags;
	unsigned			our_signature;
	unsigned			phys_ncpus;
	unsigned			logical_ncpus;
	unsigned			hyper_shift;

	/* security cookie */
	uintptr_t			cookie;

	/*
	 * The nano zone constructed by create_nano_zone() would like to hand off tiny, small, and large
	 * allocations to the default scalable zone. Record the latter as the "helper" zone here.
	 */
	malloc_zone_t		*helper_zone;
} nanozone_t;

#define SZONE_PAGED_SIZE	((sizeof(nanozone_t) + vm_page_size - 1) & ~ (vm_page_size - 1))

/*********************             PROTOTYPES		***********************/
extern void malloc_error_break(void);

// msg prints after fmt, ...
static NOINLINE void	nanozone_error(nanozone_t *nanozone, int is_corruption, const char *msg, const void *ptr, const char *fmt, ...)
__printflike(5, 6);

static void nano_statistics(nanozone_t *nanozone, malloc_statistics_t *stats);

/*********************	   VERY LOW LEVEL UTILITIES    ************************/
// msg prints after fmt, ...

static NOINLINE void
nanozone_error(nanozone_t *nanozone, int is_corruption, const char *msg, const void *ptr, const char *fmt, ...)
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

	// Call abort() if this is a memory corruption error and the abort on
	// corruption flag is set, or if any error should abort.
	if ((is_corruption && (nanozone->debug_flags & SCALABLE_MALLOC_ABORT_ON_CORRUPTION)) ||
		(nanozone->debug_flags & SCALABLE_MALLOC_ABORT_ON_ERROR)) {
		CRSetCrashLogMessage(b ? _simple_string(b) : msg);
		abort();
	} else if (b) {
		_simple_sfree(b);
	}
}

static void
protect(void *address, size_t size, unsigned protection, unsigned debug_flags)
{
	kern_return_t	err;

	if (!(debug_flags & SCALABLE_MALLOC_DONT_PROTECT_PRELUDE)) {
		err = mprotect((void *)((uintptr_t)address - vm_page_size), vm_page_size, protection);
		if (err) {
			malloc_printf("*** can't protect(%p) region for prelude guard page at %p\n",
						  protection,(uintptr_t)address - (1 << vm_page_shift));
		}
	}
	if (!(debug_flags & SCALABLE_MALLOC_DONT_PROTECT_POSTLUDE)) {
		err = mprotect((void *)((uintptr_t)address + size), vm_page_size, protection);
		if (err) {
			malloc_printf("*** can't protect(%p) region for postlude guard page at %p\n",
						  protection, (uintptr_t)address + size);
		}
	}
}

static void *
allocate_based_pages(nanozone_t *nanozone, size_t size, unsigned char align, unsigned debug_flags, int vm_page_label, void *base_addr)
{
	boolean_t add_guard_pages = debug_flags & SCALABLE_MALLOC_ADD_GUARD_PAGES;
	mach_vm_address_t vm_addr;
	uintptr_t addr;
	mach_vm_size_t allocation_size = round_page(size);
	mach_vm_offset_t allocation_mask = ((mach_vm_offset_t)1 << align) - 1;
	int alloc_flags = VM_FLAGS_ANYWHERE | VM_MAKE_TAG(vm_page_label);
	kern_return_t kr;

	if (!allocation_size) allocation_size = vm_page_size;
	if (add_guard_pages) allocation_size += 2 * vm_page_size;
	if (allocation_size < size) // size_t arithmetic wrapped!
		return NULL;

	vm_addr = round_page((mach_vm_address_t)base_addr);
	if (!vm_addr) vm_addr = vm_page_size;
	kr = mach_vm_map(mach_task_self(), &vm_addr, allocation_size,
			allocation_mask, alloc_flags, MEMORY_OBJECT_NULL, 0, FALSE,
			VM_PROT_DEFAULT, VM_PROT_ALL, VM_INHERIT_DEFAULT);
	if (kr) {
		nanozone_error(nanozone, 0, "can't allocate pages", NULL,
				"*** mach_vm_map(size=%lu) failed (error code=%d)\n",
					   size, kr);
		return NULL;
	}
	addr = (uintptr_t)vm_addr;

	if (add_guard_pages) {
		addr += vm_page_size;
		protect((void *)addr, size, PROT_NONE, debug_flags);
	}
	return (void *)addr;
}

static void *
allocate_pages(nanozone_t *nanozone, size_t size, unsigned char align, unsigned debug_flags, int vm_page_label)
{
	return allocate_based_pages(nanozone, size, align, debug_flags, vm_page_label, 0);
}

static void
deallocate_pages(nanozone_t *nanozone, void *addr, size_t size, unsigned debug_flags)
{
	boolean_t add_guard_pages = debug_flags & SCALABLE_MALLOC_ADD_GUARD_PAGES;
	mach_vm_address_t vm_addr = (mach_vm_address_t)addr;
	mach_vm_size_t allocation_size = size;
	kern_return_t kr;

	if (add_guard_pages) {
		vm_addr -= vm_page_size;
		allocation_size += 2 * vm_page_size;
	}
	kr = mach_vm_deallocate(mach_task_self(), vm_addr, allocation_size);
	if (kr && nanozone)
		nanozone_error(nanozone, 0, "Can't deallocate_pages at", addr, NULL);
}

/*
 * We maintain separate free lists for each (quantized) size. The literature
 * calls this the "segregated policy".
 */

static boolean_t
segregated_band_grow(nanozone_t *nanozone, nano_meta_admin_t pMeta, unsigned int slot_bytes, unsigned int mag_index)
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
		pMeta->slot_bytes = slot_bytes;
		pMeta->slot_objects = SLOT_IN_BAND_SIZE / slot_bytes;
	} else {
		p = pMeta->slot_current_base_addr + BAND_SIZE; // Growing, so stride ahead by BAND_SIZE

		u.addr = (uint64_t)p;
		if (0 == u.fields.nano_band) // Did the band index wrap?
			return FALSE;

		assert(slot_bytes == pMeta->slot_bytes);
	}
	pMeta->slot_current_base_addr = p;

	mach_vm_address_t vm_addr = p & ~((uintptr_t)(BAND_SIZE - 1)); // Address of the (2MB) band covering this (128KB) slot
    
    if (nanozone->band_max_mapped_baseaddr[mag_index] < vm_addr) {
        // Obtain the next band to cover this slot
        kern_return_t kr = mach_vm_map(mach_task_self(), &vm_addr, BAND_SIZE,
                0, VM_MAKE_TAG(VM_MEMORY_MALLOC_NANO), MEMORY_OBJECT_NULL, 0, FALSE,
                VM_PROT_DEFAULT, VM_PROT_ALL, VM_INHERIT_DEFAULT);

        void *q = (uintptr_t)vm_addr;
        if (kr || q != (void *)(p & ~((uintptr_t)(BAND_SIZE - 1)))) // Must get exactly what we asked for
            return FALSE;
        
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
    hiwater = MAX( watermark, p - s + SLOT_IN_BAND_SIZE );
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

static INLINE void *
segregated_next_block(nanozone_t *nanozone, nano_meta_admin_t pMeta, unsigned int slot_bytes, unsigned int mag_index)
{
	while (1) {
		uintptr_t theLimit = pMeta->slot_limit_addr; // Capture the slot limit that bounds slot_bump_addr right now
		uintptr_t b = OSAtomicAdd64Barrier(slot_bytes, (volatile int64_t *)&(pMeta->slot_bump_addr));
		b -= slot_bytes; // Atomic op returned addr of *next* free block. Subtract to get addr for *this* allocation.

		if (b < theLimit) { // Did we stay within the bound of the present slot allocation?
			return (void *)b; // Yep, so the slot_bump_addr this thread incremented is good to go
		} else {
			if (pMeta->slot_exhausted) { // exhausted all the bands availble for this slot?
				return 0; // We're toast
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
					_malloc_lock_unlock(&nanozone->band_resupply_lock[mag_index]);
					return 0;
				}
			}
		}
	}
}

static INLINE unsigned int
segregated_size_to_fit(nanozone_t *nanozone, size_t size, unsigned int *pKey)
{
	unsigned int k, slot_bytes;

	if (0 == size)
		size = NANO_REGIME_QUANTA_SIZE; // Historical behavior

	k = (size + NANO_REGIME_QUANTA_SIZE - 1) >> SHIFT_NANO_QUANTUM; // round up and shift for number of quanta
	slot_bytes = k << SHIFT_NANO_QUANTUM; // multiply by power of two quanta size
	*pKey = k - 1; // Zero-based!

	return slot_bytes;
}

static INLINE index_t
offset_to_index(nanozone_t *nanozone, nano_meta_admin_t pMeta, uintptr_t offset)
{
	unsigned int slot_bytes = pMeta->slot_bytes;
	unsigned int slot_objects = pMeta->slot_objects; // SLOT_IN_BAND_SIZE / slot_bytes;
    unsigned int rem;
    unsigned long quo = divrem(offset, BAND_SIZE, &rem);

	assert(0 == rem%slot_bytes);
    return (quo * slot_objects) + (rem / slot_bytes);
}

static INLINE uintptr_t
index_to_offset(nanozone_t *nanozone, nano_meta_admin_t pMeta, index_t i)
{
	unsigned int slot_bytes = pMeta->slot_bytes;
	unsigned int slot_objects = pMeta->slot_objects; // SLOT_IN_BAND_SIZE / slot_bytes;
    unsigned int rem;
    unsigned long quo = divrem(i, slot_objects, &rem);
    
	return (quo * BAND_SIZE) + (rem * slot_bytes);
}

static kern_return_t
segregated_in_use_enumerator(task_t task, void *context, unsigned type_mask, nanozone_t *nanozone,
							 memory_reader_t reader, vm_range_recorder_t recorder)
{
	unsigned int	mag_index, slot_key;
	vm_range_t		ptr_range;
	vm_range_t		buffer[MAX_RECORDER_BUFFER];
	kern_return_t	err;
	unsigned		count = 0;

	for (mag_index = 0; mag_index < nanozone->phys_ncpus; mag_index++) {
		uintptr_t	clone_magazine; // magazine base for ourselves
		nano_blk_addr_t	p; // slot base for remote
		uintptr_t	clone_slot_base; // slot base for ourselves (tracks with "p")

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

		for (slot_key = 0; slot_key < SLOT_KEY_LIMIT;
			 p.addr += SLOT_IN_BAND_SIZE,	    // Advance to next slot base for remote
			 clone_slot_base += SLOT_IN_BAND_SIZE,   // Advance to next slot base for ourselves
			 slot_key++) {
			nano_meta_admin_t pMeta = &(nanozone->meta_data[mag_index][slot_key]);
			size_t slot_objects_mapped = pMeta->slot_objects_mapped; // capture this volatile count

			if (0 == slot_objects_mapped) // Nothing allocated in this magazine for this slot?
				continue;

			if (type_mask & MALLOC_ADMIN_REGION_RANGE_TYPE) {
				/* do NOTHING as there is no distinct admin region */
			}

			if (type_mask & (MALLOC_PTR_REGION_RANGE_TYPE | MALLOC_ADMIN_REGION_RANGE_TYPE)) {
				nano_blk_addr_t q = p;
				uintptr_t skip_adj = index_to_offset(nanozone, pMeta, pMeta->slot_objects_skipped);

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
				uintptr_t skip_adj = index_to_offset(nanozone, pMeta, pMeta->slot_objects_skipped);

				while (q.addr < pMeta->slot_limit_addr) {
					// read slot in each remote band. Lands in some random location.
					size_t len = MIN(pMeta->slot_bump_addr - q.addr, SLOT_IN_BAND_SIZE);
					err = reader(task, (vm_address_t)(q.addr + skip_adj), len - skip_adj, (void **)&slot_band);
					if (err)
						return err;

					// Place the data just read in the correct position relative to the local magazine.
					memcpy((void *)(clone_slot_band_base + skip_adj), (void *)slot_band, len - skip_adj);

					// Simultaneously advance pointers in remote and ourselves to the next band.
					q.addr += BAND_SIZE;
					clone_slot_band_base += BAND_SIZE;
					skip_adj = 0;
				}

				// Walk the slot free list and populate a bitarray_t
				int log_size = 64 - __builtin_clzl(slot_objects_mapped);
				bitarray_t slot_bitarray = bitarray_create(log_size);

				if (!slot_bitarray)
					return errno;

				chained_block_t t;
				unsigned stoploss = slot_objects_mapped;
				while ((t = OSAtomicDequeue( &(pMeta->slot_LIFO), offsetof(struct chained_block_s,next) + (clone_slot_base - p.addr)))) {
					if (0 == stoploss) {
						malloc_printf("Free list walk in segregated_in_use_enumerator exceeded object count.");
						break;
					}
					stoploss--;

					uintptr_t	offset = ((uintptr_t)t - p.addr); // offset from beginning of slot, task-independent
					index_t 	block_index = offset_to_index(nanozone, pMeta, offset);

					if (block_index < slot_objects_mapped)
						bitarray_set(slot_bitarray, log_size, block_index);
				}
				// N.B. pMeta->slot_LIFO in *this* task is now drained (remote free list has *not* been disturbed)

				// Copy the bitarray_t denoting madvise()'d pages (if any) into *this* task's address space
				bitarray_t madv_page_bitarray;
				int log_page_count;
				
				if (pMeta->slot_madvised_pages) {
					log_page_count = pMeta->slot_madvised_log_page_count;
					err = reader(task, (vm_address_t)(pMeta->slot_madvised_pages), bitarray_size(log_page_count), (void **)&madv_page_bitarray);
					if (err)
						return err;
				} else {
					madv_page_bitarray = NULL;
					log_page_count = 0;
				}
				
				// Enumerate all the block indices issued to date, and report those not on the free list
				index_t i;
				for (i = pMeta->slot_objects_skipped; i < slot_objects_mapped; ++i)  {
					uintptr_t block_offset = index_to_offset(nanozone, pMeta, i);
					if (p.addr + block_offset >= pMeta->slot_bump_addr)
						break;

					// blocks falling on madvise()'d pages are free! So not enumerated.
					if (madv_page_bitarray) {
						nano_blk_addr_t q;
						index_t pgnum, pgnum_end;
						
						q.addr = p.addr + block_offset;
						pgnum = ((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >> vm_page_shift;
						q.addr += pMeta->slot_bytes - 1;
						pgnum_end = ((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >> vm_page_shift;
						
						if (pgnum < (1 << log_page_count)) {// bounds check for bitarray_get()'s that follow
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

static INLINE size_t
__nano_vet_and_size(nanozone_t *nanozone, const void *ptr)
{
	// Extracts the size of the block in bytes. Checks for a plausible ptr.
	nano_blk_addr_t	p; // the compiler holds this in a register
	nano_meta_admin_t	pMeta;

	p.addr = (uint64_t)ptr; // Begin the dissection of ptr

	if (nanozone->phys_ncpus <= p.fields.nano_mag_index)
		return 0;

	if (p.fields.nano_offset & NANO_QUANTA_MASK) // stray low-order bits?
		return 0;

	pMeta = &(nanozone->meta_data[p.fields.nano_mag_index][p.fields.nano_slot]);
	if ((void *)(pMeta->slot_bump_addr) <= ptr)
		return 0; // Beyond what's ever been allocated!

	if ((p.fields.nano_offset % pMeta->slot_bytes) != 0)
		return 0; // Not an exact multiple of the block size for this slot

	return pMeta->slot_bytes;
}

static INLINE size_t
_nano_vet_and_size_of_live(nanozone_t *nanozone, const void *ptr)
{
    size_t size = __nano_vet_and_size(nanozone, ptr);
    if (size && ((((chained_block_t)ptr)->double_free_guard ^ nanozone->cookie) != 0xBADDC0DEDEADBEADULL))
        return size; // Common case: not on a free list, hence live. Return its size.
    else
        // ptr is either on a free list (its got the correct canary) in which case return zero, OR
        // the caller has stored the canary value in the double_free_guard slot entirely by coincidence
        // and the block is a live allocation. The latter is very unlikely (1 in 2^64) so just return 0.
        return 0;
}

static INLINE size_t
_nano_vet_and_size_of_free(nanozone_t *nanozone, const void *ptr)
{
    size_t size = __nano_vet_and_size(nanozone, ptr);
    if (size && ((((chained_block_t)ptr)->double_free_guard ^ nanozone->cookie) == 0xBADDC0DEDEADBEADULL))
        return size;
    else
        return 0;
}

static void *
_nano_malloc_check_clear(nanozone_t *nanozone, size_t size, boolean_t cleared_requested)
{
	void		*ptr;
	unsigned int	slot_key;
	unsigned int	slot_bytes = segregated_size_to_fit(nanozone, size, &slot_key); // Note slot_key is set here
	unsigned int	mag_index = NANO_MAG_INDEX(nanozone);

	nano_meta_admin_t	pMeta = &(nanozone->meta_data[mag_index][slot_key]);

	ptr = OSAtomicDequeue( &(pMeta->slot_LIFO), offsetof(struct chained_block_s,next));
	if (ptr) {
#if NANO_FREE_DEQUEUE_DILIGENCE
		size_t		gotSize;
		nano_blk_addr_t	p; // the compiler holds this in a register

		p.addr = (uint64_t)ptr; // Begin the dissection of ptr
		if (nanozone->our_signature != p.fields.nano_signature) {
			nanozone_error(nanozone, 1,
						   "Invalid signature for pointer dequeued from free list", ptr, NULL);
		}

		if (mag_index != p.fields.nano_mag_index) {
			nanozone_error(nanozone, 1,
						   "Mismatched magazine for pointer dequeued from free list", ptr, NULL);
		}

		gotSize = _nano_vet_and_size_of_free(nanozone, ptr);
		if (0 == gotSize) {
			nanozone_error(nanozone, 1,
						   "Invalid pointer dequeued from free list", ptr, NULL);
		}
		if (gotSize != slot_bytes) {
			nanozone_error(nanozone, 1,
						   "Mismatched size for pointer dequeued from free list", ptr, NULL);
		}

		if ((((chained_block_t)ptr)->double_free_guard ^ nanozone->cookie) != 0xBADDC0DEDEADBEADULL) {
			nanozone_error(nanozone, 1,
						   "Heap corruption detected, free list canary is damaged", ptr, NULL);
		}
#if defined(DEBUG)
		void *next = (void *) (((chained_block_t)ptr)->next);
		if (next) {
			p.addr = (uint64_t)next; // Begin the dissection of next
			if (nanozone->our_signature != p.fields.nano_signature) {
				nanozone_error(nanozone, 1,
							   "Invalid next signature for pointer dequeued from free list (showing ptr, next)",
							   ptr, ", %p", next);
			}

			if (mag_index != p.fields.nano_mag_index) {
				nanozone_error(nanozone, 1,
							   "Mismatched next magazine for pointer dequeued from free list (showing ptr, next)",
							   ptr, ", %p", next);
			}

			gotSize = _nano_vet_and_size_of_free(nanozone, next);
			if (0 == gotSize) {
				nanozone_error(nanozone, 1,
							   "Invalid next for pointer dequeued from free list (showing ptr, next)",
							   ptr, ", %p", next);
			}
			if (gotSize != slot_bytes) {
				nanozone_error(nanozone, 1,
							   "Mismatched next size for pointer dequeued from free list (showing ptr, next)",
							   ptr, ", %p", next);
			}
		}
#endif /* DEBUG */
#endif /* NANO_FREE_DEQUEUE_DILIGENCE */

		((chained_block_t)ptr)->double_free_guard = 0;
		((chained_block_t)ptr)->next = NULL; // clear out next pointer to protect free list
	} else {
		ptr = segregated_next_block(nanozone, pMeta, slot_bytes, mag_index);
	}

	if (cleared_requested && ptr)
		memset(ptr, 0, slot_bytes); // TODO: Needs a memory barrier after memset to ensure zeroes land first?

	return ptr;
}

static void *
_nano_malloc_check_scribble(nanozone_t *nanozone, size_t size)
{
	void *ptr = _nano_malloc_check_clear(nanozone, size, 0);

	/*
	 * Scribble on allocated memory when requested.
	 */
	if ((nanozone->debug_flags & SCALABLE_MALLOC_DO_SCRIBBLE) && ptr && size)
		memset(ptr, SCRIBBLE_BYTE, _nano_vet_and_size_of_live(nanozone, ptr));

	return ptr;
}

static INLINE boolean_t
_nano_block_inuse_p(nanozone_t *nanozone, const void *ptr)
{
	nano_blk_addr_t p; // happily, the compiler holds this in a register
	nano_meta_admin_t pMeta;
	chained_block_t head = NULL, tail = NULL, t;
	boolean_t inuse = TRUE;

	p.addr = (uint64_t)ptr; // place ptr on the dissecting table

	pMeta = &(nanozone->meta_data[p.fields.nano_mag_index][p.fields.nano_slot]);

	if ((void *)(pMeta->slot_bump_addr) <= ptr)
		return FALSE; // Beyond what's ever been allocated, so trivially not in use.

	// pop elements off the free list all the while looking for ptr.
	unsigned stoploss = pMeta->slot_objects_mapped;
	while ((t = OSAtomicDequeue( &(pMeta->slot_LIFO), offsetof(struct chained_block_s,next)))) {
		if (0 == stoploss) {
			nanozone_error(nanozone, 1, "Free list walk in _nano_block_inuse_p exceeded object count.",
						   (void *)&(pMeta->slot_LIFO), NULL);
		}
		stoploss--;

		if (NULL == head)
			head = t;
		else
			tail->next = t;
		tail = t;

		if (ptr == t) {
			inuse = FALSE;
			break;
		}
	}
	if (tail)
		tail->next = NULL;

	// push the free list extracted above back onto the LIFO, all at once
	if (head)
		OSAtomicEnqueue( &(pMeta->slot_LIFO), head, (uintptr_t)tail - (uintptr_t)head + offsetof(struct chained_block_s,next));

	return inuse;
}

static INLINE size_t
_nano_size(nanozone_t *nanozone, const void *ptr)
{
	return  _nano_vet_and_size_of_live(nanozone, ptr);
}

static INLINE size_t
_nano_good_size(nanozone_t *nanozone, size_t size)
{
	return (size <= NANO_REGIME_QUANTA_SIZE) ?
	NANO_REGIME_QUANTA_SIZE :
	(((size + NANO_REGIME_QUANTA_SIZE - 1) >> SHIFT_NANO_QUANTUM) << SHIFT_NANO_QUANTUM);
}

static INLINE void _nano_free_trusted_size_check_scribble(nanozone_t *nanozone, void *ptr, size_t trusted_size, boolean_t do_scribble) ALWAYSINLINE;

static INLINE void
_nano_free_trusted_size_check_scribble(nanozone_t *nanozone, void *ptr, size_t trusted_size, boolean_t do_scribble)
{
	if (trusted_size) {
		nano_blk_addr_t p; // happily, the compiler holds this in a register
		nano_meta_admin_t pMeta;

		if (do_scribble)
			(void)memset(ptr, SCRABBLE_BYTE, trusted_size);
		((chained_block_t)ptr)->double_free_guard = (0xBADDC0DEDEADBEADULL ^ nanozone->cookie);

		p.addr = (uint64_t)ptr; // place ptr on the dissecting table
		pMeta = &(nanozone->meta_data[p.fields.nano_mag_index][p.fields.nano_slot]);
		OSAtomicEnqueue( &(pMeta->slot_LIFO), ptr, offsetof(struct chained_block_s,next));
	} else {
		nanozone_error(nanozone, 1, "Freeing unallocated pointer", ptr, NULL);
	}
}

static INLINE void _nano_free_check_scribble(nanozone_t *nanozone, void *ptr, boolean_t do_scribble) ALWAYSINLINE;

static INLINE void
_nano_free_check_scribble(nanozone_t *nanozone, void *ptr, boolean_t do_scribble)
{
    _nano_free_trusted_size_check_scribble(nanozone, ptr, _nano_vet_and_size_of_live(nanozone, ptr), do_scribble);
}

static INLINE void *
_nano_realloc(nanozone_t *nanozone, void *ptr, size_t new_size)
{
	size_t	old_size, new_good_size, valid_size;
	void	*new_ptr;

	if (FALSE && NULL == ptr) { // ptr has our_signature so can't be NULL, but if it were Posix sez ...
		// If ptr is a null pointer, realloc() shall be equivalent to malloc() for the specified size.
		return _nano_malloc_check_scribble(nanozone, new_size);
	} else if (0 == new_size) {
		// If size is 0 and ptr is not a null pointer, the object pointed to is freed.
		_nano_free_check_scribble(nanozone, ptr, (nanozone->debug_flags & SCALABLE_MALLOC_DO_SCRIBBLE));
		// If size is 0, either a null pointer or a unique pointer that can be successfully passed
		// to free() shall be returned.
		return _nano_malloc_check_scribble(nanozone, 1);
	}

	old_size = _nano_vet_and_size_of_live(nanozone, ptr);
	if (!old_size) {
		nanozone_error(nanozone, 1, "pointer being reallocated was not allocated", ptr, NULL);
		return NULL;
	}

	new_good_size = _nano_good_size(nanozone, new_size);
	if (new_good_size > old_size) {
		/* Must grow. FALL THROUGH to alloc/copy/free. */
	} else if (new_good_size <= (old_size >> 1)) {
		/* Serious shrinkage (more than half). FALL THROUGH to alloc/copy/free. */
	} else {
		/* Let's hang on to what we got. */
		if (nanozone->debug_flags & SCALABLE_MALLOC_DO_SCRIBBLE)
			memset(ptr + new_size, SCRIBBLE_BYTE, old_size - new_size);
		return ptr;
	}

	/*
	 * Allocate a new buffer and copy.
	 */
	new_ptr = _nano_malloc_check_scribble(nanozone, new_good_size);
	if (new_ptr == NULL)
		return NULL;

	valid_size = MIN(old_size, new_good_size);
	memcpy(new_ptr, ptr, valid_size);
	_nano_free_check_scribble(nanozone, ptr, (nanozone->debug_flags & SCALABLE_MALLOC_DO_SCRIBBLE));

	return new_ptr;
}

static INLINE void
_nano_destroy(nanozone_t *nanozone)
{
	/* Now destroy the separate nanozone region */
	deallocate_pages(nanozone, (void *)nanozone, SZONE_PAGED_SIZE, 0);
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
			if (size)
				memset(ptr, SCRIBBLE_BYTE, _nano_vet_and_size_of_live(nanozone, ptr));

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
	size_t total_bytes = num_items * size;

	// Check for overflow of integer multiplication
	if (num_items > 1) {
		/* size_t is uint64_t */
		if ((num_items | size) & 0xffffffff00000000ul) {
			// num_items or size equals or exceeds sqrt(2^64) == 2^32, appeal to wider arithmetic
			__uint128_t product = ((__uint128_t)num_items) * ((__uint128_t)size);
			if ((uint64_t)(product >> 64)) // compiles to test on upper register of register pair
				return NULL;
		}
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

static INLINE void __nano_free_definite_size(nanozone_t *nanozone, void *ptr, size_t size, boolean_t do_scribble) ALWAYSINLINE;

static INLINE void
__nano_free_definite_size(nanozone_t *nanozone, void *ptr, size_t size, boolean_t do_scribble)
{
	nano_blk_addr_t p; // happily, the compiler holds this in a register

	p.addr = (uint64_t)ptr; // place ptr on the dissecting table
	if (nanozone->our_signature == p.fields.nano_signature) {
	if (size == ((p.fields.nano_slot + 1) << SHIFT_NANO_QUANTUM)) { // "Trust but verify."
	    _nano_free_trusted_size_check_scribble(nanozone, ptr, size, do_scribble);
		return;
	} else {
            nanozone_error(nanozone, 1, "Freeing pointer whose size was misdeclared", ptr, NULL);
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

static INLINE void __nano_free(nanozone_t *nanozone, void *ptr, boolean_t do_scribble) ALWAYSINLINE;

static INLINE void
__nano_free(nanozone_t *nanozone, void *ptr, boolean_t do_scribble)
{
	nano_blk_addr_t p; // happily, the compiler holds this in a register

	if (!ptr)
		return; // Protect against malloc_zone_free() passing NULL.

	p.addr = (uint64_t)ptr; // place ptr on the dissecting table
	if (nanozone->our_signature == p.fields.nano_signature) {
		_nano_free_check_scribble(nanozone, ptr, do_scribble);
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
	nano_blk_addr_t p; // happily, the compiler holds this in a register

	if (!ptr)
		return; // Protect against malloc_zone_free() passing NULL.

	p.addr = (uint64_t)ptr; // place ptr on the dissecting table
	if (nanozone->our_signature == p.fields.nano_signature) {
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

	if (nanozone->our_signature == p.fields.nano_signature) { // Our signature?
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
	nano_blk_addr_t p; // happily, the compiler holds this in a register

	p.addr = (uint64_t)ptr; // place ptr on the dissecting table

	if (NULL == ptr) { // could occur through malloc_zone_realloc() path
        // If ptr is a null pointer, realloc() shall be equivalent to malloc() for the specified size.
		return nano_malloc(nanozone, new_size);
	} else if (nanozone->our_signature == p.fields.nano_signature) { // Our signature?
		if (new_size <= NANO_MAX_SIZE) { // nano to nano?
			void *q = _nano_realloc(nanozone, ptr, new_size);
			if (q) {
				return q;
			} else { // nano exhausted
				/* FALLTHROUGH to helper zone copying case */
			}
		}

		// nano to larger-than-nano (or FALLTHROUGH from just above)
		size_t old_size = _nano_vet_and_size_of_live(nanozone, ptr);

		if (!old_size) {
			nanozone_error(nanozone, 1, "pointer being reallocated was not allocated", ptr, NULL);
			return NULL;
		} else {
			malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
			void *new_ptr = zone->malloc(zone, new_size);

			if (new_ptr) {
				size_t valid_size = MIN(old_size, new_size);
				memcpy(new_ptr, ptr, valid_size);
				_nano_free_check_scribble(nanozone, ptr, (nanozone->debug_flags & SCALABLE_MALLOC_DO_SCRIBBLE));
				return new_ptr;
			} else {
				/* Original ptr is left intact */
				return NULL;
			}
			/* NOTREACHED */
		}
	} else {
		// other-than-nano (not necessarily larger! possibly NULL!) to whatever
		malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);

		return zone->realloc(zone, ptr, new_size);
	}
	/* NOTREACHED */
}

static void *
nano_forked_realloc(nanozone_t *nanozone, void *ptr, size_t new_size)
{
	nano_blk_addr_t p; // happily, the compiler holds this in a register

	p.addr = (uint64_t)ptr; // place ptr on the dissecting table

	if (NULL == ptr) { // could occur through malloc_zone_realloc() path
        // If ptr is a null pointer, realloc() shall be equivalent to malloc() for the specified size.
		return nano_forked_malloc(nanozone, new_size);
	} else if (nanozone->our_signature == p.fields.nano_signature) { // Our signature?
        if (0 == new_size) {
            // If size is 0 and ptr is not a null pointer, the object pointed to is freed.
            // However as nanozone metadata could be fouled by fork, we'll intentionally leak it.
            
            // If size is 0, either a null pointer or a unique pointer that can be successfully passed
            // to free() shall be returned.
            return nano_forked_malloc(nanozone, 1);
        }
        
		size_t old_size = _nano_vet_and_size_of_live(nanozone, ptr);

		if (!old_size) {
			nanozone_error(nanozone, 1, "pointer being reallocated was not allocated", ptr, NULL);
			return NULL;
		} else {
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
	} else {
		// other-than-nano (not necessarily larger! possibly NULL!) to whatever
		malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);

		return zone->realloc(zone, ptr, new_size);
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
	unsigned	found = 0;

	if (size <= NANO_MAX_SIZE) {
		while (found < count) {
			void *ptr = _nano_malloc_check_clear(nanozone, size, 0);
			if (!ptr)
				break;

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
	void	*ptr;

	// frees all the pointers in to_be_freed
	// note that to_be_freed may be overwritten during the process
	if (!count)
		return;

	while (count--) {
		ptr = to_be_freed[count];
		if (ptr)
			nano_free(nanozone, ptr);
	}
}

static void
nano_forked_batch_free(nanozone_t *nanozone, void **to_be_freed, unsigned count)
{
	void	*ptr;

	// frees all the pointers in to_be_freed
	// note that to_be_freed may be overwritten during the process
	if (!count)
		return;

	while (count--) {
		ptr = to_be_freed[count];
		if (ptr)
			nano_forked_free(nanozone, ptr);
	}
}

static void *
nano_memalign(nanozone_t *nanozone, size_t alignment, size_t size)
{
	malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
	return zone->memalign(zone, alignment, size);
}

static size_t
nano_try_madvise(nanozone_t *nanozone, size_t goal)
{
	unsigned int	mag_index, slot_key;
	size_t		bytes_toward_goal = 0;

	for (mag_index = 0; mag_index < nanozone->phys_ncpus; mag_index++) {
		nano_blk_addr_t	p;

		// Establish p as base address for band 0, slot 0, offset 0
		p.fields.nano_signature = NANOZONE_SIGNATURE;
		p.fields.nano_mag_index = mag_index;
		p.fields.nano_band = 0;
		p.fields.nano_slot = 0;
		p.fields.nano_offset = 0;

		for (slot_key = 0; slot_key < SLOT_KEY_LIMIT;
			 p.addr += SLOT_IN_BAND_SIZE,	    // Advance to next slot base
			 slot_key++) {

			// _malloc_printf(ASL_LEVEL_WARNING,"nano_try_madvise examining slot base %p\n", p.addr);
			nano_meta_admin_t pMeta = &(nanozone->meta_data[mag_index][slot_key]);
			uintptr_t slot_bump_addr = pMeta->slot_bump_addr; // capture this volatile pointer
			size_t slot_objects_mapped = pMeta->slot_objects_mapped; // capture this volatile count

			if (0 == slot_objects_mapped) { // Nothing allocated in this magazine for this slot?
				continue;
			} else {
				// Walk the slot free list and populate a bitarray_t
				int log_size = 64 - __builtin_clzl(slot_objects_mapped);
				bitarray_t slot_bitarray = bitarray_create(log_size);

				unsigned int slot_bytes = pMeta->slot_bytes;
				int log_page_count = 64 - __builtin_clzl((slot_objects_mapped * slot_bytes) / vm_page_size);
				log_page_count = 1 + MAX(0, log_page_count);
				bitarray_t page_bitarray = bitarray_create(log_page_count);

				// _malloc_printf(ASL_LEVEL_WARNING,"slot_bitarray: %db page_bitarray: %db\n", bitarray_size(log_size), bitarray_size(log_page_count));
				if (!slot_bitarray) {
					malloc_printf("bitarray_create(%d) in nano_try_madvise returned errno=%d.", log_size, errno);
					return bytes_toward_goal;
				}

				if (!page_bitarray) {
					malloc_printf("bitarray_create(%d) in nano_try_madvise returned errno=%d.", log_page_count, errno);
					free(slot_bitarray);
					return bytes_toward_goal;
				}

				chained_block_t head = NULL, tail = NULL, t;
				unsigned stoploss = slot_objects_mapped;
				while ((t = OSAtomicDequeue( &(pMeta->slot_LIFO), offsetof(struct chained_block_s,next)))) {
					if (0 == stoploss) {
						malloc_printf("Free list walk in nano_try_madvise exceeded object count.");
						break;
					}
					stoploss--;

					uintptr_t	offset = ((uintptr_t)t - p.addr); // offset from beginning of slot
					index_t 	block_index = offset_to_index(nanozone, pMeta, offset);

					// build a simple linked list of the free blocks we're able to obtain
					if (NULL == head)
						head = t;
					else
						tail->next = t;
					tail = t;

					// take note in a bitarray_t of each free block we're able to obtain (allows fast lookup below)
					if (block_index < slot_objects_mapped)
						bitarray_set(slot_bitarray, log_size, block_index);
				}
				if (tail)
					tail->next = NULL;

				if (NULL == head) {
					free(slot_bitarray);
					free(page_bitarray);
					continue;
				}

				index_t i;
				nano_blk_addr_t q;
				size_t pgnum;
				for (i = pMeta->slot_objects_skipped; i < slot_objects_mapped; ++i)  {
					uintptr_t block_offset = index_to_offset(nanozone, pMeta, i);
					if (p.addr + block_offset >= slot_bump_addr)
						break;

					if (!bitarray_get(slot_bitarray, log_size, i)) { // is block i allocated or already on an madvise'd page?

						// Mark the page(s) it resides on as live
						q.addr = p.addr + block_offset;
						pgnum = ((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >> vm_page_shift;
						bitarray_set(page_bitarray, log_page_count, pgnum);

						q.addr += slot_bytes - 1;
						pgnum = ((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >> vm_page_shift;
						bitarray_set(page_bitarray, log_page_count, pgnum);
					}
				}

				free(slot_bitarray);

				q.addr = p.addr + index_to_offset(nanozone, pMeta, pMeta->slot_objects_skipped);
				index_t pgstart = ((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >> vm_page_shift;

				q.addr = slot_bump_addr - slot_bytes;
				pgnum = ((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >> vm_page_shift;

				// _malloc_printf(ASL_LEVEL_WARNING,"Examining %d pages. Slot base %p.\n", pgnum - pgstart + 1, p.addr);

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
						!bitarray_get(page_bitarray, log_page_count, i)) // no live allocations?
					{
						num_advised++;
						bitarray_set(will_madvise_pages, log_page_count, i);
					}
				}
				free(page_bitarray);

				if (num_advised) {
					chained_block_t new_head = NULL, new_tail = NULL;
					// _malloc_printf(ASL_LEVEL_WARNING,"Constructing residual free list starting at %p num_advised %d\n", head, num_advised);
					t = head;
					while (t) {
						q.addr = (uintptr_t)t;
						index_t pgnum_start = ((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >> vm_page_shift;
						q.addr += slot_bytes - 1;
						index_t pgnum_end = ((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >> vm_page_shift;

						// bounds check for the bitarray_get()'s that follow. If the pgnum is beyond the
						// capacity of the will_madvise_pages just restore the block to the free list.
						if (pgnum_start >= (1 << log_page_count)) {
							if (NULL == new_head)
								new_head = t;
							else
								new_tail->next = t;
							new_tail = t;
						}
						// If the block nowhere lies on an madvise()'d page restore it to the slot free list.
						else if (!bitarray_get(will_madvise_pages, log_page_count, pgnum_start) &&
								 !bitarray_get(will_madvise_pages, log_page_count, pgnum_end)) {
							if (NULL == new_head)
								new_head = t;
							else
								new_tail->next = t;
							new_tail = t;
						}

						t = t->next;
					}
					if (new_tail)
						new_tail->next = NULL;

					// push the free list extracted above back onto the LIFO, all at once
					if (new_head)
						OSAtomicEnqueue( &(pMeta->slot_LIFO), new_head,
										(uintptr_t)new_tail - (uintptr_t)new_head + offsetof(struct chained_block_s,next));
				} else {
					// _malloc_printf(ASL_LEVEL_WARNING,"Reinstating free list since no pages were madvised (%d).\n", num_advised);
					if (head)
						OSAtomicEnqueue( &(pMeta->slot_LIFO), head,
										(uintptr_t)tail - (uintptr_t)head + offsetof(struct chained_block_s,next));
				}

				for (i = pgstart; i < pgnum; ++i) {
					if ((i < (1 << log_page_count)) && bitarray_get(will_madvise_pages, log_page_count, i)) {
						q = p;
						q.fields.nano_band = (i << vm_page_shift) >> NANO_OFFSET_BITS;
						q.fields.nano_offset = (i << vm_page_shift) & ((1 << NANO_OFFSET_BITS) - 1);
						// _malloc_printf(ASL_LEVEL_WARNING,"Entire page non-live: %d. Slot base %p, madvising %p\n", i, p.addr, q.addr);

						if (nanozone->debug_flags & SCALABLE_MALLOC_DO_SCRIBBLE)
							memset((void *)q.addr, SCRUBBLE_BYTE, vm_page_size);
#if TARGET_OS_EMBEDDED
						if (-1 == madvise((void *)q.addr, vm_page_size, MADV_FREE))
#else
						if (-1 == madvise((void *)q.addr, vm_page_size, MADV_FREE_REUSABLE))
#endif
						{
						    /* -1 return: VM map entry change makes this unfit for reuse. Something evil lurks. */
#if DEBUG_MADVISE
						    nanozone_error(nanozone, 0, "madvise(..., MADV_FREE_REUSABLE) failed",
								    (void *)cwq.addrpgLo, "length=%d\n", vm_page_size);
#endif
						} else {
						    bytes_toward_goal += vm_page_size;
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

				if (goal && bytes_toward_goal >= goal)
					return bytes_toward_goal;
			}
		}
	}
	return bytes_toward_goal;
}

static size_t
nano_pressure_relief(nanozone_t *nanozone, size_t goal)
{
	return nano_try_madvise(nanozone, goal);
}

/****************           introspection methods         *********************/

static kern_return_t
nanozone_default_reader(task_t task, vm_address_t address, vm_size_t size, void **ptr)
{
	*ptr = (void *)address;
	return 0;
}

static kern_return_t
nano_ptr_in_use_enumerator(task_t task, void *context, unsigned type_mask, vm_address_t zone_address,
						   memory_reader_t reader, vm_range_recorder_t recorder)
{
	nanozone_t		*nanozone;
	kern_return_t	err;

	if (!reader) reader = nanozone_default_reader;

	err = reader(task, zone_address, sizeof(nanozone_t), (void **)&nanozone);
	if (err) return err;

	err = segregated_in_use_enumerator(task, context, type_mask, nanozone, reader, recorder);
	return err;
}

static size_t
nano_good_size(nanozone_t *nanozone, size_t size)
{
	if (size <= NANO_MAX_SIZE)
		return _nano_good_size(nanozone, size);
	else {
		malloc_zone_t *zone = (malloc_zone_t *)(nanozone->helper_zone);
		return zone->introspect->good_size(zone, size);
	}
}

// TODO sanity checks
unsigned nanozone_check_counter = 0;
unsigned nanozone_check_start = 0;
unsigned nanozone_check_modulo = 1;

static boolean_t
nano_check_all(nanozone_t *nanozone, const char *function)
{
	return 1;
}

static boolean_t
nanozone_check(nanozone_t *nanozone)
{
	if ((++nanozone_check_counter % 10000) == 0)
		_malloc_printf(ASL_LEVEL_NOTICE, "at nanozone_check counter=%d\n", nanozone_check_counter);

	if (nanozone_check_counter < nanozone_check_start)
		return 1;

	if (nanozone_check_counter % nanozone_check_modulo)
		return 1;

	return nano_check_all(nanozone, "");
}

static unsigned
count_free(nanozone_t *nanozone, nano_meta_admin_t pMeta)
{
	chained_block_t head = NULL, tail = NULL, t;
	unsigned count = 0;

	unsigned stoploss = pMeta->slot_objects_mapped;
	while ((t = OSAtomicDequeue( &(pMeta->slot_LIFO), offsetof(struct chained_block_s,next)))) {
		if (0 == stoploss) {
			nanozone_error(nanozone, 1, "Free list walk in count_free exceeded object count.",
						   (void *)&(pMeta->slot_LIFO), NULL);
		}
		stoploss--;

		if (NULL == head)
			head = t;
		else
			tail->next = t;
		tail = t;

		count++;
	}
	if (tail)
		tail->next = NULL;

	// push the free list extracted above back onto the LIFO, all at once
	if (head)
		OSAtomicEnqueue( &(pMeta->slot_LIFO), head, (uintptr_t)tail - (uintptr_t)head + offsetof(struct chained_block_s,next));

	return count;
}

static void
nano_print(nanozone_t *nanozone, boolean_t verbose)
{
	unsigned int	mag_index, slot_key;
	malloc_statistics_t stats;

	nano_statistics(nanozone, &stats);
	_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX,
				   "Nanozone %p: inUse=%d(%dKB) touched=%dKB allocated=%dMB\n",
				   nanozone, stats.blocks_in_use, stats.size_in_use>>10, stats.max_size_in_use>>10, stats.size_allocated>>20);

	for (mag_index = 0; mag_index < nanozone->phys_ncpus; mag_index++) {
		nano_blk_addr_t	p;

		// Establish p as base address for band 0, slot 0, offset 0
		p.fields.nano_signature = NANOZONE_SIGNATURE;
		p.fields.nano_mag_index = mag_index;
		p.fields.nano_band = 0;
		p.fields.nano_slot = 0;
		p.fields.nano_offset = 0;

		for (slot_key = 0; slot_key < SLOT_KEY_LIMIT;
			 p.addr += SLOT_IN_BAND_SIZE,	    // Advance to next slot base
			 slot_key++) {

			nano_meta_admin_t pMeta = &(nanozone->meta_data[mag_index][slot_key]);
			uintptr_t slot_bump_addr = pMeta->slot_bump_addr; // capture this volatile pointer
			size_t slot_objects_mapped = pMeta->slot_objects_mapped; // capture this volatile count

			if (0 == slot_objects_mapped) { // Nothing allocated in this magazine for this slot?
				_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX,
							   "Magazine %2d(%3d) Unrealized\n",mag_index, (slot_key + 1) << SHIFT_NANO_QUANTUM);
				continue;
			}

			uintptr_t offset = (0 == slot_bump_addr ? 0 : slot_bump_addr - p.addr);
			unsigned blocks_touched = offset_to_index(nanozone, pMeta, offset) - pMeta->slot_objects_skipped;
			unsigned blocks_now_free = count_free(nanozone, pMeta);
			unsigned blocks_in_use = blocks_touched - blocks_now_free;

			size_t size_hiwater = ((slot_key + 1) << SHIFT_NANO_QUANTUM) * blocks_touched;
			size_t size_in_use = ((slot_key + 1) << SHIFT_NANO_QUANTUM) * blocks_in_use;
			size_t size_allocated = ((offset / BAND_SIZE) + 1) * SLOT_IN_BAND_SIZE;

			_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX,
						   "Magazine %2d(%3d) [%p, %3dKB] \t Allocations in use=%4d \t Bytes in use=%db \t Untouched=%dKB\n",
						   mag_index, (slot_key + 1) << SHIFT_NANO_QUANTUM, p,
						   (size_allocated>>10), blocks_in_use, size_in_use, (size_allocated - size_hiwater)>>10);

			if (!verbose) {
				continue;
			} else {
				// Walk the slot free list and populate a bitarray_t
				int log_size = 64 - __builtin_clzl(slot_objects_mapped);
				bitarray_t slot_bitarray = bitarray_create(log_size);

				if (!slot_bitarray) {
					malloc_printf("bitarray_create(%d) in nano_print returned errno=%d.", log_size, errno);
					return;
				}

				chained_block_t head = NULL, tail = NULL, t;
				unsigned stoploss = slot_objects_mapped;
				while ((t = OSAtomicDequeue( &(pMeta->slot_LIFO), offsetof(struct chained_block_s,next)))) {
					if (0 == stoploss) {
						malloc_printf("Free list walk in nano_print exceeded object count.");
						break;
					}
					stoploss--;

					uintptr_t	offset = ((uintptr_t)t - p.addr); // offset from beginning of slot
					index_t 	block_index = offset_to_index(nanozone, pMeta, offset);

					if (NULL == head)
						head = t;
					else
						tail->next = t;
					tail = t;

					if (block_index < slot_objects_mapped)
						bitarray_set(slot_bitarray, log_size, block_index);
				}
				if (tail)
					tail->next = NULL;

				index_t i;
				for (i = 0; i < slot_objects_mapped; ++i)  {
					nano_blk_addr_t q;
					size_t pgnum;
					uintptr_t block_offset = index_to_offset(nanozone, pMeta, i);
					if (p.addr + block_offset >= slot_bump_addr)
						break;

					q.addr = p.addr + block_offset;
					pgnum = ((((unsigned)q.fields.nano_band) << NANO_OFFSET_BITS) | ((unsigned)q.fields.nano_offset)) >> vm_page_shift;

					if (i < pMeta->slot_objects_skipped) {
						_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX,"_");
					} else if (bitarray_get(slot_bitarray, log_size, i)) {
						_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX,"F");
					} else if (pMeta->slot_madvised_pages && (pgnum < ( 1 << pMeta->slot_madvised_log_page_count)) &&
							   bitarray_get(pMeta->slot_madvised_pages, pMeta->slot_madvised_log_page_count, pgnum)) {
						_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX,"M");
					} else {
						_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX,".");
					}
				}
				_malloc_printf(MALLOC_PRINTF_NOLOG | MALLOC_PRINTF_NOPREFIX,"\n");

				free(slot_bitarray);

				// push the free list extracted above back onto the LIFO, all at once
				if (head)
					OSAtomicEnqueue( &(pMeta->slot_LIFO), head, (uintptr_t)tail - (uintptr_t)head + offsetof(struct chained_block_s,next));
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

	for (i = 0; i < nanozone->phys_ncpus; ++i) {
        _malloc_lock_lock(&nanozone->band_resupply_lock[i]);
	}
}

static void
nano_force_unlock(nanozone_t *nanozone)
{
	int i;

	for (i = 0; i < nanozone->phys_ncpus; ++i) {
        _malloc_lock_unlock(&nanozone->band_resupply_lock[i]);
	}
}

static void
nano_statistics(nanozone_t *nanozone, malloc_statistics_t *stats)
{
	int i,j;

	bzero(stats, sizeof(*stats));

	for (i = 0; i < nanozone->phys_ncpus; ++i) {
		nano_blk_addr_t	p;

		// Establish p as base address for slot 0 in this CPU magazine
		p.fields.nano_signature = NANOZONE_SIGNATURE;
		p.fields.nano_mag_index = i;
		p.fields.nano_band = 0;
		p.fields.nano_slot = 0;
		p.fields.nano_offset = 0;

		for (j = 0; j < NANO_SLOT_SIZE;
			 p.addr += SLOT_IN_BAND_SIZE,	    // Advance to next slot base
			 ++j) {
			nano_meta_admin_t pMeta = &nanozone->meta_data[i][j];
			uintptr_t offset = pMeta->slot_bump_addr - p.addr;

			if (0 == pMeta->slot_current_base_addr) { // Nothing allocated in this magazine for this slot?
				continue;
			} else {
				unsigned blocks_touched = offset_to_index(nanozone, pMeta, offset) - pMeta->slot_objects_skipped;
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

	for (i = 0; i < nanozone->phys_ncpus; ++i) {
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
	(void *)nano_ptr_in_use_enumerator,
	(void *)nano_good_size,
	(void *)nanozone_check,
	(void *)nano_print,
	nano_log,
	(void *)nano_force_lock,
	(void *)nano_force_unlock,
	(void *)nano_statistics,
	(void *)nano_locked,
	NULL, NULL, NULL, NULL, /* Zone enumeration version 7 and forward. */
}; // marked as const to spare the DATA section

__attribute__((visibility("hidden")))
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
	nanozone->basic_zone.memalign = (void *)nano_memalign; /* Unchanged. */
	nanozone->basic_zone.free_definite_size = (void *)nano_forked_free_definite_size;

	mprotect(nanozone, sizeof(nanozone->basic_zone), PROT_READ);

}

__attribute__((visibility("hidden")))
malloc_zone_t *
create_nano_zone(size_t initial_size, malloc_zone_t *helper_zone, unsigned debug_flags)
{
	nanozone_t	*nanozone;
	int i, j;

	if (!_malloc_engaged_nano) return NULL;

#if defined(__x86_64__)
	if (_COMM_PAGE_VERSION_REQD > (*((uint16_t *)_COMM_PAGE_VERSION))) {
		malloc_printf("*** FATAL ERROR - comm page version mismatch.\n");
		exit(-1);
	}
#endif

	/* get memory for the zone. */
	nanozone = allocate_pages(NULL, SZONE_PAGED_SIZE, 0, 0, VM_MEMORY_MALLOC);
	if (!nanozone)
		return NULL;

	/* set up the basic_zone portion of the nanozone structure */
	nanozone->basic_zone.version = 8;
	nanozone->basic_zone.size = (void *)nano_size;
	nanozone->basic_zone.malloc = (debug_flags & SCALABLE_MALLOC_DO_SCRIBBLE) ? (void *)nano_malloc_scribble : (void *)nano_malloc;
	nanozone->basic_zone.calloc = (void *)nano_calloc;
	nanozone->basic_zone.valloc = (void *)nano_valloc;
	nanozone->basic_zone.free = (debug_flags & SCALABLE_MALLOC_DO_SCRIBBLE) ? (void *)nano_free_scribble : (void *)nano_free;
	nanozone->basic_zone.realloc = (void *)nano_realloc;
	nanozone->basic_zone.destroy = (void *)nano_destroy;
	nanozone->basic_zone.batch_malloc = (void *)nano_batch_malloc;
	nanozone->basic_zone.batch_free = (void *)nano_batch_free;
	nanozone->basic_zone.introspect = (struct malloc_introspection_t *)&nano_introspect;
	nanozone->basic_zone.memalign = (void *)nano_memalign;
	nanozone->basic_zone.free_definite_size = (debug_flags & SCALABLE_MALLOC_DO_SCRIBBLE) ?
	(void *)nano_free_definite_size_scribble : (void *)nano_free_definite_size;
	
	nanozone->basic_zone.pressure_relief = (void *)nano_pressure_relief;
	
	nanozone->basic_zone.reserved1 = 0; /* Set to zero once and for all as required by CFAllocator. */
	nanozone->basic_zone.reserved2 = 0; /* Set to zero once and for all as required by CFAllocator. */
	
	mprotect(nanozone, sizeof(nanozone->basic_zone), PROT_READ); /* Prevent overwriting the function pointers in basic_zone. */
	
	/* set up the remainder of the nanozone structure */
	nanozone->debug_flags = debug_flags;
	nanozone->our_signature = NANOZONE_SIGNATURE;
	
	/* Query the number of configured processors. */
#if defined(__x86_64__)
	nanozone->phys_ncpus = *(uint8_t *)(uintptr_t)_COMM_PAGE_PHYSICAL_CPUS;
	nanozone->logical_ncpus = *(uint8_t *)(uintptr_t)_COMM_PAGE_LOGICAL_CPUS;
#else
#error Unknown architecture
#endif
	
	if (nanozone->phys_ncpus > sizeof(nanozone->core_mapped_size)/sizeof(nanozone->core_mapped_size[0])) {
		_malloc_printf(ASL_LEVEL_NOTICE, "nano zone abandoned because NCPUS mismatch.\n");
		return NULL;
	}
	
	if (0 != (nanozone->logical_ncpus % nanozone->phys_ncpus)) {
		malloc_printf("*** FATAL ERROR - logical_ncpus % phys_ncpus != 0.\n");
		exit(-1);
	}
	
	switch (nanozone->logical_ncpus/nanozone->phys_ncpus) {
		case 1:
			nanozone->hyper_shift = 0;
			break;
		case 2:
			nanozone->hyper_shift = 1;
			break;
		case 4:
			nanozone->hyper_shift = 2;
			break;
		default:
			malloc_printf("*** FATAL ERROR - logical_ncpus / phys_ncpus not 1, 2, or 4.\n");
			exit(-1);
	}
	
	/* Initialize slot queue heads and resupply locks. */
	OSQueueHead q0 = OS_ATOMIC_QUEUE_INIT;
	for (i = 0; i < nanozone->phys_ncpus; ++i) {
        _malloc_lock_init(&nanozone->band_resupply_lock[i]);
        
		for (j = 0; j < NANO_SLOT_SIZE; ++j) {
			nanozone->meta_data[i][j].slot_LIFO = q0;
		}
	}
	
	/* Initialize the security token. */
	if (0 == _dyld_get_image_slide((const struct mach_header*)_NSGetMachExecuteHeader())) {
		// zero slide when ASLR has been disabled by boot-arg. Eliminate cloaking.
		malloc_entropy[0] = 0;
		malloc_entropy[1] = 0;
	}
	nanozone->cookie = (uintptr_t)malloc_entropy[0] & 0x0000ffffffff0000ULL; // scramble central 32bits with this cookie
	
	/* Nano zone does not support SCALABLE_MALLOC_ADD_GUARD_PAGES. */
	if (nanozone->debug_flags & SCALABLE_MALLOC_ADD_GUARD_PAGES) {
		_malloc_printf(ASL_LEVEL_INFO, "nano zone does not support guard pages\n");
		nanozone->debug_flags &= ~SCALABLE_MALLOC_ADD_GUARD_PAGES;
	}
	
	nanozone->helper_zone = helper_zone;
	
	return (malloc_zone_t *)nanozone;
}
#endif /* defined(__LP64__) */

/* vim: set noet:ts=4:sw=4:cindent: */
