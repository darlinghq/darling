/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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

#ifndef _VM_VM_MAP_STORE_H
#define _VM_VM_MAP_STORE_H

/*
 #ifndef VM_MAP_STORE_USE_LL
 #define VM_MAP_STORE_USE_LL
 #endif
 */
#ifndef VM_MAP_STORE_USE_RB
#define VM_MAP_STORE_USE_RB
#endif

#include <libkern/tree.h>

struct _vm_map;
struct vm_map_entry;
struct vm_map_copy;
struct vm_map_header;

struct vm_map_store {
#ifdef VM_MAP_STORE_USE_RB
	RB_ENTRY(vm_map_store) entry;
#endif
};

#ifdef VM_MAP_STORE_USE_RB
RB_HEAD( rb_head, vm_map_store );
#endif

#include <vm/vm_map.h>
#include <vm/vm_map_store_ll.h>
#include <vm/vm_map_store_rb.h>

#define UPDATE_HIGHEST_ENTRY_END(map, highest_entry)                    \
	MACRO_BEGIN                                                     \
	struct _vm_map*	UHEE_map;                                       \
	struct vm_map_entry*	UHEE_entry;                             \
	UHEE_map = (map);                                               \
	assert(UHEE_map->disable_vmentry_reuse);                        \
	assert(!UHEE_map->is_nested_map);                               \
	UHEE_entry = (highest_entry);                                   \
	if( UHEE_map->highest_entry_end < UHEE_entry->vme_end) {        \
	        UHEE_map->highest_entry_end = UHEE_entry->vme_end;      \
	}                                                               \
	MACRO_END

#define VM_MAP_HIGHEST_ENTRY(map, entry, start)                         \
	MACRO_BEGIN                                                     \
	struct _vm_map* VMHE_map;                                       \
	struct vm_map_entry*	tmp_entry;                              \
	vm_map_offset_t VMHE_start;                                     \
	VMHE_map = (map);                                               \
	assert(VMHE_map->disable_vmentry_reuse);                        \
	assert(!VMHE_map->is_nested_map);                               \
	VMHE_start= VMHE_map->highest_entry_end + PAGE_SIZE_64;         \
	while(vm_map_lookup_entry(VMHE_map, VMHE_start, &tmp_entry)){   \
	        VMHE_map->highest_entry_end = tmp_entry->vme_end;       \
	        VMHE_start = VMHE_map->highest_entry_end + PAGE_SIZE_64; \
	}                                                               \
	entry = tmp_entry;                                              \
	start = VMHE_start;                                             \
	MACRO_END

/*
 *	SAVE_HINT_MAP_READ:
 *
 *	Saves the specified entry as the hint for
 *	future lookups.  only a read lock is held on map,
 *      so make sure the store is atomic... OSCompareAndSwap
 *	guarantees this... also, we don't care if we collide
 *	and someone else wins and stores their 'hint'
 */
#define SAVE_HINT_MAP_READ(map, value) \
	MACRO_BEGIN                                                     \
	OSCompareAndSwapPtr((map)->hint, value, &(map)->hint); \
	MACRO_END


/*
 *	SAVE_HINT_MAP_WRITE:
 *
 *	Saves the specified entry as the hint for
 *	future lookups.  write lock held on map,
 *      so no one else can be writing or looking
 *      until the lock is dropped, so it's safe
 *      to just do an assignment
 */
#define SAVE_HINT_MAP_WRITE(map, value) \
	MACRO_BEGIN                    \
	(map)->hint = (value);         \
	MACRO_END

#define SAVE_HINT_HOLE_WRITE(map, value) \
	MACRO_BEGIN                    \
	(map)->hole_hint = (value);     \
	MACRO_END

#define SKIP_RB_TREE            0xBAADC0D1

#define VM_MAP_ENTRY_CREATE     1
#define VM_MAP_ENTRY_DELETE     2

void vm_map_store_init( struct vm_map_header*  );
boolean_t vm_map_store_lookup_entry( struct _vm_map*, vm_map_offset_t, struct vm_map_entry**);
void    vm_map_store_update( struct _vm_map*, struct vm_map_entry*, int);
void    _vm_map_store_entry_link( struct vm_map_header *, struct vm_map_entry*, struct vm_map_entry*);
void    vm_map_store_entry_link( struct _vm_map*, struct vm_map_entry*, struct vm_map_entry*, vm_map_kernel_flags_t);
void    _vm_map_store_entry_unlink( struct vm_map_header *, struct vm_map_entry*);
void    vm_map_store_entry_unlink( struct _vm_map*, struct vm_map_entry*);
void    vm_map_store_update_first_free( struct _vm_map*, struct vm_map_entry*, boolean_t new_entry_creation);
void    vm_map_store_copy_reset( struct vm_map_copy*, struct vm_map_entry*);
#if MACH_ASSERT
boolean_t first_free_is_valid_store( struct _vm_map*);
#endif
boolean_t vm_map_store_has_RB_support( struct vm_map_header *hdr );

#endif /* _VM_VM_MAP_STORE_H */
