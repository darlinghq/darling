/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#ifndef _KERN_CDATA_H_
#define _KERN_CDATA_H_

#include <kern/kcdata.h>
#include <mach/mach_types.h>

/*
 * Do not use these macros!
 *
 * Instead, you should use kcdata_iter_* functions defined in kcdata.h.  These
 * macoros have no idea where the kcdata buffer ends, so they are all unsafe.
 */
#define KCDATA_ITEM_HEADER_SIZE         (sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t))
#define KCDATA_ITEM_ITER(item)          kcdata_iter_unsafe((void*)(item))
#define KCDATA_ITEM_TYPE(item)          kcdata_iter_type(KCDATA_ITEM_ITER(item))
#define KCDATA_ITEM_SIZE(item)          kcdata_iter_size(KCDATA_ITEM_ITER(item))
#define KCDATA_ITEM_FLAGS(item)         kcdata_iter_flags(KCDATA_ITEM_ITER(item))
#define KCDATA_ITEM_ARRAY_GET_EL_TYPE(item)    kcdata_iter_array_elem_type(KCDATA_ITEM_ITER(item))
#define KCDATA_ITEM_ARRAY_GET_EL_COUNT(item)   kcdata_iter_array_elem_count(KCDATA_ITEM_ITER(item))
#define KCDATA_ITEM_ARRAY_GET_EL_SIZE(item)    kcdata_iter_array_elem_size(KCDATA_ITEM_ITER(item))
#define KCDATA_CONTAINER_ID(item)              kcdata_iter_container_id(KCDATA_ITEM_ITER(item))
#define KCDATA_ITEM_NEXT_HEADER(itemx)   (kcdata_iter_next(KCDATA_ITEM_ITER(itemx)).item)
#define KCDATA_ITEM_FOREACH(head)       for (; KCDATA_ITEM_TYPE(head) != KCDATA_TYPE_BUFFER_END; (head) = KCDATA_ITEM_NEXT_HEADER(head))
#define KCDATA_ITEM_DATA_PTR(item)      kcdata_iter_payload(KCDATA_ITEM_ITER(item))
#define KCDATA_ITEM_FIND_TYPE(itemx, type) (kcdata_iter_find_type(KCDATA_ITEM_ITER(itemx), type).item)
#define kcdata_get_container_type(buffer) kcdata_iter_container_type(KCDATA_ITEM_ITER(buffer))
#define kcdata_get_data_with_desc(buf, desc, data) kcdata_iter_get_data_with_desc(KCDATA_ITEM_ITER(buf),desc,data,NULL)
/* Do not use these macros! */

#ifdef KERNEL

#ifdef XNU_KERNEL_PRIVATE

/* Structure to save information about corpse data */
struct kcdata_descriptor {
	uint32_t            kcd_length;
	uint16_t kcd_flags;
#define KCFLAG_USE_MEMCOPY 0x0
#define KCFLAG_USE_COPYOUT 0x1
#define KCFLAG_NO_AUTO_ENDBUFFER 0x2
	uint16_t kcd_user_flags; /* reserved for subsystems using kcdata */
	mach_vm_address_t kcd_addr_begin;
	mach_vm_address_t kcd_addr_end;
};

typedef struct kcdata_descriptor * kcdata_descriptor_t;

kcdata_descriptor_t kcdata_memory_alloc_init(mach_vm_address_t crash_data_p, unsigned data_type, unsigned size, unsigned flags);
kern_return_t kcdata_memory_static_init(
	kcdata_descriptor_t data, mach_vm_address_t buffer_addr_p, unsigned data_type, unsigned size, unsigned flags);
kern_return_t kcdata_memory_destroy(kcdata_descriptor_t data);
kern_return_t
kcdata_add_container_marker(kcdata_descriptor_t data, uint32_t header_type, uint32_t container_type, uint64_t identifier);
kern_return_t kcdata_add_type_definition(kcdata_descriptor_t data,
    uint32_t type_id,
    char * type_name,
    struct kcdata_subtype_descriptor * elements_array_addr,
    uint32_t elements_count);

kern_return_t kcdata_add_uint64_with_description(kcdata_descriptor_t crashinfo, uint64_t data, const char * description);
kern_return_t kcdata_add_uint32_with_description(kcdata_descriptor_t crashinfo, uint32_t data, const char * description);

kern_return_t kcdata_undo_add_container_begin(kcdata_descriptor_t data);

kern_return_t kcdata_write_buffer_end(kcdata_descriptor_t data);
void *kcdata_memory_get_begin_addr(kcdata_descriptor_t data);

#else /* XNU_KERNEL_PRIVATE */

typedef void * kcdata_descriptor_t;

#endif /* XNU_KERNEL_PRIVATE */

uint32_t kcdata_estimate_required_buffer_size(uint32_t num_items, uint32_t payload_size);
uint64_t kcdata_memory_get_used_bytes(kcdata_descriptor_t kcd);
kern_return_t kcdata_memcpy(kcdata_descriptor_t data, mach_vm_address_t dst_addr, const void * src_addr, uint32_t size);
kern_return_t kcdata_bzero(kcdata_descriptor_t data, mach_vm_address_t dst_addr, uint32_t size);
kern_return_t kcdata_get_memory_addr(kcdata_descriptor_t data, uint32_t type, uint32_t size, mach_vm_address_t * user_addr);
kern_return_t kcdata_get_memory_addr_for_array(
	kcdata_descriptor_t data, uint32_t type_of_element, uint32_t size_of_element, uint32_t count, mach_vm_address_t * user_addr);

#endif /* KERNEL */
#endif /* _KERN_CDATA_H_ */
