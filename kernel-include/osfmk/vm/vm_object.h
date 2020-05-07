/*
 * Copyright (c) 2000-2007 Apple Inc. All rights reserved.
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
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 *	File:	vm_object.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1985
 *
 *	Virtual memory object module definitions.
 */

#ifndef _VM_VM_OBJECT_H_
#define _VM_VM_OBJECT_H_

#include <debug.h>
#include <mach_assert.h>
#include <mach_pagemap.h>
#include <task_swapper.h>

#include <mach/kern_return.h>
#include <mach/boolean.h>
#include <mach/memory_object_types.h>
#include <mach/port.h>
#include <mach/vm_prot.h>
#include <mach/vm_param.h>
#include <mach/machine/vm_types.h>
#include <kern/queue.h>
#include <kern/locks.h>
#include <kern/assert.h>
#include <kern/misc_protos.h>
#include <kern/macro_help.h>
#include <ipc/ipc_types.h>
#include <vm/pmap.h>

#include <vm/vm_external.h>

#include <vm/vm_options.h>
#include <vm/vm_page.h>

#if VM_OBJECT_TRACKING
#include <libkern/OSDebug.h>
#include <kern/btlog.h>
extern void vm_object_tracking_init(void);
extern boolean_t vm_object_tracking_inited;
extern btlog_t *vm_object_tracking_btlog;
#define VM_OBJECT_TRACKING_NUM_RECORDS  50000
#define VM_OBJECT_TRACKING_BTDEPTH 7
#define VM_OBJECT_TRACKING_OP_CREATED   1
#define VM_OBJECT_TRACKING_OP_MODIFIED  2
#define VM_OBJECT_TRACKING_OP_TRUESHARE 3
#endif /* VM_OBJECT_TRACKING */

struct vm_page;

/*
 *	Types defined:
 *
 *	vm_object_t		Virtual memory object.
 *	vm_object_fault_info_t	Used to determine cluster size.
 */

struct vm_object_fault_info {
	int             interruptible;
	uint32_t        user_tag;
	vm_size_t       cluster_size;
	vm_behavior_t   behavior;
	vm_map_offset_t lo_offset;
	vm_map_offset_t hi_offset;
	unsigned int
	/* boolean_t */ no_cache:1,
	/* boolean_t */ stealth:1,
	/* boolean_t */ io_sync:1,
	/* boolean_t */ cs_bypass:1,
	/* boolean_t */ pmap_cs_associated:1,
	/* boolean_t */ mark_zf_absent:1,
	/* boolean_t */ batch_pmap_op:1,
	/* boolean_t */ resilient_media:1,
	/* boolean_t */ no_copy_on_read:1,
	    __vm_object_fault_info_unused_bits:23;
	int             pmap_options;
};


#define vo_size                         vo_un1.vou_size
#define vo_cache_pages_to_scan          vo_un1.vou_cache_pages_to_scan
#define vo_shadow_offset                vo_un2.vou_shadow_offset
#define vo_cache_ts                     vo_un2.vou_cache_ts
#define vo_owner                        vo_un2.vou_owner

struct vm_object {
	/*
	 * on 64 bit systems we pack the pointers hung off the memq.
	 * those pointers have to be able to point back to the memq.
	 * the packed pointers are required to be on a 64 byte boundary
	 * which means 2 things for the vm_object...  (1) the memq
	 * struct has to be the first element of the structure so that
	 * we can control it's alignment... (2) the vm_object must be
	 * aligned on a 64 byte boundary... for static vm_object's
	 * this is accomplished via the 'aligned' attribute... for
	 * vm_object's in the zone pool, this is accomplished by
	 * rounding the size of the vm_object element to the nearest
	 * 64 byte size before creating the zone.
	 */
	vm_page_queue_head_t    memq;           /* Resident memory - must be first */
	lck_rw_t                Lock;           /* Synchronization */

#if DEVELOPMENT || DEBUG
	thread_t                Lock_owner;
#endif
	union {
		vm_object_size_t  vou_size;     /* Object size (only valid if internal) */
		int               vou_cache_pages_to_scan;      /* pages yet to be visited in an
		                                                 * external object in cache
		                                                 */
	} vo_un1;

	struct vm_page          *memq_hint;
	int                     ref_count;      /* Number of references */
	unsigned int            resident_page_count;
	/* number of resident pages */
	unsigned int            wired_page_count; /* number of wired pages
	                                           *  use VM_OBJECT_WIRED_PAGE_UPDATE macros to update */
	unsigned int            reusable_page_count;

	struct vm_object        *copy;          /* Object that should receive
	                                         * a copy of my changed pages,
	                                         * for copy_delay, or just the
	                                         * temporary object that
	                                         * shadows this object, for
	                                         * copy_call.
	                                         */
	struct vm_object        *shadow;        /* My shadow */
	memory_object_t         pager;          /* Where to get data */

	union {
		vm_object_offset_t vou_shadow_offset;   /* Offset into shadow */
		clock_sec_t     vou_cache_ts;   /* age of an external object
		                                 * present in cache
		                                 */
		task_t          vou_owner;      /* If the object is purgeable
		                                 * or has a "ledger_tag", this
		                                 * is the task that owns it.
		                                 */
	} vo_un2;

	vm_object_offset_t      paging_offset;  /* Offset into memory object */
	memory_object_control_t pager_control;  /* Where data comes back */

	memory_object_copy_strategy_t
	    copy_strategy;                      /* How to handle data copy */

#if __LP64__
	/*
	 * Some user processes (mostly VirtualMachine software) take a large
	 * number of UPLs (via IOMemoryDescriptors) to wire pages in large
	 * VM objects and overflow the 16-bit "activity_in_progress" counter.
	 * Since we never enforced any limit there, let's give them 32 bits
	 * for backwards compatibility's sake.
	 */
	unsigned int            paging_in_progress:16,
	    __object1_unused_bits:16;
	unsigned int            activity_in_progress;
#else /* __LP64__ */
	/*
	 * On 32-bit platforms, enlarging "activity_in_progress" would increase
	 * the size of "struct vm_object".  Since we don't know of any actual
	 * overflow of these counters on these platforms, let's keep the
	 * counters as 16-bit integers.
	 */
	unsigned short          paging_in_progress;
	unsigned short          activity_in_progress;
#endif /* __LP64__ */
	/* The memory object ports are
	 * being used (e.g., for pagein
	 * or pageout) -- don't change
	 * any of these fields (i.e.,
	 * don't collapse, destroy or
	 * terminate)
	 */

	unsigned int
	/* boolean_t array */ all_wanted:11,    /* Bit array of "want to be
	                                         * awakened" notations.  See
	                                         * VM_OBJECT_EVENT_* items
	                                         * below */
	/* boolean_t */ pager_created:1,        /* Has pager been created? */
	/* boolean_t */ pager_initialized:1,    /* Are fields ready to use? */
	/* boolean_t */ pager_ready:1,          /* Will pager take requests? */

	/* boolean_t */ pager_trusted:1,        /* The pager for this object
	                                         * is trusted. This is true for
	                                         * all internal objects (backed
	                                         * by the default pager)
	                                         */
	/* boolean_t */ can_persist:1,          /* The kernel may keep the data
	                                         * for this object (and rights
	                                         * to the memory object) after
	                                         * all address map references
	                                         * are deallocated?
	                                         */
	/* boolean_t */ internal:1,             /* Created by the kernel (and
	                                         * therefore, managed by the
	                                         * default memory manger)
	                                         */
	/* boolean_t */ private:1,              /* magic device_pager object,
	                                        * holds private pages only */
	/* boolean_t */ pageout:1,              /* pageout object. contains
	                                         * private pages that refer to
	                                         * a real memory object. */
	/* boolean_t */ alive:1,                /* Not yet terminated */

	/* boolean_t */ purgable:2,             /* Purgable state.  See
	                                         * VM_PURGABLE_*
	                                         */
	/* boolean_t */ purgeable_only_by_kernel:1,
	/* boolean_t */ purgeable_when_ripe:1,         /* Purgeable when a token
	                                                * becomes ripe.
	                                                */
	/* boolean_t */ shadowed:1,             /* Shadow may exist */
	/* boolean_t */ true_share:1,
	/* This object is mapped
	 * in more than one place
	 * and hence cannot be
	 * coalesced */
	/* boolean_t */ terminating:1,
	/* Allows vm_object_lookup
	 * and vm_object_deallocate
	 * to special case their
	 * behavior when they are
	 * called as a result of
	 * page cleaning during
	 * object termination
	 */
	/* boolean_t */ named:1,                /* An enforces an internal
	                                         * naming convention, by
	                                         * calling the right routines
	                                         * for allocation and
	                                         * destruction, UBC references
	                                         * against the vm_object are
	                                         * checked.
	                                         */
	/* boolean_t */ shadow_severed:1,
	/* When a permanent object
	 * backing a COW goes away
	 * unexpectedly.  This bit
	 * allows vm_fault to return
	 * an error rather than a
	 * zero filled page.
	 */
	/* boolean_t */ phys_contiguous:1,
	/* Memory is wired and
	 * guaranteed physically
	 * contiguous.  However
	 * it is not device memory
	 * and obeys normal virtual
	 * memory rules w.r.t pmap
	 * access bits.
	 */
	/* boolean_t */ nophyscache:1,
	/* When mapped at the
	 * pmap level, don't allow
	 * primary caching. (for
	 * I/O)
	 */
	/* boolean_t */ _object5_unused_bits:1;

	queue_chain_t           cached_list;    /* Attachment point for the
	                                         * list of objects cached as a
	                                         * result of their can_persist
	                                         * value
	                                         */
	/*
	 * the following fields are not protected by any locks
	 * they are updated via atomic compare and swap
	 */
	vm_object_offset_t      last_alloc;     /* last allocation offset */
	vm_offset_t             cow_hint;       /* last page present in     */
	                                        /* shadow but not in object */
	int                     sequential;     /* sequential access size */

	uint32_t                pages_created;
	uint32_t                pages_used;
	/* hold object lock when altering */
	unsigned        int
	    wimg_bits:8,                /* cache WIMG bits         */
	    code_signed:1,              /* pages are signed and should be
	                                 *  validated; the signatures are stored
	                                 *  with the pager */
	    transposed:1,               /* object was transposed with another */
	    mapping_in_progress:1,      /* pager being mapped/unmapped */
	    phantom_isssd:1,
	    volatile_empty:1,
	    volatile_fault:1,
	    all_reusable:1,
	    blocked_access:1,
	    set_cache_attr:1,
	    object_is_shared_cache:1,
	    purgeable_queue_type:2,
	    purgeable_queue_group:3,
	    io_tracking:1,
	    no_tag_update:1,            /*  */
#if CONFIG_SECLUDED_MEMORY
	    eligible_for_secluded:1,
	    can_grab_secluded:1,
#else /* CONFIG_SECLUDED_MEMORY */
	__object3_unused_bits:2,
#endif /* CONFIG_SECLUDED_MEMORY */
#if VM_OBJECT_ACCESS_TRACKING
	    access_tracking:1,
#else /* VM_OBJECT_ACCESS_TRACKING */
	__unused_access_tracking:1,
#endif /* VM_OBJECT_ACCESS_TRACKING */
	vo_ledger_tag:3,
	    vo_no_footprint:1;

#if VM_OBJECT_ACCESS_TRACKING
	uint32_t        access_tracking_reads;
	uint32_t        access_tracking_writes;
#endif /* VM_OBJECT_ACCESS_TRACKING */

	uint8_t                 scan_collisions;
	uint8_t                 __object4_unused_bits[1];
	vm_tag_t                wire_tag;

#if CONFIG_PHANTOM_CACHE
	uint32_t                phantom_object_id;
#endif
#if CONFIG_IOSCHED || UPL_DEBUG
	queue_head_t            uplq;           /* List of outstanding upls */
#endif

#ifdef  VM_PIP_DEBUG
/*
 * Keep track of the stack traces for the first holders
 * of a "paging_in_progress" reference for this VM object.
 */
#define VM_PIP_DEBUG_STACK_FRAMES       25      /* depth of each stack trace */
#define VM_PIP_DEBUG_MAX_REFS           10      /* track that many references */
	struct __pip_backtrace {
		void *pip_retaddr[VM_PIP_DEBUG_STACK_FRAMES];
	} pip_holders[VM_PIP_DEBUG_MAX_REFS];
#endif  /* VM_PIP_DEBUG  */

	queue_chain_t           objq;      /* object queue - currently used for purgable queues */
	queue_chain_t           task_objq; /* objects owned by task - protected by task lock */

#if !VM_TAG_ACTIVE_UPDATE
	queue_chain_t           wired_objq;
#endif /* !VM_TAG_ACTIVE_UPDATE */

#if DEBUG
	void *purgeable_owner_bt[16];
	task_t vo_purgeable_volatilizer; /* who made it volatile? */
	void *purgeable_volatilizer_bt[16];
#endif /* DEBUG */
};

#define VM_OBJECT_PURGEABLE_FAULT_ERROR(object)                         \
	((object)->volatile_fault &&                                    \
	 ((object)->purgable == VM_PURGABLE_VOLATILE ||                 \
	  (object)->purgable == VM_PURGABLE_EMPTY))

#if VM_OBJECT_ACCESS_TRACKING
extern uint64_t vm_object_access_tracking_reads;
extern uint64_t vm_object_access_tracking_writes;
extern void vm_object_access_tracking(vm_object_t object,
    int *access_tracking,
    uint32_t *access_tracking_reads,
    uint32_t *acess_tracking_writes);
#endif /* VM_OBJECT_ACCESS_TRACKING */

extern
vm_object_t     kernel_object;          /* the single kernel object */

extern
vm_object_t     compressor_object;      /* the single compressor object */

extern
unsigned int    vm_object_absent_max;   /* maximum number of absent pages
                                         *  at a time for each object */

# define        VM_MSYNC_INITIALIZED                    0
# define        VM_MSYNC_SYNCHRONIZING                  1
# define        VM_MSYNC_DONE                           2


extern lck_grp_t                vm_map_lck_grp;
extern lck_attr_t               vm_map_lck_attr;

#ifndef VM_TAG_ACTIVE_UPDATE
#error VM_TAG_ACTIVE_UPDATE
#endif

#if VM_TAG_ACTIVE_UPDATE
#define VM_OBJECT_WIRED_ENQUEUE(object) panic("VM_OBJECT_WIRED_ENQUEUE")
#define VM_OBJECT_WIRED_DEQUEUE(object) panic("VM_OBJECT_WIRED_DEQUEUE")
#else /* VM_TAG_ACTIVE_UPDATE */
#define VM_OBJECT_WIRED_ENQUEUE(object)                                 \
	MACRO_BEGIN                                                     \
	lck_spin_lock_grp(&vm_objects_wired_lock, &vm_page_lck_grp_bucket);                          \
	assert(!(object)->wired_objq.next);                             \
	assert(!(object)->wired_objq.prev);                             \
	queue_enter(&vm_objects_wired, (object),                        \
	            vm_object_t, wired_objq);                           \
	lck_spin_unlock(&vm_objects_wired_lock);                        \
	MACRO_END
#define VM_OBJECT_WIRED_DEQUEUE(object)                                 \
	MACRO_BEGIN                                                     \
	if ((object)->wired_objq.next) {                                \
	        lck_spin_lock_grp(&vm_objects_wired_lock, &vm_page_lck_grp_bucket);                  \
	        queue_remove(&vm_objects_wired, (object),               \
	                     vm_object_t, wired_objq);                  \
	        lck_spin_unlock(&vm_objects_wired_lock);                \
	}                                                               \
	MACRO_END
#endif /* VM_TAG_ACTIVE_UPDATE */

#define VM_OBJECT_WIRED(object, tag)                                    \
    MACRO_BEGIN                                                         \
    assert(VM_KERN_MEMORY_NONE != (tag));                               \
    assert(VM_KERN_MEMORY_NONE == (object)->wire_tag);                  \
    (object)->wire_tag = (tag);                                         \
    if (!VM_TAG_ACTIVE_UPDATE) {                                        \
	VM_OBJECT_WIRED_ENQUEUE((object));                              \
    }                                                                   \
    MACRO_END

#define VM_OBJECT_UNWIRED(object)                                                       \
    MACRO_BEGIN                                                                         \
    if (!VM_TAG_ACTIVE_UPDATE) {                                                        \
	    VM_OBJECT_WIRED_DEQUEUE((object));                                          \
    }                                                                                   \
    if (VM_KERN_MEMORY_NONE != (object)->wire_tag) {                                    \
	vm_tag_update_size((object)->wire_tag, -ptoa_64((object)->wired_page_count));   \
	(object)->wire_tag = VM_KERN_MEMORY_NONE;                                       \
    }                                                                                   \
    MACRO_END

// These two macros start & end a C block
#define VM_OBJECT_WIRED_PAGE_UPDATE_START(object)                                       \
    MACRO_BEGIN                                                                         \
    {                                                                                   \
	int64_t __wireddelta = 0; vm_tag_t __waswired = (object)->wire_tag;

#define VM_OBJECT_WIRED_PAGE_UPDATE_END(object, tag)                                    \
	if (__wireddelta) {                                                             \
	    boolean_t __overflow __assert_only =                                        \
	    os_add_overflow((object)->wired_page_count, __wireddelta,                   \
	                    &(object)->wired_page_count);                               \
	    assert(!__overflow);                                                        \
	    if (!(object)->pageout && !(object)->no_tag_update) {                       \
	        if (__wireddelta > 0) {                                                 \
	            assert (VM_KERN_MEMORY_NONE != (tag));                              \
	            if (VM_KERN_MEMORY_NONE == __waswired) {                            \
	                VM_OBJECT_WIRED((object), (tag));                               \
	            }                                                                   \
	            vm_tag_update_size((object)->wire_tag, ptoa_64(__wireddelta));      \
	        } else if (VM_KERN_MEMORY_NONE != __waswired) {                         \
	            assert (VM_KERN_MEMORY_NONE != (object)->wire_tag);                 \
	            vm_tag_update_size((object)->wire_tag, ptoa_64(__wireddelta));      \
	            if (!(object)->wired_page_count) {                                  \
	                VM_OBJECT_UNWIRED((object));                                    \
	            }                                                                   \
	        }                                                                       \
	    }                                                                           \
	}                                                                               \
    }                                                                                   \
    MACRO_END

#define VM_OBJECT_WIRED_PAGE_COUNT(object, delta)               \
    __wireddelta += delta; \

#define VM_OBJECT_WIRED_PAGE_ADD(object, m)                     \
    if (!(m)->vmp_private && !(m)->vmp_fictitious) __wireddelta++;

#define VM_OBJECT_WIRED_PAGE_REMOVE(object, m)                  \
    if (!(m)->vmp_private && !(m)->vmp_fictitious) __wireddelta--;



#define OBJECT_LOCK_SHARED      0
#define OBJECT_LOCK_EXCLUSIVE   1

extern lck_grp_t        vm_object_lck_grp;
extern lck_grp_attr_t   vm_object_lck_grp_attr;
extern lck_attr_t       vm_object_lck_attr;
extern lck_attr_t       kernel_object_lck_attr;
extern lck_attr_t       compressor_object_lck_attr;

extern vm_object_t      vm_pageout_scan_wants_object;

extern void             vm_object_lock(vm_object_t);
extern boolean_t        vm_object_lock_try(vm_object_t);
extern boolean_t        _vm_object_lock_try(vm_object_t);
extern boolean_t        vm_object_lock_avoid(vm_object_t);
extern void             vm_object_lock_shared(vm_object_t);
extern boolean_t        vm_object_lock_yield_shared(vm_object_t);
extern boolean_t        vm_object_lock_try_shared(vm_object_t);
extern void             vm_object_unlock(vm_object_t);
extern boolean_t        vm_object_lock_upgrade(vm_object_t);

/*
 *	Object locking macros
 */

#define vm_object_lock_init(object)                                     \
	lck_rw_init(&(object)->Lock, &vm_object_lck_grp,                \
	            (((object) == kernel_object ||                      \
	              (object) == vm_submap_object) ?                   \
	             &kernel_object_lck_attr :                          \
	             (((object) == compressor_object) ?                 \
	             &compressor_object_lck_attr :                      \
	              &vm_object_lck_attr)))
#define vm_object_lock_destroy(object)  lck_rw_destroy(&(object)->Lock, &vm_object_lck_grp)

#define vm_object_lock_try_scan(object) _vm_object_lock_try(object)

/*
 * CAUTION: the following vm_object_lock_assert_held*() macros merely
 * check if anyone is holding the lock, but the holder may not necessarily
 * be the caller...
 */
#if MACH_ASSERT || DEBUG
#define vm_object_lock_assert_held(object) \
	lck_rw_assert(&(object)->Lock, LCK_RW_ASSERT_HELD)
#define vm_object_lock_assert_shared(object) \
	lck_rw_assert(&(object)->Lock, LCK_RW_ASSERT_SHARED)
#define vm_object_lock_assert_exclusive(object) \
	lck_rw_assert(&(object)->Lock, LCK_RW_ASSERT_EXCLUSIVE)
#define vm_object_lock_assert_notheld(object) \
	lck_rw_assert(&(object)->Lock, LCK_RW_ASSERT_NOTHELD)
#else  /* MACH_ASSERT || DEBUG */
#define vm_object_lock_assert_held(object)
#define vm_object_lock_assert_shared(object)
#define vm_object_lock_assert_exclusive(object)
#define vm_object_lock_assert_notheld(object)
#endif /* MACH_ASSERT || DEBUG */


/*
 *	Declare procedures that operate on VM objects.
 */

__private_extern__ void         vm_object_bootstrap(void);

__private_extern__ void         vm_object_init(void);

__private_extern__ void         vm_object_init_lck_grp(void);

__private_extern__ void         vm_object_reaper_init(void);

__private_extern__ vm_object_t  vm_object_allocate(vm_object_size_t size);

__private_extern__ void    _vm_object_allocate(vm_object_size_t size,
    vm_object_t object);

#if     TASK_SWAPPER

__private_extern__ void vm_object_res_reference(
	vm_object_t             object);
__private_extern__ void vm_object_res_deallocate(
	vm_object_t             object);
#define VM_OBJ_RES_INCR(object) (object)->res_count++
#define VM_OBJ_RES_DECR(object) (object)->res_count--

#else   /* TASK_SWAPPER */

#define VM_OBJ_RES_INCR(object)
#define VM_OBJ_RES_DECR(object)
#define vm_object_res_reference(object)
#define vm_object_res_deallocate(object)

#endif  /* TASK_SWAPPER */

#define vm_object_reference_locked(object)              \
	MACRO_BEGIN                                     \
	vm_object_t RLObject = (object);                \
	vm_object_lock_assert_exclusive(object);        \
	assert((RLObject)->ref_count > 0);              \
	(RLObject)->ref_count++;                        \
	assert((RLObject)->ref_count > 1);              \
	vm_object_res_reference(RLObject);              \
	MACRO_END


#define vm_object_reference_shared(object)                              \
	MACRO_BEGIN                                                     \
	vm_object_t RLObject = (object);                                \
	vm_object_lock_assert_shared(object);                           \
	assert((RLObject)->ref_count > 0);                              \
	OSAddAtomic(1, &(RLObject)->ref_count);         \
	assert((RLObject)->ref_count > 0);                              \
	/* XXX we would need an atomic version of the following ... */  \
	vm_object_res_reference(RLObject);                              \
	MACRO_END


__private_extern__ void         vm_object_reference(
	vm_object_t     object);

#if     !MACH_ASSERT

#define vm_object_reference(object)                     \
MACRO_BEGIN                                             \
	vm_object_t RObject = (object);                 \
	if (RObject) {                                  \
	        vm_object_lock_shared(RObject);         \
	        vm_object_reference_shared(RObject);    \
	        vm_object_unlock(RObject);              \
	}                                               \
MACRO_END

#endif  /* MACH_ASSERT */

__private_extern__ void         vm_object_deallocate(
	vm_object_t     object);

__private_extern__ kern_return_t vm_object_release_name(
	vm_object_t     object,
	int             flags);

__private_extern__ void         vm_object_pmap_protect(
	vm_object_t             object,
	vm_object_offset_t      offset,
	vm_object_size_t        size,
	pmap_t                  pmap,
	vm_map_offset_t         pmap_start,
	vm_prot_t               prot);

__private_extern__ void         vm_object_pmap_protect_options(
	vm_object_t             object,
	vm_object_offset_t      offset,
	vm_object_size_t        size,
	pmap_t                  pmap,
	vm_map_offset_t         pmap_start,
	vm_prot_t               prot,
	int                     options);

__private_extern__ void         vm_object_page_remove(
	vm_object_t             object,
	vm_object_offset_t      start,
	vm_object_offset_t      end);

__private_extern__ void         vm_object_deactivate_pages(
	vm_object_t             object,
	vm_object_offset_t      offset,
	vm_object_size_t        size,
	boolean_t               kill_page,
	boolean_t               reusable_page,
	struct pmap             *pmap,
	vm_map_offset_t         pmap_offset);

__private_extern__ void vm_object_reuse_pages(
	vm_object_t             object,
	vm_object_offset_t      start_offset,
	vm_object_offset_t      end_offset,
	boolean_t               allow_partial_reuse);

__private_extern__ uint64_t     vm_object_purge(
	vm_object_t              object,
	int                      flags);

__private_extern__ kern_return_t vm_object_purgable_control(
	vm_object_t     object,
	vm_purgable_t   control,
	int             *state);

__private_extern__ kern_return_t vm_object_get_page_counts(
	vm_object_t             object,
	vm_object_offset_t      offset,
	vm_object_size_t        size,
	unsigned int            *resident_page_count,
	unsigned int            *dirty_page_count);

__private_extern__ boolean_t    vm_object_coalesce(
	vm_object_t             prev_object,
	vm_object_t             next_object,
	vm_object_offset_t      prev_offset,
	vm_object_offset_t      next_offset,
	vm_object_size_t        prev_size,
	vm_object_size_t        next_size);

__private_extern__ boolean_t    vm_object_shadow(
	vm_object_t             *object,
	vm_object_offset_t      *offset,
	vm_object_size_t        length);

__private_extern__ void         vm_object_collapse(
	vm_object_t             object,
	vm_object_offset_t      offset,
	boolean_t               can_bypass);

__private_extern__ boolean_t    vm_object_copy_quickly(
	vm_object_t             *_object,
	vm_object_offset_t      src_offset,
	vm_object_size_t        size,
	boolean_t               *_src_needs_copy,
	boolean_t               *_dst_needs_copy);

__private_extern__ kern_return_t        vm_object_copy_strategically(
	vm_object_t             src_object,
	vm_object_offset_t      src_offset,
	vm_object_size_t        size,
	vm_object_t             *dst_object,
	vm_object_offset_t      *dst_offset,
	boolean_t               *dst_needs_copy);

__private_extern__ kern_return_t        vm_object_copy_slowly(
	vm_object_t             src_object,
	vm_object_offset_t      src_offset,
	vm_object_size_t        size,
	boolean_t               interruptible,
	vm_object_t             *_result_object);

__private_extern__ vm_object_t  vm_object_copy_delayed(
	vm_object_t             src_object,
	vm_object_offset_t      src_offset,
	vm_object_size_t        size,
	boolean_t               src_object_shared);



__private_extern__ kern_return_t        vm_object_destroy(
	vm_object_t     object,
	kern_return_t   reason);

__private_extern__ void         vm_object_pager_create(
	vm_object_t     object);

__private_extern__ void         vm_object_compressor_pager_create(
	vm_object_t     object);

__private_extern__ void         vm_object_page_map(
	vm_object_t     object,
	vm_object_offset_t      offset,
	vm_object_size_t        size,
	vm_object_offset_t      (*map_fn)
	(void *, vm_object_offset_t),
	void            *map_fn_data);

__private_extern__ kern_return_t vm_object_upl_request(
	vm_object_t             object,
	vm_object_offset_t      offset,
	upl_size_t              size,
	upl_t                   *upl,
	upl_page_info_t         *page_info,
	unsigned int            *count,
	upl_control_flags_t     flags,
	vm_tag_t            tag);

__private_extern__ kern_return_t vm_object_transpose(
	vm_object_t             object1,
	vm_object_t             object2,
	vm_object_size_t        transpose_size);

__private_extern__ boolean_t vm_object_sync(
	vm_object_t             object,
	vm_object_offset_t      offset,
	vm_object_size_t        size,
	boolean_t               should_flush,
	boolean_t               should_return,
	boolean_t               should_iosync);

__private_extern__ kern_return_t vm_object_update(
	vm_object_t             object,
	vm_object_offset_t      offset,
	vm_object_size_t        size,
	vm_object_offset_t      *error_offset,
	int                     *io_errno,
	memory_object_return_t  should_return,
	int                     flags,
	vm_prot_t               prot);

__private_extern__ kern_return_t vm_object_lock_request(
	vm_object_t             object,
	vm_object_offset_t      offset,
	vm_object_size_t        size,
	memory_object_return_t  should_return,
	int                     flags,
	vm_prot_t               prot);



__private_extern__ vm_object_t  vm_object_memory_object_associate(
	memory_object_t         pager,
	vm_object_t             object,
	vm_object_size_t        size,
	boolean_t               check_named);


__private_extern__ void vm_object_cluster_size(
	vm_object_t             object,
	vm_object_offset_t      *start,
	vm_size_t               *length,
	vm_object_fault_info_t  fault_info,
	uint32_t                *io_streaming);

__private_extern__ kern_return_t vm_object_populate_with_private(
	vm_object_t             object,
	vm_object_offset_t      offset,
	ppnum_t                 phys_page,
	vm_size_t               size);

__private_extern__ void vm_object_change_wimg_mode(
	vm_object_t             object,
	unsigned int            wimg_mode);

extern kern_return_t adjust_vm_object_cache(
	vm_size_t oval,
	vm_size_t nval);

extern kern_return_t vm_object_page_op(
	vm_object_t             object,
	vm_object_offset_t      offset,
	int                     ops,
	ppnum_t                 *phys_entry,
	int                     *flags);

extern kern_return_t vm_object_range_op(
	vm_object_t             object,
	vm_object_offset_t      offset_beg,
	vm_object_offset_t      offset_end,
	int                     ops,
	uint32_t                *range);


__private_extern__ void         vm_object_reap_pages(
	vm_object_t object,
	int     reap_type);
#define REAP_REAP       0
#define REAP_TERMINATE  1
#define REAP_PURGEABLE  2
#define REAP_DATA_FLUSH 3

#if CONFIG_FREEZE

__private_extern__ uint32_t
vm_object_compressed_freezer_pageout(
	vm_object_t     object, uint32_t dirty_budget);

__private_extern__ void
vm_object_compressed_freezer_done(
	void);

#endif /* CONFIG_FREEZE */

__private_extern__ void
vm_object_pageout(
	vm_object_t     object);

#if CONFIG_IOSCHED
struct io_reprioritize_req {
	uint64_t        blkno;
	uint32_t        len;
	int             priority;
	struct vnode    *devvp;
	queue_chain_t   io_reprioritize_list;
};
typedef struct io_reprioritize_req *io_reprioritize_req_t;

extern void vm_io_reprioritize_init(void);
#endif

/*
 *	Event waiting handling
 */

#define VM_OBJECT_EVENT_INITIALIZED             0
#define VM_OBJECT_EVENT_PAGER_READY             1
#define VM_OBJECT_EVENT_PAGING_IN_PROGRESS      2
#define VM_OBJECT_EVENT_MAPPING_IN_PROGRESS     3
#define VM_OBJECT_EVENT_LOCK_IN_PROGRESS        4
#define VM_OBJECT_EVENT_UNCACHING               5
#define VM_OBJECT_EVENT_COPY_CALL               6
#define VM_OBJECT_EVENT_CACHING                 7
#define VM_OBJECT_EVENT_UNBLOCKED               8
#define VM_OBJECT_EVENT_PAGING_ONLY_IN_PROGRESS 9

#define VM_OBJECT_EVENT_MAX 10 /* 11 bits in "all_wanted", so 0->10 */

static __inline__ wait_result_t
vm_object_assert_wait(
	vm_object_t             object,
	int                     event,
	wait_interrupt_t        interruptible)
{
	wait_result_t wr;

	vm_object_lock_assert_exclusive(object);
	assert(event >= 0 && event <= VM_OBJECT_EVENT_MAX);

	object->all_wanted |= 1 << event;
	wr = assert_wait((event_t)((vm_offset_t)object + event),
	    interruptible);
	return wr;
}

static __inline__ wait_result_t
vm_object_wait(
	vm_object_t             object,
	int                     event,
	wait_interrupt_t        interruptible)
{
	wait_result_t wr;

	vm_object_assert_wait(object, event, interruptible);
	vm_object_unlock(object);
	wr = thread_block(THREAD_CONTINUE_NULL);
	return wr;
}

static __inline__ wait_result_t
thread_sleep_vm_object(
	vm_object_t             object,
	event_t                 event,
	wait_interrupt_t        interruptible)
{
	wait_result_t wr;

#if DEVELOPMENT || DEBUG
	if (object->Lock_owner != current_thread()) {
		panic("thread_sleep_vm_object: now owner - %p\n", object);
	}
	object->Lock_owner = 0;
#endif
	wr = lck_rw_sleep(&object->Lock,
	    LCK_SLEEP_PROMOTED_PRI,
	    event,
	    interruptible);
#if DEVELOPMENT || DEBUG
	object->Lock_owner = current_thread();
#endif
	return wr;
}

static __inline__ wait_result_t
vm_object_sleep(
	vm_object_t             object,
	int                     event,
	wait_interrupt_t        interruptible)
{
	wait_result_t wr;

	vm_object_lock_assert_exclusive(object);
	assert(event >= 0 && event <= VM_OBJECT_EVENT_MAX);

	object->all_wanted |= 1 << event;
	wr = thread_sleep_vm_object(object,
	    (event_t)((vm_offset_t)object + event),
	    interruptible);
	return wr;
}

static __inline__ void
vm_object_wakeup(
	vm_object_t             object,
	int                     event)
{
	vm_object_lock_assert_exclusive(object);
	assert(event >= 0 && event <= VM_OBJECT_EVENT_MAX);

	if (object->all_wanted & (1 << event)) {
		thread_wakeup((event_t)((vm_offset_t)object + event));
	}
	object->all_wanted &= ~(1 << event);
}

static __inline__ void
vm_object_set_wanted(
	vm_object_t             object,
	int                     event)
{
	vm_object_lock_assert_exclusive(object);
	assert(event >= 0 && event <= VM_OBJECT_EVENT_MAX);

	object->all_wanted |= (1 << event);
}

static __inline__ int
vm_object_wanted(
	vm_object_t             object,
	int                     event)
{
	vm_object_lock_assert_held(object);
	assert(event >= 0 && event <= VM_OBJECT_EVENT_MAX);

	return object->all_wanted & (1 << event);
}

/*
 *	Routines implemented as macros
 */
#ifdef VM_PIP_DEBUG
#include <libkern/OSDebug.h>
#define VM_PIP_DEBUG_BEGIN(object)                                      \
	MACRO_BEGIN                                                     \
	int pip = ((object)->paging_in_progress +                       \
	           (object)->activity_in_progress);                     \
	if (pip < VM_PIP_DEBUG_MAX_REFS) {                              \
	        (void) OSBacktrace(&(object)->pip_holders[pip].pip_retaddr[0], \
	                           VM_PIP_DEBUG_STACK_FRAMES);          \
	}                                                               \
	MACRO_END
#else   /* VM_PIP_DEBUG */
#define VM_PIP_DEBUG_BEGIN(object)
#endif  /* VM_PIP_DEBUG */

#define         vm_object_activity_begin(object)                        \
	MACRO_BEGIN                                                     \
	vm_object_lock_assert_exclusive((object));                      \
	VM_PIP_DEBUG_BEGIN((object));                                   \
	(object)->activity_in_progress++;                               \
	if ((object)->activity_in_progress == 0) {                      \
	        panic("vm_object_activity_begin(%p): overflow\n", (object));\
	}                                                               \
	MACRO_END

#define         vm_object_activity_end(object)                          \
	MACRO_BEGIN                                                     \
	vm_object_lock_assert_exclusive((object));                      \
	if ((object)->activity_in_progress == 0) {                      \
	        panic("vm_object_activity_end(%p): underflow\n", (object));\
	}                                                               \
	(object)->activity_in_progress--;                               \
	if ((object)->paging_in_progress == 0 &&                        \
	    (object)->activity_in_progress == 0)                        \
	        vm_object_wakeup((object),                              \
	                         VM_OBJECT_EVENT_PAGING_IN_PROGRESS);   \
	MACRO_END

#define         vm_object_paging_begin(object)                          \
	MACRO_BEGIN                                                     \
	vm_object_lock_assert_exclusive((object));                      \
	VM_PIP_DEBUG_BEGIN((object));                                   \
	(object)->paging_in_progress++;                                 \
	if ((object)->paging_in_progress == 0) {                        \
	        panic("vm_object_paging_begin(%p): overflow\n", (object));\
	}                                                               \
	MACRO_END

#define         vm_object_paging_end(object)                            \
	MACRO_BEGIN                                                     \
	vm_object_lock_assert_exclusive((object));                      \
	if ((object)->paging_in_progress == 0) {                        \
	        panic("vm_object_paging_end(%p): underflow\n", (object));\
	}                                                               \
	(object)->paging_in_progress--;                                 \
	if ((object)->paging_in_progress == 0) {                        \
	        vm_object_wakeup((object),                              \
	                         VM_OBJECT_EVENT_PAGING_ONLY_IN_PROGRESS); \
	        if ((object)->activity_in_progress == 0)                \
	                vm_object_wakeup((object),                      \
	                                 VM_OBJECT_EVENT_PAGING_IN_PROGRESS); \
	}                                                               \
	MACRO_END

#define         vm_object_paging_wait(object, interruptible)            \
	MACRO_BEGIN                                                     \
	vm_object_lock_assert_exclusive((object));                      \
	while ((object)->paging_in_progress != 0 ||                     \
	       (object)->activity_in_progress != 0) {                   \
	        wait_result_t  _wr;                                     \
                                                                        \
	        _wr = vm_object_sleep((object),                         \
	                        VM_OBJECT_EVENT_PAGING_IN_PROGRESS,     \
	                        (interruptible));                       \
                                                                        \
	/*XXX if ((interruptible) && (_wr != THREAD_AWAKENED))*/ \
	/*XXX break; */                                 \
	}                                                               \
	MACRO_END

#define vm_object_paging_only_wait(object, interruptible)               \
	MACRO_BEGIN                                                     \
	vm_object_lock_assert_exclusive((object));                      \
	while ((object)->paging_in_progress != 0) {                     \
	        wait_result_t  _wr;                                     \
                                                                        \
	        _wr = vm_object_sleep((object),                         \
	                        VM_OBJECT_EVENT_PAGING_ONLY_IN_PROGRESS,\
	                        (interruptible));                       \
                                                                        \
	/*XXX if ((interruptible) && (_wr != THREAD_AWAKENED))*/ \
	/*XXX break; */                                 \
	}                                                               \
	MACRO_END


#define vm_object_mapping_begin(object)                                 \
	MACRO_BEGIN                                                     \
	vm_object_lock_assert_exclusive((object));                      \
	assert(! (object)->mapping_in_progress);                        \
	(object)->mapping_in_progress = TRUE;                           \
	MACRO_END

#define vm_object_mapping_end(object)                                   \
	MACRO_BEGIN                                                     \
	vm_object_lock_assert_exclusive((object));                      \
	assert((object)->mapping_in_progress);                          \
	(object)->mapping_in_progress = FALSE;                          \
	vm_object_wakeup((object),                                      \
	                 VM_OBJECT_EVENT_MAPPING_IN_PROGRESS);          \
	MACRO_END

#define vm_object_mapping_wait(object, interruptible)                   \
	MACRO_BEGIN                                                     \
	vm_object_lock_assert_exclusive((object));                      \
	while ((object)->mapping_in_progress) {                         \
	        wait_result_t	_wr;                                    \
                                                                        \
	        _wr = vm_object_sleep((object),                         \
	                              VM_OBJECT_EVENT_MAPPING_IN_PROGRESS, \
	                              (interruptible));                 \
	/*XXX if ((interruptible) && (_wr != THREAD_AWAKENED))*/ \
	/*XXX break; */                                 \
	}                                                               \
	assert(!(object)->mapping_in_progress);                         \
	MACRO_END



#define vm_object_round_page(x) (((vm_object_offset_t)(x) + PAGE_MASK) & ~((signed)PAGE_MASK))
#define vm_object_trunc_page(x) ((vm_object_offset_t)(x) & ~((signed)PAGE_MASK))

extern void     vm_object_cache_add(vm_object_t);
extern void     vm_object_cache_remove(vm_object_t);
extern int      vm_object_cache_evict(int, int);

#define VM_OBJECT_OWNER_DISOWNED ((task_t) -1)
#define VM_OBJECT_OWNER(object)                                         \
	((((object)->purgable == VM_PURGABLE_DENY &&                    \
	   (object)->vo_ledger_tag == 0) ||                             \
	  (object)->vo_owner == TASK_NULL)                              \
	 ? TASK_NULL    /* not owned */                                 \
	 : (((object)->vo_owner == VM_OBJECT_OWNER_DISOWNED)            \
	    ? kernel_task /* disowned -> kernel */                      \
	    : (object)->vo_owner)) /* explicit owner */                 \

extern void     vm_object_ledger_tag_ledgers(
	vm_object_t object,
	int *ledger_idx_volatile,
	int *ledger_idx_nonvolatile,
	int *ledger_idx_volatile_compressed,
	int *ledger_idx_nonvolatile_compressed,
	boolean_t *do_footprint);
extern kern_return_t vm_object_ownership_change(
	vm_object_t object,
	int new_ledger_tag,
	task_t new_owner,
	int new_ledger_flags,
	boolean_t task_objq_locked);

#endif  /* _VM_VM_OBJECT_H_ */
