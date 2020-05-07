/*
 * Copyright (c) 2000-2004 Apple Computer, Inc. All rights reserved.
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
/*
 * @OSF_COPYRIGHT@
 */

#ifndef _KERN_EXCEPTION_H_
#define _KERN_EXCEPTION_H_

#include <mach/mach_types.h>
#include <mach/thread_status.h>
#include <mach/exception_types.h>
#include <kern/kern_types.h>
#include <security/_label.h>

/*
 * Common storage for exception actions.
 * There are arrays of these maintained at the activation, task, and host.
 */
struct exception_action {
	struct ipc_port         *port;          /* exception port */
	thread_state_flavor_t   flavor;         /* state flavor to send */
	exception_behavior_t    behavior;       /* exception type to raise */
	boolean_t               privileged;     /* survives ipc_task_reset */
	struct label            *label;         /* MAC label associated with action */
};

/* Initialize global state needed for exceptions. */
extern void exception_init(void);

/* Make an up-call to a thread's exception server */
extern kern_return_t exception_triage(
	exception_type_t        exception,
	mach_exception_data_t   code,
	mach_msg_type_number_t  codeCnt);

extern kern_return_t exception_triage_thread(
	exception_type_t        exception,
	mach_exception_data_t   code,
	mach_msg_type_number_t  codeCnt,
	thread_t                thread);

/* Notify system performance monitor */
extern kern_return_t sys_perf_notify(thread_t thread, int pid);

/* Notify crash reporter */
extern kern_return_t task_exception_notify(exception_type_t exception,
    mach_exception_data_type_t code, mach_exception_data_type_t subcode);

#endif  /* _KERN_EXCEPTION_H_ */
