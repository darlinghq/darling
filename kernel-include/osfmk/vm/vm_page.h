/*
 * Copyright (c) 2000-2006 Apple Computer, Inc. All rights reserved.
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
/*
 * @OSF_COPYRIGHT@
 */
/*
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 */
/*
 *	File:	vm/vm_page.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1985
 *
 *	Resident memory system definitions.
 */

#ifndef _VM_VM_PAGE_H_
#define _VM_VM_PAGE_H_

#include <debug.h>
#include <vm/vm_options.h>
#include <mach/boolean.h>
#include <mach/vm_prot.h>
#include <mach/vm_param.h>


#if    defined(__LP64__)

/*
 * in order to make the size of a vm_page_t 64 bytes (cache line size for both arm64 and x86_64)
 * we'll keep the next_m pointer packed... as long as the kernel virtual space where we allocate
 * vm_page_t's from doesn't span more then 256 Gbytes, we're safe.   There are live tests in the
 * vm_page_t array allocation and the zone init code to determine if we can safely pack and unpack
 * pointers from the 2 ends of these spaces
 */
typedef uint32_t        vm_page_packed_t;

struct vm_page_packed_queue_entry {
	vm_page_packed_t        next;          /* next element */
	vm_page_packed_t        prev;          /* previous element */
};

typedef struct vm_page_packed_queue_entry       *vm_page_queue_t;
typedef struct vm_page_packed_queue_entry       vm_page_queue_head_t;
typedef struct vm_page_packed_queue_entry       vm_page_queue_chain_t;
typedef struct vm_page_packed_queue_entry       *vm_page_queue_entry_t;

typedef vm_page_packed_t                        vm_page_object_t;

#else

/*
 * we can't do the packing trick on 32 bit architectures, so
 * just turn the macros into noops.
 */
typedef struct vm_page          *vm_page_packed_t;

#define vm_page_queue_t         queue_t
#define vm_page_queue_head_t    queue_head_t
#define vm_page_queue_chain_t   queue_chain_t
#define vm_page_queue_entry_t   queue_entry_t

#define vm_page_object_t        vm_object_t
#endif


#include <vm/vm_object.h>
#include <kern/queue.h>
#include <kern/locks.h>

#include <kern/macro_help.h>
#include <libkern/OSAtomic.h>



#define VM_PAGE_COMPRESSOR_COUNT        (compressor_object->resident_page_count)

/*
 *	Management of resident (logical) pages.
 *
 *	A small structure is kept for each resident
 *	page, indexed by page number.  Each structure
 *	is an element of several lists:
 *
 *		A hash table bucket used to quickly
 *		perform object/offset lookups
 *
 *		A list of all pages for a given object,
 *		so they can be quickly deactivated at
 *		time of deallocation.
 *
 *		An ordered list of pages due for pageout.
 *
 *	In addition, the structure contains the object
 *	and offset to which this page belongs (for pageout),
 *	and sundry status bits.
 *
 *	Fields in this structure are locked either by the lock on the
 *	object that the page belongs to (O) or by the lock on the page
 *	queues (P).  [Some fields require that both locks be held to
 *	change that field; holding either lock is sufficient to read.]
 */

#define VM_PAGE_NULL            ((vm_page_t) 0)

extern  char    vm_page_inactive_states[];
extern  char    vm_page_pageable_states[];
extern  char    vm_page_non_speculative_pageable_states[];
extern  char    vm_page_active_or_inactive_states[];


#define VM_PAGE_INACTIVE(m)                     (vm_page_inactive_states[m->vmp_q_state])
#define VM_PAGE_PAGEABLE(m)                     (vm_page_pageable_states[m->vmp_q_state])
#define VM_PAGE_NON_SPECULATIVE_PAGEABLE(m)     (vm_page_non_speculative_pageable_states[m->vmp_q_state])
#define VM_PAGE_ACTIVE_OR_INACTIVE(m)           (vm_page_active_or_inactive_states[m->vmp_q_state])


#define VM_PAGE_NOT_ON_Q                0               /* page is not present on any queue, nor is it wired... mainly a transient state */
#define VM_PAGE_IS_WIRED                1               /* page is currently wired */
#define VM_PAGE_USED_BY_COMPRESSOR      2               /* page is in use by the compressor to hold compressed data */
#define VM_PAGE_ON_FREE_Q               3               /* page is on the main free queue */
#define VM_PAGE_ON_FREE_LOCAL_Q         4               /* page is on one of the per-CPU free queues */
#define VM_PAGE_ON_FREE_LOPAGE_Q        5               /* page is on the lopage pool free list */
#define VM_PAGE_ON_THROTTLED_Q          6               /* page is on the throttled queue... we stash anonymous pages here when not paging */
#define VM_PAGE_ON_PAGEOUT_Q            7               /* page is on one of the pageout queues (internal/external) awaiting processing */
#define VM_PAGE_ON_SPECULATIVE_Q        8               /* page is on one of the speculative queues */
#define VM_PAGE_ON_ACTIVE_LOCAL_Q       9               /* page has recently been created and is being held in one of the per-CPU local queues */
#define VM_PAGE_ON_ACTIVE_Q             10              /* page is in global active queue */
#define VM_PAGE_ON_INACTIVE_INTERNAL_Q  11              /* page is on the inactive internal queue a.k.a.  anonymous queue */
#define VM_PAGE_ON_INACTIVE_EXTERNAL_Q  12              /* page in on the inactive external queue a.k.a.  file backed queue */
#define VM_PAGE_ON_INACTIVE_CLEANED_Q   13              /* page has been cleaned to a backing file and is ready to be stolen */
#define VM_PAGE_ON_SECLUDED_Q           14              /* page is on secluded queue */
#define VM_PAGE_Q_STATE_LAST_VALID_VALUE        14      /* we currently use 4 bits for the state... don't let this go beyond 15 */

#define VM_PAGE_Q_STATE_ARRAY_SIZE      (VM_PAGE_Q_STATE_LAST_VALID_VALUE+1)


/*
 * The structure itself. See the block comment above for what (O) and (P) mean.
 */
#define vmp_pageq vmp_q_un.vmp_q_pageq
#define vmp_snext vmp_q_un.vmp_q_snext

struct vm_page {
	union {
		vm_page_queue_chain_t vmp_q_pageq;           /* queue info for FIFO queue or free list (P) */
		struct vm_page        *vmp_q_snext;
	} vmp_q_un;

	vm_page_queue_chain_t         vmp_listq;           /* all pages in same object (O) */

#if CONFIG_BACKGROUND_QUEUE
	vm_page_queue_chain_t         vmp_backgroundq;     /* anonymous pages in the background pool (P) */
#endif

	vm_object_offset_t            vmp_offset;          /* offset into that object (O,P) */
	vm_page_object_t              vmp_object;          /* which object am I in (O&P) */

	/*
	 * The following word of flags is always protected by the "page queues" lock.
	 *
	 * We use 'vmp_wire_count' to store the local queue id if local queues are enabled.
	 * See the comments at 'vm_page_queues_remove' as to why this is safe to do.
	 */
#define vmp_local_id vmp_wire_count
	unsigned int vmp_wire_count:16,      /* how many wired down maps use me? (O&P) */
	    vmp_q_state:4,                   /* which q is the page on (P) */
	    vmp_in_background:1,
	    vmp_on_backgroundq:1,
	    vmp_gobbled:1,                   /* page used internally (P) */
	    vmp_laundry:1,                   /* page is being cleaned now (P)*/
	    vmp_no_cache:1,                  /* page is not to be cached and should */
	                                     /* be reused ahead of other pages (P) */
	    vmp_private:1,                   /* Page should not be returned to the free list (P) */
	    vmp_reference:1,                 /* page has been used (P) */
	    vmp_unused_page_bits:5;

	/*
	 * MUST keep the 2 32 bit words used as bit fields
	 * separated since the compiler has a nasty habit
	 * of using 64 bit loads and stores on them as
	 * if they were a single 64 bit field... since
	 * they are protected by 2 different locks, this
	 * is a real problem
	 */
	vm_page_packed_t vmp_next_m;            /* VP bucket link (O) */

	/*
	 * The following word of flags is protected by the "VM object" lock.
	 *
	 * IMPORTANT: the "vmp_pmapped", "vmp_xpmapped" and "vmp_clustered" bits can be modified while holding the
	 * VM object "shared" lock + the page lock provided through the pmap_lock_phys_page function.
	 * This is done in vm_fault_enter() and the CONSUME_CLUSTERED macro.
	 * It's also ok to modify them behind just the VM object "exclusive" lock.
	 */
	unsigned int    vmp_busy:1,           /* page is in transit (O) */
	    vmp_wanted:1,                     /* someone is waiting for page (O) */
	    vmp_tabled:1,                     /* page is in VP table (O) */
	    vmp_hashed:1,                     /* page is in vm_page_buckets[] (O) + the bucket lock */
	    vmp_fictitious:1,                 /* Physical page doesn't exist (O) */
	    vmp_clustered:1,                  /* page is not the faulted page (O) or (O-shared AND pmap_page) */
	    vmp_pmapped:1,                    /* page has at some time been entered into a pmap (O) or */
	                                      /* (O-shared AND pmap_page) */
	    vmp_xpmapped:1,                   /* page has been entered with execute permission (O) or */
	                                      /* (O-shared AND pmap_page) */
	    vmp_wpmapped:1,                   /* page has been entered at some point into a pmap for write (O) */
	    vmp_free_when_done:1,             /* page is to be freed once cleaning is completed (O) */
	    vmp_absent:1,                     /* Data has been requested, but is not yet available (O) */
	    vmp_error:1,                      /* Data manager was unable to provide data due to error (O) */
	    vmp_dirty:1,                      /* Page must be cleaned (O) */
	    vmp_cleaning:1,                   /* Page clean has begun (O) */
	    vmp_precious:1,                   /* Page is precious; data must be returned even if clean (O) */
	    vmp_overwriting:1,                /* Request to unlock has been made without having data. (O) */
	                                      /* [See vm_fault_page_overwrite] */
	    vmp_restart:1,                    /* Page was pushed higher in shadow chain by copy_call-related pagers */
	                                      /* start again at top of chain */
	    vmp_unusual:1,                    /* Page is absent, error, restart or page locked */
	    vmp_cs_validated:1,               /* code-signing: page was checked */
	    vmp_cs_tainted:1,                 /* code-signing: page is tainted */
	    vmp_cs_nx:1,                      /* code-signing: page is nx */
	    vmp_reusable:1,
	    vmp_lopage:1,
	    vmp_written_by_kernel:1,             /* page was written by kernel (i.e. decompressed) */
	    vmp_unused_object_bits:8;

#if    !defined(__arm__) && !defined(__arm64__)
	ppnum_t         vmp_phys_page;        /* Physical page number of the page */
#endif
};


typedef struct vm_page  *vm_page_t;
extern vm_page_t        vm_pages;
extern vm_page_t        vm_page_array_beginning_addr;
extern vm_page_t        vm_page_array_ending_addr;


#if defined(__arm__) || defined(__arm64__)

extern  unsigned int vm_first_phys_ppnum;

struct vm_page_with_ppnum {
	struct  vm_page vm_page_wo_ppnum;

	ppnum_t vmp_phys_page;
};
typedef struct vm_page_with_ppnum *vm_page_with_ppnum_t;


static inline ppnum_t
VM_PAGE_GET_PHYS_PAGE(vm_page_t m)
{
	if (m >= vm_page_array_beginning_addr && m < vm_page_array_ending_addr) {
		return (ppnum_t)((uintptr_t)(m - vm_page_array_beginning_addr) + vm_first_phys_ppnum);
	} else {
		return ((vm_page_with_ppnum_t)m)->vmp_phys_page;
	}
}

#define VM_PAGE_SET_PHYS_PAGE(m, ppnum)         \
	MACRO_BEGIN                             \
	if ((m) < vm_page_array_beginning_addr || (m) >= vm_page_array_ending_addr)     \
	        ((vm_page_with_ppnum_t)(m))->vmp_phys_page = ppnum;     \
	assert(ppnum == VM_PAGE_GET_PHYS_PAGE(m));              \
	MACRO_END

#define VM_PAGE_GET_COLOR(m)    (VM_PAGE_GET_PHYS_PAGE(m) & vm_color_mask)

#else   /* defined(__arm__) || defined(__arm64__) */


struct vm_page_with_ppnum {
	struct  vm_page vm_page_with_ppnum;
};
typedef struct vm_page_with_ppnum *vm_page_with_ppnum_t;


#define VM_PAGE_GET_PHYS_PAGE(page)     (page)->vmp_phys_page
#define VM_PAGE_SET_PHYS_PAGE(page, ppnum)      \
	MACRO_BEGIN                             \
	(page)->vmp_phys_page = ppnum;          \
	MACRO_END

#define VM_PAGE_GET_CLUMP(m)    ((VM_PAGE_GET_PHYS_PAGE(m)) >> vm_clump_shift)
#define VM_PAGE_GET_COLOR(m)    ((VM_PAGE_GET_CLUMP(m)) & vm_color_mask)

#endif  /* defined(__arm__) || defined(__arm64__) */



#if    defined(__LP64__)

#define VM_VPLQ_ALIGNMENT               128
#define VM_PACKED_POINTER_ALIGNMENT     64              /* must be a power of 2 */
#define VM_PACKED_POINTER_SHIFT         6

#define VM_PACKED_FROM_VM_PAGES_ARRAY   0x80000000

static inline vm_page_packed_t
vm_page_pack_ptr(uintptr_t p)
{
	vm_page_packed_t packed_ptr;

	if (!p) {
		return (vm_page_packed_t)0;
	}

	if (p >= (uintptr_t)(vm_page_array_beginning_addr) && p < (uintptr_t)(vm_page_array_ending_addr)) {
		packed_ptr = ((vm_page_packed_t)(((vm_page_t)p - vm_page_array_beginning_addr)));
		assert(!(packed_ptr & VM_PACKED_FROM_VM_PAGES_ARRAY));
		packed_ptr |= VM_PACKED_FROM_VM_PAGES_ARRAY;
		return packed_ptr;
	}

	assert((p & (VM_PACKED_POINTER_ALIGNMENT - 1)) == 0);

	packed_ptr = ((vm_page_packed_t)(((uintptr_t)(p - (uintptr_t) VM_MIN_KERNEL_AND_KEXT_ADDRESS)) >> VM_PACKED_POINTER_SHIFT));
	assert(packed_ptr != 0);
	assert(!(packed_ptr & VM_PACKED_FROM_VM_PAGES_ARRAY));
	return packed_ptr;
}


static inline uintptr_t
vm_page_unpack_ptr(uintptr_t p)
{
	extern unsigned int vm_pages_count;

	if (!p) {
		return (uintptr_t)0;
	}

	if (p & VM_PACKED_FROM_VM_PAGES_ARRAY) {
		assert((uint32_t)(p & ~VM_PACKED_FROM_VM_PAGES_ARRAY) < vm_pages_count);
		return (uintptr_t)(&vm_pages[(uint32_t)(p & ~VM_PACKED_FROM_VM_PAGES_ARRAY)]);
	}
	return (p << VM_PACKED_POINTER_SHIFT) + (uintptr_t) VM_MIN_KERNEL_AND_KEXT_ADDRESS;
}


#define VM_PAGE_PACK_PTR(p)     vm_page_pack_ptr((uintptr_t)(p))
#define VM_PAGE_UNPACK_PTR(p)   vm_page_unpack_ptr((uintptr_t)(p))

#define VM_PAGE_OBJECT(p)       ((vm_object_t)(VM_PAGE_UNPACK_PTR(p->vmp_object)))
#define VM_PAGE_PACK_OBJECT(o)  ((vm_page_object_t)(VM_PAGE_PACK_PTR(o)))


#define VM_PAGE_ZERO_PAGEQ_ENTRY(p)     \
MACRO_BEGIN                             \
	(p)->vmp_snext = 0;             \
MACRO_END


#define VM_PAGE_CONVERT_TO_QUEUE_ENTRY(p)       VM_PAGE_PACK_PTR(p)


static __inline__ void
vm_page_enqueue_tail(
	vm_page_queue_t         que,
	vm_page_queue_entry_t   elt)
{
	vm_page_queue_entry_t   old_tail;

	old_tail = (vm_page_queue_entry_t)VM_PAGE_UNPACK_PTR(que->prev);
	elt->next = VM_PAGE_PACK_PTR(que);
	elt->prev = que->prev;
	que->prev = old_tail->next = VM_PAGE_PACK_PTR(elt);
}


static __inline__ void
vm_page_remque(
	vm_page_queue_entry_t elt)
{
	vm_page_queue_entry_t next;
	vm_page_queue_entry_t prev;
	vm_page_packed_t      next_pck = elt->next;
	vm_page_packed_t      prev_pck = elt->prev;

	next = (vm_page_queue_entry_t)VM_PAGE_UNPACK_PTR(next_pck);

	/* next may equal prev (and the queue head) if elt was the only element */
	prev = (vm_page_queue_entry_t)VM_PAGE_UNPACK_PTR(prev_pck);

	next->prev = prev_pck;
	prev->next = next_pck;

	elt->next = 0;
	elt->prev = 0;
}


/*
 *	Macro:	vm_page_queue_init
 *	Function:
 *		Initialize the given queue.
 *	Header:
 *	void vm_page_queue_init(q)
 *		vm_page_queue_t	q;	\* MODIFIED *\
 */
#define vm_page_queue_init(q)                   \
MACRO_BEGIN                                     \
	assert((((uintptr_t)q) & (VM_PACKED_POINTER_ALIGNMENT-1)) == 0);        \
	assert((VM_PAGE_UNPACK_PTR(VM_PAGE_PACK_PTR((uintptr_t)q))) == (uintptr_t)q);   \
	(q)->next = VM_PAGE_PACK_PTR(q);        \
	(q)->prev = VM_PAGE_PACK_PTR(q);        \
MACRO_END


/*
 * Macro: vm_page_queue_enter
 * Function:
 *     Insert a new element at the tail of the vm_page queue.
 * Header:
 *     void vm_page_queue_enter(q, elt, field)
 *         queue_t q;
 *         vm_page_t elt;
 *         <field> is the list field in vm_page_t
 *
 * This macro's arguments have to match the generic "queue_enter()" macro which is
 * what is used for this on 32 bit kernels.
 */
#define vm_page_queue_enter(head, elt, field)                       \
MACRO_BEGIN                                                         \
	vm_page_packed_t __pck_elt = VM_PAGE_PACK_PTR(elt);         \
	vm_page_packed_t __pck_head = VM_PAGE_PACK_PTR(head);       \
	vm_page_packed_t __pck_prev = (head)->prev;                 \
                                                                    \
	if (__pck_head == __pck_prev) {                             \
	        (head)->next = __pck_elt;                           \
	} else {                                                    \
	        vm_page_t __prev;                                   \
	        __prev = (vm_page_t)VM_PAGE_UNPACK_PTR(__pck_prev); \
	        __prev->field.next = __pck_elt;                     \
	}                                                           \
	(elt)->field.prev = __pck_prev;                             \
	(elt)->field.next = __pck_head;                             \
	(head)->prev = __pck_elt;                                   \
MACRO_END


#if defined(__x86_64__)
/*
 * These are helper macros for vm_page_queue_enter_clump to assist
 * with conditional compilation (release / debug / development)
 */
#if DEVELOPMENT || DEBUG

#define __DEBUG_CHECK_BUDDIES(__prev, __p, field)                                             \
MACRO_BEGIN                                                                                   \
	if (__prev != NULL) {                                                                 \
	        assert(__p == (vm_page_t)VM_PAGE_UNPACK_PTR(__prev->next));                   \
	        assert(__prev == (vm_page_queue_entry_t)VM_PAGE_UNPACK_PTR(__p->field.prev)); \
	}                                                                                     \
MACRO_END

#define __DEBUG_VERIFY_LINKS(__first, __n_free, __last_next)                    \
MACRO_BEGIN                                                                     \
	unsigned int __i;                                                       \
	vm_page_queue_entry_t __tmp;                                            \
	for (__i = 0, __tmp = __first; __i < __n_free; __i++) {                 \
	        __tmp = (vm_page_queue_entry_t)VM_PAGE_UNPACK_PTR(__tmp->next); \
	}                                                                       \
	assert(__tmp == __last_next);                                           \
MACRO_END

#define __DEBUG_STAT_INCREMENT_INRANGE              vm_clump_inrange++
#define __DEBUG_STAT_INCREMENT_INSERTS              vm_clump_inserts++
#define __DEBUG_STAT_INCREMENT_PROMOTES(__n_free)   vm_clump_promotes+=__n_free

#else

#define __DEBUG_CHECK_BUDDIES(__prev, __p, field)
#define __DEBUG_VERIFY_LINKS(__first, __n_free, __last_next)
#define __DEBUG_STAT_INCREMENT_INRANGE
#define __DEBUG_STAT_INCREMENT_INSERTS
#define __DEBUG_STAT_INCREMENT_PROMOTES(__n_free)

#endif  /* if DEVELOPMENT || DEBUG */

/*
 * Insert a new page into a free queue and clump pages within the same 16K boundary together
 */
static inline void
vm_page_queue_enter_clump(
	vm_page_queue_t       head,
	vm_page_t             elt)
{
	vm_page_queue_entry_t first;    /* first page in the clump */
	vm_page_queue_entry_t last;     /* last page in the clump */
	vm_page_queue_entry_t prev = NULL;
	vm_page_queue_entry_t next;
	uint_t                n_free = 1;
	extern unsigned int   vm_pages_count;
	extern unsigned int   vm_clump_size, vm_clump_mask, vm_clump_shift, vm_clump_promote_threshold;
	extern unsigned long  vm_clump_allocs, vm_clump_inserts, vm_clump_inrange, vm_clump_promotes;

	/*
	 * If elt is part of the vm_pages[] array, find its neighboring buddies in the array.
	 */
	if (vm_page_array_beginning_addr <= elt && elt < &vm_pages[vm_pages_count]) {
		vm_page_t p;
		uint_t    i;
		uint_t    n;
		ppnum_t   clump_num;

		first = last = (vm_page_queue_entry_t)elt;
		clump_num = VM_PAGE_GET_CLUMP(elt);
		n = VM_PAGE_GET_PHYS_PAGE(elt) & vm_clump_mask;

		/*
		 * Check for preceeding vm_pages[] entries in the same chunk
		 */
		for (i = 0, p = elt - 1; i < n && vm_page_array_beginning_addr <= p; i++, p--) {
			if (p->vmp_q_state == VM_PAGE_ON_FREE_Q && clump_num == VM_PAGE_GET_CLUMP(p)) {
				if (prev == NULL) {
					prev = (vm_page_queue_entry_t)p;
				}
				first = (vm_page_queue_entry_t)p;
				n_free++;
			}
		}

		/*
		 * Check the following vm_pages[] entries in the same chunk
		 */
		for (i = n + 1, p = elt + 1; i < vm_clump_size && p < &vm_pages[vm_pages_count]; i++, p++) {
			if (p->vmp_q_state == VM_PAGE_ON_FREE_Q && clump_num == VM_PAGE_GET_CLUMP(p)) {
				if (last == (vm_page_queue_entry_t)elt) {               /* first one only */
					__DEBUG_CHECK_BUDDIES(prev, p, vmp_pageq);
				}

				if (prev == NULL) {
					prev = (vm_page_queue_entry_t)VM_PAGE_UNPACK_PTR(p->vmp_pageq.prev);
				}
				last = (vm_page_queue_entry_t)p;
				n_free++;
			}
		}
		__DEBUG_STAT_INCREMENT_INRANGE;
	}

	/* if elt is not part of vm_pages or if 1st page in clump, insert at tail */
	if (prev == NULL) {
		prev = (vm_page_queue_entry_t)VM_PAGE_UNPACK_PTR(head->prev);
	}

	/* insert the element */
	next = (vm_page_queue_entry_t)VM_PAGE_UNPACK_PTR(prev->next);
	elt->vmp_pageq.next = prev->next;
	elt->vmp_pageq.prev = next->prev;
	prev->next = next->prev = VM_PAGE_PACK_PTR(elt);
	__DEBUG_STAT_INCREMENT_INSERTS;

	/*
	 * Check if clump needs to be promoted to head.
	 */
	if (n_free >= vm_clump_promote_threshold && n_free > 1) {
		vm_page_queue_entry_t first_prev;

		first_prev = (vm_page_queue_entry_t)VM_PAGE_UNPACK_PTR(first->prev);

		/* If not at head already */
		if (first_prev != head) {
			vm_page_queue_entry_t last_next;
			vm_page_queue_entry_t head_next;

			last_next = (vm_page_queue_entry_t)VM_PAGE_UNPACK_PTR(last->next);

			/* verify that the links within the clump are consistent */
			__DEBUG_VERIFY_LINKS(first, n_free, last_next);

			/* promote clump to head */
			first_prev->next = last->next;
			last_next->prev = first->prev;
			first->prev = VM_PAGE_PACK_PTR(head);
			last->next = head->next;

			head_next = (vm_page_queue_entry_t)VM_PAGE_UNPACK_PTR(head->next);
			head_next->prev = VM_PAGE_PACK_PTR(last);
			head->next = VM_PAGE_PACK_PTR(first);
			__DEBUG_STAT_INCREMENT_PROMOTES(n_free);
		}
	}
}
#endif

/*
 * Macro: vm_page_queue_enter_first
 * Function:
 *     Insert a new element at the head of the vm_page queue.
 * Header:
 *     void queue_enter_first(q, elt, , field)
 *         queue_t q;
 *         vm_page_t elt;
 *         <field> is the linkage field in vm_page
 *
 * This macro's arguments have to match the generic "queue_enter_first()" macro which is
 * what is used for this on 32 bit kernels.
 */
#define vm_page_queue_enter_first(head, elt, field)                 \
MACRO_BEGIN                                                         \
	vm_page_packed_t __pck_next = (head)->next;                 \
	vm_page_packed_t __pck_head = VM_PAGE_PACK_PTR(head);       \
	vm_page_packed_t __pck_elt = VM_PAGE_PACK_PTR(elt);         \
                                                                    \
	if (__pck_head == __pck_next) {                             \
	        (head)->prev = __pck_elt;                           \
	} else {                                                    \
	        vm_page_t __next;                                   \
	        __next = (vm_page_t)VM_PAGE_UNPACK_PTR(__pck_next); \
	        __next->field.prev = __pck_elt;                     \
	}                                                           \
                                                                    \
	(elt)->field.next = __pck_next;                             \
	(elt)->field.prev = __pck_head;                             \
	(head)->next = __pck_elt;                                   \
MACRO_END


/*
 * Macro:	vm_page_queue_remove
 * Function:
 *     Remove an arbitrary page from a vm_page queue.
 * Header:
 *     void vm_page_queue_remove(q, qe, field)
 *         arguments as in vm_page_queue_enter
 *
 * This macro's arguments have to match the generic "queue_enter()" macro which is
 * what is used for this on 32 bit kernels.
 */
#define vm_page_queue_remove(head, elt, field)                          \
MACRO_BEGIN                                                             \
	vm_page_packed_t __pck_next = (elt)->field.next;                \
	vm_page_packed_t __pck_prev = (elt)->field.prev;                \
	vm_page_t        __next = (vm_page_t)VM_PAGE_UNPACK_PTR(__pck_next); \
	vm_page_t        __prev = (vm_page_t)VM_PAGE_UNPACK_PTR(__pck_prev); \
                                                                        \
	if ((void *)(head) == (void *)__next) {                         \
	        (head)->prev = __pck_prev;                              \
	} else {                                                        \
	        __next->field.prev = __pck_prev;                        \
	}                                                               \
                                                                        \
	if ((void *)(head) == (void *)__prev) {                         \
	        (head)->next = __pck_next;                              \
	} else {                                                        \
	        __prev->field.next = __pck_next;                        \
	}                                                               \
                                                                        \
	(elt)->field.next = 0;                                          \
	(elt)->field.prev = 0;                                          \
MACRO_END


/*
 * Macro: vm_page_queue_remove_first
 *
 * Function:
 *     Remove and return the entry at the head of a vm_page queue.
 *
 * Header:
 *     vm_page_queue_remove_first(head, entry, field)
 *     N.B. entry is returned by reference
 *
 * This macro's arguments have to match the generic "queue_remove_first()" macro which is
 * what is used for this on 32 bit kernels.
 */
#define vm_page_queue_remove_first(head, entry, field)            \
MACRO_BEGIN                                                       \
	vm_page_packed_t __pck_head = VM_PAGE_PACK_PTR(head);     \
	vm_page_packed_t __pck_next;                              \
	vm_page_t        __next;                                  \
                                                                  \
	(entry) = (vm_page_t)VM_PAGE_UNPACK_PTR((head)->next);    \
	__pck_next = (entry)->field.next;                         \
	__next = (vm_page_t)VM_PAGE_UNPACK_PTR(__pck_next);       \
                                                                  \
	if (__pck_head == __pck_next) {                           \
	        (head)->prev = __pck_head;                        \
	} else {                                                  \
	        __next->field.prev = __pck_head;                  \
	}                                                         \
                                                                  \
	(head)->next = __pck_next;                                \
	(entry)->field.next = 0;                                  \
	(entry)->field.prev = 0;                                  \
MACRO_END


#if defined(__x86_64__)
/*
 * Macro:  vm_page_queue_remove_first_with_clump
 * Function:
 *     Remove and return the entry at the head of the free queue
 *     end is set to 1 to indicate that we just returned the last page in a clump
 *
 * Header:
 *     vm_page_queue_remove_first_with_clump(head, entry, end)
 *     entry is returned by reference
 *     end is returned by reference
 */
#define vm_page_queue_remove_first_with_clump(head, entry, end)              \
MACRO_BEGIN                                                                  \
	vm_page_packed_t __pck_head = VM_PAGE_PACK_PTR(head);                \
	vm_page_packed_t __pck_next;                                         \
	vm_page_t        __next;                                             \
                                                                             \
	(entry) = (vm_page_t)VM_PAGE_UNPACK_PTR((head)->next);               \
	__pck_next = (entry)->vmp_pageq.next;                                \
	__next = (vm_page_t)VM_PAGE_UNPACK_PTR(__pck_next);                  \
                                                                             \
	(end) = 0;                                                           \
	if (__pck_head == __pck_next) {                                      \
	        (head)->prev = __pck_head;                                   \
	        (end) = 1;                                                   \
	} else {                                                             \
	        __next->vmp_pageq.prev = __pck_head;                         \
	        if (VM_PAGE_GET_CLUMP(entry) != VM_PAGE_GET_CLUMP(__next)) { \
	                (end) = 1;                                           \
	        }                                                            \
	}                                                                    \
                                                                             \
	(head)->next = __pck_next;                                           \
	(entry)->vmp_pageq.next = 0;                                         \
	(entry)->vmp_pageq.prev = 0;                                         \
MACRO_END
#endif

/*
 *	Macro:	vm_page_queue_end
 *	Function:
 *	Tests whether a new entry is really the end of
 *		the queue.
 *	Header:
 *		boolean_t vm_page_queue_end(q, qe)
 *			vm_page_queue_t q;
 *			vm_page_queue_entry_t qe;
 */
#define vm_page_queue_end(q, qe)        ((q) == (qe))


/*
 *	Macro:	vm_page_queue_empty
 *	Function:
 *		Tests whether a queue is empty.
 *	Header:
 *		boolean_t vm_page_queue_empty(q)
 *			vm_page_queue_t q;
 */
#define vm_page_queue_empty(q)          vm_page_queue_end((q), ((vm_page_queue_entry_t)vm_page_queue_first(q)))



/*
 *	Macro:	vm_page_queue_first
 *	Function:
 *		Returns the first entry in the queue,
 *	Header:
 *		uintpr_t vm_page_queue_first(q)
 *			vm_page_queue_t q;	\* IN *\
 */
#define vm_page_queue_first(q)          (VM_PAGE_UNPACK_PTR((q)->next))



/*
 *	Macro:		vm_page_queue_last
 *	Function:
 *		Returns the last entry in the queue.
 *	Header:
 *		vm_page_queue_entry_t queue_last(q)
 *			queue_t	q;		\* IN *\
 */
#define vm_page_queue_last(q)           (VM_PAGE_UNPACK_PTR((q)->prev))



/*
 *	Macro:	vm_page_queue_next
 *	Function:
 *		Returns the entry after an item in the queue.
 *	Header:
 *		uintpr_t vm_page_queue_next(qc)
 *			vm_page_queue_t qc;
 */
#define vm_page_queue_next(qc)          (VM_PAGE_UNPACK_PTR((qc)->next))



/*
 *	Macro:	vm_page_queue_prev
 *	Function:
 *		Returns the entry before an item in the queue.
 *	Header:
 *		uinptr_t vm_page_queue_prev(qc)
 *			vm_page_queue_t qc;
 */
#define vm_page_queue_prev(qc)          (VM_PAGE_UNPACK_PTR((qc)->prev))



/*
 *	Macro:	vm_page_queue_iterate
 *	Function:
 *		iterate over each item in a vm_page queue.
 *		Generates a 'for' loop, setting elt to
 *		each item in turn (by reference).
 *	Header:
 *		vm_page_queue_iterate(q, elt, field)
 *			queue_t q;
 *			vm_page_t elt;
 *			<field> is the chain field in vm_page_t
 */
#define vm_page_queue_iterate(head, elt, field)                       \
	for ((elt) = (vm_page_t)vm_page_queue_first(head);            \
	    !vm_page_queue_end((head), (vm_page_queue_entry_t)(elt)); \
	    (elt) = (vm_page_t)vm_page_queue_next(&(elt)->field))     \

#else

#define VM_VPLQ_ALIGNMENT               128
#define VM_PACKED_POINTER_ALIGNMENT     4
#define VM_PACKED_POINTER_SHIFT         0

#define VM_PACKED_FROM_VM_PAGES_ARRAY   0

#define VM_PAGE_PACK_PTR(p)     (p)
#define VM_PAGE_UNPACK_PTR(p)   ((uintptr_t)(p))

#define VM_PAGE_OBJECT(p)       (vm_object_t)(p->vmp_object)
#define VM_PAGE_PACK_OBJECT(o)  ((vm_page_object_t)(VM_PAGE_PACK_PTR(o)))


#define VM_PAGE_ZERO_PAGEQ_ENTRY(p)     \
MACRO_BEGIN                             \
	(p)->vmp_pageq.next = 0;                \
	(p)->vmp_pageq.prev = 0;                \
MACRO_END

#define VM_PAGE_CONVERT_TO_QUEUE_ENTRY(p)   ((queue_entry_t)(p))

#define vm_page_remque                      remque
#define vm_page_enqueue_tail                enqueue_tail
#define vm_page_queue_init                  queue_init
#define vm_page_queue_enter(h, e, f)        queue_enter(h, e, vm_page_t, f)
#define vm_page_queue_enter_first(h, e, f)  queue_enter_first(h, e, vm_page_t, f)
#define vm_page_queue_remove(h, e, f)       queue_remove(h, e, vm_page_t, f)
#define vm_page_queue_remove_first(h, e, f) queue_remove_first(h, e, vm_page_t, f)
#define vm_page_queue_end                   queue_end
#define vm_page_queue_empty                 queue_empty
#define vm_page_queue_first                 queue_first
#define vm_page_queue_last                  queue_last
#define vm_page_queue_next                  queue_next
#define vm_page_queue_prev                  queue_prev
#define vm_page_queue_iterate(h, e, f)      queue_iterate(h, e, vm_page_t, f)

#endif



/*
 * VM_PAGE_MIN_SPECULATIVE_AGE_Q through VM_PAGE_MAX_SPECULATIVE_AGE_Q
 * represents a set of aging bins that are 'protected'...
 *
 * VM_PAGE_SPECULATIVE_AGED_Q is a list of the speculative pages that have
 * not yet been 'claimed' but have been aged out of the protective bins
 * this occurs in vm_page_speculate when it advances to the next bin
 * and discovers that it is still occupied... at that point, all of the
 * pages in that bin are moved to the VM_PAGE_SPECULATIVE_AGED_Q.  the pages
 * in that bin are all guaranteed to have reached at least the maximum age
 * we allow for a protected page... they can be older if there is no
 * memory pressure to pull them from the bin, or there are no new speculative pages
 * being generated to push them out.
 * this list is the one that vm_pageout_scan will prefer when looking
 * for pages to move to the underweight free list
 *
 * VM_PAGE_MAX_SPECULATIVE_AGE_Q * VM_PAGE_SPECULATIVE_Q_AGE_MS
 * defines the amount of time a speculative page is normally
 * allowed to live in the 'protected' state (i.e. not available
 * to be stolen if vm_pageout_scan is running and looking for
 * pages)...  however, if the total number of speculative pages
 * in the protected state exceeds our limit (defined in vm_pageout.c)
 * and there are none available in VM_PAGE_SPECULATIVE_AGED_Q, then
 * vm_pageout_scan is allowed to steal pages from the protected
 * bucket even if they are underage.
 *
 * vm_pageout_scan is also allowed to pull pages from a protected
 * bin if the bin has reached the "age of consent" we've set
 */
#define VM_PAGE_MAX_SPECULATIVE_AGE_Q   10
#define VM_PAGE_MIN_SPECULATIVE_AGE_Q   1
#define VM_PAGE_SPECULATIVE_AGED_Q      0

#define VM_PAGE_SPECULATIVE_Q_AGE_MS    500

struct vm_speculative_age_q {
	/*
	 * memory queue for speculative pages via clustered pageins
	 */
	vm_page_queue_head_t    age_q;
	mach_timespec_t age_ts;
} __attribute__((aligned(VM_PACKED_POINTER_ALIGNMENT)));



extern
struct vm_speculative_age_q     vm_page_queue_speculative[];

extern int                      speculative_steal_index;
extern int                      speculative_age_index;
extern unsigned int             vm_page_speculative_q_age_ms;


typedef struct vm_locks_array {
	char    pad  __attribute__ ((aligned(64)));
	lck_mtx_t       vm_page_queue_lock2 __attribute__ ((aligned(64)));
	lck_mtx_t       vm_page_queue_free_lock2 __attribute__ ((aligned(64)));
	char    pad2  __attribute__ ((aligned(64)));
} vm_locks_array_t;


#if CONFIG_BACKGROUND_QUEUE
extern  void    vm_page_assign_background_state(vm_page_t mem);
extern  void    vm_page_update_background_state(vm_page_t mem);
extern  void    vm_page_add_to_backgroundq(vm_page_t mem, boolean_t first);
extern  void    vm_page_remove_from_backgroundq(vm_page_t mem);
#endif

#define VM_PAGE_WIRED(m)        ((m)->vmp_q_state == VM_PAGE_IS_WIRED)
#define NEXT_PAGE(m)            ((m)->vmp_snext)
#define NEXT_PAGE_PTR(m)        (&(m)->vmp_snext)

/*
 * XXX	The unusual bit should not be necessary.  Most of the bit
 * XXX	fields above really want to be masks.
 */

/*
 *	For debugging, this macro can be defined to perform
 *	some useful check on a page structure.
 *	INTENTIONALLY left as a no-op so that the
 *	current call-sites can be left intact for future uses.
 */

#define VM_PAGE_CHECK(mem)                      \
	MACRO_BEGIN                             \
	MACRO_END

/*     Page coloring:
 *
 *     The free page list is actually n lists, one per color,
 *     where the number of colors is a function of the machine's
 *     cache geometry set at system initialization.  To disable
 *     coloring, set vm_colors to 1 and vm_color_mask to 0.
 *     The boot-arg "colors" may be used to override vm_colors.
 *     Note that there is little harm in having more colors than needed.
 */

#define MAX_COLORS      128
#define DEFAULT_COLORS  32

extern
unsigned int    vm_colors;              /* must be in range 1..MAX_COLORS */
extern
unsigned int    vm_color_mask;          /* must be (vm_colors-1) */
extern
unsigned int    vm_cache_geometry_colors; /* optimal #colors based on cache geometry */

/*
 * Wired memory is a very limited resource and we can't let users exhaust it
 * and deadlock the entire system.  We enforce the following limits:
 *
 * vm_per_task_user_wire_limit
 *      how much memory can be user-wired in one user task
 *
 * vm_global_user_wire_limit (default: same as vm_per_task_user_wire_limit)
 *      how much memory can be user-wired in all user tasks
 *
 * These values are set to defaults based on the number of pages managed
 * by the VM system. They can be overriden via sysctls.
 * See kmem_set_user_wire_limits for details on the default values.
 *
 * Regardless of the amount of memory in the system, we never reserve
 * more than VM_NOT_USER_WIREABLE_MAX bytes as unlockable.
 */
#if defined(__LP64__)
#define VM_NOT_USER_WIREABLE_MAX (32ULL*1024*1024*1024)     /* 32GB */
#else
#define VM_NOT_USER_WIREABLE_MAX (1UL*1024*1024*1024)     /* 1GB */
#endif /* __LP64__ */
extern
vm_map_size_t   vm_per_task_user_wire_limit;
extern
vm_map_size_t   vm_global_user_wire_limit;

/*
 *	Each pageable resident page falls into one of three lists:
 *
 *	free
 *		Available for allocation now.  The free list is
 *		actually an array of lists, one per color.
 *	inactive
 *		Not referenced in any map, but still has an
 *		object/offset-page mapping, and may be dirty.
 *		This is the list of pages that should be
 *		paged out next.  There are actually two
 *		inactive lists, one for pages brought in from
 *		disk or other backing store, and another
 *		for "zero-filled" pages.  See vm_pageout_scan()
 *		for the distinction and usage.
 *	active
 *		A list of pages which have been placed in
 *		at least one physical map.  This list is
 *		ordered, in LRU-like fashion.
 */


#define VPL_LOCK_SPIN 1

struct vpl {
	vm_page_queue_head_t    vpl_queue;
	unsigned int    vpl_count;
	unsigned int    vpl_internal_count;
	unsigned int    vpl_external_count;
#ifdef  VPL_LOCK_SPIN
	lck_spin_t      vpl_lock;
#else
	lck_mtx_t       vpl_lock;
	lck_mtx_ext_t   vpl_lock_ext;
#endif
};

struct  vplq {
	union {
		char   cache_line_pad[VM_VPLQ_ALIGNMENT];
		struct vpl vpl;
	} vpl_un;
};
extern
unsigned int    vm_page_local_q_count;
extern
struct vplq     *vm_page_local_q;
extern
unsigned int    vm_page_local_q_soft_limit;
extern
unsigned int    vm_page_local_q_hard_limit;
extern
vm_locks_array_t vm_page_locks;

extern
vm_page_queue_head_t    vm_lopage_queue_free;           /* low memory free queue */
extern
vm_page_queue_head_t    vm_page_queue_active;   /* active memory queue */
extern
vm_page_queue_head_t    vm_page_queue_inactive; /* inactive memory queue for normal pages */
#if CONFIG_SECLUDED_MEMORY
extern
vm_page_queue_head_t    vm_page_queue_secluded; /* reclaimable pages secluded for Camera */
#endif /* CONFIG_SECLUDED_MEMORY */
extern
vm_page_queue_head_t    vm_page_queue_cleaned; /* clean-queue inactive memory */
extern
vm_page_queue_head_t    vm_page_queue_anonymous;        /* inactive memory queue for anonymous pages */
extern
vm_page_queue_head_t    vm_page_queue_throttled;        /* memory queue for throttled pageout pages */

extern
queue_head_t    vm_objects_wired;
extern
lck_spin_t      vm_objects_wired_lock;

#if CONFIG_BACKGROUND_QUEUE

#define VM_PAGE_BACKGROUND_TARGET_MAX   50000

#define VM_PAGE_BG_DISABLED     0
#define VM_PAGE_BG_LEVEL_1      1

extern
vm_page_queue_head_t    vm_page_queue_background;
extern
uint64_t        vm_page_background_promoted_count;
extern
uint32_t        vm_page_background_count;
extern
uint32_t        vm_page_background_target;
extern
uint32_t        vm_page_background_internal_count;
extern
uint32_t        vm_page_background_external_count;
extern
uint32_t        vm_page_background_mode;
extern
uint32_t        vm_page_background_exclude_external;

#endif

extern
vm_offset_t     first_phys_addr;        /* physical address for first_page */
extern
vm_offset_t     last_phys_addr;         /* physical address for last_page */

extern
unsigned int    vm_page_free_count;     /* How many pages are free? (sum of all colors) */
extern
unsigned int    vm_page_active_count;   /* How many pages are active? */
extern
unsigned int    vm_page_inactive_count; /* How many pages are inactive? */
#if CONFIG_SECLUDED_MEMORY
extern
unsigned int    vm_page_secluded_count; /* How many pages are secluded? */
extern
unsigned int    vm_page_secluded_count_free; /* how many of them are free? */
extern
unsigned int    vm_page_secluded_count_inuse; /* how many of them are in use? */
/*
 * We keep filling the secluded pool with new eligible pages and
 * we can overshoot our target by a lot.
 * When there's memory pressure, vm_pageout_scan() will re-balance the queues,
 * pushing the extra secluded pages to the active or free queue.
 * Since these "over target" secluded pages are actually "available", jetsam
 * should consider them as such, so make them visible to jetsam via the
 * "vm_page_secluded_count_over_target" counter and update it whenever we
 * update vm_page_secluded_count or vm_page_secluded_target.
 */
extern
unsigned int    vm_page_secluded_count_over_target;
#define VM_PAGE_SECLUDED_COUNT_OVER_TARGET_UPDATE()                     \
	MACRO_BEGIN                                                     \
	if (vm_page_secluded_count > vm_page_secluded_target) {         \
	        vm_page_secluded_count_over_target =                    \
	                (vm_page_secluded_count - vm_page_secluded_target); \
	} else {                                                        \
	        vm_page_secluded_count_over_target = 0;                 \
	}                                                               \
	MACRO_END
#define VM_PAGE_SECLUDED_COUNT_OVER_TARGET() vm_page_secluded_count_over_target
#else /* CONFIG_SECLUDED_MEMORY */
#define VM_PAGE_SECLUDED_COUNT_OVER_TARGET_UPDATE() \
	MACRO_BEGIN                                 \
	MACRO_END
#define VM_PAGE_SECLUDED_COUNT_OVER_TARGET() 0
#endif /* CONFIG_SECLUDED_MEMORY */
extern
unsigned int    vm_page_cleaned_count; /* How many pages are in the clean queue? */
extern
unsigned int    vm_page_throttled_count;/* How many inactives are throttled */
extern
unsigned int    vm_page_speculative_count;      /* How many speculative pages are unclaimed? */
extern unsigned int     vm_page_pageable_internal_count;
extern unsigned int     vm_page_pageable_external_count;
extern
unsigned int    vm_page_xpmapped_external_count;        /* How many pages are mapped executable? */
extern
unsigned int    vm_page_external_count; /* How many pages are file-backed? */
extern
unsigned int    vm_page_internal_count; /* How many pages are anonymous? */
extern
unsigned int    vm_page_wire_count;             /* How many pages are wired? */
extern
unsigned int    vm_page_wire_count_initial;     /* How many pages wired at startup */
extern
unsigned int    vm_page_wire_count_on_boot;     /* even earlier than _initial */
extern
unsigned int    vm_page_free_target;    /* How many do we want free? */
extern
unsigned int    vm_page_free_min;       /* When to wakeup pageout */
extern
unsigned int    vm_page_throttle_limit; /* When to throttle new page creation */
extern
unsigned int    vm_page_inactive_target;/* How many do we want inactive? */
#if CONFIG_SECLUDED_MEMORY
extern
unsigned int    vm_page_secluded_target;/* How many do we want secluded? */
#endif /* CONFIG_SECLUDED_MEMORY */
extern
unsigned int    vm_page_anonymous_min;  /* When it's ok to pre-clean */
extern
unsigned int    vm_page_free_reserved;  /* How many pages reserved to do pageout */
extern
unsigned int    vm_page_gobble_count;
extern
unsigned int    vm_page_stolen_count;   /* Count of stolen pages not acccounted in zones */
extern
unsigned int    vm_page_kern_lpage_count;   /* Count of large pages used in early boot */


#if DEVELOPMENT || DEBUG
extern
unsigned int    vm_page_speculative_used;
#endif

extern
unsigned int    vm_page_purgeable_count;/* How many pages are purgeable now ? */
extern
unsigned int    vm_page_purgeable_wired_count;/* How many purgeable pages are wired now ? */
extern
uint64_t        vm_page_purged_count;   /* How many pages got purged so far ? */

extern unsigned int     vm_page_free_wanted;
/* how many threads are waiting for memory */

extern unsigned int     vm_page_free_wanted_privileged;
/* how many VM privileged threads are waiting for memory */
#if CONFIG_SECLUDED_MEMORY
extern unsigned int     vm_page_free_wanted_secluded;
/* how many threads are waiting for secluded memory */
#endif /* CONFIG_SECLUDED_MEMORY */

extern const ppnum_t    vm_page_fictitious_addr;
/* (fake) phys_addr of fictitious pages */

extern const ppnum_t    vm_page_guard_addr;
/* (fake) phys_addr of guard pages */


extern boolean_t        vm_page_deactivate_hint;

extern int              vm_compressor_mode;

/*
 * Defaults to true, so highest memory is used first.
 */
extern boolean_t        vm_himemory_mode;

extern boolean_t        vm_lopage_needed;
extern uint32_t         vm_lopage_free_count;
extern uint32_t         vm_lopage_free_limit;
extern uint32_t         vm_lopage_lowater;
extern boolean_t        vm_lopage_refill;
extern uint64_t         max_valid_dma_address;
extern ppnum_t          max_valid_low_ppnum;

/*
 * Prototypes for functions exported by this module.
 */
extern void             vm_page_bootstrap(
	vm_offset_t     *startp,
	vm_offset_t     *endp);

extern void             vm_page_module_init(void);

extern void             vm_page_init_local_q(void);

extern void             vm_page_create(
	ppnum_t         start,
	ppnum_t         end);

extern vm_page_t        kdp_vm_page_lookup(
	vm_object_t             object,
	vm_object_offset_t      offset);

extern vm_page_t        vm_page_lookup(
	vm_object_t             object,
	vm_object_offset_t      offset);

extern vm_page_t        vm_page_grab_fictitious(void);

extern vm_page_t        vm_page_grab_guard(void);

extern void             vm_page_release_fictitious(
	vm_page_t page);

extern void             vm_free_delayed_pages(void);

extern void             vm_page_more_fictitious(void);

extern int              vm_pool_low(void);

extern vm_page_t        vm_page_grab(void);
extern vm_page_t        vm_page_grab_options(int flags);

#define VM_PAGE_GRAB_OPTIONS_NONE 0x00000000
#if CONFIG_SECLUDED_MEMORY
#define VM_PAGE_GRAB_SECLUDED     0x00000001
#endif /* CONFIG_SECLUDED_MEMORY */
#define VM_PAGE_GRAB_Q_LOCK_HELD  0x00000002

extern vm_page_t        vm_page_grablo(void);

extern void             vm_page_release(
	vm_page_t       page,
	boolean_t       page_queues_locked);

extern boolean_t        vm_page_wait(
	int             interruptible );

extern vm_page_t        vm_page_alloc(
	vm_object_t             object,
	vm_object_offset_t      offset);

extern vm_page_t        vm_page_alloc_guard(
	vm_object_t             object,
	vm_object_offset_t      offset);

extern void             vm_page_init(
	vm_page_t       page,
	ppnum_t         phys_page,
	boolean_t       lopage);

extern void             vm_page_free(
	vm_page_t       page);

extern void             vm_page_free_unlocked(
	vm_page_t       page,
	boolean_t       remove_from_hash);

extern void             vm_page_balance_inactive(
	int             max_to_move);

extern void             vm_page_activate(
	vm_page_t       page);

extern void             vm_page_deactivate(
	vm_page_t       page);

extern void             vm_page_deactivate_internal(
	vm_page_t       page,
	boolean_t       clear_hw_reference);

extern void             vm_page_enqueue_cleaned(vm_page_t page);

extern void             vm_page_lru(
	vm_page_t       page);

extern void             vm_page_speculate(
	vm_page_t       page,
	boolean_t       new);

extern void             vm_page_speculate_ageit(
	struct vm_speculative_age_q *aq);

extern void             vm_page_reactivate_all_throttled(void);

extern void             vm_page_reactivate_local(uint32_t lid, boolean_t force, boolean_t nolocks);

extern void             vm_page_rename(
	vm_page_t               page,
	vm_object_t             new_object,
	vm_object_offset_t      new_offset);

extern void             vm_page_insert(
	vm_page_t               page,
	vm_object_t             object,
	vm_object_offset_t      offset);

extern void             vm_page_insert_wired(
	vm_page_t               page,
	vm_object_t             object,
	vm_object_offset_t      offset,
	vm_tag_t                tag);

extern void             vm_page_insert_internal(
	vm_page_t               page,
	vm_object_t             object,
	vm_object_offset_t      offset,
	vm_tag_t                tag,
	boolean_t               queues_lock_held,
	boolean_t               insert_in_hash,
	boolean_t               batch_pmap_op,
	boolean_t               delayed_accounting,
	uint64_t                *delayed_ledger_update);

extern void             vm_page_replace(
	vm_page_t               mem,
	vm_object_t             object,
	vm_object_offset_t      offset);

extern void             vm_page_remove(
	vm_page_t       page,
	boolean_t       remove_from_hash);

extern void             vm_page_zero_fill(
	vm_page_t       page);

extern void             vm_page_part_zero_fill(
	vm_page_t       m,
	vm_offset_t     m_pa,
	vm_size_t       len);

extern void             vm_page_copy(
	vm_page_t       src_page,
	vm_page_t       dest_page);

extern void             vm_page_part_copy(
	vm_page_t       src_m,
	vm_offset_t     src_pa,
	vm_page_t       dst_m,
	vm_offset_t     dst_pa,
	vm_size_t       len);

extern void             vm_page_wire(
	vm_page_t       page,
	vm_tag_t        tag,
	boolean_t       check_memorystatus);

extern void             vm_page_unwire(
	vm_page_t       page,
	boolean_t       queueit);

extern void             vm_set_page_size(void);

extern void             vm_page_gobble(
	vm_page_t      page);

extern void             vm_page_validate_cs(vm_page_t   page);
extern void             vm_page_validate_cs_mapped(
	vm_page_t       page,
	const void      *kaddr);
extern void             vm_page_validate_cs_mapped_slow(
	vm_page_t       page,
	const void      *kaddr);
extern void             vm_page_validate_cs_mapped_chunk(
	vm_page_t       page,
	const void      *kaddr,
	vm_offset_t     chunk_offset,
	vm_size_t       chunk_size,
	boolean_t       *validated,
	unsigned        *tainted);

extern void             vm_page_free_prepare_queues(
	vm_page_t       page);

extern void             vm_page_free_prepare_object(
	vm_page_t       page,
	boolean_t       remove_from_hash);

#if CONFIG_IOSCHED
extern wait_result_t    vm_page_sleep(
	vm_object_t     object,
	vm_page_t       m,
	int     interruptible);
#endif

extern void vm_pressure_response(void);

#if CONFIG_JETSAM
extern void memorystatus_pages_update(unsigned int pages_avail);

#define VM_CHECK_MEMORYSTATUS do { \
	memorystatus_pages_update(              \
	        vm_page_pageable_external_count + \
	        vm_page_free_count +            \
	        VM_PAGE_SECLUDED_COUNT_OVER_TARGET() + \
	        (VM_DYNAMIC_PAGING_ENABLED() ? 0 : vm_page_purgeable_count) \
	        ); \
	} while(0)

#else /* CONFIG_JETSAM */

#if CONFIG_EMBEDDED

#define VM_CHECK_MEMORYSTATUS do {} while(0)

#else /* CONFIG_EMBEDDED */

#define VM_CHECK_MEMORYSTATUS   vm_pressure_response()

#endif /* CONFIG_EMBEDDED */

#endif /* CONFIG_JETSAM */

/*
 * Functions implemented as macros. m->vmp_wanted and m->vmp_busy are
 * protected by the object lock.
 */

#if CONFIG_EMBEDDED
#define SET_PAGE_DIRTY(m, set_pmap_modified)                            \
	        MACRO_BEGIN                                             \
	        vm_page_t __page__ = (m);                               \
	        if (__page__->vmp_pmapped == TRUE &&                    \
	            __page__->vmp_wpmapped == TRUE &&                   \
	            __page__->vmp_dirty == FALSE &&                     \
	            (set_pmap_modified)) {                              \
	                pmap_set_modify(VM_PAGE_GET_PHYS_PAGE(__page__)); \
	        }                                                       \
	        __page__->vmp_dirty = TRUE;                             \
	        MACRO_END
#else /* CONFIG_EMBEDDED */
#define SET_PAGE_DIRTY(m, set_pmap_modified)                            \
	        MACRO_BEGIN                                             \
	        vm_page_t __page__ = (m);                               \
	        __page__->vmp_dirty = TRUE;                             \
	        MACRO_END
#endif /* CONFIG_EMBEDDED */

#define PAGE_ASSERT_WAIT(m, interruptible)                      \
	        (((m)->vmp_wanted = TRUE),                      \
	         assert_wait((event_t) (m), (interruptible)))

#if CONFIG_IOSCHED
#define PAGE_SLEEP(o, m, interruptible)                         \
	        vm_page_sleep(o, m, interruptible)
#else
#define PAGE_SLEEP(o, m, interruptible)                         \
	(((m)->vmp_wanted = TRUE),                              \
	 thread_sleep_vm_object((o), (m), (interruptible)))
#endif

#define PAGE_WAKEUP_DONE(m)                                     \
	        MACRO_BEGIN                                     \
	        (m)->vmp_busy = FALSE;                          \
	        if ((m)->vmp_wanted) {                          \
	                (m)->vmp_wanted = FALSE;                \
	                thread_wakeup((event_t) (m));           \
	        }                                               \
	        MACRO_END

#define PAGE_WAKEUP(m)                                          \
	        MACRO_BEGIN                                     \
	        if ((m)->vmp_wanted) {                          \
	                (m)->vmp_wanted = FALSE;                \
	                thread_wakeup((event_t) (m));           \
	        }                                               \
	        MACRO_END

#define VM_PAGE_FREE(p)                         \
	        MACRO_BEGIN                     \
	        vm_page_free_unlocked(p, TRUE); \
	        MACRO_END

#define VM_PAGE_GRAB_FICTITIOUS(M)                                      \
	        MACRO_BEGIN                                             \
	        while ((M = vm_page_grab_fictitious()) == VM_PAGE_NULL) \
	                vm_page_more_fictitious();                      \
	        MACRO_END

#define VM_PAGE_WAIT()          ((void)vm_page_wait(THREAD_UNINT))

#define vm_page_queue_lock (vm_page_locks.vm_page_queue_lock2)
#define vm_page_queue_free_lock (vm_page_locks.vm_page_queue_free_lock2)

#define vm_page_lock_queues()   lck_mtx_lock(&vm_page_queue_lock)
#define vm_page_trylock_queues() lck_mtx_try_lock(&vm_page_queue_lock)
#define vm_page_unlock_queues() lck_mtx_unlock(&vm_page_queue_lock)

#define vm_page_lockspin_queues()       lck_mtx_lock_spin(&vm_page_queue_lock)
#define vm_page_trylockspin_queues()    lck_mtx_try_lock_spin(&vm_page_queue_lock)
#define vm_page_lockconvert_queues()    lck_mtx_convert_spin(&vm_page_queue_lock)

#ifdef  VPL_LOCK_SPIN
extern lck_grp_t vm_page_lck_grp_local;

#define VPL_LOCK_INIT(vlq, vpl_grp, vpl_attr) lck_spin_init(&vlq->vpl_lock, vpl_grp, vpl_attr)
#define VPL_LOCK(vpl) lck_spin_lock_grp(vpl, &vm_page_lck_grp_local)
#define VPL_UNLOCK(vpl) lck_spin_unlock(vpl)
#else
#define VPL_LOCK_INIT(vlq, vpl_grp, vpl_attr) lck_mtx_init_ext(&vlq->vpl_lock, &vlq->vpl_lock_ext, vpl_grp, vpl_attr)
#define VPL_LOCK(vpl) lck_mtx_lock_spin(vpl)
#define VPL_UNLOCK(vpl) lck_mtx_unlock(vpl)
#endif


#if DEVELOPMENT || DEBUG
#define VM_PAGE_SPECULATIVE_USED_ADD()                          \
	MACRO_BEGIN                                             \
	OSAddAtomic(1, &vm_page_speculative_used);              \
	MACRO_END
#else
#define VM_PAGE_SPECULATIVE_USED_ADD()
#endif


#define VM_PAGE_CONSUME_CLUSTERED(mem)                          \
	MACRO_BEGIN                                             \
	ppnum_t	__phys_page;                                    \
	__phys_page = VM_PAGE_GET_PHYS_PAGE(mem);               \
	pmap_lock_phys_page(__phys_page);                       \
	if (mem->vmp_clustered) {                               \
	        vm_object_t o;                                  \
	        o = VM_PAGE_OBJECT(mem);                        \
	        assert(o);                                      \
	        o->pages_used++;                                \
	        mem->vmp_clustered = FALSE;                     \
	        VM_PAGE_SPECULATIVE_USED_ADD();                 \
	}                                                       \
	pmap_unlock_phys_page(__phys_page);                     \
	MACRO_END


#define VM_PAGE_COUNT_AS_PAGEIN(mem)                            \
	MACRO_BEGIN                                             \
	{                                                       \
	vm_object_t o;                                          \
	o = VM_PAGE_OBJECT(mem);                                \
	DTRACE_VM2(pgin, int, 1, (uint64_t *), NULL);           \
	current_task()->pageins++;                              \
	if (o->internal) {                                      \
	        DTRACE_VM2(anonpgin, int, 1, (uint64_t *), NULL);       \
	} else {                                                \
	        DTRACE_VM2(fspgin, int, 1, (uint64_t *), NULL); \
	}                                                       \
	}                                                       \
	MACRO_END

/* adjust for stolen pages accounted elsewhere */
#define VM_PAGE_MOVE_STOLEN(page_count)                         \
	MACRO_BEGIN                                             \
	vm_page_stolen_count -=	(page_count);                   \
	vm_page_wire_count_initial -= (page_count);             \
	MACRO_END

#define DW_vm_page_unwire               0x01
#define DW_vm_page_wire                 0x02
#define DW_vm_page_free                 0x04
#define DW_vm_page_activate             0x08
#define DW_vm_page_deactivate_internal  0x10
#define DW_vm_page_speculate            0x20
#define DW_vm_page_lru                  0x40
#define DW_vm_pageout_throttle_up       0x80
#define DW_PAGE_WAKEUP                  0x100
#define DW_clear_busy                   0x200
#define DW_clear_reference              0x400
#define DW_set_reference                0x800
#define DW_move_page                    0x1000
#define DW_VM_PAGE_QUEUES_REMOVE        0x2000
#define DW_enqueue_cleaned              0x4000
#define DW_vm_phantom_cache_update      0x8000

struct vm_page_delayed_work {
	vm_page_t       dw_m;
	int             dw_mask;
};

void vm_page_do_delayed_work(vm_object_t object, vm_tag_t tag, struct vm_page_delayed_work *dwp, int dw_count);

extern unsigned int vm_max_delayed_work_limit;

#define DEFAULT_DELAYED_WORK_LIMIT      32

#define DELAYED_WORK_LIMIT(max) ((vm_max_delayed_work_limit >= max ? max : vm_max_delayed_work_limit))

/*
 * vm_page_do_delayed_work may need to drop the object lock...
 * if it does, we need the pages it's looking at to
 * be held stable via the busy bit, so if busy isn't already
 * set, we need to set it and ask vm_page_do_delayed_work
 * to clear it and wakeup anyone that might have blocked on
 * it once we're done processing the page.
 */

#define VM_PAGE_ADD_DELAYED_WORK(dwp, mem, dw_cnt)              \
	MACRO_BEGIN                                             \
	if (mem->vmp_busy == FALSE) {                           \
	        mem->vmp_busy = TRUE;                           \
	        if ( !(dwp->dw_mask & DW_vm_page_free))         \
	                dwp->dw_mask |= (DW_clear_busy | DW_PAGE_WAKEUP); \
	}                                                       \
	dwp->dw_m = mem;                                        \
	dwp++;                                                  \
	dw_cnt++;                                               \
	MACRO_END

extern vm_page_t vm_object_page_grab(vm_object_t);

#if VM_PAGE_BUCKETS_CHECK
extern void vm_page_buckets_check(void);
#endif /* VM_PAGE_BUCKETS_CHECK */

extern void vm_page_queues_remove(vm_page_t mem, boolean_t remove_from_backgroundq);
extern void vm_page_remove_internal(vm_page_t page);
extern void vm_page_enqueue_inactive(vm_page_t mem, boolean_t first);
extern void vm_page_enqueue_active(vm_page_t mem, boolean_t first);
extern void vm_page_check_pageable_safe(vm_page_t page);

#if CONFIG_SECLUDED_MEMORY
extern uint64_t secluded_shutoff_trigger;
extern void start_secluded_suppression(task_t);
extern void stop_secluded_suppression(task_t);
#endif /* CONFIG_SECLUDED_MEMORY */


#endif  /* _VM_VM_PAGE_H_ */
