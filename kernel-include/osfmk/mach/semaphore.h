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

#ifndef _MACH_SEMAPHORE_H_
#define _MACH_SEMAPHORE_H_

#include <mach/port.h>
#include <mach/mach_types.h>
#include <mach/kern_return.h>
#include <mach/sync_policy.h>

/*
 *	Forward Declarations
 *
 *	The semaphore creation and deallocation routines are
 *	defined with the Mach task APIs in <mach/task.h>.
 *
 *      kern_return_t	semaphore_create(task_t task,
 *                                       semaphore_t *new_semaphore,
 *					 sync_policy_t policy,
 *					 int value);
 *
 *	kern_return_t	semaphore_destroy(task_t task,
 *					  semaphore_t semaphore);
 */

#include <sys/cdefs.h>
__BEGIN_DECLS

extern  kern_return_t   semaphore_signal(semaphore_t semaphore);
extern  kern_return_t   semaphore_signal_all(semaphore_t semaphore);

extern  kern_return_t   semaphore_wait(semaphore_t semaphore);

#ifdef  KERNEL

#ifdef  __LP64__

#ifdef  KERNEL_PRIVATE

extern  kern_return_t   semaphore_timedwait(semaphore_t semaphore,
    mach_timespec_t wait_time);

#endif  /* KERNEL_PRIVATE */

#else   /* __LP64__ */

extern  kern_return_t   semaphore_timedwait(semaphore_t semaphore,
    mach_timespec_t wait_time);

#endif  /* __LP64__ */

extern  kern_return_t   semaphore_wait_deadline(semaphore_t semaphore,
    uint64_t deadline);
extern  kern_return_t   semaphore_wait_noblock(semaphore_t semaphore);

#ifdef  XNU_KERNEL_PRIVATE

extern  kern_return_t   semaphore_wait_signal(semaphore_t wait_semaphore,
    semaphore_t signal_semaphore);

extern  kern_return_t   semaphore_timedwait_signal(semaphore_t wait_semaphore,
    semaphore_t signal_semaphore,
    mach_timespec_t wait_time);

extern  kern_return_t   semaphore_signal_thread(semaphore_t semaphore,
    thread_t thread);

#endif  /* XNU_KERNEL_PRIVATE */

#else   /* KERNEL */

extern  kern_return_t   semaphore_timedwait(semaphore_t semaphore,
    mach_timespec_t wait_time);

extern  kern_return_t   semaphore_timedwait_signal(semaphore_t wait_semaphore,
    semaphore_t signal_semaphore,
    mach_timespec_t wait_time);

extern  kern_return_t   semaphore_wait_signal(semaphore_t wait_semaphore,
    semaphore_t signal_semaphore);

extern  kern_return_t   semaphore_signal_thread(semaphore_t semaphore,
    thread_t thread);

#endif  /* KERNEL */

__END_DECLS

#ifdef  PRIVATE

#define SEMAPHORE_OPTION_NONE           0x00000000

#define SEMAPHORE_SIGNAL                0x00000001
#define SEMAPHORE_WAIT                  0x00000002
#define SEMAPHORE_WAIT_ON_SIGNAL        0x00000008

#define SEMAPHORE_SIGNAL_TIMEOUT        0x00000010
#define SEMAPHORE_SIGNAL_ALL            0x00000020
#define SEMAPHORE_SIGNAL_INTERRUPT      0x00000040      /* libmach implements */
#define SEMAPHORE_SIGNAL_PREPOST        0x00000080

#define SEMAPHORE_WAIT_TIMEOUT          0x00000100
#define SEMAPHORE_WAIT_INTERRUPT        0x00000400      /* libmach implements */

#define SEMAPHORE_TIMEOUT_NOBLOCK       0x00100000
#define SEMAPHORE_TIMEOUT_RELATIVE      0x00200000

#define SEMAPHORE_USE_SAVED_RESULT      0x01000000      /* internal use only */
#define SEMAPHORE_SIGNAL_RELEASE        0x02000000      /* internal use only */

#endif  /* PRIVATE */

#endif  /* _MACH_SEMAPHORE_H_ */
