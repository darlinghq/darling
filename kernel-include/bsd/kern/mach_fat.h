/*
 * Copyright (c) 2000-2008 Apple Inc. All rights reserved.
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

#ifndef _BSD_KERN_MACH_FAT_H_
#define _BSD_KERN_MACH_FAT_H_

#include <mach/mach_types.h>
#include <kern/mach_loader.h>
#include <mach-o/fat.h>
#include <sys/vnode.h>

load_return_t fatfile_validate_fatarches(vm_offset_t data_ptr, vm_size_t data_size);

load_return_t fatfile_getbestarch(vm_offset_t data_ptr, vm_size_t data_size, struct image_params *imgp, struct fat_arch *archret);
load_return_t fatfile_getbestarch_for_cputype(cpu_type_t cputype,
    vm_offset_t data_ptr, vm_size_t data_size, struct image_params *imgp, struct fat_arch *archret);
load_return_t fatfile_getarch_with_bits(integer_t archbits,
    vm_offset_t data_ptr, vm_size_t data_size, struct fat_arch *archret);

#endif /* _BSD_KERN_MACH_FAT_H_ */
