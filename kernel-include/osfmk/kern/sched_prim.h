/*
 * Copyright (c) 2000-2012 Apple Inc. All rights reserved.
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
/*
 *	File:	sched_prim.h
 *	Author:	David Golub
 *
 *	Scheduling primitive definitions file
 *
 */

#ifndef _KERN_SCHED_PRIM_H_
#define _KERN_SCHED_PRIM_H_

#include <mach/boolean.h>
#include <mach/machine/vm_types.h>
#include <mach/kern_return.h>
#include <kern/clock.h>
#include <kern/kern_types.h>
#include <kern/thread.h>
#include <sys/cdefs.h>
#include <kern/block_hint.h>

#ifdef  MACH_KERNEL_PRIVATE

#include <kern/sched_urgency.h>
#include <kern/thread_group.h>

/* Initialization */
extern void             sched_init(void);

extern void             sched_startup(void);

extern void             sched_timebase_init(void);

extern void             pset_rt_init(processor_set_t pset);

extern void             sched_rtglobal_init(processor_set_t pset);

extern rt_queue_t       sched_rtglobal_runq(processor_set_t pset);

extern void             sched_rtglobal_queue_shutdown(processor_t processor);

extern int64_t          sched_rtglobal_runq_count_sum(void);

extern void             sched_check_spill(processor_set_t pset, thread_t thread);

extern bool             sched_thread_should_yield(processor_t processor, thread_t thread);

extern bool             sched_steal_thread_DISABLED(processor_set_t pset);
extern bool             sched_steal_thread_enabled(processor_set_t pset);

/* Force a preemption point for a thread and wait for it to stop running */
extern boolean_t        thread_stop(
	thread_t        thread,
	boolean_t       until_not_runnable);

/* Release a previous stop request */
extern void                     thread_unstop(
	thread_t        thread);

/* Wait for a thread to stop running */
extern void                     thread_wait(
	thread_t        thread,
	boolean_t       until_not_runnable);

/* Unblock thread on wake up */
extern boolean_t        thread_unblock(
	thread_t                thread,
	wait_result_t   wresult);

/* Unblock and dispatch thread */
extern kern_return_t    thread_go(
	thread_t                thread,
	wait_result_t   wresult);

/* Handle threads at context switch */
extern void                     thread_dispatch(
	thread_t                old_thread,
	thread_t                new_thread);

/* Switch directly to a particular thread */
extern int                      thread_run(
	thread_t                        self,
	thread_continue_t       continuation,
	void                            *parameter,
	thread_t                        new_thread);

/* Resume thread with new stack */
extern __dead2 void     thread_continue(thread_t old_thread);

/* Invoke continuation */
extern __dead2 void     call_continuation(
	thread_continue_t       continuation,
	void                    *parameter,
	wait_result_t           wresult,
	boolean_t               enable_interrupts);

/*
 * Flags that can be passed to set_sched_pri
 * to skip side effects
 */
__options_decl(set_sched_pri_options_t, uint32_t, {
	SETPRI_DEFAULT  = 0x0,
	SETPRI_LAZY     = 0x1,  /* Avoid setting AST flags or sending IPIs */
});

/* Set the current scheduled priority */
extern void set_sched_pri(
	thread_t      thread,
	int           priority,
	set_sched_pri_options_t options);

/* Set base priority of the specified thread */
extern void             sched_set_thread_base_priority(
	thread_t                thread,
	int                             priority);

/* Set absolute base priority of the specified thread */
extern void             sched_set_kernel_thread_priority(
	thread_t                thread,
	int                             priority);


/* Set the thread's true scheduling mode */
extern void             sched_set_thread_mode(thread_t thread,
    sched_mode_t mode);
/* Demote the true scheduler mode */
extern void             sched_thread_mode_demote(thread_t thread,
    uint32_t reason);
/* Un-demote the true scheduler mode */
extern void             sched_thread_mode_undemote(thread_t thread,
    uint32_t reason);

extern void sched_thread_promote_reason(thread_t thread, uint32_t reason, uintptr_t trace_obj);
extern void sched_thread_unpromote_reason(thread_t thread, uint32_t reason, uintptr_t trace_obj);

/* Re-evaluate base priority of thread (thread locked) */
void thread_recompute_priority(thread_t thread);

/* Re-evaluate scheduled priority of thread (thread locked) */
extern void thread_recompute_sched_pri(
	thread_t thread,
	set_sched_pri_options_t options);

/* Periodic scheduler activity */
extern void             sched_init_thread(void (*)(void));

/* Perform sched_tick housekeeping activities */
extern boolean_t                can_update_priority(
	thread_t                thread);

extern void             update_priority(
	thread_t                thread);

extern void             lightweight_update_priority(
	thread_t                thread);

extern void             sched_default_quantum_expire(thread_t thread);

/* Idle processor thread continuation */
extern void             idle_thread(
	void*           parameter,
	wait_result_t   result);

extern kern_return_t    idle_thread_create(
	processor_t             processor);

/* Continuation return from syscall */
extern void     thread_syscall_return(
	kern_return_t   ret);

/* Context switch */
extern wait_result_t    thread_block_reason(
	thread_continue_t       continuation,
	void                            *parameter,
	ast_t                           reason);

__options_decl(sched_options_t, uint32_t, {
	SCHED_NONE      = 0x0,
	SCHED_TAILQ     = 0x1,
	SCHED_HEADQ     = 0x2,
	SCHED_PREEMPT   = 0x4,
	SCHED_REBALANCE = 0x8,
});

/* Reschedule thread for execution */
extern void             thread_setrun(
	thread_t        thread,
	sched_options_t options);

extern processor_set_t  task_choose_pset(
	task_t                  task);

/* Bind the current thread to a particular processor */
extern processor_t              thread_bind(
	processor_t             processor);

/* Choose the best processor to run a thread */
extern processor_t      choose_processor(
	processor_set_t                pset,
	processor_t                    processor,
	thread_t                       thread);

extern void sched_SMT_balance(
	processor_t processor,
	processor_set_t pset);

extern void thread_quantum_init(
	thread_t thread);

extern void             run_queue_init(
	run_queue_t             runq);

extern thread_t run_queue_dequeue(
	run_queue_t           runq,
	sched_options_t       options);

extern boolean_t        run_queue_enqueue(
	run_queue_t           runq,
	thread_t              thread,
	sched_options_t       options);

extern void     run_queue_remove(
	run_queue_t            runq,
	thread_t                       thread);

extern thread_t run_queue_peek(
	run_queue_t            runq);

struct sched_update_scan_context {
	uint64_t        earliest_bg_make_runnable_time;
	uint64_t        earliest_normal_make_runnable_time;
	uint64_t        earliest_rt_make_runnable_time;
};
typedef struct sched_update_scan_context *sched_update_scan_context_t;

extern void             sched_rtglobal_runq_scan(sched_update_scan_context_t scan_context);

extern void sched_pset_made_schedulable(
	processor_t processor,
	processor_set_t pset,
	boolean_t drop_lock);

/*
 * Enum to define various events which need IPIs. The IPI policy
 * engine decides what kind of IPI to use based on destination
 * processor state, thread and one of the following scheduling events.
 */
typedef enum {
	SCHED_IPI_EVENT_BOUND_THR   = 0x1,
	SCHED_IPI_EVENT_PREEMPT     = 0x2,
	SCHED_IPI_EVENT_SMT_REBAL   = 0x3,
	SCHED_IPI_EVENT_SPILL       = 0x4,
	SCHED_IPI_EVENT_REBALANCE   = 0x5,
} sched_ipi_event_t;


/* Enum to define various IPI types used by the scheduler */
typedef enum {
	SCHED_IPI_NONE              = 0x0,
	SCHED_IPI_IMMEDIATE         = 0x1,
	SCHED_IPI_IDLE              = 0x2,
	SCHED_IPI_DEFERRED          = 0x3,
} sched_ipi_type_t;

/* The IPI policy engine behaves in the following manner:
 * - All scheduler events which need an IPI invoke sched_ipi_action() with
 *   the appropriate destination processor, thread and event.
 * - sched_ipi_action() performs basic checks, invokes the scheduler specific
 *   ipi_policy routine and sets pending_AST bits based on the result.
 * - Once the pset lock is dropped, the scheduler invokes sched_ipi_perform()
 *   routine which actually sends the appropriate IPI to the destination core.
 */
extern sched_ipi_type_t sched_ipi_action(processor_t dst, thread_t thread,
    boolean_t dst_idle, sched_ipi_event_t event);
extern void sched_ipi_perform(processor_t dst, sched_ipi_type_t ipi);

/* sched_ipi_policy() is the global default IPI policy for all schedulers */
extern sched_ipi_type_t sched_ipi_policy(processor_t dst, thread_t thread,
    boolean_t dst_idle, sched_ipi_event_t event);

/* sched_ipi_deferred_policy() is the global default deferred IPI policy for all schedulers */
extern sched_ipi_type_t sched_ipi_deferred_policy(processor_set_t pset,
    processor_t dst, sched_ipi_event_t event);

#if defined(CONFIG_SCHED_TIMESHARE_CORE)

extern boolean_t        thread_update_add_thread(thread_t thread);
extern void             thread_update_process_threads(void);
extern boolean_t        runq_scan(run_queue_t runq, sched_update_scan_context_t scan_context);

extern void sched_timeshare_init(void);
extern void sched_timeshare_timebase_init(void);
extern void sched_timeshare_maintenance_continue(void);

extern boolean_t priority_is_urgent(int priority);
extern uint32_t sched_timeshare_initial_quantum_size(thread_t thread);

extern int sched_compute_timeshare_priority(thread_t thread);

#endif /* CONFIG_SCHED_TIMESHARE_CORE */

/* Remove thread from its run queue */
extern boolean_t        thread_run_queue_remove(thread_t thread);
thread_t thread_run_queue_remove_for_handoff(thread_t thread);

/* Put a thread back in the run queue after being yanked */
extern void thread_run_queue_reinsert(thread_t thread, sched_options_t options);

extern void             thread_timer_expire(
	void                    *thread,
	void                    *p1);

extern boolean_t        thread_eager_preemption(
	thread_t thread);

extern boolean_t sched_generic_direct_dispatch_to_idle_processors;

/* Set the maximum interrupt level for the thread */
__private_extern__ wait_interrupt_t thread_interrupt_level(
	wait_interrupt_t interruptible);

__private_extern__ wait_result_t thread_mark_wait_locked(
	thread_t                 thread,
	wait_interrupt_t interruptible);

/* Wake up locked thread directly, passing result */
__private_extern__ kern_return_t clear_wait_internal(
	thread_t                thread,
	wait_result_t   result);

extern void sched_stats_handle_csw(
	processor_t processor,
	int reasons,
	int selfpri,
	int otherpri);

extern void sched_stats_handle_runq_change(
	struct runq_stats *stats,
	int old_count);


#if DEBUG

#define SCHED_STATS_CSW(processor, reasons, selfpri, otherpri)          \
do {                                                            \
	if (__builtin_expect(sched_stats_active, 0)) {  \
	        sched_stats_handle_csw((processor),             \
	                        (reasons), (selfpri), (otherpri));      \
	}                                                       \
} while (0)


#define SCHED_STATS_RUNQ_CHANGE(stats, old_count)               \
do {                                                            \
	if (__builtin_expect(sched_stats_active, 0)) {  \
	        sched_stats_handle_runq_change((stats),         \
	                                                        (old_count));           \
	}                                                       \
} while (0)

#else /* DEBUG */

#define SCHED_STATS_CSW(processor, reasons, selfpri, otherpri) do { }while(0)
#define SCHED_STATS_RUNQ_CHANGE(stats, old_count) do { }while(0)

#endif /* DEBUG */

extern uint32_t sched_debug_flags;
#define SCHED_DEBUG_FLAG_PLATFORM_TRACEPOINTS   0x00000001
#define SCHED_DEBUG_FLAG_CHOOSE_PROCESSOR_TRACEPOINTS   0x00000002

#define SCHED_DEBUG_PLATFORM_KERNEL_DEBUG_CONSTANT(...) do {                                            \
	        if (__improbable(sched_debug_flags & SCHED_DEBUG_FLAG_PLATFORM_TRACEPOINTS)) { \
	                KERNEL_DEBUG_CONSTANT(__VA_ARGS__);                                                     \
	        }                                                                                                                               \
	} while(0)

#define SCHED_DEBUG_CHOOSE_PROCESSOR_KERNEL_DEBUG_CONSTANT(...) do {                                            \
	        if (__improbable(sched_debug_flags & SCHED_DEBUG_FLAG_CHOOSE_PROCESSOR_TRACEPOINTS)) { \
	                KERNEL_DEBUG_CONSTANT(__VA_ARGS__);                                                     \
	        }                                                                                                                               \
	} while(0)

/* Tells if there are "active" RT threads in the system (provided by CPU PM) */
extern void     active_rt_threads(
	boolean_t       active);

/* Returns the perfcontrol attribute for the thread */
extern perfcontrol_class_t thread_get_perfcontrol_class(
	thread_t        thread);

/* Generic routine for Non-AMP schedulers to calculate parallelism */
extern uint32_t sched_qos_max_parallelism(int qos, uint64_t options);

#endif /* MACH_KERNEL_PRIVATE */

__BEGIN_DECLS

#ifdef  XNU_KERNEL_PRIVATE

extern void thread_bind_cluster_type(char cluster_type);

/* Toggles a global override to turn off CPU Throttling */
extern void     sys_override_cpu_throttle(boolean_t enable_override);

/*
 ****************** Only exported until BSD stops using ********************
 */

extern void                     thread_vm_bind_group_add(void);

/* Wake up thread directly, passing result */
extern kern_return_t clear_wait(
	thread_t                thread,
	wait_result_t   result);

/* Start thread running */
extern void             thread_bootstrap_return(void) __attribute__((noreturn));

/* Return from exception (BSD-visible interface) */
extern void             thread_exception_return(void) __dead2;

#define SCHED_STRING_MAX_LENGTH (48)
/* String declaring the name of the current scheduler */
extern char sched_string[SCHED_STRING_MAX_LENGTH];

/* Attempt to context switch to a specific runnable thread */
extern wait_result_t thread_handoff_deallocate(thread_t thread);

__attribute__((nonnull(1, 2)))
extern void thread_handoff_parameter(thread_t thread,
    thread_continue_t continuation, void *parameter) __dead2;

extern struct waitq     *assert_wait_queue(event_t event);

extern kern_return_t thread_wakeup_one_with_pri(event_t event, int priority);

extern thread_t thread_wakeup_identify(event_t event, int priority);

#endif  /* XNU_KERNEL_PRIVATE */

#ifdef KERNEL_PRIVATE
/* Set pending block hint for a particular object before we go into a wait state */
extern void             thread_set_pending_block_hint(
	thread_t                        thread,
	block_hint_t                    block_hint);

#define QOS_PARALLELISM_COUNT_LOGICAL   0x1
#define QOS_PARALLELISM_REALTIME        0x2
extern uint32_t qos_max_parallelism(int qos, uint64_t options);
#endif /* KERNEL_PRIVATE */

#if XNU_KERNEL_PRIVATE
extern void             thread_yield_with_continuation(
	thread_continue_t       continuation,
	void                            *parameter) __dead2;
#endif

/* Context switch */
extern wait_result_t    thread_block(
	thread_continue_t       continuation);

extern wait_result_t    thread_block_parameter(
	thread_continue_t       continuation,
	void                            *parameter);

/* Declare thread will wait on a particular event */
extern wait_result_t    assert_wait(
	event_t                         event,
	wait_interrupt_t        interruptible);

/* Assert that the thread intends to wait with a timeout */
extern wait_result_t    assert_wait_timeout(
	event_t                         event,
	wait_interrupt_t        interruptible,
	uint32_t                        interval,
	uint32_t                        scale_factor);

/* Assert that the thread intends to wait with an urgency, timeout and leeway */
extern wait_result_t    assert_wait_timeout_with_leeway(
	event_t                         event,
	wait_interrupt_t        interruptible,
	wait_timeout_urgency_t  urgency,
	uint32_t                        interval,
	uint32_t                        leeway,
	uint32_t                        scale_factor);

extern wait_result_t    assert_wait_deadline(
	event_t                         event,
	wait_interrupt_t        interruptible,
	uint64_t                        deadline);

/* Assert that the thread intends to wait with an urgency, deadline, and leeway */
extern wait_result_t    assert_wait_deadline_with_leeway(
	event_t                         event,
	wait_interrupt_t        interruptible,
	wait_timeout_urgency_t  urgency,
	uint64_t                        deadline,
	uint64_t                        leeway);

/* Wake up thread (or threads) waiting on a particular event */
extern kern_return_t    thread_wakeup_prim(
	event_t                         event,
	boolean_t                       one_thread,
	wait_result_t                   result);

#define thread_wakeup(x)                                        \
	                thread_wakeup_prim((x), FALSE, THREAD_AWAKENED)
#define thread_wakeup_with_result(x, z)         \
	                thread_wakeup_prim((x), FALSE, (z))
#define thread_wakeup_one(x)                            \
	                thread_wakeup_prim((x), TRUE, THREAD_AWAKENED)

/* Wakeup the specified thread if it is waiting on this event */
extern kern_return_t thread_wakeup_thread(event_t event, thread_t thread);

extern boolean_t preemption_enabled(void);

#ifdef MACH_KERNEL_PRIVATE

/*
 * Scheduler algorithm indirection. If only one algorithm is
 * enabled at compile-time, a direction function call is used.
 * If more than one is enabled, calls are dispatched through
 * a function pointer table.
 */

#if   !defined(CONFIG_SCHED_TRADITIONAL) && !defined(CONFIG_SCHED_PROTO) && !defined(CONFIG_SCHED_GRRR) && !defined(CONFIG_SCHED_MULTIQ) && !defined(CONFIG_SCHED_CLUTCH)
#error Enable at least one scheduler algorithm in osfmk/conf/MASTER.XXX
#endif


#if CONFIG_SCHED_CLUTCH
extern const struct sched_dispatch_table sched_clutch_dispatch;
#define SCHED(f) (sched_clutch_dispatch.f)
#else /* CONFIG_SCHED_CLUTCH */
extern const struct sched_dispatch_table sched_dualq_dispatch;
#define SCHED(f) (sched_dualq_dispatch.f)
#endif /* CONFIG_SCHED_CLUTCH */


struct sched_dispatch_table {
	const char *sched_name;
	void    (*init)(void);                          /* Init global state */
	void    (*timebase_init)(void);         /* Timebase-dependent initialization */
	void    (*processor_init)(processor_t processor);       /* Per-processor scheduler init */
	void    (*pset_init)(processor_set_t pset);     /* Per-processor set scheduler init */

	void    (*maintenance_continuation)(void);      /* Function called regularly */

	/*
	 * Choose a thread of greater or equal priority from the per-processor
	 * runqueue for timeshare/fixed threads
	 */
	thread_t        (*choose_thread)(
		processor_t           processor,
		int                           priority,
		ast_t reason);

	/* True if scheduler supports stealing threads for this pset */
	bool    (*steal_thread_enabled)(processor_set_t pset);

	/*
	 * Steal a thread from another processor in the pset so that it can run
	 * immediately
	 */
	thread_t        (*steal_thread)(
		processor_set_t         pset);

	/*
	 * Compute priority for a timeshare thread based on base priority.
	 */
	int (*compute_timeshare_priority)(thread_t thread);

	/*
	 * Pick the best processor for a thread (any kind of thread) to run on.
	 */
	processor_t     (*choose_processor)(
		processor_set_t                pset,
		processor_t                    processor,
		thread_t                       thread);
	/*
	 * Enqueue a timeshare or fixed priority thread onto the per-processor
	 * runqueue
	 */
	boolean_t (*processor_enqueue)(
		processor_t                    processor,
		thread_t                       thread,
		sched_options_t                options);

	/* Migrate threads away in preparation for processor shutdown */
	void (*processor_queue_shutdown)(
		processor_t                    processor);

	/* Remove the specific thread from the per-processor runqueue */
	boolean_t       (*processor_queue_remove)(
		processor_t             processor,
		thread_t                thread);

	/*
	 * Does the per-processor runqueue have any timeshare or fixed priority
	 * threads on it? Called without pset lock held, so should
	 * not assume immutability while executing.
	 */
	boolean_t       (*processor_queue_empty)(processor_t            processor);

	/*
	 * Would this priority trigger an urgent preemption if it's sitting
	 * on the per-processor runqueue?
	 */
	boolean_t       (*priority_is_urgent)(int priority);

	/*
	 * Does the per-processor runqueue contain runnable threads that
	 * should cause the currently-running thread to be preempted?
	 */
	ast_t           (*processor_csw_check)(processor_t processor);

	/*
	 * Does the per-processor runqueue contain a runnable thread
	 * of > or >= priority, as a preflight for choose_thread() or other
	 * thread selection
	 */
	boolean_t       (*processor_queue_has_priority)(processor_t             processor,
	    int                             priority,
	    boolean_t               gte);

	/* Quantum size for the specified non-realtime thread. */
	uint32_t        (*initial_quantum_size)(thread_t thread);

	/* Scheduler mode for a new thread */
	sched_mode_t    (*initial_thread_sched_mode)(task_t parent_task);

	/*
	 * Is it safe to call update_priority, which may change a thread's
	 * runqueue or other state. This can be used to throttle changes
	 * to dynamic priority.
	 */
	boolean_t       (*can_update_priority)(thread_t thread);

	/*
	 * Update both scheduled priority and other persistent state.
	 * Side effects may including migration to another processor's runqueue.
	 */
	void            (*update_priority)(thread_t thread);

	/* Lower overhead update to scheduled priority and state. */
	void            (*lightweight_update_priority)(thread_t thread);

	/* Callback for non-realtime threads when the quantum timer fires */
	void            (*quantum_expire)(thread_t thread);

	/*
	 * Runnable threads on per-processor runqueue. Should only
	 * be used for relative comparisons of load between processors.
	 */
	int                     (*processor_runq_count)(processor_t     processor);

	/* Aggregate runcount statistics for per-processor runqueue */
	uint64_t    (*processor_runq_stats_count_sum)(processor_t   processor);

	boolean_t       (*processor_bound_count)(processor_t processor);

	void            (*thread_update_scan)(sched_update_scan_context_t scan_context);

	/* Supports more than one pset */
	boolean_t   multiple_psets_enabled;
	/* Supports scheduler groups */
	boolean_t   sched_groups_enabled;

	/* Supports avoid-processor */
	boolean_t   avoid_processor_enabled;

	/* Returns true if this processor should avoid running this thread. */
	bool    (*thread_avoid_processor)(processor_t processor, thread_t thread);

	/*
	 * Invoked when a processor is about to choose the idle thread
	 * Used to send IPIs to a processor which would be preferred to be idle instead.
	 * Called with pset lock held, returns pset lock unlocked.
	 */
	void    (*processor_balance)(processor_t processor, processor_set_t pset);
	rt_queue_t      (*rt_runq)(processor_set_t pset);
	void    (*rt_init)(processor_set_t pset);
	void    (*rt_queue_shutdown)(processor_t processor);
	void    (*rt_runq_scan)(sched_update_scan_context_t scan_context);
	int64_t (*rt_runq_count_sum)(void);

	uint32_t (*qos_max_parallelism)(int qos, uint64_t options);
	void    (*check_spill)(processor_set_t pset, thread_t thread);
	sched_ipi_type_t (*ipi_policy)(processor_t dst, thread_t thread, boolean_t dst_idle, sched_ipi_event_t event);
	bool    (*thread_should_yield)(processor_t processor, thread_t thread);

	/* Routine to update run counts */
	uint32_t (*run_count_incr)(thread_t thread);
	uint32_t (*run_count_decr)(thread_t thread);

	/* Routine to update scheduling bucket for a thread */
	void (*update_thread_bucket)(thread_t thread);

	/* Routine to inform the scheduler when a new pset becomes schedulable */
	void (*pset_made_schedulable)(processor_t processor, processor_set_t pset, boolean_t drop_lock);
};

#if defined(CONFIG_SCHED_TRADITIONAL)
extern const struct sched_dispatch_table sched_traditional_dispatch;
extern const struct sched_dispatch_table sched_traditional_with_pset_runqueue_dispatch;
#endif

#if defined(CONFIG_SCHED_MULTIQ)
extern const struct sched_dispatch_table sched_multiq_dispatch;
extern const struct sched_dispatch_table sched_dualq_dispatch;
#endif

#if defined(CONFIG_SCHED_PROTO)
extern const struct sched_dispatch_table sched_proto_dispatch;
#endif

#if defined(CONFIG_SCHED_GRRR)
extern const struct sched_dispatch_table sched_grrr_dispatch;
#endif

#if defined(CONFIG_SCHED_CLUTCH)
extern const struct sched_dispatch_table sched_clutch_dispatch;
#endif

#endif  /* MACH_KERNEL_PRIVATE */

__END_DECLS

#endif  /* _KERN_SCHED_PRIM_H_ */
