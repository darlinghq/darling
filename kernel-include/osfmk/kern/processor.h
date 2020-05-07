/*
 * Copyright (c) 2000-2019 Apple Inc. All rights reserved.
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
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 *	processor.h:	Processor and processor-related definitions.
 */

#ifndef _KERN_PROCESSOR_H_
#define _KERN_PROCESSOR_H_

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <kern/kern_types.h>

#include <sys/cdefs.h>

#ifdef  MACH_KERNEL_PRIVATE

#include <mach/mach_types.h>
#include <kern/ast.h>
#include <kern/cpu_number.h>
#include <kern/smp.h>
#include <kern/simple_lock.h>
#include <kern/locks.h>
#include <kern/queue.h>
#include <kern/sched.h>
#include <kern/sched_urgency.h>
#include <mach/sfi_class.h>
#include <kern/processor_data.h>
#include <kern/cpu_quiesce.h>
#include <kern/sched_clutch.h>
#include <kern/assert.h>
#include <machine/limits.h>

/*
 *	Processor state is accessed by locking the scheduling lock
 *	for the assigned processor set.
 *
 *           -------------------- SHUTDOWN
 *          /                     ^     ^
 *        _/                      |      \
 *  OFF_LINE ---> START ---> RUNNING ---> IDLE ---> DISPATCHING
 *         \_________________^   ^ ^______/           /
 *                                \__________________/
 *
 *  Most of these state transitions are externally driven as a
 *  a directive (for instance telling an IDLE processor to start
 *  coming out of the idle state to run a thread). However these
 *  are typically paired with a handshake by the processor itself
 *  to indicate that it has completed a transition of indeterminate
 *  length (for example, the DISPATCHING->RUNNING or START->RUNNING
 *  transitions must occur on the processor itself).
 *
 *  The boot processor has some special cases, and skips the START state,
 *  since it has already bootstrapped and is ready to context switch threads.
 *
 *  When a processor is in DISPATCHING or RUNNING state, the current_pri,
 *  current_thmode, and deadline fields should be set, so that other
 *  processors can evaluate if it is an appropriate candidate for preemption.
 */
#if defined(CONFIG_SCHED_DEFERRED_AST)
/*
 *           -------------------- SHUTDOWN
 *          /                     ^     ^
 *        _/                      |      \
 *  OFF_LINE ---> START ---> RUNNING ---> IDLE ---> DISPATCHING
 *         \_________________^   ^ ^______/ ^_____ /  /
 *                                \__________________/
 *
 *  A DISPATCHING processor may be put back into IDLE, if another
 *  processor determines that the target processor will have nothing to do
 *  upon reaching the RUNNING state.  This is racy, but if the target
 *  responds and becomes RUNNING, it will not break the processor state
 *  machine.
 *
 *  This change allows us to cancel an outstanding signal/AST on a processor
 *  (if such an operation is supported through hardware or software), and
 *  push the processor back into the IDLE state as a power optimization.
 */
#endif

typedef enum {
	PROCESSOR_OFF_LINE      = 0,    /* Not available */
	PROCESSOR_SHUTDOWN      = 1,    /* Going off-line */
	PROCESSOR_START         = 2,    /* Being started */
	PROCESSOR_UNUSED        = 3,    /* Formerly Inactive (unavailable) */
	PROCESSOR_IDLE          = 4,    /* Idle (available) */
	PROCESSOR_DISPATCHING   = 5,    /* Dispatching (idle -> active) */
	PROCESSOR_RUNNING       = 6,    /* Normal execution */
	PROCESSOR_STATE_LEN     = (PROCESSOR_RUNNING + 1)
} processor_state_t;

typedef enum {
	PSET_SMP,
} pset_cluster_type_t;

typedef bitmap_t cpumap_t;

struct processor_set {
	int                     online_processor_count;
	int                     load_average;

	int                     cpu_set_low, cpu_set_hi;
	int                     cpu_set_count;
	int                     last_chosen;
	cpumap_t                cpu_bitmask;
	cpumap_t                recommended_bitmask;
	cpumap_t                cpu_state_map[PROCESSOR_STATE_LEN];
	cpumap_t                primary_map;
#define SCHED_PSET_TLOCK (1)
#if __SMP__
#if     defined(SCHED_PSET_TLOCK)
/* TODO: reorder struct for temporal cache locality */
	__attribute__((aligned(128))) lck_ticket_t      sched_lock;
#else /* SCHED_PSET_TLOCK*/
	__attribute__((aligned(128))) lck_spin_t        sched_lock;     /* lock for above */
#endif /* SCHED_PSET_TLOCK*/
#endif

#if defined(CONFIG_SCHED_TRADITIONAL) || defined(CONFIG_SCHED_MULTIQ)
	struct run_queue        pset_runq;      /* runq for this processor set */
#endif
	struct rt_queue         rt_runq;        /* realtime runq for this processor set */
#if CONFIG_SCHED_CLUTCH
	struct sched_clutch_root                pset_clutch_root; /* clutch hierarchy root */
#endif /* CONFIG_SCHED_CLUTCH */

#if defined(CONFIG_SCHED_TRADITIONAL)
	int                                     pset_runq_bound_count;
	/* # of threads in runq bound to any processor in pset */
#endif

	/* CPUs that have been sent an unacknowledged remote AST for scheduling purposes */
	cpumap_t                        pending_AST_URGENT_cpu_mask;
	cpumap_t                        pending_AST_PREEMPT_cpu_mask;
#if defined(CONFIG_SCHED_DEFERRED_AST)
	/*
	 * A separate mask, for ASTs that we may be able to cancel.  This is dependent on
	 * some level of support for requesting an AST on a processor, and then quashing
	 * that request later.
	 *
	 * The purpose of this field (and the associated codepaths) is to infer when we
	 * no longer need a processor that is DISPATCHING to come up, and to prevent it
	 * from coming out of IDLE if possible.  This should serve to decrease the number
	 * of spurious ASTs in the system, and let processors spend longer periods in
	 * IDLE.
	 */
	cpumap_t                        pending_deferred_AST_cpu_mask;
#endif
	cpumap_t                        pending_spill_cpu_mask;

	struct ipc_port *       pset_self;              /* port for operations */
	struct ipc_port *       pset_name_self; /* port for information */

	processor_set_t         pset_list;              /* chain of associated psets */
	pset_node_t             node;
	uint32_t                pset_cluster_id;
	pset_cluster_type_t     pset_cluster_type;
};

extern struct processor_set     pset0;

struct pset_node {
	processor_set_t         psets;                  /* list of associated psets */
	uint32_t                pset_count;             /* count of associated psets */

	pset_node_t                     nodes;                  /* list of associated subnodes */
	pset_node_t                     node_list;              /* chain of associated nodes */

	pset_node_t                     parent;
};

extern struct pset_node pset_node0;

extern queue_head_t             tasks, terminated_tasks, threads, corpse_tasks; /* Terminated tasks are ONLY for stackshot */
extern int                              tasks_count, terminated_tasks_count, threads_count;
decl_lck_mtx_data(extern, tasks_threads_lock);
decl_lck_mtx_data(extern, tasks_corpse_lock);

struct processor {
	processor_state_t       state;                  /* See above */
	bool                    is_SMT;
	bool                    is_recommended;
	struct thread           *active_thread;         /* thread running on processor */
	struct thread           *idle_thread;           /* this processor's idle thread. */
	struct thread           *startup_thread;

	processor_set_t         processor_set;  /* assigned set */

	int                     current_pri;            /* priority of current thread */
	sfi_class_id_t          current_sfi_class;      /* SFI class of current thread */
	perfcontrol_class_t     current_perfctl_class;  /* Perfcontrol class for current thread */
	pset_cluster_type_t     current_recommended_pset_type;  /* Cluster type recommended for current thread */
	thread_urgency_t        current_urgency;        /* cached urgency of current thread */
	bool                    current_is_NO_SMT;         /* cached TH_SFLAG_NO_SMT of current thread */
	bool                    current_is_bound;       /* current thread is bound to this processor */

	int                     starting_pri;           /* priority of current thread as it was when scheduled */
	int                     cpu_id;                 /* platform numeric id */
	cpu_quiescent_state_t   cpu_quiesce_state;
	uint64_t                cpu_quiesce_last_checkin;

	timer_call_data_t       quantum_timer;  /* timer for quantum expiration */
	uint64_t                        quantum_end;    /* time when current quantum ends */
	uint64_t                        last_dispatch;  /* time of last dispatch */

	uint64_t                        kperf_last_sample_time; /* time of last kperf sample */

	uint64_t                        deadline;               /* current deadline */
	bool                    first_timeslice;        /* has the quantum expired since context switch */
	bool                    processor_offlined;        /* has the processor been explicitly processor_offline'ed */
	bool                    must_idle;              /* Needs to be forced idle as next selected thread is allowed on this processor */

	processor_t             processor_primary;      /* pointer to primary processor for
	                                                 * secondary SMT processors, or a pointer
	                                                 * to ourselves for primaries or non-SMT */
	processor_t             processor_secondary;

#if defined(CONFIG_SCHED_TRADITIONAL) || defined(CONFIG_SCHED_MULTIQ)
	struct run_queue        runq;                   /* runq for this processor */
#endif

#if defined(CONFIG_SCHED_TRADITIONAL)
	int                                     runq_bound_count; /* # of threads bound to this processor */
#endif
#if defined(CONFIG_SCHED_GRRR)
	struct grrr_run_queue   grrr_runq;      /* Group Ratio Round-Robin runq */
#endif
	struct ipc_port *       processor_self; /* port for operations */

	processor_t                     processor_list; /* all existing processors */
	processor_data_t        processor_data; /* per-processor data */
};

extern processor_t              processor_list;
decl_simple_lock_data(extern, processor_list_lock);

#define MAX_SCHED_CPUS          64 /* Maximum number of CPUs supported by the scheduler.  bits.h:bitmap_*() macros need to be used to support greater than 64 */
extern processor_t              processor_array[MAX_SCHED_CPUS]; /* array indexed by cpuid */

extern uint32_t                 processor_avail_count;
extern uint32_t                 processor_avail_count_user;

extern processor_t              master_processor;

extern boolean_t                sched_stats_active;

extern processor_t      current_processor(void);

/* Lock macros, always acquired and released with interrupts disabled (splsched()) */

extern lck_grp_t pset_lck_grp;

#if __SMP__
#if defined(SCHED_PSET_TLOCK)
#define pset_lock_init(p)               lck_ticket_init(&(p)->sched_lock)
#define pset_lock(p)                    lck_ticket_lock(&(p)->sched_lock)
#define pset_unlock(p)                  lck_ticket_unlock(&(p)->sched_lock)
#define pset_assert_locked(p)           lck_ticket_assert_owned(&(p)->sched_lock)
#else /* SCHED_PSET_TLOCK*/
#define pset_lock_init(p)               lck_spin_init(&(p)->sched_lock, &pset_lck_grp, NULL)
#define pset_lock(p)                    lck_spin_lock_grp(&(p)->sched_lock, &pset_lck_grp)
#define pset_unlock(p)                  lck_spin_unlock(&(p)->sched_lock)
#define pset_assert_locked(p)           LCK_SPIN_ASSERT(&(p)->sched_lock, LCK_ASSERT_OWNED)
#endif /*!SCHED_PSET_TLOCK*/

#define rt_lock_lock(p)                 simple_lock(&SCHED(rt_runq)(p)->rt_lock, &pset_lck_grp)
#define rt_lock_unlock(p)               simple_unlock(&SCHED(rt_runq)(p)->rt_lock)
#define rt_lock_init(p)                 simple_lock_init(&SCHED(rt_runq)(p)->rt_lock, 0)
#else
#define pset_lock(p)                    do { (void)p; } while(0)
#define pset_unlock(p)                  do { (void)p; } while(0)
#define pset_lock_init(p)               do { (void)p; } while(0)
#define pset_assert_locked(p)           do { (void)p; } while(0)

#define rt_lock_lock(p)                 do { (void)p; } while(0)
#define rt_lock_unlock(p)               do { (void)p; } while(0)
#define rt_lock_init(p)                 do { (void)p; } while(0)
#endif /* SMP */

extern void             processor_bootstrap(void);

extern void             processor_init(
	processor_t             processor,
	int                             cpu_id,
	processor_set_t processor_set);

extern void             processor_set_primary(
	processor_t             processor,
	processor_t             primary);

extern kern_return_t    processor_shutdown(
	processor_t             processor);

extern kern_return_t    processor_start_from_user(
	processor_t     processor);
extern kern_return_t    processor_exit_from_user(
	processor_t     processor);

kern_return_t
sched_processor_enable(processor_t processor, boolean_t enable);

extern void             processor_queue_shutdown(
	processor_t             processor);

extern void             processor_queue_shutdown(
	processor_t             processor);

extern processor_set_t  processor_pset(
	processor_t             processor);

extern pset_node_t              pset_node_root(void);

extern processor_set_t  pset_create(
	pset_node_t             node);

extern void             pset_init(
	processor_set_t         pset,
	pset_node_t                     node);

extern processor_set_t pset_find(
	uint32_t cluster_id,
	processor_set_t default_pset);

extern kern_return_t    processor_info_count(
	processor_flavor_t              flavor,
	mach_msg_type_number_t  *count);

#define pset_deallocate(x)
#define pset_reference(x)

extern void                             machine_run_count(
	uint32_t        count);

extern processor_t              machine_choose_processor(
	processor_set_t         pset,
	processor_t                     processor);

#define next_pset(p)    (((p)->pset_list != PROCESSOR_SET_NULL)? (p)->pset_list: (p)->node->psets)

#define PSET_THING_TASK         0
#define PSET_THING_THREAD       1

extern kern_return_t    processor_set_things(
	processor_set_t pset,
	void **thing_list,
	mach_msg_type_number_t *count,
	int type);

extern pset_cluster_type_t recommended_pset_type(thread_t thread);

inline static bool
pset_is_recommended(processor_set_t pset)
{
	return (pset->recommended_bitmask & pset->cpu_bitmask) != 0;
}

extern void processor_state_update_idle(processor_t processor);
extern void processor_state_update_from_thread(processor_t processor, thread_t thread);
extern void processor_state_update_explicit(processor_t processor, int pri,
    sfi_class_id_t sfi_class, pset_cluster_type_t pset_type,
    perfcontrol_class_t perfctl_class, thread_urgency_t urgency);

#define PSET_LOAD_NUMERATOR_SHIFT   16
#define PSET_LOAD_FRACTIONAL_SHIFT   4

inline static int
sched_get_pset_load_average(processor_set_t pset)
{
	return pset->load_average >> (PSET_LOAD_NUMERATOR_SHIFT - PSET_LOAD_FRACTIONAL_SHIFT);
}
extern void sched_update_pset_load_average(processor_set_t pset);

inline static void
pset_update_processor_state(processor_set_t pset, processor_t processor, uint new_state)
{
	pset_assert_locked(pset);

	uint old_state = processor->state;
	uint cpuid = processor->cpu_id;

	assert(processor->processor_set == pset);
	assert(bit_test(pset->cpu_bitmask, cpuid));

	assert(old_state < PROCESSOR_STATE_LEN);
	assert(new_state < PROCESSOR_STATE_LEN);

	processor->state = new_state;

	bit_clear(pset->cpu_state_map[old_state], cpuid);
	bit_set(pset->cpu_state_map[new_state], cpuid);

	if ((old_state == PROCESSOR_RUNNING) || (new_state == PROCESSOR_RUNNING)) {
		sched_update_pset_load_average(pset);
		if (new_state == PROCESSOR_RUNNING) {
			assert(processor == current_processor());
		}
	}
}

#else   /* MACH_KERNEL_PRIVATE */

__BEGIN_DECLS

extern void             pset_deallocate(
	processor_set_t pset);

extern void             pset_reference(
	processor_set_t pset);

__END_DECLS

#endif  /* MACH_KERNEL_PRIVATE */

#ifdef KERNEL_PRIVATE
__BEGIN_DECLS
extern unsigned int             processor_count;
extern processor_t      cpu_to_processor(int cpu);

extern kern_return_t    enable_smt_processors(bool enable);

extern boolean_t        processor_in_panic_context(processor_t processor);
__END_DECLS

#endif /* KERNEL_PRIVATE */

#endif  /* _KERN_PROCESSOR_H_ */
