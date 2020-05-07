/*
 * Copyright (c) 1993-1995, 1999-2008 Apple Inc. All rights reserved.
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
 * Declarations for generic call outs.
 */

#ifndef _KERN_CALL_ENTRY_H_
#define _KERN_CALL_ENTRY_H_

#ifdef XNU_KERNEL_PRIVATE
#include <kern/queue.h>

#if !CONFIG_EMBEDDED
#define TIMER_TRACE     1
#endif

typedef void            *call_entry_param_t;
typedef void            (*call_entry_func_t)(
	call_entry_param_t      param0,
	call_entry_param_t      param1);

typedef struct call_entry {
	queue_chain_t       q_link;
	queue_head_t        *queue;
	call_entry_func_t   func;
	call_entry_param_t  param0;
	call_entry_param_t  param1;
	uint64_t            deadline;
#if TIMER_TRACE
	uint64_t            entry_time;
#endif
} call_entry_data_t;

typedef struct call_entry       *call_entry_t;

#ifdef MACH_KERNEL_PRIVATE

#define call_entry_setup(entry, pfun, p0)                       \
MACRO_BEGIN                                                     \
	(entry)->func		= (call_entry_func_t)(pfun);    \
	(entry)->param0		= (call_entry_param_t)(p0);     \
	(entry)->queue		= NULL;                         \
	(entry)->deadline	= 0;                            \
	queue_chain_init((entry)->q_link);                      \
MACRO_END

#define qe(x)           ((queue_entry_t)(x))
#define CE(x)           ((call_entry_t)(x))

static __inline__ queue_head_t *
call_entry_enqueue_tail(
	call_entry_t            entry,
	queue_t                 queue)
{
	queue_t                 old_queue = entry->queue;

	if (old_queue != NULL) {
		re_queue_tail(queue, &entry->q_link);
	} else {
		enqueue_tail(queue, &entry->q_link);
	}

	entry->queue = queue;

	return old_queue;
}

static __inline__ queue_head_t *
call_entry_dequeue(
	call_entry_t            entry)
{
	queue_t                 old_queue = entry->queue;

	if (old_queue != NULL) {
		(void)remque(qe(entry));

		entry->queue = NULL;
	}
	return old_queue;
}

static __inline__ queue_head_t *
call_entry_enqueue_deadline(
	call_entry_t                    entry,
	queue_head_t                    *queue,
	uint64_t                        deadline)
{
	queue_t         old_queue = entry->queue;
	call_entry_t    current;

	if (old_queue != queue || entry->deadline < deadline) {
		if (old_queue == NULL) {
			current = CE(queue_first(queue));
		} else if (old_queue != queue) {
			(void)remque(qe(entry));
			current = CE(queue_first(queue));
		} else {
			current = CE(queue_next(qe(entry)));
			(void)remque(qe(entry));
		}

		while (TRUE) {
			if (queue_end(queue, qe(current)) ||
			    deadline < current->deadline) {
				current = CE(queue_prev(qe(current)));
				break;
			}

			current = CE(queue_next(qe(current)));
		}
		insque(qe(entry), qe(current));
	} else if (deadline < entry->deadline) {
		current = CE(queue_prev(qe(entry)));

		(void)remque(qe(entry));

		while (TRUE) {
			if (queue_end(queue, qe(current)) ||
			    current->deadline <= deadline) {
				break;
			}

			current = CE(queue_prev(qe(current)));
		}
		insque(qe(entry), qe(current));
	}
	entry->queue = queue;
	entry->deadline = deadline;

	return old_queue;
}
#endif /* MACH_KERNEL_PRIVATE */

#endif /* XNU_KERNEL_PRIVATE */

#endif /* _KERN_CALL_ENTRY_H_ */
