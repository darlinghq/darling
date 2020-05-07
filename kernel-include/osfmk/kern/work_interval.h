/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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

#ifndef _KERN_WORK_INTERVAL_H_
#define _KERN_WORK_INTERVAL_H_

#include <sys/cdefs.h>

#include <stdint.h>
#include <kern/kern_types.h>

#include <kern/thread_group.h>

__BEGIN_DECLS

struct work_interval;

struct kern_work_interval_args {
	uint64_t work_interval_id;
	uint64_t start;
	uint64_t finish;
	uint64_t deadline;
	uint64_t next_start;
	uint32_t notify_flags;
	uint32_t create_flags;
	uint16_t urgency;
};

struct kern_work_interval_create_args {
	uint64_t        wica_id;          /* out param */
	uint32_t        wica_port;        /* out param */
	uint32_t        wica_create_flags;
};

/*
 * Allocate/assign a single work interval ID for a thread,
 * and support deallocating it.
 */
extern kern_return_t
kern_work_interval_create(thread_t thread, struct kern_work_interval_create_args *create_params);

extern kern_return_t
kern_work_interval_destroy(thread_t thread, uint64_t work_interval_id);
extern kern_return_t
kern_work_interval_join(thread_t thread, mach_port_name_t port_name);

extern kern_return_t
kern_work_interval_notify(thread_t thread, struct kern_work_interval_args* kwi_args);

#ifdef MACH_KERNEL_PRIVATE

extern void work_interval_port_notify(mach_msg_header_t *msg);


extern void work_interval_thread_terminate(thread_t thread);

#endif /* MACH_KERNEL_PRIVATE */

__END_DECLS

#endif /* !defined(_KERN_WORK_INTERVAL_H_) */
