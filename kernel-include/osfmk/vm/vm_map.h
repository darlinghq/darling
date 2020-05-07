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
 *	File:	vm/vm_map.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1985
 *
 *	Virtual memory map module definitions.
 *
 * Contributors:
 *	avie, dlb, mwyoung
 */

#ifndef _VM_VM_MAP_H_
#define _VM_VM_MAP_H_

#include <mach/mach_types.h>
#include <mach/kern_return.h>
#include <mach/boolean.h>
#include <mach/vm_types.h>
#include <mach/vm_prot.h>
#include <mach/vm_inherit.h>
#include <mach/vm_behavior.h>
#include <mach/vm_param.h>
#include <mach/sdt.h>
#include <vm/pmap.h>
#include <os/overflow.h>

#ifdef  KERNEL_PRIVATE

#include <sys/cdefs.h>

__BEGIN_DECLS

extern void     vm_map_reference(vm_map_t       map);
extern vm_map_t current_map(void);

/* Setup reserved areas in a new VM map */
extern kern_return_t    vm_map_exec(
	vm_map_t                new_map,
	task_t                  task,
	boolean_t               is64bit,
	void                    *fsroot,
	cpu_type_t              cpu,
	cpu_subtype_t           cpu_subtype);

__END_DECLS

#ifdef  MACH_KERNEL_PRIVATE

#include <task_swapper.h>
#include <mach_assert.h>

#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <kern/locks.h>
#include <kern/zalloc.h>
#include <kern/macro_help.h>

#include <kern/thread.h>
#include <os/refcnt.h>

#define current_map_fast()      (current_thread()->map)
#define current_map()           (current_map_fast())

#include <vm/vm_map_store.h>


/*
 *	Types defined:
 *
 *	vm_map_t		the high-level address map data structure.
 *	vm_map_entry_t		an entry in an address map.
 *	vm_map_version_t	a timestamp of a map, for use with vm_map_lookup
 *	vm_map_copy_t		represents memory copied from an address map,
 *				 used for inter-map copy operations
 */
typedef struct vm_map_entry     *vm_map_entry_t;
#define VM_MAP_ENTRY_NULL       ((vm_map_entry_t) NULL)


/*
 *	Type:		vm_map_object_t [internal use only]
 *
 *	Description:
 *		The target of an address mapping, either a virtual
 *		memory object or a sub map (of the kernel map).
 */
typedef union vm_map_object {
	vm_object_t             vmo_object;     /* object object */
	vm_map_t                vmo_submap;     /* belongs to another map */
} vm_map_object_t;

#define named_entry_lock_init(object)   lck_mtx_init(&(object)->Lock, &vm_object_lck_grp, &vm_object_lck_attr)
#define named_entry_lock_destroy(object)        lck_mtx_destroy(&(object)->Lock, &vm_object_lck_grp)
#define named_entry_lock(object)                lck_mtx_lock(&(object)->Lock)
#define named_entry_unlock(object)              lck_mtx_unlock(&(object)->Lock)
#if VM_NAMED_ENTRY_LIST
extern queue_head_t vm_named_entry_list;
#endif /* VM_NAMED_ENTRY_LIST */

/*
 *	Type:		vm_named_entry_t [internal use only]
 *
 *	Description:
 *		Description of a mapping to a memory cache object.
 *
 *	Implementation:
 *		While the handle to this object is used as a means to map
 *              and pass around the right to map regions backed by pagers
 *		of all sorts, the named_entry itself is only manipulated
 *		by the kernel.  Named entries hold information on the
 *		right to map a region of a cached object.  Namely,
 *		the target cache object, the beginning and ending of the
 *		region to be mapped, and the permissions, (read, write)
 *		with which it can be mapped.
 *
 */

struct vm_named_entry {
	decl_lck_mtx_data(, Lock);              /* Synchronization */
	union {
		vm_object_t     object;         /* object I point to */
		vm_map_t        map;            /* map backing submap */
		vm_map_copy_t   copy;           /* a VM map copy */
	} backing;
	vm_object_offset_t      offset;         /* offset into object */
	vm_object_size_t        size;           /* size of region */
	vm_object_offset_t      data_offset;    /* offset to first byte of data */
	vm_prot_t               protection;     /* access permissions */
	int                     ref_count;      /* Number of references */
	unsigned int                            /* Is backing.xxx : */
	/* boolean_t */ internal:1,             /* ... an internal object */
	/* boolean_t */ is_sub_map:1,           /* ... a submap? */
	/* boolean_t */ is_copy:1;              /* ... a VM map copy */
#if VM_NAMED_ENTRY_LIST
	queue_chain_t           named_entry_list;
	int                     named_entry_alias;
	mach_port_t             named_entry_port;
#define NAMED_ENTRY_BT_DEPTH 16
	void                    *named_entry_bt[NAMED_ENTRY_BT_DEPTH];
#endif /* VM_NAMED_ENTRY_LIST */
};

/*
 *	Type:		vm_map_entry_t [internal use only]
 *
 *	Description:
 *		A single mapping within an address map.
 *
 *	Implementation:
 *		Address map entries consist of start and end addresses,
 *		a VM object (or sub map) and offset into that object,
 *		and user-exported inheritance and protection information.
 *		Control information for virtual copy operations is also
 *		stored in the address map entry.
 */

struct vm_map_links {
	struct vm_map_entry     *prev;          /* previous entry */
	struct vm_map_entry     *next;          /* next entry */
	vm_map_offset_t         start;          /* start address */
	vm_map_offset_t         end;            /* end address */
};

/*
 * FOOTPRINT ACCOUNTING:
 * The "memory footprint" is better described in the pmap layer.
 *
 * At the VM level, these 2 vm_map_entry_t fields are relevant:
 * iokit_mapped:
 *	For an "iokit_mapped" entry, we add the size of the entry to the
 *	footprint when the entry is entered into the map and we subtract that
 *	size when the entry is removed.  No other accounting should take place.
 *	"use_pmap" should be FALSE but is not taken into account.
 * use_pmap: (only when is_sub_map is FALSE)
 *	This indicates if we should ask the pmap layer to account for pages
 *	in this mapping.  If FALSE, we expect that another form of accounting
 *	is being used (e.g. "iokit_mapped" or the explicit accounting of
 *	non-volatile purgable memory).
 *
 * So the logic is mostly:
 * if entry->is_sub_map == TRUE
 *	anything in a submap does not count for the footprint
 * else if entry->iokit_mapped == TRUE
 *	footprint includes the entire virtual size of this entry
 * else if entry->use_pmap == FALSE
 *	tell pmap NOT to account for pages being pmap_enter()'d from this
 *	mapping (i.e. use "alternate accounting")
 * else
 *	pmap will account for pages being pmap_enter()'d from this mapping
 *	as it sees fit (only if anonymous, etc...)
 */

struct vm_map_entry {
	struct vm_map_links     links;          /* links to other entries */
#define vme_prev                links.prev
#define vme_next                links.next
#define vme_start               links.start
#define vme_end                 links.end

	struct vm_map_store     store;
	union vm_map_object     vme_object;     /* object I point to */
	vm_object_offset_t      vme_offset;     /* offset into object */

	unsigned int
	/* boolean_t */ is_shared:1,    /* region is shared */
	/* boolean_t */ is_sub_map:1,   /* Is "object" a submap? */
	/* boolean_t */ in_transition:1, /* Entry being changed */
	/* boolean_t */ needs_wakeup:1, /* Waiters on in_transition */
	/* vm_behavior_t */ behavior:2, /* user paging behavior hint */
	/* behavior is not defined for submap type */
	/* boolean_t */ needs_copy:1,   /* object need to be copied? */

	/* Only in task maps: */
	/* vm_prot_t */ protection:3,   /* protection code */
	/* vm_prot_t */ max_protection:3, /* maximum protection */
	/* vm_inherit_t */ inheritance:2, /* inheritance */
	/* boolean_t */ use_pmap:1,     /*
	                                 * use_pmap is overloaded:
	                                 * if "is_sub_map":
	                                 *      use a nested pmap?
	                                 * else (i.e. if object):
	                                 *      use pmap accounting
	                                 *      for footprint?
	                                 */
	/* boolean_t */ no_cache:1,     /* should new pages be cached? */
	/* boolean_t */ permanent:1,    /* mapping can not be removed */
	/* boolean_t */ superpage_size:1, /* use superpages of a certain size */
	/* boolean_t */ map_aligned:1,  /* align to map's page size */
	/* boolean_t */ zero_wired_pages:1, /* zero out the wired pages of
	                                     * this entry it is being deleted
	                                     * without unwiring them */
	/* boolean_t */ used_for_jit:1,
	/* boolean_t */ pmap_cs_associated:1, /* pmap_cs will validate */
	/* boolean_t */ from_reserved_zone:1, /* Allocated from
	                                       * kernel reserved zone	 */

	/* iokit accounting: use the virtual size rather than resident size: */
	/* boolean_t */ iokit_acct:1,
	/* boolean_t */ vme_resilient_codesign:1,
	/* boolean_t */ vme_resilient_media:1,
	/* boolean_t */ vme_atomic:1, /* entry cannot be split/coalesced */
	/* boolean_t */ vme_no_copy_on_read:1,
	__unused:3;

	unsigned short          wired_count;    /* can be paged if = 0 */
	unsigned short          user_wired_count; /* for vm_wire */
#if     DEBUG
#define MAP_ENTRY_CREATION_DEBUG (1)
#define MAP_ENTRY_INSERTION_DEBUG (1)
#endif
#if     MAP_ENTRY_CREATION_DEBUG
	struct vm_map_header    *vme_creation_maphdr;
	uintptr_t               vme_creation_bt[16];
#endif
#if     MAP_ENTRY_INSERTION_DEBUG
	uintptr_t               vme_insertion_bt[16];
#endif
};

#define VME_SUBMAP_PTR(entry)                   \
	(&((entry)->vme_object.vmo_submap))
#define VME_SUBMAP(entry)                                       \
	((vm_map_t)((uintptr_t)0 + *VME_SUBMAP_PTR(entry)))
#define VME_OBJECT_PTR(entry)                   \
	(&((entry)->vme_object.vmo_object))
#define VME_OBJECT(entry)                                       \
	((vm_object_t)((uintptr_t)0 + *VME_OBJECT_PTR(entry)))
#define VME_OFFSET(entry)                       \
	((entry)->vme_offset & ~PAGE_MASK)
#define VME_ALIAS_MASK (PAGE_MASK)
#define VME_ALIAS(entry)                                        \
	((unsigned int)((entry)->vme_offset & VME_ALIAS_MASK))

static inline void
VME_OBJECT_SET(
	vm_map_entry_t entry,
	vm_object_t object)
{
	entry->vme_object.vmo_object = object;
	if (object != VM_OBJECT_NULL && !object->internal) {
		entry->vme_resilient_media = FALSE;
	}
	entry->vme_resilient_codesign = FALSE;
	entry->used_for_jit = FALSE;
}
static inline void
VME_SUBMAP_SET(
	vm_map_entry_t entry,
	vm_map_t submap)
{
	entry->vme_object.vmo_submap = submap;
}
static inline void
VME_OFFSET_SET(
	vm_map_entry_t entry,
	vm_map_offset_t offset)
{
	int alias;
	alias = VME_ALIAS(entry);
	assert((offset & PAGE_MASK) == 0);
	entry->vme_offset = offset | alias;
}
/*
 * IMPORTANT:
 * The "alias" field can be updated while holding the VM map lock
 * "shared".  It's OK as along as it's the only field that can be
 * updated without the VM map "exclusive" lock.
 */
static inline void
VME_ALIAS_SET(
	vm_map_entry_t entry,
	int alias)
{
	vm_map_offset_t offset;
	offset = VME_OFFSET(entry);
	entry->vme_offset = offset | (alias & VME_ALIAS_MASK);
}

static inline void
VME_OBJECT_SHADOW(
	vm_map_entry_t entry,
	vm_object_size_t length)
{
	vm_object_t object;
	vm_object_offset_t offset;

	object = VME_OBJECT(entry);
	offset = VME_OFFSET(entry);
	vm_object_shadow(&object, &offset, length);
	if (object != VME_OBJECT(entry)) {
		VME_OBJECT_SET(entry, object);
		entry->use_pmap = TRUE;
	}
	if (offset != VME_OFFSET(entry)) {
		VME_OFFSET_SET(entry, offset);
	}
}


/*
 * Convenience macros for dealing with superpages
 * SUPERPAGE_NBASEPAGES is architecture dependent and defined in pmap.h
 */
#define SUPERPAGE_SIZE (PAGE_SIZE*SUPERPAGE_NBASEPAGES)
#define SUPERPAGE_MASK (-SUPERPAGE_SIZE)
#define SUPERPAGE_ROUND_DOWN(a) (a & SUPERPAGE_MASK)
#define SUPERPAGE_ROUND_UP(a) ((a + SUPERPAGE_SIZE-1) & SUPERPAGE_MASK)

/*
 * wired_counts are unsigned short.  This value is used to safeguard
 * against any mishaps due to runaway user programs.
 */
#define MAX_WIRE_COUNT          65535



/*
 *	Type:		struct vm_map_header
 *
 *	Description:
 *		Header for a vm_map and a vm_map_copy.
 */


struct vm_map_header {
	struct vm_map_links     links;          /* first, last, min, max */
	int                     nentries;       /* Number of entries */
	boolean_t               entries_pageable;
	/* are map entries pageable? */
#ifdef VM_MAP_STORE_USE_RB
	struct rb_head  rb_head_store;
#endif
	int                     page_shift;     /* page shift */
};

#define VM_MAP_HDR_PAGE_SHIFT(hdr) ((hdr)->page_shift)
#define VM_MAP_HDR_PAGE_SIZE(hdr) (1 << VM_MAP_HDR_PAGE_SHIFT((hdr)))
#define VM_MAP_HDR_PAGE_MASK(hdr) (VM_MAP_HDR_PAGE_SIZE((hdr)) - 1)

/*
 *	Type:		vm_map_t [exported; contents invisible]
 *
 *	Description:
 *		An address map -- a directory relating valid
 *		regions of a task's address space to the corresponding
 *		virtual memory objects.
 *
 *	Implementation:
 *		Maps are doubly-linked lists of map entries, sorted
 *		by address.  One hint is used to start
 *		searches again from the last successful search,
 *		insertion, or removal.  Another hint is used to
 *		quickly find free space.
 */
struct _vm_map {
	lck_rw_t                lock;           /* map lock */
	struct vm_map_header    hdr;            /* Map entry header */
#define min_offset              hdr.links.start /* start of range */
#define max_offset              hdr.links.end   /* end of range */
	pmap_t                  pmap;           /* Physical map */
	vm_map_size_t           size;           /* virtual size */
	vm_map_size_t           user_wire_limit;/* rlimit on user locked memory */
	vm_map_size_t           user_wire_size; /* current size of user locked memory in this map */
#if !CONFIG_EMBEDDED
	vm_map_offset_t         vmmap_high_start;
#endif

	union {
		/*
		 * If map->disable_vmentry_reuse == TRUE:
		 * the end address of the highest allocated vm_map_entry_t.
		 */
		vm_map_offset_t         vmu1_highest_entry_end;
		/*
		 * For a nested VM map:
		 * the lowest address in this nested VM map that we would
		 * expect to be unnested under normal operation (i.e. for
		 * regular copy-on-write on DATA section).
		 */
		vm_map_offset_t         vmu1_lowest_unnestable_start;
	} vmu1;
#define highest_entry_end       vmu1.vmu1_highest_entry_end
#define lowest_unnestable_start vmu1.vmu1_lowest_unnestable_start
	decl_lck_mtx_data(, s_lock);                    /* Lock ref, res fields */
	lck_mtx_ext_t           s_lock_ext;
	vm_map_entry_t          hint;           /* hint for quick lookups */
	union {
		struct vm_map_links* vmmap_hole_hint;   /* hint for quick hole lookups */
		struct vm_map_corpse_footprint_header *vmmap_corpse_footprint;
	} vmmap_u_1;
#define hole_hint vmmap_u_1.vmmap_hole_hint
#define vmmap_corpse_footprint vmmap_u_1.vmmap_corpse_footprint
	union {
		vm_map_entry_t          _first_free;    /* First free space hint */
		struct vm_map_links*    _holes;         /* links all holes between entries */
	} f_s;                                          /* Union for free space data structures being used */

#define first_free              f_s._first_free
#define holes_list              f_s._holes

	struct os_refcnt        map_refcnt;     /* Reference count */

#if     TASK_SWAPPER
	int                     res_count;      /* Residence count (swap) */
	int                     sw_state;       /* Swap state */
#endif  /* TASK_SWAPPER */

	unsigned int
	/* boolean_t */ wait_for_space:1,         /* Should callers wait for space? */
	/* boolean_t */ wiring_required:1,         /* All memory wired? */
	/* boolean_t */ no_zero_fill:1,         /*No zero fill absent pages */
	/* boolean_t */ mapped_in_other_pmaps:1,         /*has this submap been mapped in maps that use a different pmap */
	/* boolean_t */ switch_protect:1,         /*  Protect map from write faults while switched */
	/* boolean_t */ disable_vmentry_reuse:1,         /*  All vm entries should keep using newer and higher addresses in the map */
	/* boolean_t */ map_disallow_data_exec:1,         /* Disallow execution from data pages on exec-permissive architectures */
	/* boolean_t */ holelistenabled:1,
	/* boolean_t */ is_nested_map:1,
	/* boolean_t */ map_disallow_new_exec:1,         /* Disallow new executable code */
	/* boolean_t */ jit_entry_exists:1,
	/* boolean_t */ has_corpse_footprint:1,
	/* reserved */ pad:20;
	unsigned int            timestamp;      /* Version number */
};

#define CAST_TO_VM_MAP_ENTRY(x) ((struct vm_map_entry *)(uintptr_t)(x))
#define vm_map_to_entry(map) CAST_TO_VM_MAP_ENTRY(&(map)->hdr.links)
#define vm_map_first_entry(map) ((map)->hdr.links.next)
#define vm_map_last_entry(map)  ((map)->hdr.links.prev)

#if     TASK_SWAPPER
/*
 * VM map swap states.  There are no transition states.
 */
#define MAP_SW_IN        1      /* map is swapped in; residence count > 0 */
#define MAP_SW_OUT       2      /* map is out (res_count == 0 */
#endif  /* TASK_SWAPPER */

/*
 *	Type:		vm_map_version_t [exported; contents invisible]
 *
 *	Description:
 *		Map versions may be used to quickly validate a previous
 *		lookup operation.
 *
 *	Usage note:
 *		Because they are bulky objects, map versions are usually
 *		passed by reference.
 *
 *	Implementation:
 *		Just a timestamp for the main map.
 */
typedef struct vm_map_version {
	unsigned int    main_timestamp;
} vm_map_version_t;

/*
 *	Type:		vm_map_copy_t [exported; contents invisible]
 *
 *	Description:
 *		A map copy object represents a region of virtual memory
 *		that has been copied from an address map but is still
 *		in transit.
 *
 *		A map copy object may only be used by a single thread
 *		at a time.
 *
 *	Implementation:
 *              There are three formats for map copy objects.
 *		The first is very similar to the main
 *		address map in structure, and as a result, some
 *		of the internal maintenance functions/macros can
 *		be used with either address maps or map copy objects.
 *
 *		The map copy object contains a header links
 *		entry onto which the other entries that represent
 *		the region are chained.
 *
 *		The second format is a single vm object.  This was used
 *		primarily in the pageout path - but is not currently used
 *		except for placeholder copy objects (see vm_map_copy_copy()).
 *
 *		The third format is a kernel buffer copy object - for data
 *              small enough that physical copies were the most efficient
 *		method. This method uses a zero-sized array unioned with
 *		other format-specific data in the 'c_u' member. This unsized
 *		array overlaps the other elements and allows us to use this
 *		extra structure space for physical memory copies. On 64-bit
 *		systems this saves ~64 bytes per vm_map_copy.
 */

struct vm_map_copy {
	int                     type;
#define VM_MAP_COPY_ENTRY_LIST          1
#define VM_MAP_COPY_OBJECT              2
#define VM_MAP_COPY_KERNEL_BUFFER       3
	vm_object_offset_t      offset;
	vm_map_size_t           size;
	union {
		struct vm_map_header    hdr;      /* ENTRY_LIST */
		vm_object_t             object;   /* OBJECT */
		uint8_t                 kdata[0]; /* KERNEL_BUFFER */
	} c_u;
};


#define cpy_hdr                 c_u.hdr

#define cpy_object              c_u.object
#define cpy_kdata               c_u.kdata
#define cpy_kdata_hdr_sz        (offsetof(struct vm_map_copy, c_u.kdata))

#define VM_MAP_COPY_PAGE_SHIFT(copy) ((copy)->cpy_hdr.page_shift)
#define VM_MAP_COPY_PAGE_SIZE(copy) (1 << VM_MAP_COPY_PAGE_SHIFT((copy)))
#define VM_MAP_COPY_PAGE_MASK(copy) (VM_MAP_COPY_PAGE_SIZE((copy)) - 1)

/*
 *	Useful macros for entry list copy objects
 */

#define vm_map_copy_to_entry(copy) CAST_TO_VM_MAP_ENTRY(&(copy)->cpy_hdr.links)
#define vm_map_copy_first_entry(copy)           \
	        ((copy)->cpy_hdr.links.next)
#define vm_map_copy_last_entry(copy)            \
	        ((copy)->cpy_hdr.links.prev)

/*
 *	Macros:		vm_map_lock, etc. [internal use only]
 *	Description:
 *		Perform locking on the data portion of a map.
 *	When multiple maps are to be locked, order by map address.
 *	(See vm_map.c::vm_remap())
 */

#define vm_map_lock_init(map)                                           \
	((map)->timestamp = 0 ,                                         \
	lck_rw_init(&(map)->lock, &vm_map_lck_grp, &vm_map_lck_rw_attr))

#define vm_map_lock(map)                     \
	MACRO_BEGIN                          \
	DTRACE_VM(vm_map_lock_w);            \
	lck_rw_lock_exclusive(&(map)->lock); \
	MACRO_END

#define vm_map_unlock(map)          \
	MACRO_BEGIN                 \
	DTRACE_VM(vm_map_unlock_w); \
	(map)->timestamp++;         \
	lck_rw_done(&(map)->lock);  \
	MACRO_END

#define vm_map_lock_read(map)             \
	MACRO_BEGIN                       \
	DTRACE_VM(vm_map_lock_r);         \
	lck_rw_lock_shared(&(map)->lock); \
	MACRO_END

#define vm_map_unlock_read(map)     \
	MACRO_BEGIN                 \
	DTRACE_VM(vm_map_unlock_r); \
	lck_rw_done(&(map)->lock);  \
	MACRO_END

#define vm_map_lock_write_to_read(map)                 \
	MACRO_BEGIN                                    \
	DTRACE_VM(vm_map_lock_downgrade);              \
	(map)->timestamp++;                            \
	lck_rw_lock_exclusive_to_shared(&(map)->lock); \
	MACRO_END

__attribute__((always_inline))
int vm_map_lock_read_to_write(vm_map_t map);

__attribute__((always_inline))
boolean_t vm_map_try_lock(vm_map_t map);

__attribute__((always_inline))
boolean_t vm_map_try_lock_read(vm_map_t map);

#if MACH_ASSERT || DEBUG
#define vm_map_lock_assert_held(map) \
	lck_rw_assert(&(map)->lock, LCK_RW_ASSERT_HELD)
#define vm_map_lock_assert_shared(map)  \
	lck_rw_assert(&(map)->lock, LCK_RW_ASSERT_SHARED)
#define vm_map_lock_assert_exclusive(map) \
	lck_rw_assert(&(map)->lock, LCK_RW_ASSERT_EXCLUSIVE)
#define vm_map_lock_assert_notheld(map) \
	lck_rw_assert(&(map)->lock, LCK_RW_ASSERT_NOTHELD)
#else  /* MACH_ASSERT || DEBUG */
#define vm_map_lock_assert_held(map)
#define vm_map_lock_assert_shared(map)
#define vm_map_lock_assert_exclusive(map)
#define vm_map_lock_assert_notheld(map)
#endif /* MACH_ASSERT || DEBUG */

/*
 *	Exported procedures that operate on vm_map_t.
 */

/* Initialize the module */
extern void             vm_map_init(void);

extern void             vm_kernel_reserved_entry_init(void);

/* Allocate a range in the specified virtual address map and
 * return the entry allocated for that range. */
extern kern_return_t vm_map_find_space(
	vm_map_t                map,
	vm_map_address_t        *address,                               /* OUT */
	vm_map_size_t           size,
	vm_map_offset_t         mask,
	int                     flags,
	vm_map_kernel_flags_t   vmk_flags,
	vm_tag_t                tag,
	vm_map_entry_t          *o_entry);                              /* OUT */

extern void vm_map_clip_start(
	vm_map_t        map,
	vm_map_entry_t  entry,
	vm_map_offset_t endaddr);
extern void vm_map_clip_end(
	vm_map_t        map,
	vm_map_entry_t  entry,
	vm_map_offset_t endaddr);
extern boolean_t vm_map_entry_should_cow_for_true_share(
	vm_map_entry_t  entry);

/* Lookup map entry containing or the specified address in the given map */
extern boolean_t        vm_map_lookup_entry(
	vm_map_t                map,
	vm_map_address_t        address,
	vm_map_entry_t          *entry);                                /* OUT */

extern void             vm_map_copy_remap(
	vm_map_t                map,
	vm_map_entry_t          where,
	vm_map_copy_t           copy,
	vm_map_offset_t         adjustment,
	vm_prot_t               cur_prot,
	vm_prot_t               max_prot,
	vm_inherit_t            inheritance);

/* Find the VM object, offset, and protection for a given virtual address
 * in the specified map, assuming a page fault of the	type specified. */
extern kern_return_t    vm_map_lookup_locked(
	vm_map_t                *var_map,                               /* IN/OUT */
	vm_map_address_t        vaddr,
	vm_prot_t               fault_type,
	int                     object_lock_type,
	vm_map_version_t        *out_version,                           /* OUT */
	vm_object_t             *object,                                /* OUT */
	vm_object_offset_t      *offset,                                /* OUT */
	vm_prot_t               *out_prot,                              /* OUT */
	boolean_t               *wired,                                 /* OUT */
	vm_object_fault_info_t  fault_info,                             /* OUT */
	vm_map_t                *real_map);                             /* OUT */

/* Verifies that the map has not changed since the given version. */
extern boolean_t        vm_map_verify(
	vm_map_t                map,
	vm_map_version_t        *version);                              /* REF */

extern vm_map_entry_t   vm_map_entry_insert(
	vm_map_t                map,
	vm_map_entry_t          insp_entry,
	vm_map_offset_t         start,
	vm_map_offset_t         end,
	vm_object_t             object,
	vm_object_offset_t      offset,
	boolean_t               needs_copy,
	boolean_t               is_shared,
	boolean_t               in_transition,
	vm_prot_t               cur_protection,
	vm_prot_t               max_protection,
	vm_behavior_t           behavior,
	vm_inherit_t            inheritance,
	unsigned                wired_count,
	boolean_t               no_cache,
	boolean_t               permanent,
	boolean_t               no_copy_on_read,
	unsigned int            superpage_size,
	boolean_t               clear_map_aligned,
	boolean_t               is_submap,
	boolean_t               used_for_jit,
	int                     alias);


/*
 *	Functions implemented as macros
 */
#define         vm_map_min(map) ((map)->min_offset)
/* Lowest valid address in
 * a map */

#define         vm_map_max(map) ((map)->max_offset)
/* Highest valid address */

#define         vm_map_pmap(map)        ((map)->pmap)
/* Physical map associated
* with this address map */

/*
 * Macros/functions for map residence counts and swapin/out of vm maps
 */
#if     TASK_SWAPPER

#if     MACH_ASSERT
/* Gain a reference to an existing map */
extern void             vm_map_reference(
	vm_map_t        map);
/* Lose a residence count */
extern void             vm_map_res_deallocate(
	vm_map_t        map);
/* Gain a residence count on a map */
extern void             vm_map_res_reference(
	vm_map_t        map);
/* Gain reference & residence counts to possibly swapped-out map */
extern void             vm_map_reference_swap(
	vm_map_t        map);

#else   /* MACH_ASSERT */

#define vm_map_reference(map)           \
MACRO_BEGIN                                      \
	vm_map_t Map = (map);                    \
	if (Map) {                               \
	        lck_mtx_lock(&Map->s_lock);      \
	        Map->res_count++;                \
	        os_ref_retain(&Map->map_refcnt); \
	        lck_mtx_unlock(&Map->s_lock);    \
	}                                        \
MACRO_END

#define vm_map_res_reference(map)               \
MACRO_BEGIN                                     \
	vm_map_t Lmap = (map);          \
	if (Lmap->res_count == 0) {             \
	        lck_mtx_unlock(&Lmap->s_lock);\
	        vm_map_lock(Lmap);              \
	        vm_map_swapin(Lmap);            \
	        lck_mtx_lock(&Lmap->s_lock);    \
	        ++Lmap->res_count;              \
	        vm_map_unlock(Lmap);            \
	} else                                  \
	        ++Lmap->res_count;              \
MACRO_END

#define vm_map_res_deallocate(map)              \
MACRO_BEGIN                                     \
	vm_map_t Map = (map);           \
	if (--Map->res_count == 0) {    \
	        lck_mtx_unlock(&Map->s_lock);   \
	        vm_map_lock(Map);               \
	        vm_map_swapout(Map);            \
	        vm_map_unlock(Map);             \
	        lck_mtx_lock(&Map->s_lock);     \
	}                                       \
MACRO_END

#define vm_map_reference_swap(map)      \
MACRO_BEGIN                             \
	vm_map_t Map = (map);           \
	lck_mtx_lock(&Map->s_lock);     \
	os_ref_retain(&Map->map_refcnt);\
	vm_map_res_reference(Map);      \
	lck_mtx_unlock(&Map->s_lock);   \
MACRO_END
#endif  /* MACH_ASSERT */

extern void             vm_map_swapin(
	vm_map_t        map);

extern void             vm_map_swapout(
	vm_map_t        map);

#else   /* TASK_SWAPPER */

#define vm_map_reference(map)                   \
MACRO_BEGIN                                     \
	vm_map_t Map = (map);                   \
	if (Map) {                              \
	        lck_mtx_lock(&Map->s_lock);     \
	        os_ref_retain(&Map->map_refcnt);\
	        lck_mtx_unlock(&Map->s_lock);   \
	}                                       \
MACRO_END

#define vm_map_reference_swap(map)      vm_map_reference(map)
#define vm_map_res_reference(map)
#define vm_map_res_deallocate(map)

#endif  /* TASK_SWAPPER */

/*
 *	Submap object.  Must be used to create memory to be put
 *	in a submap by vm_map_submap.
 */
extern vm_object_t      vm_submap_object;

/*
 *	Wait and wakeup macros for in_transition map entries.
 */
#define vm_map_entry_wait(map, interruptible)           \
	((map)->timestamp++ ,                           \
	 lck_rw_sleep(&(map)->lock, LCK_SLEEP_EXCLUSIVE|LCK_SLEEP_PROMOTED_PRI, \
	                          (event_t)&(map)->hdr,	interruptible))


#define vm_map_entry_wakeup(map)        \
	thread_wakeup((event_t)(&(map)->hdr))


#define vm_map_ref_fast(map)                    \
	MACRO_BEGIN                                     \
	lck_mtx_lock(&map->s_lock);                     \
	map->ref_count++;                               \
	vm_map_res_reference(map);                      \
	lck_mtx_unlock(&map->s_lock);                   \
	MACRO_END

#define vm_map_dealloc_fast(map)                \
	MACRO_BEGIN                                     \
	int c;                                          \
                                                        \
	lck_mtx_lock(&map->s_lock);                     \
	c = --map->ref_count;                   \
	if (c > 0)                                      \
	        vm_map_res_deallocate(map);             \
	lck_mtx_unlock(&map->s_lock);                   \
	if (c == 0)                                     \
	        vm_map_destroy(map);                    \
	MACRO_END


/* simplify map entries */
extern void             vm_map_simplify_entry(
	vm_map_t        map,
	vm_map_entry_t  this_entry);
extern void             vm_map_simplify(
	vm_map_t                map,
	vm_map_offset_t         start);

/* Move the information in a map copy object to a new map copy object */
extern vm_map_copy_t    vm_map_copy_copy(
	vm_map_copy_t           copy);

/* Create a copy object from an object. */
extern kern_return_t    vm_map_copyin_object(
	vm_object_t             object,
	vm_object_offset_t      offset,
	vm_object_size_t        size,
	vm_map_copy_t           *copy_result);                         /* OUT */

extern kern_return_t    vm_map_random_address_for_size(
	vm_map_t        map,
	vm_map_offset_t *address,
	vm_map_size_t   size);

/* Enter a mapping */
extern kern_return_t    vm_map_enter(
	vm_map_t                map,
	vm_map_offset_t         *address,
	vm_map_size_t           size,
	vm_map_offset_t         mask,
	int                     flags,
	vm_map_kernel_flags_t   vmk_flags,
	vm_tag_t                tag,
	vm_object_t             object,
	vm_object_offset_t      offset,
	boolean_t               needs_copy,
	vm_prot_t               cur_protection,
	vm_prot_t               max_protection,
	vm_inherit_t            inheritance);

#if __arm64__
extern kern_return_t    vm_map_enter_fourk(
	vm_map_t                map,
	vm_map_offset_t         *address,
	vm_map_size_t           size,
	vm_map_offset_t         mask,
	int                     flags,
	vm_map_kernel_flags_t   vmk_flags,
	vm_tag_t                tag,
	vm_object_t             object,
	vm_object_offset_t      offset,
	boolean_t               needs_copy,
	vm_prot_t               cur_protection,
	vm_prot_t               max_protection,
	vm_inherit_t            inheritance);
#endif /* __arm64__ */

/* XXX should go away - replaced with regular enter of contig object */
extern  kern_return_t   vm_map_enter_cpm(
	vm_map_t                map,
	vm_map_address_t        *addr,
	vm_map_size_t           size,
	int                     flags);

extern kern_return_t vm_map_remap(
	vm_map_t                target_map,
	vm_map_offset_t         *address,
	vm_map_size_t           size,
	vm_map_offset_t         mask,
	int                     flags,
	vm_map_kernel_flags_t   vmk_flags,
	vm_tag_t                tag,
	vm_map_t                src_map,
	vm_map_offset_t         memory_address,
	boolean_t               copy,
	vm_prot_t               *cur_protection,
	vm_prot_t               *max_protection,
	vm_inherit_t            inheritance);


/*
 * Read and write from a kernel buffer to a specified map.
 */
extern  kern_return_t   vm_map_write_user(
	vm_map_t                map,
	void                    *src_p,
	vm_map_offset_t         dst_addr,
	vm_size_t               size);

extern  kern_return_t   vm_map_read_user(
	vm_map_t                map,
	vm_map_offset_t         src_addr,
	void                    *dst_p,
	vm_size_t               size);

/* Create a new task map using an existing task map as a template. */
extern vm_map_t         vm_map_fork(
	ledger_t                ledger,
	vm_map_t                old_map,
	int                     options);
#define VM_MAP_FORK_SHARE_IF_INHERIT_NONE       0x00000001
#define VM_MAP_FORK_PRESERVE_PURGEABLE          0x00000002
#define VM_MAP_FORK_CORPSE_FOOTPRINT            0x00000004

/* Change inheritance */
extern kern_return_t    vm_map_inherit(
	vm_map_t                map,
	vm_map_offset_t         start,
	vm_map_offset_t         end,
	vm_inherit_t            new_inheritance);

/* Add or remove machine-dependent attributes from map regions */
extern kern_return_t    vm_map_machine_attribute(
	vm_map_t                map,
	vm_map_offset_t         start,
	vm_map_offset_t         end,
	vm_machine_attribute_t  attribute,
	vm_machine_attribute_val_t* value);                         /* IN/OUT */

extern kern_return_t    vm_map_msync(
	vm_map_t                map,
	vm_map_address_t        address,
	vm_map_size_t           size,
	vm_sync_t               sync_flags);

/* Set paging behavior */
extern kern_return_t    vm_map_behavior_set(
	vm_map_t                map,
	vm_map_offset_t         start,
	vm_map_offset_t         end,
	vm_behavior_t           new_behavior);

extern kern_return_t vm_map_region(
	vm_map_t                 map,
	vm_map_offset_t         *address,
	vm_map_size_t           *size,
	vm_region_flavor_t       flavor,
	vm_region_info_t         info,
	mach_msg_type_number_t  *count,
	mach_port_t             *object_name);

extern kern_return_t vm_map_region_recurse_64(
	vm_map_t                 map,
	vm_map_offset_t         *address,
	vm_map_size_t           *size,
	natural_t               *nesting_depth,
	vm_region_submap_info_64_t info,
	mach_msg_type_number_t  *count);

extern kern_return_t vm_map_page_query_internal(
	vm_map_t                map,
	vm_map_offset_t         offset,
	int                     *disposition,
	int                     *ref_count);

extern kern_return_t vm_map_query_volatile(
	vm_map_t        map,
	mach_vm_size_t  *volatile_virtual_size_p,
	mach_vm_size_t  *volatile_resident_size_p,
	mach_vm_size_t  *volatile_compressed_size_p,
	mach_vm_size_t  *volatile_pmap_size_p,
	mach_vm_size_t  *volatile_compressed_pmap_size_p);

extern kern_return_t    vm_map_submap(
	vm_map_t                map,
	vm_map_offset_t         start,
	vm_map_offset_t         end,
	vm_map_t                submap,
	vm_map_offset_t         offset,
	boolean_t               use_pmap);

extern void vm_map_submap_pmap_clean(
	vm_map_t        map,
	vm_map_offset_t start,
	vm_map_offset_t end,
	vm_map_t        sub_map,
	vm_map_offset_t offset);

/* Convert from a map entry port to a map */
extern vm_map_t convert_port_entry_to_map(
	ipc_port_t      port);

/* Convert from a port to a vm_object */
extern vm_object_t convert_port_entry_to_object(
	ipc_port_t      port);


extern kern_return_t vm_map_set_cache_attr(
	vm_map_t        map,
	vm_map_offset_t va);


/* definitions related to overriding the NX behavior */

#define VM_ABI_32       0x1
#define VM_ABI_64       0x2

extern int override_nx(vm_map_t map, uint32_t user_tag);

#if PMAP_CS
extern kern_return_t vm_map_entry_cs_associate(
	vm_map_t                map,
	vm_map_entry_t          entry,
	vm_map_kernel_flags_t   vmk_flags);
#endif /* PMAP_CS */

extern void vm_map_region_top_walk(
	vm_map_entry_t entry,
	vm_region_top_info_t top);
extern void vm_map_region_walk(
	vm_map_t map,
	vm_map_offset_t va,
	vm_map_entry_t entry,
	vm_object_offset_t offset,
	vm_object_size_t range,
	vm_region_extended_info_t extended,
	boolean_t look_for_pages,
	mach_msg_type_number_t count);


struct vm_map_corpse_footprint_header {
	vm_size_t       cf_size;        /* allocated buffer size */
	uint32_t        cf_last_region; /* offset of last region in buffer */
	union {
		uint32_t cfu_last_zeroes; /* during creation:
		                           * number of "zero" dispositions at
		                           * end of last region */
		uint32_t cfu_hint_region; /* during lookup:
		                           * offset of last looked up region */
#define cf_last_zeroes cfu.cfu_last_zeroes
#define cf_hint_region cfu.cfu_hint_region
	} cfu;
};
struct vm_map_corpse_footprint_region {
	vm_map_offset_t cfr_vaddr;      /* region start virtual address */
	uint32_t        cfr_num_pages;  /* number of pages in this "region" */
	unsigned char   cfr_disposition[0];     /* disposition of each page */
} __attribute__((packed));

extern kern_return_t vm_map_corpse_footprint_collect(
	vm_map_t        old_map,
	vm_map_entry_t  old_entry,
	vm_map_t        new_map);
extern void vm_map_corpse_footprint_collect_done(
	vm_map_t        new_map);

extern kern_return_t vm_map_corpse_footprint_query_page_info(
	vm_map_t        map,
	vm_map_offset_t va,
	int             *disp);

extern void vm_map_copy_footprint_ledgers(
	task_t  old_task,
	task_t  new_task);
extern void vm_map_copy_ledger(
	task_t  old_task,
	task_t  new_task,
	int     ledger_entry);

#endif /* MACH_KERNEL_PRIVATE */

__BEGIN_DECLS

/* Create an empty map */
extern vm_map_t         vm_map_create(
	pmap_t                  pmap,
	vm_map_offset_t         min_off,
	vm_map_offset_t         max_off,
	boolean_t               pageable);
extern vm_map_t vm_map_create_options(
	pmap_t                  pmap,
	vm_map_offset_t         min_off,
	vm_map_offset_t         max_off,
	int                     options);
#define VM_MAP_CREATE_PAGEABLE          0x00000001
#define VM_MAP_CREATE_CORPSE_FOOTPRINT  0x00000002
#define VM_MAP_CREATE_ALL_OPTIONS (VM_MAP_CREATE_PAGEABLE | \
	                           VM_MAP_CREATE_CORPSE_FOOTPRINT)

extern void             vm_map_disable_hole_optimization(vm_map_t map);

/* Get rid of a map */
extern void             vm_map_destroy(
	vm_map_t                map,
	int                     flags);

/* Lose a reference */
extern void             vm_map_deallocate(
	vm_map_t                map);

extern vm_map_t         vm_map_switch(
	vm_map_t                map);

/* Change protection */
extern kern_return_t    vm_map_protect(
	vm_map_t                map,
	vm_map_offset_t         start,
	vm_map_offset_t         end,
	vm_prot_t               new_prot,
	boolean_t               set_max);

/* Check protection */
extern boolean_t vm_map_check_protection(
	vm_map_t                map,
	vm_map_offset_t         start,
	vm_map_offset_t         end,
	vm_prot_t               protection);

/* wire down a region */

#ifdef XNU_KERNEL_PRIVATE

extern kern_return_t    vm_map_wire_kernel(
	vm_map_t                map,
	vm_map_offset_t         start,
	vm_map_offset_t         end,
	vm_prot_t               access_type,
	vm_tag_t                tag,
	boolean_t               user_wire);

extern kern_return_t    vm_map_wire_and_extract_kernel(
	vm_map_t                map,
	vm_map_offset_t         start,
	vm_prot_t               access_type,
	vm_tag_t                tag,
	boolean_t               user_wire,
	ppnum_t                 *physpage_p);

/* kext exported versions */

extern kern_return_t    vm_map_wire_external(
	vm_map_t                map,
	vm_map_offset_t         start,
	vm_map_offset_t         end,
	vm_prot_t               access_type,
	boolean_t               user_wire);

extern kern_return_t    vm_map_wire_and_extract_external(
	vm_map_t                map,
	vm_map_offset_t         start,
	vm_prot_t               access_type,
	boolean_t               user_wire,
	ppnum_t                 *physpage_p);

#else /* XNU_KERNEL_PRIVATE */

extern kern_return_t    vm_map_wire(
	vm_map_t                map,
	vm_map_offset_t         start,
	vm_map_offset_t         end,
	vm_prot_t               access_type,
	boolean_t               user_wire);

extern kern_return_t    vm_map_wire_and_extract(
	vm_map_t                map,
	vm_map_offset_t         start,
	vm_prot_t               access_type,
	boolean_t               user_wire,
	ppnum_t                 *physpage_p);

#endif /* !XNU_KERNEL_PRIVATE */

/* unwire a region */
extern kern_return_t    vm_map_unwire(
	vm_map_t                map,
	vm_map_offset_t         start,
	vm_map_offset_t         end,
	boolean_t               user_wire);

#ifdef XNU_KERNEL_PRIVATE

/* Enter a mapping of a memory object */
extern kern_return_t    vm_map_enter_mem_object(
	vm_map_t                map,
	vm_map_offset_t         *address,
	vm_map_size_t           size,
	vm_map_offset_t         mask,
	int                     flags,
	vm_map_kernel_flags_t   vmk_flags,
	vm_tag_t                tag,
	ipc_port_t              port,
	vm_object_offset_t      offset,
	boolean_t               needs_copy,
	vm_prot_t               cur_protection,
	vm_prot_t               max_protection,
	vm_inherit_t            inheritance);

/* Enter a mapping of a memory object */
extern kern_return_t    vm_map_enter_mem_object_prefault(
	vm_map_t                map,
	vm_map_offset_t         *address,
	vm_map_size_t           size,
	vm_map_offset_t         mask,
	int                     flags,
	vm_map_kernel_flags_t   vmk_flags,
	vm_tag_t                tag,
	ipc_port_t              port,
	vm_object_offset_t      offset,
	vm_prot_t               cur_protection,
	vm_prot_t               max_protection,
	upl_page_list_ptr_t     page_list,
	unsigned int            page_list_count);

/* Enter a mapping of a memory object */
extern kern_return_t    vm_map_enter_mem_object_control(
	vm_map_t                map,
	vm_map_offset_t         *address,
	vm_map_size_t           size,
	vm_map_offset_t         mask,
	int                     flags,
	vm_map_kernel_flags_t   vmk_flags,
	vm_tag_t                tag,
	memory_object_control_t control,
	vm_object_offset_t      offset,
	boolean_t               needs_copy,
	vm_prot_t               cur_protection,
	vm_prot_t               max_protection,
	vm_inherit_t            inheritance);

#endif /* !XNU_KERNEL_PRIVATE */

/* Deallocate a region */
extern kern_return_t    vm_map_remove(
	vm_map_t                map,
	vm_map_offset_t         start,
	vm_map_offset_t         end,
	boolean_t               flags);

/* Deallocate a region when the map is already locked */
extern kern_return_t    vm_map_remove_locked(
	vm_map_t        map,
	vm_map_offset_t     start,
	vm_map_offset_t     end,
	boolean_t       flags);

/* Discard a copy without using it */
extern void             vm_map_copy_discard(
	vm_map_copy_t           copy);

/* Overwrite existing memory with a copy */
extern kern_return_t    vm_map_copy_overwrite(
	vm_map_t                dst_map,
	vm_map_address_t        dst_addr,
	vm_map_copy_t           copy,
	boolean_t               interruptible);

/* returns TRUE if size of vm_map_copy == size parameter FALSE otherwise */
extern boolean_t        vm_map_copy_validate_size(
	vm_map_t                dst_map,
	vm_map_copy_t           copy,
	vm_map_size_t           *size);

/* Place a copy into a map */
extern kern_return_t    vm_map_copyout(
	vm_map_t                dst_map,
	vm_map_address_t        *dst_addr,                              /* OUT */
	vm_map_copy_t           copy);

extern kern_return_t vm_map_copyout_size(
	vm_map_t                dst_map,
	vm_map_address_t        *dst_addr,                              /* OUT */
	vm_map_copy_t           copy,
	vm_map_size_t           copy_size);

extern kern_return_t    vm_map_copyout_internal(
	vm_map_t                dst_map,
	vm_map_address_t        *dst_addr,      /* OUT */
	vm_map_copy_t           copy,
	vm_map_size_t           copy_size,
	boolean_t               consume_on_success,
	vm_prot_t               cur_protection,
	vm_prot_t               max_protection,
	vm_inherit_t            inheritance);

extern kern_return_t    vm_map_copyin(
	vm_map_t                        src_map,
	vm_map_address_t        src_addr,
	vm_map_size_t           len,
	boolean_t                       src_destroy,
	vm_map_copy_t           *copy_result);                          /* OUT */

extern kern_return_t    vm_map_copyin_common(
	vm_map_t                src_map,
	vm_map_address_t        src_addr,
	vm_map_size_t           len,
	boolean_t               src_destroy,
	boolean_t               src_volatile,
	vm_map_copy_t           *copy_result,                           /* OUT */
	boolean_t               use_maxprot);

#define VM_MAP_COPYIN_SRC_DESTROY       0x00000001
#define VM_MAP_COPYIN_USE_MAXPROT       0x00000002
#define VM_MAP_COPYIN_ENTRY_LIST        0x00000004
#define VM_MAP_COPYIN_PRESERVE_PURGEABLE 0x00000008
#define VM_MAP_COPYIN_ALL_FLAGS         0x0000000F
extern kern_return_t    vm_map_copyin_internal(
	vm_map_t                src_map,
	vm_map_address_t        src_addr,
	vm_map_size_t           len,
	int                     flags,
	vm_map_copy_t           *copy_result);                         /* OUT */

extern kern_return_t    vm_map_copy_extract(
	vm_map_t                src_map,
	vm_map_address_t        src_addr,
	vm_map_size_t           len,
	vm_map_copy_t           *copy_result,   /* OUT */
	vm_prot_t               *cur_prot,      /* OUT */
	vm_prot_t               *max_prot);


extern void             vm_map_disable_NX(
	vm_map_t                map);

extern void             vm_map_disallow_data_exec(
	vm_map_t                map);

extern void             vm_map_set_64bit(
	vm_map_t                map);

extern void             vm_map_set_32bit(
	vm_map_t                map);

extern void             vm_map_set_jumbo(
	vm_map_t                map);

extern void             vm_map_set_jit_entitled(
	vm_map_t                map);

extern void             vm_map_set_max_addr(
	vm_map_t                map, vm_map_offset_t new_max_offset);

extern boolean_t        vm_map_has_hard_pagezero(
	vm_map_t                map,
	vm_map_offset_t         pagezero_size);
extern void             vm_commit_pagezero_status(vm_map_t      tmap);

#ifdef __arm__
static inline boolean_t
vm_map_is_64bit(__unused vm_map_t map)
{
	return 0;
}
#else
extern boolean_t        vm_map_is_64bit(
	vm_map_t                map);
#endif


extern kern_return_t    vm_map_raise_max_offset(
	vm_map_t        map,
	vm_map_offset_t new_max_offset);

extern kern_return_t    vm_map_raise_min_offset(
	vm_map_t        map,
	vm_map_offset_t new_min_offset);
#if !CONFIG_EMBEDDED
extern void vm_map_set_high_start(
	vm_map_t        map,
	vm_map_offset_t high_start);
#endif

extern vm_map_offset_t  vm_compute_max_offset(
	boolean_t               is64);

extern void             vm_map_get_max_aslr_slide_section(
	vm_map_t                map,
	int64_t                 *max_sections,
	int64_t                 *section_size);

extern uint64_t         vm_map_get_max_aslr_slide_pages(
	vm_map_t map);

extern uint64_t         vm_map_get_max_loader_aslr_slide_pages(
	vm_map_t map);

extern void             vm_map_set_user_wire_limit(
	vm_map_t                map,
	vm_size_t               limit);

extern void vm_map_switch_protect(
	vm_map_t                map,
	boolean_t               val);

extern void vm_map_iokit_mapped_region(
	vm_map_t                map,
	vm_size_t               bytes);

extern void vm_map_iokit_unmapped_region(
	vm_map_t                map,
	vm_size_t               bytes);


extern boolean_t first_free_is_valid(vm_map_t);

extern int              vm_map_page_shift(
	vm_map_t                map);

extern vm_map_offset_t  vm_map_page_mask(
	vm_map_t                map);

extern int              vm_map_page_size(
	vm_map_t                map);

extern vm_map_offset_t  vm_map_round_page_mask(
	vm_map_offset_t         offset,
	vm_map_offset_t         mask);

extern vm_map_offset_t  vm_map_trunc_page_mask(
	vm_map_offset_t         offset,
	vm_map_offset_t         mask);

extern boolean_t        vm_map_page_aligned(
	vm_map_offset_t         offset,
	vm_map_offset_t         mask);

static inline int
vm_map_range_overflows(vm_map_offset_t addr, vm_map_size_t size)
{
	vm_map_offset_t sum;
	return os_add_overflow(addr, size, &sum);
}

static inline int
mach_vm_range_overflows(mach_vm_offset_t addr, mach_vm_size_t size)
{
	mach_vm_offset_t sum;
	return os_add_overflow(addr, size, &sum);
}

#ifdef XNU_KERNEL_PRIVATE
extern kern_return_t vm_map_page_info(
	vm_map_t                map,
	vm_map_offset_t         offset,
	vm_page_info_flavor_t   flavor,
	vm_page_info_t          info,
	mach_msg_type_number_t  *count);
extern kern_return_t vm_map_page_range_info_internal(
	vm_map_t                map,
	vm_map_offset_t         start_offset,
	vm_map_offset_t         end_offset,
	vm_page_info_flavor_t   flavor,
	vm_page_info_t          info,
	mach_msg_type_number_t  *count);
#endif /* XNU_KERNEL_PRIVATE */


#ifdef  MACH_KERNEL_PRIVATE

/*
 *	Macros to invoke vm_map_copyin_common.  vm_map_copyin is the
 *	usual form; it handles a copyin based on the current protection
 *	(current protection == VM_PROT_NONE) is a failure.
 *	vm_map_copyin_maxprot handles a copyin based on maximum possible
 *	access.  The difference is that a region with no current access
 *	BUT possible maximum access is rejected by vm_map_copyin(), but
 *	returned by vm_map_copyin_maxprot.
 */
#define vm_map_copyin(src_map, src_addr, len, src_destroy, copy_result) \
	        vm_map_copyin_common(src_map, src_addr, len, src_destroy, \
	                                FALSE, copy_result, FALSE)

#define vm_map_copyin_maxprot(src_map, \
	    src_addr, len, src_destroy, copy_result) \
	        vm_map_copyin_common(src_map, src_addr, len, src_destroy, \
	                                FALSE, copy_result, TRUE)


/*
 * Internal macros for rounding and truncation of vm_map offsets and sizes
 */
#define VM_MAP_ROUND_PAGE(x, pgmask) (((vm_map_offset_t)(x) + (pgmask)) & ~((signed)(pgmask)))
#define VM_MAP_TRUNC_PAGE(x, pgmask) ((vm_map_offset_t)(x) & ~((signed)(pgmask)))

/*
 * Macros for rounding and truncation of vm_map offsets and sizes
 */
#define VM_MAP_PAGE_SHIFT(map) ((map) ? (map)->hdr.page_shift : PAGE_SHIFT)
#define VM_MAP_PAGE_SIZE(map) (1 << VM_MAP_PAGE_SHIFT((map)))
#define VM_MAP_PAGE_MASK(map) (VM_MAP_PAGE_SIZE((map)) - 1)
#define VM_MAP_PAGE_ALIGNED(x, pgmask) (((x) & (pgmask)) == 0)

static inline void
vm_prot_to_wimg(unsigned int prot, unsigned int *wimg)
{
	switch (prot) {
	case MAP_MEM_NOOP:                      break;
	case MAP_MEM_IO:                        *wimg = VM_WIMG_IO; break;
	case MAP_MEM_COPYBACK:                  *wimg = VM_WIMG_USE_DEFAULT; break;
	case MAP_MEM_INNERWBACK:                *wimg = VM_WIMG_INNERWBACK; break;
	case MAP_MEM_POSTED:                    *wimg = VM_WIMG_POSTED; break;
	case MAP_MEM_POSTED_REORDERED:          *wimg = VM_WIMG_POSTED_REORDERED; break;
	case MAP_MEM_POSTED_COMBINED_REORDERED: *wimg = VM_WIMG_POSTED_COMBINED_REORDERED; break;
	case MAP_MEM_WTHRU:                     *wimg = VM_WIMG_WTHRU; break;
	case MAP_MEM_WCOMB:                     *wimg = VM_WIMG_WCOMB; break;
	case MAP_MEM_RT:                        *wimg = VM_WIMG_RT; break;
	default:
		panic("Unrecognized mapping type %u\n", prot);
	}
}

#endif /* MACH_KERNEL_PRIVATE */

#ifdef XNU_KERNEL_PRIVATE
extern kern_return_t vm_map_set_page_shift(vm_map_t map, int pageshift);
#endif /* XNU_KERNEL_PRIVATE */

#define vm_map_round_page(x, pgmask) (((vm_map_offset_t)(x) + (pgmask)) & ~((signed)(pgmask)))
#define vm_map_trunc_page(x, pgmask) ((vm_map_offset_t)(x) & ~((signed)(pgmask)))

/*
 * Flags for vm_map_remove() and vm_map_delete()
 */
#define VM_MAP_REMOVE_NO_FLAGS          0x0
#define VM_MAP_REMOVE_KUNWIRE           0x1
#define VM_MAP_REMOVE_INTERRUPTIBLE     0x2
#define VM_MAP_REMOVE_WAIT_FOR_KWIRE    0x4
#define VM_MAP_REMOVE_SAVE_ENTRIES      0x8
#define VM_MAP_REMOVE_NO_PMAP_CLEANUP   0x10
#define VM_MAP_REMOVE_NO_MAP_ALIGN      0x20
#define VM_MAP_REMOVE_NO_UNNESTING      0x40
#define VM_MAP_REMOVE_IMMUTABLE         0x80
#define VM_MAP_REMOVE_GAPS_OK           0x100

/* Support for UPLs from vm_maps */

#ifdef XNU_KERNEL_PRIVATE

extern kern_return_t vm_map_get_upl(
	vm_map_t                target_map,
	vm_map_offset_t         map_offset,
	upl_size_t              *size,
	upl_t                   *upl,
	upl_page_info_array_t   page_info,
	unsigned int            *page_infoCnt,
	upl_control_flags_t     *flags,
	vm_tag_t                tag,
	int                     force_data_sync);

#endif /* XNU_KERNEL_PRIVATE */

extern void
vm_map_sizes(vm_map_t map,
    vm_map_size_t * psize,
    vm_map_size_t * pfree,
    vm_map_size_t * plargest_free);

#if CONFIG_DYNAMIC_CODE_SIGNING
extern kern_return_t vm_map_sign(vm_map_t map,
    vm_map_offset_t start,
    vm_map_offset_t end);
#endif

extern kern_return_t vm_map_partial_reap(
	vm_map_t map,
	unsigned int *reclaimed_resident,
	unsigned int *reclaimed_compressed);


#if DEVELOPMENT || DEBUG

extern int vm_map_disconnect_page_mappings(
	vm_map_t map,
	boolean_t);
#endif


#if CONFIG_FREEZE

extern kern_return_t vm_map_freeze(
	task_t       task,
	unsigned int *purgeable_count,
	unsigned int *wired_count,
	unsigned int *clean_count,
	unsigned int *dirty_count,
	unsigned int dirty_budget,
	unsigned int *shared_count,
	int          *freezer_error_code,
	boolean_t    eval_only);


#define FREEZER_ERROR_GENERIC                   (-1)
#define FREEZER_ERROR_EXCESS_SHARED_MEMORY      (-2)
#define FREEZER_ERROR_LOW_PRIVATE_SHARED_RATIO  (-3)
#define FREEZER_ERROR_NO_COMPRESSOR_SPACE       (-4)
#define FREEZER_ERROR_NO_SWAP_SPACE             (-5)

#endif

__END_DECLS

/*
 * In some cases, we don't have a real VM object but still want to return a
 * unique ID (to avoid a memory region looking like shared memory), so build
 * a fake pointer based on the map's ledger and the index of the ledger being
 * reported.
 */
#define INFO_MAKE_FAKE_OBJECT_ID(map, ledger_id) ((uint32_t)(uintptr_t)VM_KERNEL_ADDRPERM((int*)((map)->pmap->ledger)+(ledger_id)))

#endif  /* KERNEL_PRIVATE */

#endif  /* _VM_VM_MAP_H_ */
