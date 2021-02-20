/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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

#include <mach/boolean.h>
#include <mach/message.h>
#include <mach/kern_return.h>
#include <mach/mach_traps.h>
#include <mach/mach_types.h>
#include <mach/clock_types.h>

kern_return_t
semaphore_signal(mach_port_t signal_semaphore)
{
	return semaphore_signal_trap(signal_semaphore);
}

kern_return_t
semaphore_signal_all(mach_port_t signal_semaphore)
{
	return semaphore_signal_all_trap(signal_semaphore);
}

kern_return_t
semaphore_signal_thread(mach_port_t signal_semaphore, mach_port_t thread_act)
{
	return semaphore_signal_thread_trap(signal_semaphore, thread_act);
}

kern_return_t
semaphore_wait(mach_port_t wait_semaphore)
{
	return semaphore_wait_trap(wait_semaphore);
}

kern_return_t
semaphore_timedwait(mach_port_t wait_semaphore, mach_timespec_t wait_time)
{
	return semaphore_timedwait_trap(wait_semaphore,
	           wait_time.tv_sec,
	           wait_time.tv_nsec);
}

kern_return_t
semaphore_wait_signal(mach_port_t wait_semaphore, mach_port_t signal_semaphore)
{
	return semaphore_wait_signal_trap(wait_semaphore, signal_semaphore);
}

kern_return_t
semaphore_timedwait_signal(mach_port_t wait_semaphore,
    mach_port_t signal_semaphore,
    mach_timespec_t wait_time)
{
	return semaphore_timedwait_signal_trap(wait_semaphore,
	           signal_semaphore,
	           wait_time.tv_sec,
	           wait_time.tv_nsec);
}
