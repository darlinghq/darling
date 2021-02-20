/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

// These declarations must match those in Libc's stack_logging.h

#include <mach/vm_statistics.h>         // to get VM_FLAGS_ALIAS_MASK

#define stack_logging_type_vm_allocate 16      // mach_vm_allocate, mmap, mach_vm_map, mach_vm_remap, etc
#define stack_logging_type_vm_deallocate 32    // mach_vm_deallocate or munmap
#define stack_logging_type_mapped_file_or_shared_mem 128    // a hint that the VM region *might* be a mapped file or shared memory

// For logging VM allocation and deallocation, arg1 here
// is the mach_port_name_t of the target task in which the
// alloc or dealloc is occurring. For example, for mmap()
// that would be mach_task_self(), but for a cross-task-capable
// call such as mach_vm_map(), it is the target task.

typedef void (malloc_logger_t)(uint32_t type, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t result, uint32_t num_hot_frames_to_skip);

extern malloc_logger_t *__syscall_logger;
