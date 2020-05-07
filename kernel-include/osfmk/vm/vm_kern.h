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
 *	File:	vm/vm_kern.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1985
 *
 *	Kernel memory management definitions.
 */

#ifndef _VM_VM_KERN_H_
#define _VM_VM_KERN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <mach/mach_types.h>
#include <mach/boolean.h>
#include <mach/kern_return.h>

#ifdef  KERNEL_PRIVATE

#ifdef XNU_KERNEL_PRIVATE

#include <kern/locks.h>

extern kern_return_t    kernel_memory_allocate(
	vm_map_t        map,
	vm_offset_t     *addrp,
	vm_size_t       size,
	vm_offset_t     mask,
	int             flags,
	vm_tag_t        tag);

/* flags for kernel_memory_allocate */
#define KMA_HERE        0x01
#define KMA_NOPAGEWAIT  0x02
#define KMA_KOBJECT     0x04
#define KMA_LOMEM       0x08
#define KMA_GUARD_FIRST 0x10
#define KMA_GUARD_LAST  0x20
#define KMA_PERMANENT   0x40
#define KMA_NOENCRYPT   0x80
#define KMA_KSTACK      0x100
#define KMA_VAONLY      0x200
#define KMA_COMPRESSOR  0x400   /* Pages belonging to the compressor are not on the paging queues, nor are they counted as wired. */
#define KMA_ATOMIC      0x800
#define KMA_ZERO        0x1000
#define KMA_PAGEABLE    0x2000

extern kern_return_t kmem_alloc(
	vm_map_t    map,
	vm_offset_t *addrp,
	vm_size_t   size,
	vm_tag_t    tag) __XNU_INTERNAL(kmem_alloc);

extern kern_return_t kmem_alloc_contig(
	vm_map_t        map,
	vm_offset_t     *addrp,
	vm_size_t       size,
	vm_offset_t     mask,
	ppnum_t         max_pnum,
	ppnum_t         pnum_mask,
	int             flags,
	vm_tag_t        tag);

extern kern_return_t    kmem_alloc_flags(
	vm_map_t        map,
	vm_offset_t     *addrp,
	vm_size_t       size,
	vm_tag_t        tag,
	int             flags);

extern kern_return_t    kmem_alloc_pageable(
	vm_map_t        map,
	vm_offset_t     *addrp,
	vm_size_t       size,
	vm_tag_t        tag) __XNU_INTERNAL(kmem_alloc_pageable);

extern kern_return_t    kmem_alloc_aligned(
	vm_map_t        map,
	vm_offset_t     *addrp,
	vm_size_t       size,
	vm_tag_t        tag);

extern kern_return_t    kmem_realloc(
	vm_map_t        map,
	vm_offset_t     oldaddr,
	vm_size_t       oldsize,
	vm_offset_t     *newaddrp,
	vm_size_t       newsize,
	vm_tag_t        tag);

extern void             kmem_free(
	vm_map_t        map,
	vm_offset_t     addr,
	vm_size_t       size);

extern kern_return_t    kmem_suballoc(
	vm_map_t        parent,
	vm_offset_t     *addr,
	vm_size_t       size,
	boolean_t       pageable,
	int             flags,
	vm_map_kernel_flags_t vmk_flags,
	vm_tag_t    tag,
	vm_map_t        *new_map);

extern kern_return_t    kmem_alloc_kobject(
	vm_map_t        map,
	vm_offset_t     *addrp,
	vm_size_t       size,
	vm_tag_t        tag) __XNU_INTERNAL(kmem_alloc_kobject);

extern kern_return_t kernel_memory_populate(
	vm_map_t        map,
	vm_offset_t     addr,
	vm_size_t       size,
	int             flags,
	vm_tag_t        tag);

extern void kernel_memory_depopulate(
	vm_map_t        map,
	vm_offset_t     addr,
	vm_size_t       size,
	int             flags);

extern kern_return_t    memory_object_iopl_request(
	ipc_port_t              port,
	memory_object_offset_t  offset,
	upl_size_t              *upl_size,
	upl_t                   *upl_ptr,
	upl_page_info_array_t   user_page_list,
	unsigned int            *page_list_count,
	upl_control_flags_t     *flags,
	vm_tag_t                tag);

struct mach_memory_info;
extern kern_return_t    vm_page_diagnose(struct mach_memory_info * info,
    unsigned int num_info, uint64_t zones_collectable_bytes);

extern uint32_t         vm_page_diagnose_estimate(void);

#if DEBUG || DEVELOPMENT

extern kern_return_t    mach_memory_info_check(void);

extern kern_return_t    vm_kern_allocation_info(uintptr_t addr, vm_size_t * size, vm_tag_t * tag, vm_size_t * zone_size);

#endif /* DEBUG || DEVELOPMENT */

extern vm_tag_t         vm_tag_bt(void);

extern vm_tag_t         vm_tag_alloc(vm_allocation_site_t * site);

extern void             vm_tag_alloc_locked(vm_allocation_site_t * site, vm_allocation_site_t ** releasesiteP);

extern void             vm_tag_update_size(vm_tag_t tag, int64_t size);

#if VM_MAX_TAG_ZONES
extern void             vm_allocation_zones_init(void);
extern void     vm_tag_will_update_zone(vm_tag_t tag, uint32_t zidx);
extern void             vm_tag_update_zone_size(vm_tag_t tag, uint32_t zidx, int64_t delta, int64_t dwaste);

extern vm_allocation_zone_total_t **   vm_allocation_zone_totals;

#endif /* VM_MAX_TAG_ZONES */

extern vm_tag_t         vm_tag_bt_debug(void);

extern uint32_t         vm_tag_get_kext(vm_tag_t tag, char * name, vm_size_t namelen);

extern boolean_t        vm_kernel_map_is_kernel(vm_map_t map);

extern ppnum_t          kernel_pmap_present_mapping(uint64_t vaddr, uint64_t * pvincr, uintptr_t * pvphysaddr);

#else /* XNU_KERNEL_PRIVATE */

extern kern_return_t    kmem_alloc(
	vm_map_t        map,
	vm_offset_t     *addrp,
	vm_size_t       size);

extern kern_return_t    kmem_alloc_pageable(
	vm_map_t        map,
	vm_offset_t     *addrp,
	vm_size_t       size);

extern kern_return_t    kmem_alloc_kobject(
	vm_map_t        map,
	vm_offset_t     *addrp,
	vm_size_t       size);

extern void             kmem_free(
	vm_map_t        map,
	vm_offset_t     addr,
	vm_size_t       size);

#endif /* !XNU_KERNEL_PRIVATE */


#ifdef XNU_KERNEL_PRIVATE
typedef struct vm_allocation_site kern_allocation_name;
typedef kern_allocation_name * kern_allocation_name_t;
#else /* XNU_KERNEL_PRIVATE */
struct kern_allocation_name;
typedef struct kern_allocation_name * kern_allocation_name_t;
#endif /* !XNU_KERNEL_PRIVATE */

extern kern_allocation_name_t   kern_allocation_name_allocate(const char * name, uint32_t suballocs);
extern void                     kern_allocation_name_release(kern_allocation_name_t allocation);
extern const char *             kern_allocation_get_name(kern_allocation_name_t allocation);
#ifdef XNU_KERNEL_PRIVATE
extern void                     kern_allocation_update_size(kern_allocation_name_t allocation, int64_t delta);
extern void                     kern_allocation_update_subtotal(kern_allocation_name_t allocation, uint32_t subtag, int64_t delta);
extern vm_tag_t                 kern_allocation_name_get_vm_tag(kern_allocation_name_t allocation);
#endif /* XNU_KERNEL_PRIVATE */

#ifdef  MACH_KERNEL_PRIVATE

extern void             kmem_init(
	vm_offset_t     start,
	vm_offset_t     end);

extern kern_return_t    copyinmap(
	vm_map_t        map,
	vm_map_offset_t fromaddr,
	void            *todata,
	vm_size_t       length);

extern kern_return_t    copyoutmap(
	vm_map_t        map,
	void            *fromdata,
	vm_map_offset_t toaddr,
	vm_size_t       length);

extern kern_return_t    kmem_alloc_external(
	vm_map_t        map,
	vm_offset_t     *addrp,
	vm_size_t       size);

extern kern_return_t    kmem_alloc_kobject_external(
	vm_map_t        map,
	vm_offset_t     *addrp,
	vm_size_t       size);

extern kern_return_t    kmem_alloc_pageable_external(
	vm_map_t        map,
	vm_offset_t     *addrp,
	vm_size_t       size);

#endif  /* MACH_KERNEL_PRIVATE */

#ifdef XNU_KERNEL_PRIVATE

extern kern_return_t    mach_vm_allocate_kernel(
	vm_map_t                map,
	mach_vm_offset_t        *addr,
	mach_vm_size_t  size,
	int                     flags,
	vm_tag_t    tag);

extern kern_return_t    vm_allocate_kernel(
	vm_map_t        map,
	vm_offset_t     *addr,
	vm_size_t       size,
	int         flags,
	vm_tag_t    tag);


extern kern_return_t mach_vm_map_kernel(
	vm_map_t                target_map,
	mach_vm_offset_t        *address,
	mach_vm_size_t  initial_size,
	mach_vm_offset_t        mask,
	int                     flags,
	vm_map_kernel_flags_t   vmk_flags,
	vm_tag_t                tag,
	ipc_port_t              port,
	vm_object_offset_t      offset,
	boolean_t               copy,
	vm_prot_t               cur_protection,
	vm_prot_t               max_protection,
	vm_inherit_t            inheritance);


extern kern_return_t vm_map_kernel(
	vm_map_t                target_map,
	vm_offset_t             *address,
	vm_size_t               size,
	vm_offset_t             mask,
	int                     flags,
	vm_map_kernel_flags_t   vmk_flags,
	vm_tag_t                tag,
	ipc_port_t              port,
	vm_offset_t             offset,
	boolean_t               copy,
	vm_prot_t               cur_protection,
	vm_prot_t               max_protection,
	vm_inherit_t            inheritance);

extern kern_return_t mach_vm_remap_kernel(
	vm_map_t                target_map,
	mach_vm_offset_t        *address,
	mach_vm_size_t  size,
	mach_vm_offset_t        mask,
	int                     flags,
	vm_tag_t        tag,
	vm_map_t                src_map,
	mach_vm_offset_t        memory_address,
	boolean_t               copy,
	vm_prot_t               *cur_protection,
	vm_prot_t               *max_protection,
	vm_inherit_t            inheritance);

extern kern_return_t vm_remap_kernel(
	vm_map_t                target_map,
	vm_offset_t             *address,
	vm_size_t               size,
	vm_offset_t             mask,
	int                     flags,
	vm_tag_t        tag,
	vm_map_t                src_map,
	vm_offset_t             memory_address,
	boolean_t               copy,
	vm_prot_t               *cur_protection,
	vm_prot_t               *max_protection,
	vm_inherit_t            inheritance);

extern kern_return_t vm_map_64_kernel(
	vm_map_t                target_map,
	vm_offset_t             *address,
	vm_size_t               size,
	vm_offset_t             mask,
	int                     flags,
	vm_map_kernel_flags_t   vmk_flags,
	vm_tag_t                tag,
	ipc_port_t              port,
	vm_object_offset_t      offset,
	boolean_t               copy,
	vm_prot_t               cur_protection,
	vm_prot_t               max_protection,
	vm_inherit_t            inheritance);

extern kern_return_t mach_vm_wire_kernel(
	host_priv_t             host_priv,
	vm_map_t                map,
	mach_vm_offset_t        start,
	mach_vm_size_t  size,
	vm_prot_t               access,
	vm_tag_t                tag);

extern kern_return_t vm_map_wire_kernel(
	vm_map_t                map,
	vm_map_offset_t         start,
	vm_map_offset_t         end,
	vm_prot_t               caller_prot,
	vm_tag_t                tag,
	boolean_t               user_wire);

extern kern_return_t vm_map_wire_and_extract_kernel(
	vm_map_t        map,
	vm_map_offset_t start,
	vm_prot_t       caller_prot,
	vm_tag_t        tag,
	boolean_t       user_wire,
	ppnum_t         *physpage_p);

#endif  /* XNU_KERNEL_PRIVATE */

extern vm_map_t kernel_map;
extern vm_map_t kernel_pageable_map;
extern vm_map_t ipc_kernel_map;

#endif  /* KERNEL_PRIVATE */

#ifdef KERNEL

__BEGIN_DECLS
#if MACH_KERNEL_PRIVATE
extern vm_offset_t vm_kernel_addrhash(vm_offset_t addr)
__XNU_INTERNAL(vm_kernel_addrhash);
#else
extern vm_offset_t vm_kernel_addrhash(vm_offset_t addr);
#endif
__END_DECLS

extern void vm_kernel_addrhide(
	vm_offset_t addr,
	vm_offset_t *hide_addr);

extern vm_offset_t vm_kernel_addrperm_ext;

extern void     vm_kernel_addrperm_external(
	vm_offset_t addr,
	vm_offset_t *perm_addr);

extern void     vm_kernel_unslide_or_perm_external(
	vm_offset_t addr,
	vm_offset_t *up_addr);

#if MACH_KERNEL_PRIVATE
extern uint64_t vm_kernel_addrhash_salt;
extern uint64_t vm_kernel_addrhash_salt_ext;

extern void vm_kernel_addrhash_external(
	vm_offset_t addr,
	vm_offset_t *perm_addr);
#endif /* MACH_KERNEL_PRIVATE */

extern void vm_init_before_launchd(void);

#endif /* KERNEL */

#ifdef __cplusplus
}
#endif

#endif  /* _VM_VM_KERN_H_ */
