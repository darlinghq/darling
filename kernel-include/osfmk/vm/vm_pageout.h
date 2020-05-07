/*
 * Copyright (c) 2000-2019 Apple Inc. All rights reserved.
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
 *	File:	vm/vm_pageout.h
 *	Author:	Avadis Tevanian, Jr.
 *	Date:	1986
 *
 *	Declarations for the pageout daemon interface.
 */

#ifndef _VM_VM_PAGEOUT_H_
#define _VM_VM_PAGEOUT_H_

#ifdef  KERNEL_PRIVATE

#include <mach/mach_types.h>
#include <mach/boolean.h>
#include <mach/machine/vm_types.h>
#include <mach/memory_object_types.h>

#include <kern/kern_types.h>
#include <kern/locks.h>

#include <libkern/OSAtomic.h>


#include <vm/vm_options.h>

#ifdef  MACH_KERNEL_PRIVATE
#include <vm/vm_page.h>
#endif

#include <sys/kdebug.h>

#define VM_PAGE_AVAILABLE_COUNT()               ((unsigned int)(vm_page_cleaned_count))

/* externally manipulated counters */
extern unsigned int vm_pageout_cleaned_fault_reactivated;

#if CONFIG_FREEZE
extern boolean_t memorystatus_freeze_enabled;
#endif

#define VM_DYNAMIC_PAGING_ENABLED() (VM_CONFIG_COMPRESSOR_IS_ACTIVE)

#if VM_PRESSURE_EVENTS
extern boolean_t vm_pressure_events_enabled;
#endif /* VM_PRESSURE_EVENTS */


/*
 * the following codes are used in the DBG_MACH_WORKINGSET subclass
 * of the DBG_MACH class
 */
#define VM_DISCONNECT_ALL_PAGE_MAPPINGS         0x00
#define VM_DISCONNECT_TASK_PAGE_MAPPINGS        0x01
#define VM_REAL_FAULT_ADDR_INTERNAL             0x02
#define VM_REAL_FAULT_ADDR_PURGABLE             0x03
#define VM_REAL_FAULT_ADDR_EXTERNAL             0x04
#define VM_REAL_FAULT_ADDR_SHAREDCACHE          0x05
#define VM_REAL_FAULT_FAST                      0x06
#define VM_REAL_FAULT_SLOW                      0x07
#define VM_MAP_LOOKUP_OBJECT                    0x08



extern int      vm_debug_events;

#define VMF_CHECK_ZFDELAY               0x100
#define VMF_COWDELAY                    0x101
#define VMF_ZFDELAY                     0x102
#define VMF_COMPRESSORDELAY             0x103

#define VM_PAGEOUT_SCAN                 0x104
#define VM_PAGEOUT_BALANCE              0x105
#define VM_PAGEOUT_FREELIST             0x106
#define VM_PAGEOUT_PURGEONE             0x107
#define VM_PAGEOUT_CACHE_EVICT          0x108
#define VM_PAGEOUT_THREAD_BLOCK         0x109
#define VM_PAGEOUT_JETSAM               0x10A
#define VM_INFO1                        0x10B
#define VM_INFO2                        0x10C
#define VM_INFO3                        0x10D
#define VM_INFO4                        0x10E
#define VM_INFO5                        0x10F
#define VM_INFO6                        0x110
#define VM_INFO7                        0x111
#define VM_INFO8                        0x112
#define VM_INFO9                        0x113

#define VM_UPL_PAGE_WAIT                0x120
#define VM_IOPL_PAGE_WAIT               0x121
#define VM_PAGE_WAIT_BLOCK              0x122

#if CONFIG_IOSCHED
#define VM_PAGE_SLEEP                   0x123
#define VM_PAGE_EXPEDITE                0x124
#define VM_PAGE_EXPEDITE_NO_MEMORY      0x125
#endif

#define VM_PAGE_GRAB                    0x126
#define VM_PAGE_RELEASE                 0x127

#define VM_PRESSURE_EVENT               0x130
#define VM_EXECVE                       0x131
#define VM_WAKEUP_COMPACTOR_SWAPPER     0x132
#define VM_UPL_REQUEST                  0x133
#define VM_IOPL_REQUEST                 0x134
#define VM_KERN_REQUEST                 0x135

#define VM_DATA_WRITE                   0x140

#define VM_PRESSURE_LEVEL_CHANGE        0x141

#define VM_DEBUG_EVENT(name, event, control, arg1, arg2, arg3, arg4)    \
	MACRO_BEGIN                                             \
	if (__improbable(vm_debug_events)) {                    \
	        KERNEL_DEBUG_CONSTANT((MACHDBG_CODE(DBG_MACH_VM, event)) | control, arg1, arg2, arg3, arg4, 0); \
	}                                                       \
	MACRO_END

#define VM_DEBUG_CONSTANT_EVENT(name, event, control, arg1, arg2, arg3, arg4)   \
	MACRO_BEGIN                                             \
	        KERNEL_DEBUG_CONSTANT((MACHDBG_CODE(DBG_MACH_VM, event)) | control, arg1, arg2, arg3, arg4, 0); \
	MACRO_END

extern void memoryshot(unsigned int event, unsigned int control);

extern void update_vm_info(void);

#if CONFIG_IOSCHED
extern int upl_get_cached_tier(
	upl_t                   upl);
#endif

extern void upl_set_iodone(upl_t, void *);
extern void upl_set_iodone_error(upl_t, int);
extern void upl_callout_iodone(upl_t);

extern ppnum_t upl_get_highest_page(
	upl_t                   upl);

extern upl_size_t upl_get_size(
	upl_t                   upl);

extern upl_t upl_associated_upl(upl_t upl);
extern void upl_set_associated_upl(upl_t upl, upl_t associated_upl);

#ifdef  XNU_KERNEL_PRIVATE

extern kern_return_t vm_map_create_upl(
	vm_map_t                map,
	vm_map_address_t        offset,
	upl_size_t              *upl_size,
	upl_t                   *upl,
	upl_page_info_array_t   page_list,
	unsigned int            *count,
	upl_control_flags_t     *flags,
	vm_tag_t            tag);

extern void iopl_valid_data(
	upl_t                   upl_ptr,
	vm_tag_t        tag);

#endif  /* XNU_KERNEL_PRIVATE */

extern struct vnode * upl_lookup_vnode(upl_t upl);

#ifndef MACH_KERNEL_PRIVATE
typedef struct vm_page  *vm_page_t;
#endif

extern void                vm_page_free_list(
	vm_page_t   mem,
	boolean_t   prepare_object);

extern kern_return_t      vm_page_alloc_list(
	int         page_count,
	int                 flags,
	vm_page_t * list);

extern void               vm_page_set_offset(vm_page_t page, vm_object_offset_t offset);
extern vm_object_offset_t vm_page_get_offset(vm_page_t page);
extern ppnum_t            vm_page_get_phys_page(vm_page_t page);
extern vm_page_t          vm_page_get_next(vm_page_t page);

extern kern_return_t    mach_vm_pressure_level_monitor(boolean_t wait_for_pressure, unsigned int *pressure_level);

#if !CONFIG_EMBEDDED
extern kern_return_t    vm_pageout_wait(uint64_t deadline);
#endif

#ifdef  MACH_KERNEL_PRIVATE

#include <vm/vm_page.h>

extern unsigned int     vm_pageout_scan_event_counter;
extern unsigned int     vm_page_anonymous_count;


/*
 * must hold the page queues lock to
 * manipulate this structure
 */
struct vm_pageout_queue {
	vm_page_queue_head_t    pgo_pending;    /* laundry pages to be processed by pager's iothread */
	unsigned int    pgo_laundry;    /* current count of laundry pages on queue or in flight */
	unsigned int    pgo_maxlaundry;
	uint64_t        pgo_tid;        /* thread ID of I/O thread that services this queue */
	uint8_t         pgo_lowpriority; /* iothread is set to use low priority I/O */

	unsigned int    pgo_idle:1,     /* iothread is blocked waiting for work to do */
	    pgo_busy:1,                 /* iothread is currently processing request from pgo_pending */
	    pgo_throttled:1,            /* vm_pageout_scan thread needs a wakeup when pgo_laundry drops */
	    pgo_draining:1,
	    pgo_inited:1,
	:0;
};

#define VM_PAGE_Q_THROTTLED(q)          \
	((q)->pgo_laundry >= (q)->pgo_maxlaundry)

extern struct   vm_pageout_queue        vm_pageout_queue_internal;
extern struct   vm_pageout_queue        vm_pageout_queue_external;


/*
 *	Routines exported to Mach.
 */
extern void             vm_pageout(void);

extern kern_return_t    vm_pageout_internal_start(void);

extern void             vm_pageout_object_terminate(
	vm_object_t     object);

extern void             vm_pageout_cluster(
	vm_page_t       m);

extern void             vm_pageout_initialize_page(
	vm_page_t       m);

/* UPL exported routines and structures */

#define upl_lock_init(object)   lck_mtx_init(&(object)->Lock, &vm_object_lck_grp, &vm_object_lck_attr)
#define upl_lock_destroy(object)        lck_mtx_destroy(&(object)->Lock, &vm_object_lck_grp)
#define upl_lock(object)        lck_mtx_lock(&(object)->Lock)
#define upl_unlock(object)      lck_mtx_unlock(&(object)->Lock)
#define upl_try_lock(object)    lck_mtx_try_lock(&(object)->Lock)

#define MAX_VECTOR_UPL_ELEMENTS 8

struct _vector_upl_iostates {
	upl_offset_t offset;
	upl_size_t   size;
};

typedef struct _vector_upl_iostates vector_upl_iostates_t;

struct _vector_upl {
	upl_size_t              size;
	uint32_t                num_upls;
	uint32_t                invalid_upls;
	uint32_t                _reserved;
	vm_map_t                submap;
	vm_offset_t             submap_dst_addr;
	vm_object_offset_t      offset;
	upl_t                   upl_elems[MAX_VECTOR_UPL_ELEMENTS];
	upl_page_info_array_t   pagelist;
	vector_upl_iostates_t   upl_iostates[MAX_VECTOR_UPL_ELEMENTS];
};

typedef struct _vector_upl* vector_upl_t;

/* universal page list structure */

#if UPL_DEBUG
#define UPL_DEBUG_STACK_FRAMES  16
#define UPL_DEBUG_COMMIT_RECORDS 4

struct ucd {
	upl_offset_t    c_beg;
	upl_offset_t    c_end;
	int             c_aborted;
	void *          c_retaddr[UPL_DEBUG_STACK_FRAMES];
};
#endif

struct upl_io_completion {
	void     *io_context;
	void     (*io_done)(void *, int);

	int      io_error;
};


struct upl {
	decl_lck_mtx_data(, Lock);      /* Synchronization */
	int             ref_count;
	int             ext_ref_count;
	int             flags;
	vm_object_offset_t offset;
	upl_size_t      size;       /* size in bytes of the address space */
	vm_offset_t     kaddr;      /* secondary mapping in kernel */
	vm_object_t     map_object;
	ppnum_t         highest_page;
	void*           vector_upl;
	upl_t           associated_upl;
	struct upl_io_completion *upl_iodone;
#if CONFIG_IOSCHED
	int             upl_priority;
	uint64_t        *upl_reprio_info;
	void            *decmp_io_upl;
#endif
#if CONFIG_IOSCHED || UPL_DEBUG
	thread_t        upl_creator;
	queue_chain_t   uplq;       /* List of outstanding upls on an obj */
#endif
#if     UPL_DEBUG
	uintptr_t       ubc_alias1;
	uintptr_t       ubc_alias2;

	uint32_t        upl_state;
	uint32_t        upl_commit_index;
	void    *upl_create_retaddr[UPL_DEBUG_STACK_FRAMES];

	struct  ucd     upl_commit_records[UPL_DEBUG_COMMIT_RECORDS];
#endif  /* UPL_DEBUG */
};

/* upl struct flags */
#define UPL_PAGE_LIST_MAPPED    0x1
#define UPL_KERNEL_MAPPED       0x2
#define UPL_CLEAR_DIRTY         0x4
#define UPL_COMPOSITE_LIST      0x8
#define UPL_INTERNAL            0x10
#define UPL_PAGE_SYNC_DONE      0x20
#define UPL_DEVICE_MEMORY       0x40
#define UPL_PAGEOUT             0x80
#define UPL_LITE                0x100
#define UPL_IO_WIRE             0x200
#define UPL_ACCESS_BLOCKED      0x400
#define UPL_SHADOWED            0x1000
#define UPL_KERNEL_OBJECT       0x2000
#define UPL_VECTOR              0x4000
#define UPL_SET_DIRTY           0x8000
#define UPL_HAS_BUSY            0x10000
#define UPL_TRACKED_BY_OBJECT   0x20000
#define UPL_EXPEDITE_SUPPORTED  0x40000
#define UPL_DECMP_REQ           0x80000
#define UPL_DECMP_REAL_IO       0x100000

/* flags for upl_create flags parameter */
#define UPL_CREATE_EXTERNAL     0
#define UPL_CREATE_INTERNAL     0x1
#define UPL_CREATE_LITE         0x2
#define UPL_CREATE_IO_TRACKING  0x4
#define UPL_CREATE_EXPEDITE_SUP 0x8

extern upl_t vector_upl_create(vm_offset_t);
extern void vector_upl_deallocate(upl_t);
extern boolean_t vector_upl_is_valid(upl_t);
extern boolean_t vector_upl_set_subupl(upl_t, upl_t, u_int32_t);
extern void vector_upl_set_pagelist(upl_t);
extern void vector_upl_set_submap(upl_t, vm_map_t, vm_offset_t);
extern void vector_upl_get_submap(upl_t, vm_map_t*, vm_offset_t*);
extern void vector_upl_set_iostate(upl_t, upl_t, upl_offset_t, upl_size_t);
extern void vector_upl_get_iostate(upl_t, upl_t, upl_offset_t*, upl_size_t*);
extern void vector_upl_get_iostate_byindex(upl_t, uint32_t, upl_offset_t*, upl_size_t*);
extern upl_t vector_upl_subupl_byindex(upl_t, uint32_t);
extern upl_t vector_upl_subupl_byoffset(upl_t, upl_offset_t*, upl_size_t*);

extern void vm_object_set_pmap_cache_attr(
	vm_object_t             object,
	upl_page_info_array_t   user_page_list,
	unsigned int            num_pages,
	boolean_t               batch_pmap_op);

extern kern_return_t vm_object_iopl_request(
	vm_object_t             object,
	vm_object_offset_t      offset,
	upl_size_t              size,
	upl_t                   *upl_ptr,
	upl_page_info_array_t   user_page_list,
	unsigned int            *page_list_count,
	upl_control_flags_t     cntrl_flags,
	vm_tag_t            tag);

extern kern_return_t vm_object_super_upl_request(
	vm_object_t             object,
	vm_object_offset_t      offset,
	upl_size_t              size,
	upl_size_t              super_cluster,
	upl_t                   *upl,
	upl_page_info_t         *user_page_list,
	unsigned int            *page_list_count,
	upl_control_flags_t     cntrl_flags,
	vm_tag_t            tag);

/* should be just a regular vm_map_enter() */
extern kern_return_t vm_map_enter_upl(
	vm_map_t                map,
	upl_t                   upl,
	vm_map_offset_t         *dst_addr);

/* should be just a regular vm_map_remove() */
extern kern_return_t vm_map_remove_upl(
	vm_map_t                map,
	upl_t                   upl);

/* wired  page list structure */
typedef uint32_t *wpl_array_t;

extern void vm_page_free_reserve(int pages);

extern void vm_pageout_throttle_down(vm_page_t page);
extern void vm_pageout_throttle_up(vm_page_t page);

extern kern_return_t vm_paging_map_object(
	vm_page_t               page,
	vm_object_t             object,
	vm_object_offset_t      offset,
	vm_prot_t               protection,
	boolean_t               can_unlock_object,
	vm_map_size_t           *size,          /* IN/OUT */
	vm_map_offset_t         *address,       /* OUT */
	boolean_t               *need_unmap);   /* OUT */
extern void vm_paging_unmap_object(
	vm_object_t             object,
	vm_map_offset_t         start,
	vm_map_offset_t         end);
decl_simple_lock_data(extern, vm_paging_lock);

/*
 * Backing store throttle when BS is exhausted
 */
extern unsigned int    vm_backing_store_low;

extern void vm_pageout_steal_laundry(
	vm_page_t page,
	boolean_t queues_locked);

#endif  /* MACH_KERNEL_PRIVATE */

#if UPL_DEBUG
extern kern_return_t  upl_ubc_alias_set(
	upl_t upl,
	uintptr_t alias1,
	uintptr_t alias2);
extern int  upl_ubc_alias_get(
	upl_t upl,
	uintptr_t * al,
	uintptr_t * al2);
#endif /* UPL_DEBUG */

extern void vm_countdirtypages(void);

extern void vm_backing_store_disable(
	boolean_t       suspend);

extern kern_return_t upl_transpose(
	upl_t   upl1,
	upl_t   upl2);

extern kern_return_t mach_vm_pressure_monitor(
	boolean_t       wait_for_pressure,
	unsigned int    nsecs_monitored,
	unsigned int    *pages_reclaimed_p,
	unsigned int    *pages_wanted_p);

extern kern_return_t
vm_set_buffer_cleanup_callout(
	boolean_t       (*func)(int));

struct vm_page_stats_reusable {
	SInt32          reusable_count;
	uint64_t        reusable;
	uint64_t        reused;
	uint64_t        reused_wire;
	uint64_t        reused_remove;
	uint64_t        all_reusable_calls;
	uint64_t        partial_reusable_calls;
	uint64_t        all_reuse_calls;
	uint64_t        partial_reuse_calls;
	uint64_t        reusable_pages_success;
	uint64_t        reusable_pages_failure;
	uint64_t        reusable_pages_shared;
	uint64_t        reuse_pages_success;
	uint64_t        reuse_pages_failure;
	uint64_t        can_reuse_success;
	uint64_t        can_reuse_failure;
	uint64_t        reusable_reclaimed;
	uint64_t        reusable_nonwritable;
	uint64_t        reusable_shared;
	uint64_t        free_shared;
};
extern struct vm_page_stats_reusable vm_page_stats_reusable;

extern int hibernate_flush_memory(void);
extern void hibernate_reset_stats(void);
extern void hibernate_create_paddr_map(void);

extern void vm_set_restrictions(void);

extern int vm_compressor_mode;
extern kern_return_t vm_pageout_compress_page(void **, char *, vm_page_t);
extern void vm_pageout_anonymous_pages(void);
extern void vm_pageout_disconnect_all_pages(void);


struct  vm_config {
	boolean_t       compressor_is_present;          /* compressor is initialized and can be used by the freezer, the sweep or the pager */
	boolean_t       compressor_is_active;           /* pager can actively compress pages...  'compressor_is_present' must be set */
	boolean_t       swap_is_present;                /* swap is initialized and can be used by the freezer, the sweep or the pager */
	boolean_t       swap_is_active;                 /* pager can actively swap out compressed segments... 'swap_is_present' must be set */
	boolean_t       freezer_swap_is_active;         /* freezer can swap out frozen tasks... "compressor_is_present + swap_is_present" must be set */
};

extern  struct vm_config        vm_config;


#define VM_PAGER_NOT_CONFIGURED                         0x0     /* no compresser or swap configured */
#define VM_PAGER_DEFAULT                                0x1     /* Use default pager... DEPRECATED */
#define VM_PAGER_COMPRESSOR_NO_SWAP                     0x2     /* Active in-core compressor only. */
#define VM_PAGER_COMPRESSOR_WITH_SWAP                   0x4     /* Active in-core compressor + swap backend. */
#define VM_PAGER_FREEZER_DEFAULT                        0x8     /* Freezer backed by default pager... DEPRECATED */
#define VM_PAGER_FREEZER_COMPRESSOR_NO_SWAP             0x10    /* Freezer backed by in-core compressor only i.e. frozen data remain in-core compressed.*/
#define VM_PAGER_COMPRESSOR_NO_SWAP_PLUS_FREEZER_COMPRESSOR_WITH_SWAP   0x20    /* Active in-core compressor + Freezer backed by in-core compressor with swap support too.*/

#define VM_PAGER_MAX_MODES                              6       /* Total number of vm compressor modes supported */


#define VM_CONFIG_COMPRESSOR_IS_PRESENT         (vm_config.compressor_is_present == TRUE)
#define VM_CONFIG_COMPRESSOR_IS_ACTIVE          (vm_config.compressor_is_active == TRUE)
#define VM_CONFIG_SWAP_IS_PRESENT               (vm_config.swap_is_present == TRUE)
#define VM_CONFIG_SWAP_IS_ACTIVE                (vm_config.swap_is_active == TRUE)
#define VM_CONFIG_FREEZER_SWAP_IS_ACTIVE        (vm_config.freezer_swap_is_active == TRUE)

#endif  /* KERNEL_PRIVATE */

#ifdef XNU_KERNEL_PRIVATE

struct vm_pageout_state {
	boolean_t vm_pressure_thread_running;
	boolean_t vm_pressure_changed;
	boolean_t vm_restricted_to_single_processor;
	int vm_compressor_thread_count;

	unsigned int vm_page_speculative_q_age_ms;
	unsigned int vm_page_speculative_percentage;
	unsigned int vm_page_speculative_target;

	unsigned int vm_pageout_swap_wait;
	unsigned int vm_pageout_idle_wait;      /* milliseconds */
	unsigned int vm_pageout_empty_wait;     /* milliseconds */
	unsigned int vm_pageout_burst_wait;     /* milliseconds */
	unsigned int vm_pageout_deadlock_wait;  /* milliseconds */
	unsigned int vm_pageout_deadlock_relief;
	unsigned int vm_pageout_burst_inactive_throttle;

	unsigned int vm_pageout_inactive;
	unsigned int vm_pageout_inactive_used;  /* debugging */
	unsigned int vm_pageout_inactive_clean; /* debugging */

	uint32_t vm_page_filecache_min;
	uint32_t vm_page_filecache_min_divisor;
	uint32_t vm_page_xpmapped_min;
	uint32_t vm_page_xpmapped_min_divisor;
	uint64_t vm_pageout_considered_page_last;

	int vm_page_free_count_init;

	unsigned int vm_memory_pressure;

	int memorystatus_purge_on_critical;
	int memorystatus_purge_on_warning;
	int memorystatus_purge_on_urgent;

	thread_t vm_pageout_external_iothread;
	thread_t vm_pageout_internal_iothread;
};

extern struct vm_pageout_state vm_pageout_state;

/*
 * This structure is used to track the VM_INFO instrumentation
 */
struct vm_pageout_vminfo {
	unsigned long vm_pageout_considered_page;
	unsigned long vm_pageout_considered_bq_internal;
	unsigned long vm_pageout_considered_bq_external;
	unsigned long vm_pageout_skipped_external;

	unsigned long vm_pageout_pages_evicted;
	unsigned long vm_pageout_pages_purged;
	unsigned long vm_pageout_freed_cleaned;
	unsigned long vm_pageout_freed_speculative;
	unsigned long vm_pageout_freed_external;
	unsigned long vm_pageout_freed_internal;
	unsigned long vm_pageout_inactive_dirty_internal;
	unsigned long vm_pageout_inactive_dirty_external;
	unsigned long vm_pageout_inactive_referenced;
	unsigned long vm_pageout_reactivation_limit_exceeded;
	unsigned long vm_pageout_inactive_force_reclaim;
	unsigned long vm_pageout_inactive_nolock;
	unsigned long vm_pageout_filecache_min_reactivated;
	unsigned long vm_pageout_scan_inactive_throttled_internal;
	unsigned long vm_pageout_scan_inactive_throttled_external;

	uint64_t      vm_pageout_compressions;
	uint64_t      vm_compressor_pages_grabbed;
	unsigned long vm_compressor_failed;

	unsigned long vm_page_pages_freed;

	unsigned long vm_phantom_cache_found_ghost;
	unsigned long vm_phantom_cache_added_ghost;
};

extern struct vm_pageout_vminfo vm_pageout_vminfo;


#if DEVELOPMENT || DEBUG

/*
 *	This structure records the pageout daemon's actions:
 *	how many pages it looks at and what happens to those pages.
 *	No locking needed because only one thread modifies the fields.
 */
struct vm_pageout_debug {
	uint32_t vm_pageout_balanced;
	uint32_t vm_pageout_scan_event_counter;
	uint32_t vm_pageout_speculative_dirty;

	uint32_t vm_pageout_inactive_busy;
	uint32_t vm_pageout_inactive_absent;
	uint32_t vm_pageout_inactive_notalive;
	uint32_t vm_pageout_inactive_error;
	uint32_t vm_pageout_inactive_deactivated;

	uint32_t vm_pageout_enqueued_cleaned;

	uint32_t vm_pageout_cleaned_busy;
	uint32_t vm_pageout_cleaned_nolock;
	uint32_t vm_pageout_cleaned_reference_reactivated;
	uint32_t vm_pageout_cleaned_volatile_reactivated;
	uint32_t vm_pageout_cleaned_reactivated;  /* debugging; how many cleaned pages are found to be referenced on pageout (and are therefore reactivated) */
	uint32_t vm_pageout_cleaned_fault_reactivated;

	uint32_t vm_pageout_dirty_no_pager;
	uint32_t vm_pageout_purged_objects;

	uint32_t vm_pageout_scan_throttle;
	uint32_t vm_pageout_scan_reclaimed_throttled;
	uint32_t vm_pageout_scan_burst_throttle;
	uint32_t vm_pageout_scan_empty_throttle;
	uint32_t vm_pageout_scan_swap_throttle;
	uint32_t vm_pageout_scan_deadlock_detected;
	uint32_t vm_pageout_scan_inactive_throttle_success;
	uint32_t vm_pageout_scan_throttle_deferred;

	uint32_t vm_pageout_inactive_external_forced_jetsam_count;

	uint32_t vm_grab_anon_overrides;
	uint32_t vm_grab_anon_nops;

	uint32_t vm_pageout_no_victim;
	unsigned long vm_pageout_throttle_up_count;
	uint32_t vm_page_steal_pageout_page;

	uint32_t vm_cs_validated_resets;
	uint32_t vm_object_iopl_request_sleep_for_cleaning;
	uint32_t vm_page_slide_counter;
	uint32_t vm_page_slide_errors;
	uint32_t vm_page_throttle_count;
	/*
	 * Statistics about UPL enforcement of copy-on-write obligations.
	 */
	unsigned long upl_cow;
	unsigned long upl_cow_again;
	unsigned long upl_cow_pages;
	unsigned long upl_cow_again_pages;
	unsigned long iopl_cow;
	unsigned long iopl_cow_pages;
};

extern struct vm_pageout_debug vm_pageout_debug;

#define VM_PAGEOUT_DEBUG(member, value)                 \
	MACRO_BEGIN                                     \
	        vm_pageout_debug.member += value;       \
	MACRO_END
#else
#define VM_PAGEOUT_DEBUG(member, value)
#endif

#define MAX_COMPRESSOR_THREAD_COUNT      8

#if DEVELOPMENT || DEBUG
typedef struct vmct_stats_s {
	uint64_t vmct_runtimes[MAX_COMPRESSOR_THREAD_COUNT];
	uint64_t vmct_pages[MAX_COMPRESSOR_THREAD_COUNT];
	uint64_t vmct_iterations[MAX_COMPRESSOR_THREAD_COUNT];
	uint64_t vmct_cthreads_total;
	int32_t vmct_minpages[MAX_COMPRESSOR_THREAD_COUNT];
	int32_t vmct_maxpages[MAX_COMPRESSOR_THREAD_COUNT];
} vmct_stats_t;
#endif
#endif
#endif  /* _VM_VM_PAGEOUT_H_ */
