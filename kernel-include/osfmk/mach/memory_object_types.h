/*
 * Copyright (c) 2000-2016 Apple Computer, Inc. All rights reserved.
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
 *	File:	memory_object.h
 *	Author:	Michael Wayne Young
 *
 *	External memory management interface definition.
 */

#ifndef _MACH_MEMORY_OBJECT_TYPES_H_
#define _MACH_MEMORY_OBJECT_TYPES_H_

/*
 *	User-visible types used in the external memory
 *	management interface:
 */

#include <mach/port.h>
#include <mach/message.h>
#include <mach/vm_prot.h>
#include <mach/vm_sync.h>
#include <mach/vm_types.h>
#include <mach/machine/vm_types.h>

#include <sys/cdefs.h>

#define VM_64_BIT_DATA_OBJECTS

typedef unsigned long long      memory_object_offset_t;
typedef unsigned long long      memory_object_size_t;
typedef natural_t               memory_object_cluster_size_t;
typedef natural_t *             memory_object_fault_info_t;

typedef unsigned long long      vm_object_id_t;


/*
 * Temporary until real EMMI version gets re-implemented
 */

#ifdef  KERNEL_PRIVATE

/* IMPORTANT: this type must match "ipc_object_bits_t" from ipc/ipc_port.h */
typedef natural_t mo_ipc_object_bits_t;

struct memory_object_pager_ops; /* forward declaration */

/*
 * "memory_object" and "memory_object_control" types used to be Mach ports
 * in user space and can be passed as such to some kernel APIs.
 * Their first field must match the "io_bits" field of a
 * "struct ipc_object" to identify them as a "IKOT_MEMORY_OBJECT" and
 * "IKOT_MEM_OBJ_CONTROL" respectively.
 */
typedef struct          memory_object {
	mo_ipc_object_bits_t                    mo_ikot; /* DO NOT CHANGE */
	const struct memory_object_pager_ops    *mo_pager_ops;
	struct memory_object_control            *mo_control;
} *memory_object_t;

typedef struct          memory_object_control {
	mo_ipc_object_bits_t    moc_ikot; /* DO NOT CHANGE */
	struct vm_object        *moc_object;
} *memory_object_control_t;

typedef const struct memory_object_pager_ops {
	void (*memory_object_reference)(
		memory_object_t mem_obj);
	void (*memory_object_deallocate)(
		memory_object_t mem_obj);
	kern_return_t (*memory_object_init)(
		memory_object_t mem_obj,
		memory_object_control_t mem_control,
		memory_object_cluster_size_t size);
	kern_return_t (*memory_object_terminate)(
		memory_object_t mem_obj);
	kern_return_t (*memory_object_data_request)(
		memory_object_t mem_obj,
		memory_object_offset_t offset,
		memory_object_cluster_size_t length,
		vm_prot_t desired_access,
		memory_object_fault_info_t fault_info);
	kern_return_t (*memory_object_data_return)(
		memory_object_t mem_obj,
		memory_object_offset_t offset,
		memory_object_cluster_size_t size,
		memory_object_offset_t *resid_offset,
		int *io_error,
		boolean_t dirty,
		boolean_t kernel_copy,
		int upl_flags);
	kern_return_t (*memory_object_data_initialize)(
		memory_object_t mem_obj,
		memory_object_offset_t offset,
		memory_object_cluster_size_t size);
	kern_return_t (*memory_object_data_unlock)(
		memory_object_t mem_obj,
		memory_object_offset_t offset,
		memory_object_size_t size,
		vm_prot_t desired_access);
	kern_return_t (*memory_object_synchronize)(
		memory_object_t mem_obj,
		memory_object_offset_t offset,
		memory_object_size_t size,
		vm_sync_t sync_flags);
	kern_return_t (*memory_object_map)(
		memory_object_t mem_obj,
		vm_prot_t prot);
	kern_return_t (*memory_object_last_unmap)(
		memory_object_t mem_obj);
	kern_return_t (*memory_object_data_reclaim)(
		memory_object_t mem_obj,
		boolean_t reclaim_backing_store);
	const char *memory_object_pager_name;
} * memory_object_pager_ops_t;

#else   /* KERNEL_PRIVATE */

typedef mach_port_t     memory_object_t;
typedef mach_port_t     memory_object_control_t;

#endif  /* KERNEL_PRIVATE */

typedef memory_object_t *memory_object_array_t;
/* A memory object ... */
/*  Used by the kernel to retrieve */
/*  or store data */

typedef mach_port_t     memory_object_name_t;
/* Used to describe the memory ... */
/*  object in vm_regions() calls */

typedef mach_port_t     memory_object_default_t;
/* Registered with the host ... */
/*  for creating new internal objects */

#define MEMORY_OBJECT_NULL              ((memory_object_t) 0)
#define MEMORY_OBJECT_CONTROL_NULL      ((memory_object_control_t) 0)
#define MEMORY_OBJECT_NAME_NULL         ((memory_object_name_t) 0)
#define MEMORY_OBJECT_DEFAULT_NULL      ((memory_object_default_t) 0)


typedef int             memory_object_copy_strategy_t;
/* How memory manager handles copy: */
#define         MEMORY_OBJECT_COPY_NONE         0
/* ... No special support */
#define         MEMORY_OBJECT_COPY_CALL         1
/* ... Make call on memory manager */
#define         MEMORY_OBJECT_COPY_DELAY        2
/* ... Memory manager doesn't
 *     change data externally.
 */
#define         MEMORY_OBJECT_COPY_TEMPORARY    3
/* ... Memory manager doesn't
 *     change data externally, and
 *     doesn't need to see changes.
 */
#define         MEMORY_OBJECT_COPY_SYMMETRIC    4
/* ... Memory manager doesn't
 *     change data externally,
 *     doesn't need to see changes,
 *     and object will not be
 *     multiply mapped.
 *
 *     XXX
 *     Not yet safe for non-kernel use.
 */

#define         MEMORY_OBJECT_COPY_INVALID      5
/* ...	An invalid copy strategy,
 *	for external objects which
 *	have not been initialized.
 *	Allows copy_strategy to be
 *	examined without also
 *	examining pager_ready and
 *	internal.
 */

typedef int             memory_object_return_t;
/* Which pages to return to manager
 *  this time (lock_request) */
#define         MEMORY_OBJECT_RETURN_NONE       0
/* ... don't return any. */
#define         MEMORY_OBJECT_RETURN_DIRTY      1
/* ... only dirty pages. */
#define         MEMORY_OBJECT_RETURN_ALL        2
/* ... dirty and precious pages. */
#define         MEMORY_OBJECT_RETURN_ANYTHING   3
/* ... any resident page. */

/*
 *	Data lock request flags
 */

#define         MEMORY_OBJECT_DATA_FLUSH        0x1
#define         MEMORY_OBJECT_DATA_NO_CHANGE    0x2
#define         MEMORY_OBJECT_DATA_PURGE        0x4
#define         MEMORY_OBJECT_COPY_SYNC         0x8
#define         MEMORY_OBJECT_DATA_SYNC         0x10
#define         MEMORY_OBJECT_IO_SYNC           0x20
#define         MEMORY_OBJECT_DATA_FLUSH_ALL    0x40

/*
 *	Types for the memory object flavor interfaces
 */

#define MEMORY_OBJECT_INFO_MAX      (1024)
typedef int     *memory_object_info_t;
typedef int      memory_object_flavor_t;
typedef int      memory_object_info_data_t[MEMORY_OBJECT_INFO_MAX];


#define MEMORY_OBJECT_PERFORMANCE_INFO  11
#define MEMORY_OBJECT_ATTRIBUTE_INFO    14
#define MEMORY_OBJECT_BEHAVIOR_INFO     15

#ifdef  PRIVATE

#define OLD_MEMORY_OBJECT_BEHAVIOR_INFO         10
#define OLD_MEMORY_OBJECT_ATTRIBUTE_INFO        12

struct old_memory_object_behave_info {
	memory_object_copy_strategy_t   copy_strategy;
	boolean_t                       temporary;
	boolean_t                       invalidate;
};

struct old_memory_object_attr_info {                    /* old attr list */
	boolean_t                       object_ready;
	boolean_t                       may_cache;
	memory_object_copy_strategy_t   copy_strategy;
};

typedef struct old_memory_object_behave_info *old_memory_object_behave_info_t;
typedef struct old_memory_object_behave_info old_memory_object_behave_info_data_t;
typedef struct old_memory_object_attr_info *old_memory_object_attr_info_t;
typedef struct old_memory_object_attr_info old_memory_object_attr_info_data_t;

#define OLD_MEMORY_OBJECT_BEHAVE_INFO_COUNT     ((mach_msg_type_number_t) \
	        (sizeof(old_memory_object_behave_info_data_t)/sizeof(int)))
#define OLD_MEMORY_OBJECT_ATTR_INFO_COUNT       ((mach_msg_type_number_t) \
	        (sizeof(old_memory_object_attr_info_data_t)/sizeof(int)))

#ifdef KERNEL

__BEGIN_DECLS
extern void memory_object_reference(memory_object_t object);
extern void memory_object_deallocate(memory_object_t object);

extern void memory_object_default_reference(memory_object_default_t);
extern void memory_object_default_deallocate(memory_object_default_t);

extern void memory_object_control_reference(memory_object_control_t control);
extern void memory_object_control_deallocate(memory_object_control_t control);
extern int  memory_object_control_uiomove(memory_object_control_t, memory_object_offset_t, void *, int, int, int, int);
__END_DECLS

#endif  /* KERNEL */

#endif  /* PRIVATE */

struct memory_object_perf_info {
	memory_object_cluster_size_t    cluster_size;
	boolean_t                       may_cache;
};

struct memory_object_attr_info {
	memory_object_copy_strategy_t   copy_strategy;
	memory_object_cluster_size_t    cluster_size;
	boolean_t                       may_cache_object;
	boolean_t                       temporary;
};

struct memory_object_behave_info {
	memory_object_copy_strategy_t   copy_strategy;
	boolean_t                       temporary;
	boolean_t                       invalidate;
	boolean_t                       silent_overwrite;
	boolean_t                       advisory_pageout;
};


typedef struct memory_object_behave_info *memory_object_behave_info_t;
typedef struct memory_object_behave_info memory_object_behave_info_data_t;

typedef struct memory_object_perf_info  *memory_object_perf_info_t;
typedef struct memory_object_perf_info  memory_object_perf_info_data_t;

typedef struct memory_object_attr_info  *memory_object_attr_info_t;
typedef struct memory_object_attr_info  memory_object_attr_info_data_t;

#define MEMORY_OBJECT_BEHAVE_INFO_COUNT ((mach_msg_type_number_t)       \
	        (sizeof(memory_object_behave_info_data_t)/sizeof(int)))
#define MEMORY_OBJECT_PERF_INFO_COUNT   ((mach_msg_type_number_t)       \
	        (sizeof(memory_object_perf_info_data_t)/sizeof(int)))
#define MEMORY_OBJECT_ATTR_INFO_COUNT   ((mach_msg_type_number_t)       \
	        (sizeof(memory_object_attr_info_data_t)/sizeof(int)))

#define invalid_memory_object_flavor(f)                                 \
	(f != MEMORY_OBJECT_ATTRIBUTE_INFO &&                           \
	 f != MEMORY_OBJECT_PERFORMANCE_INFO &&                         \
	 f != OLD_MEMORY_OBJECT_BEHAVIOR_INFO &&                        \
	 f != MEMORY_OBJECT_BEHAVIOR_INFO &&                            \
	 f != OLD_MEMORY_OBJECT_ATTRIBUTE_INFO)


/*
 * Used to support options on memory_object_release_name call
 */
#define MEMORY_OBJECT_TERMINATE_IDLE    0x1
#define MEMORY_OBJECT_RESPECT_CACHE     0x2
#define MEMORY_OBJECT_RELEASE_NO_OP     0x4


/* named entry processor mapping options */
/* enumerated */
#define MAP_MEM_NOOP                      0
#define MAP_MEM_COPYBACK                  1
#define MAP_MEM_IO                        2
#define MAP_MEM_WTHRU                     3
#define MAP_MEM_WCOMB                     4       /* Write combining mode */
                                                  /* aka store gather     */
#define MAP_MEM_INNERWBACK                5
#define MAP_MEM_POSTED                    6
#define MAP_MEM_RT                        7
#define MAP_MEM_POSTED_REORDERED          8
#define MAP_MEM_POSTED_COMBINED_REORDERED 9

#define GET_MAP_MEM(flags)      \
	((((unsigned int)(flags)) >> 24) & 0xFF)

#define SET_MAP_MEM(caching, flags)     \
	((flags) = ((((unsigned int)(caching)) << 24) \
	                & 0xFF000000) | ((flags) & 0xFFFFFF));

/* leave room for vm_prot bits (0xFF ?) */
#define MAP_MEM_LEDGER_TAGGED        0x002000 /* object owned by a specific task and ledger */
#define MAP_MEM_PURGABLE_KERNEL_ONLY 0x004000 /* volatility controlled by kernel */
#define MAP_MEM_GRAB_SECLUDED   0x008000 /* can grab secluded pages */
#define MAP_MEM_ONLY            0x010000 /* change processor caching  */
#define MAP_MEM_NAMED_CREATE    0x020000 /* create extant object      */
#define MAP_MEM_PURGABLE        0x040000 /* create a purgable VM object */
#define MAP_MEM_NAMED_REUSE     0x080000 /* reuse provided entry if identical */
#define MAP_MEM_USE_DATA_ADDR   0x100000 /* preserve address of data, rather than base of page */
#define MAP_MEM_VM_COPY         0x200000 /* make a copy of a VM range */
#define MAP_MEM_VM_SHARE        0x400000 /* extract a VM range for remap */
#define MAP_MEM_4K_DATA_ADDR    0x800000 /* preserve 4K aligned address of data */

#define MAP_MEM_FLAGS_MASK 0x00FFFF00
#define MAP_MEM_FLAGS_USER (                               \
	MAP_MEM_PURGABLE_KERNEL_ONLY |                     \
	MAP_MEM_GRAB_SECLUDED |                            \
	MAP_MEM_ONLY |                                     \
	MAP_MEM_NAMED_CREATE |                             \
	MAP_MEM_PURGABLE |                                 \
	MAP_MEM_NAMED_REUSE |                              \
	MAP_MEM_USE_DATA_ADDR |                            \
	MAP_MEM_VM_COPY |                                  \
	MAP_MEM_VM_SHARE |                                 \
	MAP_MEM_LEDGER_TAGGED |                            \
	MAP_MEM_4K_DATA_ADDR)
#define MAP_MEM_FLAGS_ALL (                     \
	MAP_MEM_FLAGS_USER)

#ifdef KERNEL

/*
 *  Universal Page List data structures
 *
 *  A UPL describes a bounded set of physical pages
 *  associated with some range of an object or map
 *  and a snapshot of the attributes associated with
 *  each of those pages.
 */
#ifdef PRIVATE
#define MAX_UPL_TRANSFER_BYTES  (1024 * 1024)
#define MAX_UPL_SIZE_BYTES      (1024 * 1024 * 64)

#ifndef CONFIG_EMBEDDED
#define MAX_UPL_SIZE            (MAX_UPL_SIZE_BYTES / PAGE_SIZE)
#define MAX_UPL_TRANSFER        (MAX_UPL_TRANSFER_BYTES / PAGE_SIZE)
#endif


struct upl_page_info {
	ppnum_t         phys_addr;      /* physical page index number */
	unsigned int
#ifdef  XNU_KERNEL_PRIVATE
	    free_when_done:1,    /* page is to be freed on commit */
	    absent:1,           /* No valid data in this page */
	    dirty:1,            /* Page must be cleaned (O) */
	    precious:1,         /* must be cleaned, we have only copy */
	    device:1,           /* no page data, mapped dev memory */
	    speculative:1,      /* page is valid, but not yet accessed */
	    cs_validated:1,     /* CODE SIGNING: page was validated */
	    cs_tainted:1,       /* CODE SIGNING: page is tainted */
	    cs_nx:1,            /* CODE SIGNING: page is NX */
	    needed:1,           /* page should be left in cache on abort */
	    mark:1,             /* a mark flag for the creator to use as they wish */
	:0;                     /* force to long boundary */
#else
	opaque;                 /* use upl_page_xxx() accessor funcs */
#endif /* XNU_KERNEL_PRIVATE */
};

#else

struct upl_page_info {
	unsigned int    opaque[2];      /* use upl_page_xxx() accessor funcs */
};

#endif /* PRIVATE */

typedef struct upl_page_info    upl_page_info_t;
typedef upl_page_info_t         *upl_page_info_array_t;
typedef upl_page_info_array_t   upl_page_list_ptr_t;

typedef uint32_t        upl_offset_t;   /* page-aligned byte offset */
typedef uint32_t        upl_size_t;     /* page-aligned byte size */

/* upl invocation flags */
/* top nibble is used by super upl */

typedef uint64_t upl_control_flags_t;

#define UPL_FLAGS_NONE          0x00000000ULL
#define UPL_COPYOUT_FROM        0x00000001ULL
#define UPL_PRECIOUS            0x00000002ULL
#define UPL_NO_SYNC             0x00000004ULL
#define UPL_CLEAN_IN_PLACE      0x00000008ULL
#define UPL_NOBLOCK             0x00000010ULL
#define UPL_RET_ONLY_DIRTY      0x00000020ULL
#define UPL_SET_INTERNAL        0x00000040ULL
#define UPL_QUERY_OBJECT_TYPE   0x00000080ULL
#define UPL_RET_ONLY_ABSENT     0x00000100ULL /* used only for COPY_FROM = FALSE */
#define UPL_FILE_IO             0x00000200ULL
#define UPL_SET_LITE            0x00000400ULL
#define UPL_SET_INTERRUPTIBLE   0x00000800ULL
#define UPL_SET_IO_WIRE         0x00001000ULL
#define UPL_FOR_PAGEOUT         0x00002000ULL
#define UPL_WILL_BE_DUMPED      0x00004000ULL
#define UPL_FORCE_DATA_SYNC     0x00008000ULL
/* continued after the ticket bits... */

#define UPL_PAGE_TICKET_MASK    0x000F0000ULL
#define UPL_PAGE_TICKET_SHIFT   16

/* ... flags resume here */
#define UPL_BLOCK_ACCESS        0x00100000ULL
#define UPL_ENCRYPT             0x00200000ULL
#define UPL_NOZEROFILL          0x00400000ULL
#define UPL_WILL_MODIFY         0x00800000ULL /* caller will modify the pages */

#define UPL_NEED_32BIT_ADDR     0x01000000ULL
#define UPL_UBC_MSYNC           0x02000000ULL
#define UPL_UBC_PAGEOUT         0x04000000ULL
#define UPL_UBC_PAGEIN          0x08000000ULL
#define UPL_REQUEST_SET_DIRTY   0x10000000ULL
#define UPL_REQUEST_NO_FAULT    0x20000000ULL /* fail if pages not all resident */
#define UPL_NOZEROFILLIO        0x40000000ULL /* allow non zerofill pages present */
#define UPL_REQUEST_FORCE_COHERENCY     0x80000000ULL

/* UPL flags known by this kernel */
#define UPL_VALID_FLAGS         0xFFFFFFFFFFULL


/* upl abort error flags */
#define UPL_ABORT_RESTART               0x1
#define UPL_ABORT_UNAVAILABLE   0x2
#define UPL_ABORT_ERROR         0x4
#define UPL_ABORT_FREE_ON_EMPTY 0x8  /* only implemented in wrappers */
#define UPL_ABORT_DUMP_PAGES    0x10
#define UPL_ABORT_NOTIFY_EMPTY  0x20
/* deprecated: #define UPL_ABORT_ALLOW_ACCESS	0x40 */
#define UPL_ABORT_REFERENCE     0x80

/* upl pages check flags */
#define UPL_CHECK_DIRTY         0x1


/*
 *  upl pagein/pageout  flags
 *
 *
 * when I/O is issued from this UPL it should be done synchronously
 */
#define UPL_IOSYNC      0x1

/*
 * the passed in UPL should not have either a commit or abort
 * applied to it by the underlying layers... the site that
 * created the UPL is responsible for cleaning it up.
 */
#define UPL_NOCOMMIT    0x2

/*
 * turn off any speculative read-ahead applied at the I/O layer
 */
#define UPL_NORDAHEAD   0x4

/*
 * pageout request is targeting a real file
 * as opposed to a swap file.
 */

#define UPL_VNODE_PAGER 0x8
/*
 * this pageout is being originated as part of an explicit
 * memory synchronization operation... no speculative clustering
 * should be applied, only the range specified should be pushed.
 */
#define UPL_MSYNC               0x10

/*
 *
 */
#define UPL_PAGING_ENCRYPTED    0x20

/*
 * this pageout is being originated as part of an explicit
 * memory synchronization operation that is checking for I/O
 * errors and taking it's own action... if an error occurs,
 * just abort the pages back into the cache unchanged
 */
#define UPL_KEEPCACHED          0x40

/*
 * this pageout originated from within cluster_io to deal
 * with a dirty page that hasn't yet been seen by the FS
 * that backs it... tag it so that the FS can take the
 * appropriate action w/r to its locking model since the
 * pageout will reenter the FS for the same file currently
 * being handled in this context.
 */
#define UPL_NESTED_PAGEOUT      0x80

/*
 * we've detected a sequential access pattern and
 * we are speculatively and aggressively pulling
 * pages in... do not count these as real PAGEINs
 * w/r to our hard throttle maintenance
 */
#define UPL_IOSTREAMING         0x100

/*
 * Currently, it's only used for the swap pagein path.
 * Since the swap + compressed pager layer manage their
 * pages, these pages are not marked "absent" i.e. these
 * are "valid" pages. The pagein path will _not_ issue an
 * I/O (correctly) for valid pages. So, this flag is used
 * to override that logic in the vnode I/O path.
 */
#define UPL_IGNORE_VALID_PAGE_CHECK     0x200



/* upl commit flags */
#define UPL_COMMIT_FREE_ON_EMPTY        0x1 /* only implemented in wrappers */
#define UPL_COMMIT_CLEAR_DIRTY          0x2
#define UPL_COMMIT_SET_DIRTY            0x4
#define UPL_COMMIT_INACTIVATE           0x8
#define UPL_COMMIT_NOTIFY_EMPTY         0x10
/* deprecated: #define UPL_COMMIT_ALLOW_ACCESS		0x20 */
#define UPL_COMMIT_CS_VALIDATED         0x40
#define UPL_COMMIT_CLEAR_PRECIOUS       0x80
#define UPL_COMMIT_SPECULATE            0x100
#define UPL_COMMIT_FREE_ABSENT          0x200
#define UPL_COMMIT_WRITTEN_BY_KERNEL    0x400

#define UPL_COMMIT_KERNEL_ONLY_FLAGS    (UPL_COMMIT_CS_VALIDATED | UPL_COMMIT_FREE_ABSENT)

/* flags for return of state from vm_map_get_upl,  vm_upl address space */
/* based call */
#define UPL_DEV_MEMORY                  0x1
#define UPL_PHYS_CONTIG                 0x2


/*
 * Flags for the UPL page ops routine.  This routine is not exported
 * out of the kernel at the moment and so the defs live here.
 */
#define UPL_POP_DIRTY           0x1
#define UPL_POP_PAGEOUT         0x2
#define UPL_POP_PRECIOUS                0x4
#define UPL_POP_ABSENT          0x8
#define UPL_POP_BUSY                    0x10

#define UPL_POP_PHYSICAL        0x10000000
#define UPL_POP_DUMP            0x20000000
#define UPL_POP_SET             0x40000000
#define UPL_POP_CLR             0x80000000

/*
 * Flags for the UPL range op routine.  This routine is not exported
 * out of the kernel at the moemet and so the defs live here.
 */
/*
 * UPL_ROP_ABSENT: Returns the extent of the range presented which
 * is absent, starting with the start address presented
 */
#define UPL_ROP_ABSENT          0x01
/*
 * UPL_ROP_PRESENT: Returns the extent of the range presented which
 * is present (i.e. resident), starting with the start address presented
 */
#define UPL_ROP_PRESENT         0x02
/*
 * UPL_ROP_DUMP: Dump the pages which are found in the target object
 * for the target range.
 */
#define UPL_ROP_DUMP                    0x04

#ifdef  PRIVATE

#define UPL_REPRIO_INFO_MASK    (0xFFFFFFFF)
#define UPL_REPRIO_INFO_SHIFT   32

/* access macros for upl_t */

#define UPL_DEVICE_PAGE(upl) \
	(((upl)[0].phys_addr != 0) ? ((upl)[0].device) : FALSE)

#define UPL_PAGE_PRESENT(upl, index) \
	((upl)[(index)].phys_addr != 0)

#define UPL_PHYS_PAGE(upl, index) \
	((upl)[(index)].phys_addr)

#define UPL_SPECULATIVE_PAGE(upl, index) \
	(((upl)[(index)].phys_addr != 0) ? ((upl)[(index)].speculative) : FALSE)

#define UPL_DIRTY_PAGE(upl, index) \
	(((upl)[(index)].phys_addr != 0) ? ((upl)[(index)].dirty) : FALSE)

#define UPL_PRECIOUS_PAGE(upl, index) \
	(((upl)[(index)].phys_addr != 0) ? ((upl)[(index)].precious) : FALSE)

#define UPL_VALID_PAGE(upl, index) \
	(((upl)[(index)].phys_addr != 0) ? (!((upl)[(index)].absent)) : FALSE)

#define UPL_PAGEOUT_PAGE(upl, index) \
	(((upl)[(index)].phys_addr != 0) ? ((upl)[(index)].free_when_done) : FALSE)

#define UPL_SET_PAGE_FREE_ON_COMMIT(upl, index) \
	(((upl)[(index)].phys_addr != 0) ?            \
	 ((upl)[(index)].free_when_done = TRUE) : FALSE)

#define UPL_CLR_PAGE_FREE_ON_COMMIT(upl, index) \
	(((upl)[(index)].phys_addr != 0) ?       \
	 ((upl)[(index)].free_when_done = FALSE) : FALSE)

#define UPL_REPRIO_INFO_BLKNO(upl, index) \
	(((upl)->upl_reprio_info[(index)]) & UPL_REPRIO_INFO_MASK)

#define UPL_REPRIO_INFO_LEN(upl, index) \
	((((upl)->upl_reprio_info[(index)]) >> UPL_REPRIO_INFO_SHIFT) & UPL_REPRIO_INFO_MASK)

/* modifier macros for upl_t */

#define UPL_SET_CS_VALIDATED(upl, index, value) \
	((upl)[(index)].cs_validated = ((value) ? TRUE : FALSE))

#define UPL_SET_CS_TAINTED(upl, index, value) \
	((upl)[(index)].cs_tainted = ((value) ? TRUE : FALSE))

#define UPL_SET_CS_NX(upl, index, value) \
	((upl)[(index)].cs_nx = ((value) ? TRUE : FALSE))

#define UPL_SET_REPRIO_INFO(upl, index, blkno, len) \
	((upl)->upl_reprio_info[(index)]) = (((uint64_t)(blkno) & UPL_REPRIO_INFO_MASK) | \
	(((uint64_t)(len) & UPL_REPRIO_INFO_MASK) << UPL_REPRIO_INFO_SHIFT))

/* The call prototyped below is used strictly by UPL_GET_INTERNAL_PAGE_LIST */

extern vm_size_t        upl_offset_to_pagelist;
extern vm_size_t        upl_get_internal_pagelist_offset(void);
extern void*            upl_get_internal_vectorupl(upl_t);
extern upl_page_info_t*         upl_get_internal_vectorupl_pagelist(upl_t);

/*Use this variant to get the UPL's page list iff:*/
/*- the upl being passed in is already part of a vector UPL*/
/*- the page list you want is that of this "sub-upl" and not that of the entire vector-upl*/

#define UPL_GET_INTERNAL_PAGE_LIST_SIMPLE(upl) \
	((upl_page_info_t *)((upl_offset_to_pagelist == 0) ?  \
	(uintptr_t)upl + (unsigned int)(upl_offset_to_pagelist = upl_get_internal_pagelist_offset()): \
	(uintptr_t)upl + (unsigned int)upl_offset_to_pagelist))

/* UPL_GET_INTERNAL_PAGE_LIST is only valid on internal objects where the */
/* list request was made with the UPL_INTERNAL flag */


#define UPL_GET_INTERNAL_PAGE_LIST(upl) \
	((upl_get_internal_vectorupl(upl) != NULL ) ? (upl_get_internal_vectorupl_pagelist(upl)) : \
	((upl_page_info_t *)((upl_offset_to_pagelist == 0) ?  \
	(uintptr_t)upl + (unsigned int)(upl_offset_to_pagelist = upl_get_internal_pagelist_offset()): \
	(uintptr_t)upl + (unsigned int)upl_offset_to_pagelist)))

__BEGIN_DECLS

extern ppnum_t  upl_phys_page(upl_page_info_t *upl, int index);
extern boolean_t        upl_device_page(upl_page_info_t *upl);
extern boolean_t        upl_speculative_page(upl_page_info_t *upl, int index);
extern void     upl_clear_dirty(upl_t upl, boolean_t value);
extern void     upl_set_referenced(upl_t upl, boolean_t value);
extern void     upl_range_needed(upl_t upl, int index, int count);
#if CONFIG_IOSCHED
extern int64_t upl_blkno(upl_page_info_t *upl, int index);
extern void     upl_set_blkno(upl_t upl, vm_offset_t upl_offset, int size, int64_t blkno);
#endif

__END_DECLS

#endif /* PRIVATE */

__BEGIN_DECLS

extern boolean_t        upl_page_present(upl_page_info_t *upl, int index);
extern boolean_t        upl_dirty_page(upl_page_info_t *upl, int index);
extern boolean_t        upl_valid_page(upl_page_info_t *upl, int index);
extern void             upl_deallocate(upl_t upl);
extern void             upl_mark_decmp(upl_t upl);
extern void             upl_unmark_decmp(upl_t upl);

#ifdef KERNEL_PRIVATE

void upl_page_set_mark(upl_page_info_t *upl, int index, boolean_t v);
boolean_t upl_page_get_mark(upl_page_info_t *upl, int index);

#endif // KERNEL_PRIVATE

__END_DECLS

#endif  /* KERNEL */

#endif  /* _MACH_MEMORY_OBJECT_TYPES_H_ */
