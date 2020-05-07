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

#ifndef _KERN_SCHED_URGENCY_H_
#define _KERN_SCHED_URGENCY_H_

#ifdef MACH_KERNEL_PRIVATE

#include <kern/kern_types.h>

typedef enum thread_urgency {
	THREAD_URGENCY_NONE             = 0,    /* processor is idle */
	THREAD_URGENCY_BACKGROUND       = 1,    /* "background" thread (i.e. min-power) */
	THREAD_URGENCY_NORMAL           = 2,    /* "normal" thread */
	THREAD_URGENCY_REAL_TIME        = 3,    /* "real-time" or urgent thread */
	THREAD_URGENCY_LOWPRI           = 4,    /* low priority but not "background" hint for performance management subsystem */
	THREAD_URGENCY_MAX              = 5,    /* Max */
} thread_urgency_t;

/* Returns the "urgency" of a thread (provided by scheduler) */
extern thread_urgency_t      thread_get_urgency(
	thread_t        thread,
	uint64_t        *rt_period,
	uint64_t        *rt_deadline);

/* Tells the "urgency" of the just scheduled thread (provided by CPU PM) */
extern void     thread_tell_urgency(
	thread_urgency_t             urgency,
	uint64_t        rt_period,
	uint64_t        rt_deadline,
	uint64_t        sched_latency,
	thread_t nthread);

#endif /* MACH_KERNEL_PRIVATE */

__END_DECLS

#endif /* _KERN_SCHED_URGENCY_H_ */
