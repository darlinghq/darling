/*
 * Copyright (c) 2003-2009 Apple Inc. All rights reserved.
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
 * Machine independent per processor data.
 */

#ifndef _KERN_PROCESSOR_DATA_H_
#define _KERN_PROCESSOR_DATA_H_

/*
 * #include kern/processor.h instead of this file.
 */

#ifdef XNU_KERNEL_PRIVATE

#ifdef MACH_KERNEL_PRIVATE

#include <ipc/ipc_kmsg.h>
#include <kern/timer.h>
#include <kern/debug.h>

struct processor_sched_statistics {
	uint32_t                csw_count;
	uint32_t                preempt_count;
	uint32_t                preempted_rt_count;
	uint32_t                preempted_by_rt_count;
	uint32_t                rt_sched_count;
	uint32_t                interrupt_count;
	uint32_t                ipi_count;
	uint32_t                timer_pop_count;
	uint32_t                idle_transitions;
	uint32_t                quantum_timer_expirations;
};

struct processor_data {
	/* Processor state statistics */
	timer_data_t                    idle_state;
	timer_data_t                    system_state;
	timer_data_t                    user_state;

	timer_t                                 current_state; /* points to processor's idle, system, or user state timer */

	/* Thread execution timers */
	timer_t                                 thread_timer; /* points to current thread's user or system timer */
	timer_t                                 kernel_timer; /* points to current thread's system_timer */

	/* Kernel stack cache */
	struct stack_cache {
		vm_offset_t                             free;
		unsigned int                    count;
	}                                               stack_cache;

	/* VM event counters */
	vm_statistics64_data_t  vm_stat;

	/* waitq prepost cache */
#define WQP_CACHE_MAX   50
	struct wqp_cache {
		uint64_t                head;
		unsigned int            avail;
	} wqp_cache;

	int                                             start_color;
	unsigned long                   page_grab_count;
	void                                    *free_pages;
	struct processor_sched_statistics sched_stats;
	uint64_t        timer_call_ttd; /* current timer call time-to-deadline */
	uint64_t        wakeups_issued_total; /* Count of thread wakeups issued
	                                       * by this processor
	                                       */
	struct debugger_state {
		debugger_op db_current_op;
		const char *db_message;
		const char *db_panic_str;
		va_list *db_panic_args;
		uint64_t db_panic_options;
		void *db_panic_data_ptr;
		boolean_t db_proceed_on_sync_failure;
		uint32_t db_entry_count; /* incremented whenever we panic or call Debugger (current CPU panic level) */
		kern_return_t db_op_return;
		unsigned long db_panic_caller;
	} debugger_state;
};

typedef struct processor_data   processor_data_t;

#define PROCESSOR_DATA(processor, member)       \
	                                (processor)->processor_data.member

extern  void    processor_data_init(
	processor_t             processor);

#define SCHED_STATS_INTERRUPT(p)                                        \
MACRO_BEGIN                                                             \
	if (__builtin_expect(sched_stats_active, 0)) {          \
	        (p)->processor_data.sched_stats.interrupt_count++;      \
	}                                                               \
MACRO_END

#define SCHED_STATS_TIMER_POP(p)                                        \
MACRO_BEGIN                                                             \
	if (__builtin_expect(sched_stats_active, 0)) {          \
	        (p)->processor_data.sched_stats.timer_pop_count++;      \
	}                                                               \
MACRO_END

#define SCHED_STATS_IPI(p)                                              \
MACRO_BEGIN                                                             \
	if (__builtin_expect(sched_stats_active, 0)) {          \
	        (p)->processor_data.sched_stats.ipi_count++;            \
	}                                                               \
MACRO_END

#define SCHED_STATS_CPU_IDLE_START(p)                                                           \
MACRO_BEGIN                                                                                     \
	if (__builtin_expect(sched_stats_active, 0)) {                                  \
	        (p)->processor_data.sched_stats.idle_transitions++;                             \
	}                                                                                       \
MACRO_END

#define SCHED_STATS_QUANTUM_TIMER_EXPIRATION(p)                                                         \
MACRO_BEGIN                                                                                     \
	if (__builtin_expect(sched_stats_active, 0)) {                                  \
	        (p)->processor_data.sched_stats.quantum_timer_expirations++;            \
	}                                                                                       \
MACRO_END

#endif /* MACH_KERNEL_PRIVATE */

extern boolean_t processor_in_panic_context(processor_t processor);

#endif /* XNU_KERNEL_PRIVATE */

#endif /* _KERN_PROCESSOR_DATA_H_ */
