/*
 * Copyright (c) 2000-2016 Apple Inc. All rights reserved.
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

#ifndef _KERN_THREAD_KERNEL_STATE_H_

#include <vm/vm_kern.h>

struct thread_kernel_state {
	machine_thread_kernel_state  machine;       /* must be first */
	kern_allocation_name_t       allocation_name;
} __attribute__((aligned(16)));

typedef struct thread_kernel_state * thread_kernel_state_t;

#define thread_get_kernel_state(thread) ((thread_kernel_state_t) \
    ((thread)->kernel_stack + kernel_stack_size - sizeof(struct thread_kernel_state)))

#define thread_initialize_kernel_state(thread)  \
    thread_get_kernel_state((thread))->allocation_name = NULL;

#endif /* _KERN_THREAD_KERNEL_STATE_H_ */
