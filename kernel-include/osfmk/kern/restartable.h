/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
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

#ifndef _KERN_RESTARTABLE_H_
#define _KERN_RESTARTABLE_H_

#include <sys/cdefs.h>
#include <mach/message.h>
#include <mach/task.h>

__BEGIN_DECLS

/*!
 * @typedef task_restartable_range_t
 *
 * @brief
 * Describes a userspace recoverable range.
 *
 * @field location
 * The pointer to the beginning of a restartable section.
 *
 * @field length
 * The length of the critical section anchored at location.
 *
 * @field recovery_offs
 * The offset from the initial location that should be used for the recovery
 * codepath.
 *
 * @field flags
 * Currently unused, pass 0.
 */
typedef struct {
	mach_vm_address_t location;
	unsigned short    length;
	unsigned short    recovery_offs;
	unsigned int      flags;
} task_restartable_range_t;

typedef task_restartable_range_t *task_restartable_range_array_t;

/*!
 * @function task_restartable_ranges_register
 *
 * @brief
 * Register a set of restartable ranges for the current task.
 *
 * @param task
 * The task to operate on
 *
 * @param ranges
 * An array of address ranges for which PC resets are performed.
 *
 * @param count
 * The number of address ranges.
 *
 * @returns
 * - KERN_SUCCESS on success
 * - KERN_FAILURE if the task isn't the current one
 * - KERN_INVALID_ARGUMENT for various invalid inputs
 * - KERN_NOT_SUPPORTED the request is not supported (second registration on
 *   release kernels, registration when the task has gone wide)
 * - KERN_RESOURCE_SHORTAGE if not enough memory
 */
extern kern_return_t task_restartable_ranges_register(
	task_t                         task,
	task_restartable_range_array_t ranges,
	mach_msg_type_number_t         count);

/*!
 * @function task_restartable_ranges_synchronize
 *
 * @brief
 * Require for all threads in the task to reset their PC
 * if within a restartable range.
 *
 * @param task
 * The task to operate on (needs to be current task)
 *
 * @returns
 * - KERN_SUCCESS
 * - KERN_FAILURE if the task isn't the current one
 */
extern kern_return_t task_restartable_ranges_synchronize(task_t task);

/*!
 * @const TASK_RESTARTABLE_OFFSET_MAX
 * The maximum value length / recovery_offs can have.
 */
#define TASK_RESTARTABLE_OFFSET_MAX  4096u

#ifdef KERNEL_PRIVATE

struct restartable_ranges;

/**
 * @function restartable_init
 *
 * @brief
 * Initializes the restartable module.
 */
extern void restartable_init(void);

/**
 * @function restartable_ranges_release
 *
 * @brief
 * Release a reference on a restartable range.
 */
extern void restartable_ranges_release(struct restartable_ranges *ranges);

/**
 * @function thread_reset_pcs_ast
 *
 * @brief
 * Perform the work at the AST boundary to reset thread PCS.
 */
extern void thread_reset_pcs_ast(struct thread *thread);

#endif // KERNEL_PRIVATE

__END_DECLS

#endif  /* _KERN_RESTARTABLE_H_ */
