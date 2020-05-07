/*
 * Copyright (c) 2000-2007 Apple Inc. All rights reserved.
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
/*
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 */

#ifndef _KERN_TIMER_H_
#define _KERN_TIMER_H_

#include <kern/kern_types.h>

/*
 * Some platforms have very expensive timebase routines. An optimization
 * is to avoid switching timers on kernel exit/entry, which results in all
 * time billed to the system timer. However, when exposed to userspace,
 * we report as user time to indicate that work was done on behalf of
 * userspace.
 *
 * Although this policy is implemented as a global variable, we snapshot it
 * at key points in the thread structure (when the thread is locked and
 * executing in the kernel) to avoid imbalances.
 */
extern int precise_user_kernel_time;

/*
 * thread must be locked, or be the current executing thread, so that
 * it doesn't transition from user to kernel while updating the
 * thread-local value (or in kernel debugger context). In the future,
 * we make take into account task-level or thread-level policy.
 */
#define use_precise_user_kernel_time(thread) (precise_user_kernel_time)

/*
 * Definitions for high resolution timers.
 */

#if __LP64__
#define TIMER_ALIGNMENT
#else
#define TIMER_ALIGNMENT __attribute__((packed, aligned(4)))
#endif

struct timer {
	uint64_t tstamp;
#if defined(__LP64__)
	uint64_t all_bits;
#else /* defined(__LP64__) */
	/* A check word on the high portion allows atomic updates. */
	uint32_t low_bits;
	uint32_t high_bits;
	uint32_t high_bits_check;
#endif /* !defined(__LP64__) */
} TIMER_ALIGNMENT;

typedef struct timer timer_data_t, *timer_t;

/*
 * Initialize the `timer`.
 */
void timer_init(timer_t timer);

/*
 * Start the `timer` at time `tstamp`.
 */
void timer_start(timer_t timer, uint64_t tstamp);

/*
 * Stop the `timer` and update it with time `tstamp`.
 */
void timer_stop(timer_t timer, uint64_t tstamp);

/*
 * Update the `timer` at time `tstamp`, leaving it running.
 */
void timer_update(timer_t timer, uint64_t tstamp);

/*
 * Update the `timer` with time `tstamp` and start `new_timer`.
 */
void timer_switch(timer_t timer, uint64_t tstamp, timer_t new_timer);

/*
 * Update the thread timer at an "event," like a context switch, at time
 * `tstamp`.  This stops the current timer and starts the `new_timer` running.
 *
 * Must be called with interrupts disabled.
 */
void processor_timer_switch_thread(uint64_t tstamp, timer_t new_timer);

/*
 * Return the difference between the `timer` and a previous value pointed to by
 * `prev_in_cur_out`.  Store the current value of the timer to
 * `prev_in_cur_out`.
 */
uint64_t timer_delta(timer_t timer, uint64_t *prev_in_cur_out);

/*
 * Read the accumulated time of `timer`.
 */
#if defined(__LP64__)
static inline
uint64_t
timer_grab(timer_t timer)
{
	return timer->all_bits;
}
#else /* defined(__LP64__) */
uint64_t timer_grab(timer_t timer);
#endif /* !defined(__LP64__) */

#endif /* _KERN_TIMER_H_ */
