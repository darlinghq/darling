/*
 * Copyright (c) 2000-2013 Apple Inc. All rights reserved.
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

#include <vm/vm_page.h>

#define         VM_GHOST_OFFSET_BITS    39
#define         VM_GHOST_OFFSET_MASK    0x7FFFFFFFFF
#define         VM_GHOST_PAGES_PER_ENTRY 4
#define         VM_GHOST_PAGE_MASK      0x3
#define         VM_GHOST_PAGE_SHIFT     2
#define         VM_GHOST_INDEX_BITS     (64 - VM_GHOST_OFFSET_BITS - VM_GHOST_PAGES_PER_ENTRY)

struct  vm_ghost {
	uint64_t        g_next_index:VM_GHOST_INDEX_BITS,
	    g_pages_held:VM_GHOST_PAGES_PER_ENTRY,
	    g_obj_offset:VM_GHOST_OFFSET_BITS;
	uint32_t        g_obj_id;
} __attribute__((packed));

typedef struct vm_ghost *vm_ghost_t;


extern  void            vm_phantom_cache_init(void);
extern  void            vm_phantom_cache_add_ghost(vm_page_t);
extern  vm_ghost_t      vm_phantom_cache_lookup_ghost(vm_page_t, uint32_t);
extern  void            vm_phantom_cache_update(vm_page_t);
extern  boolean_t       vm_phantom_cache_check_pressure(void);
extern  void            vm_phantom_cache_restart_sample(void);
