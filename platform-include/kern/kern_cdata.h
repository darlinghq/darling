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

#endif /* _KERN_CDATA_H_ */
