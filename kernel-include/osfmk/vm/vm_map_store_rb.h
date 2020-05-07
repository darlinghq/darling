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

#ifndef _VM_VM_MAP_STORE_H_RB
#define _VM_VM_MAP_STORE_H_RB

#include <vm/vm_map_store.h>

RB_PROTOTYPE_SC(__private_extern__, rb_head, vm_map_store, entry, rb_node_compare);

void vm_map_store_init_rb( struct vm_map_header*  );
int rb_node_compare(struct vm_map_store *, struct vm_map_store *);
void vm_map_store_walk_rb( struct _vm_map*, struct vm_map_entry**, struct vm_map_entry**);
boolean_t vm_map_store_lookup_entry_rb( struct _vm_map*, vm_map_offset_t, struct vm_map_entry**);
void    vm_map_store_entry_link_rb( struct vm_map_header*, struct vm_map_entry*, struct vm_map_entry*);
void    vm_map_store_entry_unlink_rb( struct vm_map_header*, struct vm_map_entry*);
void    vm_map_store_copy_reset_rb( struct vm_map_copy*, struct vm_map_entry*, int);
void    update_first_free_rb(struct _vm_map*, struct vm_map_entry*, boolean_t new_entry_creation);

#endif /* _VM_VM_MAP_STORE_RB_H */
