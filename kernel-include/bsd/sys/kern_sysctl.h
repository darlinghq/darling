/*
 * Copyright (c) 2019 Apple Computer, Inc. All rights reserved.
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

#ifndef _KERN_SYSCTL_H_
#define _KERN_SYSCTL_H_

#include <mach/mach_types.h>

typedef struct _vm_object_query_data_ vm_object_query_data_t;
typedef struct _vm_object_query_data_ *vm_object_query_t;

struct _vm_object_query_data_ {
	vm_object_id_t object_id;
	mach_vm_size_t virtual_size;
	mach_vm_size_t resident_size;
	mach_vm_size_t wired_size;
	mach_vm_size_t reusable_size;
	mach_vm_size_t compressed_size;
	struct {
		uint64_t vo_no_footprint : 1; /* object not included in footprint */
		uint64_t vo_ledger_tag   : 3; /* object ledger tag */
		uint64_t purgable        : 2; /* object "purgable" state #defines */
	};
};

typedef struct _vmobject_list_output_ vmobject_list_output_data_t;
typedef struct _vmobject_list_output_ *vmobject_list_output_t;

struct _vmobject_list_output_ {
	int64_t entries; /* int64_t for alignment reasons, instead of int32_t */
	vm_object_query_data_t data[0];
};
#endif /* _KERN_SYSCTL_H_ */
