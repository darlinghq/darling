/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

#ifdef  XNU_KERNEL_PRIVATE

#ifndef _VM_VM_COMPRESSOR_PAGER_H_
#define _VM_VM_COMPRESSOR_PAGER_H_

#include <mach/mach_types.h>
#include <kern/kern_types.h>
#include <vm/vm_external.h>

extern kern_return_t vm_compressor_pager_put(
	memory_object_t                 mem_obj,
	memory_object_offset_t          offset,
	ppnum_t                         ppnum,
	void                            **current_chead,
	char                            *scratch_buf,
	int                             *compressed_count_delta_p);
extern kern_return_t vm_compressor_pager_get(
	memory_object_t         mem_obj,
	memory_object_offset_t  offset,
	ppnum_t                 ppnum,
	int                     *my_fault_type,
	int                     flags,
	int                     *compressed_count_delta_p);

#define C_DONT_BLOCK            0x01
#define C_KEEP                  0x02
#define C_KDP                   0x04

extern unsigned int vm_compressor_pager_state_clr(
	memory_object_t         mem_obj,
	memory_object_offset_t  offset);
extern vm_external_state_t vm_compressor_pager_state_get(
	memory_object_t         mem_obj,
	memory_object_offset_t  offset);

#define VM_COMPRESSOR_PAGER_STATE_GET(object, offset)                   \
	(((object)->internal &&                                 \
	  (object)->pager != NULL &&                                    \
	  !(object)->terminating &&                                     \
	  (object)->alive)                                              \
	 ? vm_compressor_pager_state_get((object)->pager,               \
	                                 (offset) + (object)->paging_offset) \
	 : VM_EXTERNAL_STATE_UNKNOWN)

#define VM_COMPRESSOR_PAGER_STATE_CLR(object, offset)                   \
	MACRO_BEGIN                                                     \
	if ((object)->internal &&                                       \
	    (object)->pager != NULL &&                                  \
	    !(object)->terminating &&                                   \
	    (object)->alive) {                                          \
	        int _num_pages_cleared;                                 \
	        _num_pages_cleared =                                    \
	                vm_compressor_pager_state_clr(                  \
	                        (object)->pager,                        \
	                        (offset) + (object)->paging_offset);    \
	        if (_num_pages_cleared) {                               \
	                vm_compressor_pager_count((object)->pager,      \
	                                          -_num_pages_cleared,  \
	                                          FALSE, /* shared */   \
	                                          (object));            \
	        }                                                       \
	        if (_num_pages_cleared &&                               \
	            ((object)->purgable != VM_PURGABLE_DENY ||          \
	             (object)->vo_ledger_tag)) {                        \
	/* less compressed purgeable/tagged pages */    \
	                assert(_num_pages_cleared == 1);                \
	                vm_object_owner_compressed_update(              \
	                        (object),                               \
	                        -_num_pages_cleared);                   \
	        }                                                       \
	}                                                               \
	MACRO_END

extern void vm_compressor_pager_transfer(
	memory_object_t         dst_mem_obj,
	memory_object_offset_t  dst_offset,
	memory_object_t         src_mem_obj,
	memory_object_offset_t  src_offset);
extern memory_object_offset_t vm_compressor_pager_next_compressed(
	memory_object_t         mem_obj,
	memory_object_offset_t  offset);

extern void vm_compressor_init(void);
extern int vm_compressor_put(ppnum_t pn, int *slot, void **current_chead, char *scratch_buf);
extern int vm_compressor_get(ppnum_t pn, int *slot, int flags);
extern int vm_compressor_free(int *slot, int flags);
extern unsigned int vm_compressor_pager_reap_pages(memory_object_t mem_obj, int flags);
extern unsigned int vm_compressor_pager_get_count(memory_object_t mem_obj);
extern void vm_compressor_pager_count(memory_object_t mem_obj,
    int compressed_count_delta,
    boolean_t shared_lock,
    vm_object_t object);

extern void vm_compressor_transfer(int *dst_slot_p, int *src_slot_p);

#if CONFIG_FREEZE
extern kern_return_t vm_compressor_pager_relocate(memory_object_t mem_obj, memory_object_offset_t mem_offset, void **current_chead);
extern kern_return_t vm_compressor_relocate(void **current_chead, int *src_slot_p);
extern void vm_compressor_finished_filling(void **current_chead);
#endif /* CONFIG_FREEZE */

#endif  /* _VM_VM_COMPRESSOR_PAGER_H_ */

#endif  /* XNU_KERNEL_PRIVATE */
