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
 * @OSF_FREE_COPYRIGHT@
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
 *	File:	thread.h
 *	Author:	Avadis Tevanian, Jr.
 *
 *	This file contains the structure definitions for threads.
 *
 */
/*
 * Copyright (c) 1993 The University of Utah and
 * the Computer Systems Laboratory (CSL).  All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 *
 */

#ifndef _KERN_THREAD_H_
#define _KERN_THREAD_H_

#include <mach/kern_return.h>
#include <mach/mach_types.h>
#include <mach/message.h>
#include <mach/boolean.h>
#include <mach/vm_param.h>
#include <mach/thread_info.h>
#include <mach/thread_status.h>
#include <mach/exception_types.h>

#include <kern/kern_types.h>
#include <vm/vm_kern.h>

#include <sys/cdefs.h>

#ifdef  MACH_KERNEL_PRIVATE

#include <mach_assert.h>
#include <mach_ldebug.h>

#include <ipc/ipc_types.h>

#include <mach/port.h>
#include <kern/cpu_number.h>
#include <kern/smp.h>
#include <kern/queue.h>

#include <kern/timer.h>
#include <kern/simple_lock.h>
#include <kern/locks.h>
#include <kern/sched.h>
#include <kern/sched_prim.h>
#include <mach/sfi_class.h>
#include <kern/thread_call.h>
#include <kern/thread_group.h>
#include <kern/timer_call.h>
#include <kern/task.h>
#include <kern/exception.h>
#include <kern/affinity.h>
#include <kern/debug.h>
#include <kern/block_hint.h>
#include <kern/turnstile.h>
#include <kern/mpsc_queue.h>

#include <kern/waitq.h>
#include <san/kasan.h>
#include <os/refcnt.h>

#include <ipc/ipc_kmsg.h>

#include <machine/atomic.h>
#include <machine/cpu_data.h>
#include <machine/thread.h>

#ifdef XNU_KERNEL_PRIVATE
/* priority queue static asserts fail for __ARM64_ARCH_8_32__ kext builds */
#include <kern/priority_queue.h>
#endif /* XNU_KERNEL_PRIVATE */

#if MONOTONIC
#include <stdatomic.h>
#include <machine/monotonic.h>
#endif /* MONOTONIC */

#if CONFIG_EMBEDDED
/* Taskwatch related. TODO: find this a better home */
typedef struct task_watcher task_watch_t;
#endif /* CONFIG_EMBEDDED */

struct thread {
#if MACH_ASSERT
#define THREAD_MAGIC 0x1234ABCDDCBA4321ULL
	/* Ensure nothing uses &thread as a queue entry */
	uint64_t                thread_magic;
#endif /* MACH_ASSERT */

	/*
	 *	NOTE:	The runq field in the thread structure has an unusual
	 *	locking protocol.  If its value is PROCESSOR_NULL, then it is
	 *	locked by the thread_lock, but if its value is something else
	 *	then it is locked by the associated run queue lock. It is
	 *	set to PROCESSOR_NULL without holding the thread lock, but the
	 *	transition from PROCESSOR_NULL to non-null must be done
	 *	under the thread lock and the run queue lock.
	 *
	 *	New waitq APIs allow the 'links' and 'runq' fields to be
	 *	anywhere in the thread structure.
	 */
	union {
		queue_chain_t                   runq_links;             /* run queue links */
		queue_chain_t                   wait_links;             /* wait queue links */
		struct mpsc_queue_chain         mpsc_links;             /* thread daemon mpsc links */
		struct priority_queue_entry     wait_prioq_links;       /* priority ordered waitq links */
	};

	event64_t               wait_event;     /* wait queue event */
	processor_t             runq;           /* run queue assignment */
	struct waitq           *waitq;          /* wait queue this thread is enqueued on */
	struct turnstile       *turnstile;      /* thread's turnstile, protected by primitives interlock */
	void                   *inheritor;      /* inheritor of the primitive the thread will block on */
	struct priority_queue  sched_inheritor_queue; /* Inheritor queue for kernel promotion */
	struct priority_queue  base_inheritor_queue; /* Inheritor queue for user promotion */

#if CONFIG_SCHED_CLUTCH
	/*
	 * In the clutch scheduler, the threads are maintained in runqs at the clutch_bucket
	 * level (clutch_bucket defines a unique thread group and scheduling bucket pair). In
	 * order to determine the priority of the clutch bucket as a whole, it is necessary to
	 * find the highest thread in it. The thread could be present in the clutch bucket due
	 * to its base_pri or its promoted pri. This link is used to maintain that queue.
	 */
	struct priority_queue_entry sched_clutchpri_link;

#endif /* CONFIG_SCHED_CLUTCH */

	/* Data updated during assert_wait/thread_wakeup */
#if __SMP__
	decl_simple_lock_data(, sched_lock);     /* scheduling lock (thread_lock()) */
	decl_simple_lock_data(, wake_lock);      /* for thread stop / wait (wake_lock()) */
#endif
	uint16_t                options;                        /* options set by thread itself */
#define TH_OPT_INTMASK          0x0003          /* interrupt / abort level */
#define TH_OPT_VMPRIV           0x0004          /* may allocate reserved memory */
#define TH_OPT_SYSTEM_CRITICAL  0x0010          /* Thread must always be allowed to run - even under heavy load */
#define TH_OPT_PROC_CPULIMIT    0x0020          /* Thread has a task-wide CPU limit applied to it */
#define TH_OPT_PRVT_CPULIMIT    0x0040          /* Thread has a thread-private CPU limit applied to it */
#define TH_OPT_IDLE_THREAD      0x0080          /* Thread is a per-processor idle thread */
#define TH_OPT_GLOBAL_FORCED_IDLE       0x0100  /* Thread performs forced idle for thermal control */
#define TH_OPT_SCHED_VM_GROUP   0x0200          /* Thread belongs to special scheduler VM group */
#define TH_OPT_HONOR_QLIMIT     0x0400          /* Thread will honor qlimit while sending mach_msg, regardless of MACH_SEND_ALWAYS */
#define TH_OPT_SEND_IMPORTANCE  0x0800          /* Thread will allow importance donation from kernel rpc */
#define TH_OPT_ZONE_GC          0x1000          /* zone_gc() called on this thread */

	bool                            wake_active;    /* wake event on stop */
	bool                            at_safe_point;  /* thread_abort_safely allowed */
	ast_t                           reason;                 /* why we blocked */
	uint32_t                        quantum_remaining;
	wait_result_t                   wait_result;    /* outcome of wait -
	                                                * may be examined by this thread
	                                                * WITHOUT locking */
	thread_continue_t       continuation;   /* continue here next dispatch */
	void                            *parameter;             /* continuation parameter */

	/* Data updated/used in thread_invoke */
	vm_offset_t             kernel_stack;           /* current kernel stack */
	vm_offset_t                     reserved_stack;         /* reserved kernel stack */

#if KASAN
	struct kasan_thread_data kasan_data;
#endif

#if CONFIG_KSANCOV
	void *ksancov_data;
#endif

	/* Thread state: */
	int                                     state;
/*
 *	Thread states [bits or'ed]
 */
#define TH_WAIT                 0x01                    /* queued for waiting */
#define TH_SUSP                 0x02                    /* stopped or requested to stop */
#define TH_RUN                  0x04                    /* running or on runq */
#define TH_UNINT                0x08                    /* waiting uninteruptibly */
#define TH_TERMINATE    0x10                    /* halted at termination */
#define TH_TERMINATE2   0x20                    /* added to termination queue */
#define TH_WAIT_REPORT  0x40                    /* the wait is using the sched_call,
	                                         *                                  only set if TH_WAIT is also set */
#define TH_IDLE                 0x80                    /* idling processor */

	/* Scheduling information */
	sched_mode_t                    sched_mode;             /* scheduling mode */
	sched_mode_t                    saved_mode;             /* saved mode during forced mode demotion */

	/* This thread's contribution to global sched counters */
	sched_bucket_t                  th_sched_bucket;

	sfi_class_id_t                  sfi_class;              /* SFI class (XXX Updated on CSW/QE/AST) */
	sfi_class_id_t                  sfi_wait_class; /* Currently in SFI wait for this class, protected by sfi_lock */


	uint32_t                        sched_flags;            /* current flag bits */
#define TH_SFLAG_NO_SMT                 0x0001          /* On an SMT CPU, this thread must be scheduled alone */
#define TH_SFLAG_FAILSAFE               0x0002          /* fail-safe has tripped */
#define TH_SFLAG_THROTTLED              0x0004          /* throttled thread forced to timeshare mode (may be applied in addition to failsafe) */
#define TH_SFLAG_DEMOTED_MASK      (TH_SFLAG_THROTTLED | TH_SFLAG_FAILSAFE)     /* saved_mode contains previous sched_mode */

#define TH_SFLAG_PROMOTED               0x0008          /* sched pri has been promoted by kernel mutex priority promotion */
#define TH_SFLAG_ABORT                  0x0010          /* abort interruptible waits */
#define TH_SFLAG_ABORTSAFELY            0x0020          /* ... but only those at safe point */
#define TH_SFLAG_ABORTED_MASK           (TH_SFLAG_ABORT | TH_SFLAG_ABORTSAFELY)
#define TH_SFLAG_DEPRESS                0x0040          /* normal depress yield */
#define TH_SFLAG_POLLDEPRESS            0x0080          /* polled depress yield */
#define TH_SFLAG_DEPRESSED_MASK         (TH_SFLAG_DEPRESS | TH_SFLAG_POLLDEPRESS)
/* unused TH_SFLAG_PRI_UPDATE           0x0100 */
#define TH_SFLAG_EAGERPREEMPT           0x0200          /* Any preemption of this thread should be treated as if AST_URGENT applied */
#define TH_SFLAG_RW_PROMOTED            0x0400          /* promote reason: blocking with RW lock held */
#define TH_SFLAG_BASE_PRI_FROZEN        0x0800          /* (effective) base_pri is frozen */
#define TH_SFLAG_WAITQ_PROMOTED         0x1000          /* promote reason: waitq wakeup (generally for IPC receive) */


#define TH_SFLAG_EXEC_PROMOTED          0x8000          /* promote reason: thread is in an exec */

/* 'promote reasons' that request a priority floor only, not a custom priority */
#define TH_SFLAG_PROMOTE_REASON_MASK    (TH_SFLAG_RW_PROMOTED | TH_SFLAG_WAITQ_PROMOTED | TH_SFLAG_EXEC_PROMOTED)

#define TH_SFLAG_RW_PROMOTED_BIT        (10)    /* 0x400 */

	int16_t                         sched_pri;              /* scheduled (current) priority */
	int16_t                         base_pri;               /* effective base priority (equal to req_base_pri unless TH_SFLAG_BASE_PRI_FROZEN) */
	int16_t                         req_base_pri;           /* requested base priority */
	int16_t                         max_priority;           /* copy of max base priority */
	int16_t                         task_priority;          /* copy of task base priority */
	int16_t                         promotion_priority;     /* priority thread is currently promoted to */

#if defined(CONFIG_SCHED_GRRR)
#if 0
	uint16_t                        grrr_deficit;           /* fixed point (1/1000th quantum) fractional deficit */
#endif
#endif

	int                             iotier_override; /* atomic operations to set, cleared on ret to user */
	os_refcnt_t                     ref_count;              /* number of references to me */

	lck_mtx_t*                      waiting_for_mutex;      /* points to mutex we're waiting for until we acquire it */

	uint32_t                        rwlock_count;   /* Number of lck_rw_t locks held by thread */

	integer_t                       importance;                     /* task-relative importance */

	/* Priority depression expiration */
	integer_t                       depress_timer_active;
	timer_call_data_t       depress_timer;
	/* real-time parameters */
	struct {                                                                /* see mach/thread_policy.h */
		uint32_t                        period;
		uint32_t                        computation;
		uint32_t                        constraint;
		boolean_t                       preemptible;
		uint64_t                        deadline;
	}                                       realtime;

	uint64_t                        last_run_time;          /* time when thread was switched away from */
	uint64_t                        last_made_runnable_time;        /* time when thread was unblocked or preempted */
	uint64_t                        last_basepri_change_time;       /* time when thread was last changed in basepri while runnable */
	uint64_t                        same_pri_latency;
#define THREAD_NOT_RUNNABLE (~0ULL)


#if defined(CONFIG_SCHED_MULTIQ)
	sched_group_t                   sched_group;
#endif /* defined(CONFIG_SCHED_MULTIQ) */

	/* Data used during setrun/dispatch */
	timer_data_t            system_timer;           /* system mode timer */
	processor_t                     bound_processor;        /* bound to a processor? */
	processor_t                     last_processor;         /* processor last dispatched on */
	processor_t                     chosen_processor;       /* Where we want to run this thread */

	/* Fail-safe computation since last unblock or qualifying yield */
	uint64_t                        computation_metered;
	uint64_t                        computation_epoch;
	uint64_t                        safe_release;   /* when to release fail-safe */

	/* Call out from scheduler */
	void                            (*sched_call)(int type, thread_t thread);

#if defined(CONFIG_SCHED_PROTO)
	uint32_t                        runqueue_generation;    /* last time runqueue was drained */
#endif

	/* Statistics and timesharing calculations */
#if defined(CONFIG_SCHED_TIMESHARE_CORE)
	natural_t                       sched_stamp;    /* last scheduler tick */
	natural_t                       sched_usage;    /* timesharing cpu usage [sched] */
	natural_t                       pri_shift;              /* usage -> priority from pset */
	natural_t                       cpu_usage;              /* instrumented cpu usage [%cpu] */
	natural_t                       cpu_delta;              /* accumulated cpu_usage delta */
#endif /* CONFIG_SCHED_TIMESHARE_CORE */

	uint32_t                        c_switch;               /* total context switches */
	uint32_t                        p_switch;               /* total processor switches */
	uint32_t                        ps_switch;              /* total pset switches */

	integer_t mutex_count;  /* total count of locks held */
	/* Timing data structures */
	int                                     precise_user_kernel_time; /* precise user/kernel enabled for this thread */
	timer_data_t            user_timer;                     /* user mode timer */
	uint64_t                        user_timer_save;        /* saved user timer value */
	uint64_t                        system_timer_save;      /* saved system timer value */
	uint64_t                        vtimer_user_save;       /* saved values for vtimers */
	uint64_t                        vtimer_prof_save;
	uint64_t                        vtimer_rlim_save;
	uint64_t                        vtimer_qos_save;

	timer_data_t            ptime;                  /* time executing in P mode */
	timer_data_t            runnable_timer;         /* time the thread is runnable (including running) */

#if CONFIG_SCHED_SFI
	/* Timing for wait state */
	uint64_t                wait_sfi_begin_time;    /* start time for thread waiting in SFI */
#endif

	/*
	 * Processor/cache affinity
	 * - affinity_threads links task threads with the same affinity set
	 */
	queue_chain_t                   affinity_threads;
	affinity_set_t                  affinity_set;

#if CONFIG_EMBEDDED
	task_watch_t *  taskwatch;              /* task watch */
#endif /* CONFIG_EMBEDDED */

	/* Various bits of state to stash across a continuation, exclusive to the current thread block point */
	union {
		struct {
			mach_msg_return_t       state;          /* receive state */
			mach_port_seqno_t       seqno;          /* seqno of recvd message */
			ipc_object_t            object;         /* object received on */
			vm_address_t            msg_addr;       /* receive buffer pointer */
			mach_msg_size_t         rsize;          /* max size for recvd msg */
			mach_msg_size_t         msize;          /* actual size for recvd msg */
			mach_msg_option_t       option;         /* options for receive */
			mach_port_name_t        receiver_name;  /* the receive port name */
			struct knote            *knote;         /* knote fired for rcv */
			union {
				struct ipc_kmsg   *kmsg;        /* received message */
				struct ipc_mqueue *peekq;       /* mqueue to peek at */
				struct {
					mach_msg_priority_t qos;        /* received message qos */
					mach_msg_priority_t oqos;       /* override qos for message */
				} received_qos;
			};
			mach_msg_continue_t     continuation;
		} receive;
		struct {
			struct semaphore        *waitsemaphore;         /* semaphore ref */
			struct semaphore        *signalsemaphore;       /* semaphore ref */
			int                                     options;                        /* semaphore options */
			kern_return_t           result;                         /* primary result */
			mach_msg_continue_t continuation;
		} sema;
		struct {
#define THREAD_SAVE_IOKIT_TLS_COUNT     8
			void                    *tls[THREAD_SAVE_IOKIT_TLS_COUNT];
		} iokit;
	} saved;

	/* Only user threads can cause guard exceptions, only kernel threads can be thread call threads */
	union {
		/* Group and call this thread is working on behalf of */
		struct {
			struct thread_call_group * thc_group;
			struct thread_call *       thc_call;                    /* debug only, may be deallocated */
		} thc_state;

		/* Structure to save information about guard exception */
		struct {
			mach_exception_code_t           code;
			mach_exception_subcode_t        subcode;
		} guard_exc_info;
	};

	/* Kernel holds on this thread  */
	int16_t                                         suspend_count;
	/* User level suspensions */
	int16_t                                         user_stop_count;

	/* IPC data structures */
#if IMPORTANCE_INHERITANCE
	natural_t ith_assertions;                       /* assertions pending drop */
#endif
	struct ipc_kmsg_queue ith_messages;             /* messages to reap */
	mach_port_t ith_rpc_reply;                      /* reply port for kernel RPCs */

	/* Pending thread ast(s) */
	ast_t                                   ast;

	/* Ast/Halt data structures */
	vm_offset_t                             recover;                /* page fault recover(copyin/out) */

	queue_chain_t                           threads;                /* global list of all threads */

	/* Activation */
	queue_chain_t                           task_threads;

	/* Task membership */
	struct task                             *task;
	vm_map_t                                map;
#if DEVELOPMENT || DEBUG
	bool      pmap_footprint_suspended;
#endif /* DEVELOPMENT || DEBUG */

	/* Timed wait expiration */
	timer_call_data_t       wait_timer;
	uint16_t                wait_timer_active;
	bool                    wait_timer_is_set;

	/* Miscellaneous bits guarded by mutex */
	uint32_t
	    active:1,                                   /* Thread is active and has not been terminated */
	    started:1,                                  /* Thread has been started after creation */
	    static_param:1,                             /* Disallow policy parameter changes */
	    inspection:1,                               /* TRUE when task is being inspected by crash reporter */
	    policy_reset:1,                             /* Disallow policy parameter changes on terminating threads */
	    suspend_parked:1,                           /* thread parked in thread_suspended */
	    corpse_dup:1,                               /* TRUE when thread is an inactive duplicate in a corpse */
	:0;

	decl_lck_mtx_data(, mutex);

	/* Ports associated with this thread */
	struct ipc_port                 *ith_self;                      /* not a right, doesn't hold ref */
	struct ipc_port                 *ith_sself;                     /* a send right */
	struct ipc_port                 *ith_special_reply_port;         /* ref to special reply port */
	struct exception_action *exc_actions;

#ifdef  MACH_BSD
	void                                    *uthread;
#endif

#if CONFIG_DTRACE
	uint16_t t_dtrace_flags;                /* DTrace thread states */
#define TH_DTRACE_EXECSUCCESS   0x01
	uint16_t t_dtrace_inprobe;          /* Executing under dtrace_probe */
	uint32_t t_dtrace_predcache;        /* DTrace per thread predicate value hint */
	int64_t t_dtrace_tracing;               /* Thread time under dtrace_probe() */
	int64_t t_dtrace_vtime;
#endif

	clock_sec_t t_page_creation_time;
	uint32_t    t_page_creation_count;
	uint32_t    t_page_creation_throttled;
#if (DEVELOPMENT || DEBUG)
	uint64_t    t_page_creation_throttled_hard;
	uint64_t    t_page_creation_throttled_soft;
#endif /* DEVELOPMENT || DEBUG */
	int         t_pagein_error;            /* for vm_fault(), holds error from vnop_pagein() */

#ifdef KPERF
/* The high 8 bits are the number of frames to sample of a user callstack. */
#define T_KPERF_CALLSTACK_DEPTH_OFFSET     (24)
#define T_KPERF_SET_CALLSTACK_DEPTH(DEPTH) (((uint32_t)(DEPTH)) << T_KPERF_CALLSTACK_DEPTH_OFFSET)
#define T_KPERF_GET_CALLSTACK_DEPTH(FLAGS) ((FLAGS) >> T_KPERF_CALLSTACK_DEPTH_OFFSET)
#define T_KPERF_ACTIONID_OFFSET            (18)
#define T_KPERF_SET_ACTIONID(AID)          (((uint32_t)(AID)) << T_KPERF_ACTIONID_OFFSET)
#define T_KPERF_GET_ACTIONID(FLAGS)        ((FLAGS) >> T_KPERF_ACTIONID_OFFSET)
#endif

#define T_KPERF_AST_CALLSTACK 0x1 /* dump a callstack on thread's next AST */
#define T_KPERF_AST_DISPATCH  0x2 /* dump a name on thread's next AST */
#define T_KPC_ALLOC           0x4 /* thread needs a kpc_buf allocated */

#define T_KPERF_AST_ALL \
    (T_KPERF_AST_CALLSTACK | T_KPERF_AST_DISPATCH | T_KPC_ALLOC)
/* only go up to T_KPERF_ACTIONID_OFFSET - 1 */

#ifdef KPERF
	uint32_t kperf_ast;
	uint32_t kperf_pet_gen;  /* last generation of PET that sampled this thread*/
	uint32_t kperf_c_switch; /* last dispatch detection */
	uint32_t kperf_pet_cnt;  /* how many times a thread has been sampled by PET */
#endif

#ifdef KPC
	/* accumulated performance counters for this thread */
	uint64_t *kpc_buf;
#endif

#if HYPERVISOR
	/* hypervisor virtual CPU object associated with this thread */
	void *hv_thread_target;
#endif /* HYPERVISOR */

	/* Statistics accumulated per-thread and aggregated per-task */
	uint32_t                syscalls_unix;
	uint32_t                syscalls_mach;
	ledger_t                t_ledger;
	ledger_t                t_threadledger; /* per thread ledger */
	ledger_t                t_bankledger;                /* ledger to charge someone */
	uint64_t                t_deduct_bank_ledger_time;   /* cpu time to be deducted from bank ledger */
	uint64_t                t_deduct_bank_ledger_energy; /* energy to be deducted from bank ledger */

	uint64_t thread_id;             /*system wide unique thread-id*/

#if MONOTONIC
	struct mt_thread t_monotonic;
#endif /* MONOTONIC */

	/*** Machine-dependent state ***/
	struct machine_thread   machine;

	/* policy is protected by the thread mutex */
	struct thread_requested_policy  requested_policy;
	struct thread_effective_policy  effective_policy;

	/* usynch override is protected by the task lock, eventually will be thread mutex */
	struct thread_qos_override {
		struct thread_qos_override      *override_next;
		uint32_t        override_contended_resource_count;
		int16_t         override_qos;
		int16_t         override_resource_type;
		user_addr_t     override_resource;
	} *overrides;

	uint32_t        kevent_overrides;
	uint8_t         user_promotion_basepri;
	uint8_t         kern_promotion_schedpri;
	_Atomic uint16_t kevent_ast_bits;

	io_stat_info_t                  thread_io_stats; /* per-thread I/O statistics */

	uint32_t                        thread_callout_interrupt_wakeups;
	uint32_t                        thread_callout_platform_idle_wakeups;
	uint32_t                        thread_timer_wakeups_bin_1;
	uint32_t                        thread_timer_wakeups_bin_2;
	uint16_t                        thread_tag;
	/*
	 * callout_* fields are only set for thread call threads whereas guard_exc_fatal is set
	 * by user threads on themselves while taking a guard exception. So it's okay for them to
	 * share this bitfield.
	 */
	uint16_t                        callout_woken_from_icontext:1,
	    callout_woken_from_platform_idle:1,
	    callout_woke_thread:1,
	    guard_exc_fatal:1,
	    thread_bitfield_unused:12;

	mach_port_name_t                ith_voucher_name;
	ipc_voucher_t                   ith_voucher;
#if CONFIG_IOSCHED
	void                            *decmp_upl;
#endif /* CONFIG_IOSCHED */

	/* work interval (if any) associated with the thread. Uses thread mutex */
	struct work_interval            *th_work_interval;

#if     SCHED_TRACE_THREAD_WAKEUPS
	uintptr_t               thread_wakeup_bt[64];
#endif
	turnstile_update_flags_t inheritor_flags; /* inheritor flags for inheritor field */
	block_hint_t    pending_block_hint;
	block_hint_t    block_hint;      /* What type of primitive last caused us to block. */
	integer_t       decompressions;  /* Per-thread decompressions counter to be added to per-task decompressions counter */
};

#define ith_state           saved.receive.state
#define ith_object          saved.receive.object
#define ith_msg_addr        saved.receive.msg_addr
#define ith_rsize           saved.receive.rsize
#define ith_msize           saved.receive.msize
#define ith_option          saved.receive.option
#define ith_receiver_name   saved.receive.receiver_name
#define ith_continuation    saved.receive.continuation
#define ith_kmsg            saved.receive.kmsg
#define ith_peekq           saved.receive.peekq
#define ith_knote           saved.receive.knote
#define ith_qos             saved.receive.received_qos.qos
#define ith_qos_override    saved.receive.received_qos.oqos
#define ith_seqno           saved.receive.seqno

#define sth_waitsemaphore   saved.sema.waitsemaphore
#define sth_signalsemaphore saved.sema.signalsemaphore
#define sth_options         saved.sema.options
#define sth_result          saved.sema.result
#define sth_continuation    saved.sema.continuation

#define ITH_KNOTE_NULL      ((void *)NULL)
#define ITH_KNOTE_PSEUDO    ((void *)0xdeadbeef)
/*
 * The ith_knote is used during message delivery, and can safely be interpreted
 * only when used for one of these codepaths, which the test for the msgt_name
 * being RECEIVE or SEND_ONCE is about.
 */
#define ITH_KNOTE_VALID(kn, msgt_name) \
	        (((kn) != ITH_KNOTE_NULL && (kn) != ITH_KNOTE_PSEUDO) && \
	         ((msgt_name) == MACH_MSG_TYPE_PORT_RECEIVE || \
	         (msgt_name) == MACH_MSG_TYPE_PORT_SEND_ONCE))

#if MACH_ASSERT
#define assert_thread_magic(thread) assertf((thread)->thread_magic == THREAD_MAGIC, \
	                                    "bad thread magic 0x%llx for thread %p, expected 0x%llx", \
	                                    (thread)->thread_magic, (thread), THREAD_MAGIC)
#else
#define assert_thread_magic(thread) do { (void)(thread); } while (0)
#endif

extern void                     thread_bootstrap(void);

extern void                     thread_init(void);

extern void                     thread_daemon_init(void);

#define thread_reference_internal(thread)       \
	                os_ref_retain(&(thread)->ref_count);

#define thread_reference(thread)                                        \
MACRO_BEGIN                                                                                     \
	if ((thread) != THREAD_NULL)                                    \
	        thread_reference_internal(thread);              \
MACRO_END

extern void                     thread_deallocate(
	thread_t                thread);

extern void                     thread_inspect_deallocate(
	thread_inspect_t        thread);

extern void                     thread_terminate_self(void);

extern kern_return_t    thread_terminate_internal(
	thread_t                thread);

extern void                     thread_start(
	thread_t                        thread) __attribute__ ((noinline));

extern void                     thread_start_in_assert_wait(
	thread_t                        thread,
	event_t             event,
	wait_interrupt_t    interruptible) __attribute__ ((noinline));

extern void                     thread_terminate_enqueue(
	thread_t                thread);

extern void                     thread_exception_enqueue(
	task_t          task,
	thread_t        thread,
	exception_type_t etype);

extern void                     thread_copy_resource_info(
	thread_t dst_thread,
	thread_t src_thread);

extern void                     thread_terminate_crashed_threads(void);

extern void                     thread_stack_enqueue(
	thread_t                thread);

extern void                     thread_hold(
	thread_t        thread);

extern void                     thread_release(
	thread_t        thread);

extern void                     thread_corpse_continue(void) __dead2;

extern boolean_t                thread_is_active(thread_t thread);

extern lck_grp_t                thread_lck_grp;

/* Locking for scheduler state, always acquired with interrupts disabled (splsched()) */
#if __SMP__
#define thread_lock_init(th)    simple_lock_init(&(th)->sched_lock, 0)
#define thread_lock(th)                 simple_lock(&(th)->sched_lock, &thread_lck_grp)
#define thread_unlock(th)               simple_unlock(&(th)->sched_lock)

#define wake_lock_init(th)              simple_lock_init(&(th)->wake_lock, 0)
#define wake_lock(th)                   simple_lock(&(th)->wake_lock, &thread_lck_grp)
#define wake_unlock(th)                 simple_unlock(&(th)->wake_lock)
#else
#define thread_lock_init(th)    do { (void)th; } while(0)
#define thread_lock(th)                 do { (void)th; } while(0)
#define thread_unlock(th)               do { (void)th; } while(0)

#define wake_lock_init(th)              do { (void)th; } while(0)
#define wake_lock(th)                   do { (void)th; } while(0)
#define wake_unlock(th)                 do { (void)th; } while(0)
#endif

#define thread_should_halt_fast(thread)         (!(thread)->active)

extern void                             stack_alloc(
	thread_t                thread);

extern void                     stack_handoff(
	thread_t                from,
	thread_t                to);

extern void                             stack_free(
	thread_t                thread);

extern void                             stack_free_reserved(
	thread_t                thread);

extern boolean_t                stack_alloc_try(
	thread_t            thread);

extern void                             stack_collect(void);

extern void                             stack_init(void);


extern kern_return_t    thread_info_internal(
	thread_t                                thread,
	thread_flavor_t                 flavor,
	thread_info_t                   thread_info_out,
	mach_msg_type_number_t  *thread_info_count);



extern kern_return_t    kernel_thread_create(
	thread_continue_t       continuation,
	void                            *parameter,
	integer_t                       priority,
	thread_t                        *new_thread);

extern kern_return_t    kernel_thread_start_priority(
	thread_continue_t       continuation,
	void                            *parameter,
	integer_t                       priority,
	thread_t                        *new_thread);

extern void                             machine_stack_attach(
	thread_t                thread,
	vm_offset_t             stack);

extern vm_offset_t              machine_stack_detach(
	thread_t                thread);

extern void                             machine_stack_handoff(
	thread_t                old,
	thread_t                new);

extern thread_t                 machine_switch_context(
	thread_t                        old_thread,
	thread_continue_t       continuation,
	thread_t                        new_thread);

extern void                             machine_load_context(
	thread_t                thread) __attribute__((noreturn));

extern kern_return_t    machine_thread_state_initialize(
	thread_t                                thread);

extern kern_return_t    machine_thread_set_state(
	thread_t                                thread,
	thread_flavor_t                 flavor,
	thread_state_t                  state,
	mach_msg_type_number_t  count);

extern mach_vm_address_t machine_thread_pc(
	thread_t                thread);

extern void machine_thread_reset_pc(
	thread_t                thread,
	mach_vm_address_t       pc);

extern boolean_t        machine_thread_on_core(
	thread_t                thread);

extern kern_return_t    machine_thread_get_state(
	thread_t                                thread,
	thread_flavor_t                 flavor,
	thread_state_t                  state,
	mach_msg_type_number_t  *count);

extern kern_return_t    machine_thread_state_convert_from_user(
	thread_t                                thread,
	thread_flavor_t                 flavor,
	thread_state_t                  tstate,
	mach_msg_type_number_t  count);

extern kern_return_t    machine_thread_state_convert_to_user(
	thread_t                                thread,
	thread_flavor_t                 flavor,
	thread_state_t                  tstate,
	mach_msg_type_number_t  *count);

extern kern_return_t    machine_thread_dup(
	thread_t                self,
	thread_t                target,
	boolean_t               is_corpse);

extern void                             machine_thread_init(void);

extern kern_return_t    machine_thread_create(
	thread_t                thread,
	task_t                  task);
extern void             machine_thread_switch_addrmode(
	thread_t                 thread);

extern void                 machine_thread_destroy(
	thread_t                thread);

extern void                             machine_set_current_thread(
	thread_t                        thread);

extern kern_return_t    machine_thread_get_kern_state(
	thread_t                                thread,
	thread_flavor_t                 flavor,
	thread_state_t                  tstate,
	mach_msg_type_number_t  *count);

extern kern_return_t    machine_thread_inherit_taskwide(
	thread_t                thread,
	task_t                  parent_task);

extern kern_return_t    machine_thread_set_tsd_base(
	thread_t                                thread,
	mach_vm_offset_t                tsd_base);

#define thread_mtx_lock(thread)                 lck_mtx_lock(&(thread)->mutex)
#define thread_mtx_try(thread)                  lck_mtx_try_lock(&(thread)->mutex)
#define thread_mtx_unlock(thread)               lck_mtx_unlock(&(thread)->mutex)

extern void thread_apc_ast(thread_t thread);

extern void thread_update_qos_cpu_time(thread_t thread);

void act_machine_sv_free(thread_t, int);

vm_offset_t                     min_valid_stack_address(void);
vm_offset_t                     max_valid_stack_address(void);

static inline uint16_t
thread_set_tag_internal(thread_t        thread, uint16_t tag)
{
	return os_atomic_or_orig(&thread->thread_tag, tag, relaxed);
}

static inline uint16_t
thread_get_tag_internal(thread_t        thread)
{
	return thread->thread_tag;
}

extern bool thread_no_smt(thread_t thread);
extern bool processor_active_thread_no_smt(processor_t processor);

extern void thread_set_options(uint32_t thopt);


#else   /* MACH_KERNEL_PRIVATE */

__BEGIN_DECLS

extern void thread_mtx_lock(thread_t thread);

extern void thread_mtx_unlock(thread_t thread);

extern thread_t         current_thread(void) __attribute__((const));

extern void                     thread_reference(
	thread_t        thread);

extern void                     thread_deallocate(
	thread_t        thread);

#if BSD_KERNEL_PRIVATE
/* Duplicated from osfmk/kern/ipc_tt.h */
__options_decl(port_to_thread_options_t, uint32_t, {
	PORT_TO_THREAD_NONE               = 0x0000,
	PORT_TO_THREAD_IN_CURRENT_TASK    = 0x0001,
	PORT_TO_THREAD_NOT_CURRENT_THREAD = 0x0002,
});

extern thread_t port_name_to_thread(
	mach_port_name_t            port_name,
	port_to_thread_options_t    options);
#endif /* BSD_KERNEL_PRIVATE */

__END_DECLS

#endif  /* MACH_KERNEL_PRIVATE */

#ifdef  KERNEL_PRIVATE

__BEGIN_DECLS

extern void                     thread_deallocate_safe(
	thread_t                thread);

extern uint64_t                 thread_dispatchqaddr(
	thread_t thread);

extern uint64_t                 thread_rettokern_addr(
	thread_t thread);

extern integer_t        thread_kern_get_pri(thread_t thr) __attribute__((const));

extern void             thread_kern_set_pri(thread_t thr, integer_t pri);

extern integer_t        thread_kern_get_kernel_maxpri(void) __attribute__((const));

__END_DECLS

#endif  /* KERNEL_PRIVATE */

#ifdef KERNEL
__BEGIN_DECLS

extern uint64_t                 thread_tid(thread_t thread);

__END_DECLS

#endif  /* KERNEL */

__BEGIN_DECLS

#ifdef  XNU_KERNEL_PRIVATE

/*
 * Thread tags; for easy identification.
 */
#define THREAD_TAG_MAINTHREAD 0x1
#define THREAD_TAG_CALLOUT 0x2
#define THREAD_TAG_IOWORKLOOP 0x4

#define THREAD_TAG_PTHREAD 0x10
#define THREAD_TAG_WORKQUEUE 0x20

uint16_t        thread_set_tag(thread_t, uint16_t);
uint16_t        thread_get_tag(thread_t);
uint64_t        thread_last_run_time(thread_t);

extern kern_return_t    thread_state_initialize(
	thread_t                                thread);

extern kern_return_t    thread_setstatus(
	thread_t                                thread,
	int                                             flavor,
	thread_state_t                  tstate,
	mach_msg_type_number_t  count);

extern kern_return_t    thread_setstatus_from_user(
	thread_t                                thread,
	int                                             flavor,
	thread_state_t                  tstate,
	mach_msg_type_number_t  count);

extern kern_return_t    thread_getstatus(
	thread_t                                thread,
	int                                             flavor,
	thread_state_t                  tstate,
	mach_msg_type_number_t  *count);

extern kern_return_t    thread_getstatus_to_user(
	thread_t                                thread,
	int                                             flavor,
	thread_state_t                  tstate,
	mach_msg_type_number_t  *count);

extern kern_return_t    thread_create_with_continuation(
	task_t task,
	thread_t *new_thread,
	thread_continue_t continuation);

extern kern_return_t thread_create_waiting(task_t               task,
    thread_continue_t    continuation,
    event_t              event,
    thread_t             *new_thread);

extern kern_return_t    thread_create_workq_waiting(
	task_t                  task,
	thread_continue_t       thread_return,
	thread_t                *new_thread);

extern  void    thread_yield_internal(
	mach_msg_timeout_t      interval);

extern void             thread_yield_to_preemption(void);

/*
 * Thread-private CPU limits: apply a private CPU limit to this thread only. Available actions are:
 *
 * 1) Block. Prevent CPU consumption of the thread from exceeding the limit.
 * 2) Exception. Generate a resource consumption exception when the limit is exceeded.
 * 3) Disable. Remove any existing CPU limit.
 */
#define THREAD_CPULIMIT_BLOCK           0x1
#define THREAD_CPULIMIT_EXCEPTION       0x2
#define THREAD_CPULIMIT_DISABLE         0x3

struct _thread_ledger_indices {
	int cpu_time;
};

extern struct _thread_ledger_indices thread_ledgers;

extern int thread_get_cpulimit(int *action, uint8_t *percentage, uint64_t *interval_ns);
extern int thread_set_cpulimit(int action, uint8_t percentage, uint64_t interval_ns);

extern void                     thread_read_times(
	thread_t                thread,
	time_value_t    *user_time,
	time_value_t    *system_time,
	time_value_t    *runnable_time);

extern uint64_t         thread_get_runtime_self(void);

extern void                     thread_setuserstack(
	thread_t                thread,
	mach_vm_offset_t        user_stack);

extern uint64_t         thread_adjuserstack(
	thread_t                thread,
	int                             adjust);

extern void                     thread_setentrypoint(
	thread_t                thread,
	mach_vm_offset_t        entry);

extern kern_return_t    thread_set_tsd_base(
	thread_t        thread,
	mach_vm_offset_t tsd_base);

extern kern_return_t    thread_setsinglestep(
	thread_t                thread,
	int                     on);

extern kern_return_t    thread_userstack(
	thread_t,
	int,
	thread_state_t,
	unsigned int,
	mach_vm_offset_t *,
	int *,
	boolean_t);

extern kern_return_t    thread_entrypoint(
	thread_t,
	int,
	thread_state_t,
	unsigned int,
	mach_vm_offset_t *);

extern kern_return_t    thread_userstackdefault(
	mach_vm_offset_t *,
	boolean_t);

extern kern_return_t    thread_wire_internal(
	host_priv_t             host_priv,
	thread_t                thread,
	boolean_t               wired,
	boolean_t               *prev_state);


extern kern_return_t    thread_dup(thread_t);

extern kern_return_t thread_dup2(thread_t, thread_t);

#if !defined(_SCHED_CALL_T_DEFINED)
#define _SCHED_CALL_T_DEFINED
typedef void    (*sched_call_t)(
	int                             type,
	thread_t                thread);
#endif

#define SCHED_CALL_BLOCK                0x1
#define SCHED_CALL_UNBLOCK              0x2

extern void             thread_sched_call(
	thread_t                thread,
	sched_call_t    call);

extern boolean_t        thread_is_static_param(
	thread_t                thread);

extern task_t   get_threadtask(thread_t);

/*
 * Thread is running within a 64-bit address space.
 */
#define thread_is_64bit_addr(thd)       \
	task_has_64Bit_addr(get_threadtask(thd))

/*
 * Thread is using 64-bit machine state.
 */
#define thread_is_64bit_data(thd)       \
	task_has_64Bit_data(get_threadtask(thd))

#if defined(__x86_64__)
extern int              thread_task_has_ldt(thread_t);
#endif
extern void             *get_bsdthread_info(thread_t);
extern void             set_bsdthread_info(thread_t, void *);
extern void             set_thread_pagein_error(thread_t, int);
extern void             *uthread_alloc(task_t, thread_t, int);
extern event_t  workq_thread_init_and_wq_lock(task_t, thread_t); // bsd/pthread/
extern void             uthread_cleanup_name(void *uthread);
extern void             uthread_cleanup(task_t, void *, void *);
extern void             uthread_zone_free(void *);
extern void             uthread_cred_free(void *);

extern void             uthread_reset_proc_refcount(void *);
#if PROC_REF_DEBUG
extern int              uthread_get_proc_refcount(void *);
extern int              proc_ref_tracking_disabled;
#endif

extern boolean_t        thread_should_halt(
	thread_t                thread);

extern boolean_t        thread_should_abort(
	thread_t);

extern int is_64signalregset(void);

extern void act_set_kperf(thread_t);
extern void act_set_astledger(thread_t thread);
extern void act_set_astledger_async(thread_t thread);
extern void act_set_io_telemetry_ast(thread_t);

extern uint32_t dtrace_get_thread_predcache(thread_t);
extern int64_t dtrace_get_thread_vtime(thread_t);
extern int64_t dtrace_get_thread_tracing(thread_t);
extern uint16_t dtrace_get_thread_inprobe(thread_t);
extern int dtrace_get_thread_last_cpu_id(thread_t);
extern vm_offset_t dtrace_get_kernel_stack(thread_t);
extern void dtrace_set_thread_predcache(thread_t, uint32_t);
extern void dtrace_set_thread_vtime(thread_t, int64_t);
extern void dtrace_set_thread_tracing(thread_t, int64_t);
extern void dtrace_set_thread_inprobe(thread_t, uint16_t);
extern vm_offset_t dtrace_set_thread_recover(thread_t, vm_offset_t);
extern vm_offset_t dtrace_sign_and_set_thread_recover(thread_t, vm_offset_t);
extern void dtrace_thread_bootstrap(void);
extern void dtrace_thread_didexec(thread_t);

extern int64_t dtrace_calc_thread_recent_vtime(thread_t);


extern kern_return_t    thread_set_wq_state32(
	thread_t          thread,
	thread_state_t    tstate);

extern kern_return_t    thread_set_wq_state64(
	thread_t          thread,
	thread_state_t    tstate);

extern vm_offset_t      kernel_stack_mask;
extern vm_offset_t      kernel_stack_size;
extern vm_offset_t      kernel_stack_depth_max;

extern void guard_ast(thread_t);
extern void fd_guard_ast(thread_t,
    mach_exception_code_t, mach_exception_subcode_t);
#if CONFIG_VNGUARD
extern void vn_guard_ast(thread_t,
    mach_exception_code_t, mach_exception_subcode_t);
#endif
extern void mach_port_guard_ast(thread_t,
    mach_exception_code_t, mach_exception_subcode_t);
extern void virt_memory_guard_ast(thread_t,
    mach_exception_code_t, mach_exception_subcode_t);
extern void thread_guard_violation(thread_t,
    mach_exception_code_t, mach_exception_subcode_t, boolean_t);
extern void thread_update_io_stats(thread_t, int size, int io_flags);

extern kern_return_t    thread_set_voucher_name(mach_port_name_t name);
extern kern_return_t thread_get_current_voucher_origin_pid(int32_t *pid);

extern void set_thread_rwlock_boost(void);
extern void clear_thread_rwlock_boost(void);

extern void thread_enable_send_importance(thread_t thread, boolean_t enable);

/*
 * Translate signal context data pointer to userspace representation
 */

extern kern_return_t    machine_thread_siguctx_pointer_convert_to_user(
	thread_t thread,
	user_addr_t *uctxp);

extern void machine_tecs(thread_t thr);

typedef enum cpuvn {
	CPUVN_CI = 1
} cpuvn_e;

extern int machine_csv(cpuvn_e cve);

/*
 * Translate array of function pointer syscall arguments from userspace representation
 */

extern kern_return_t    machine_thread_function_pointers_convert_from_user(
	thread_t thread,
	user_addr_t *fptrs,
	uint32_t count);

/* Get a backtrace for a threads kernel or user stack (user_p), with pc and optionally
 * frame pointer (getfp). Returns bytes added to buffer, and kThreadTruncatedBT in
 * thread_trace_flags if a user page is not present after kdp_lightweight_fault() is
 * called.
 */

extern int                              machine_trace_thread(
	thread_t thread,
	char *tracepos,
	char *tracebound,
	int nframes,
	boolean_t user_p,
	boolean_t getfp,
	uint32_t *thread_trace_flags);

extern int                              machine_trace_thread64(thread_t thread,
    char *tracepos,
    char *tracebound,
    int nframes,
    boolean_t user_p,
    boolean_t getfp,
    uint32_t *thread_trace_flags,
    uint64_t *sp);

/*
 * Get the duration of the given thread's last wait.
 */
uint64_t thread_get_last_wait_duration(thread_t thread);

extern void thread_set_no_smt(bool set);
extern bool thread_get_no_smt(void);

#endif  /* XNU_KERNEL_PRIVATE */

/*! @function thread_has_thread_name
 *   @abstract Checks if a thread has a name.
 *   @discussion This function takes one input, a thread, and returns a boolean value indicating if that thread already has a name associated with it.
 *   @param th The thread to inspect.
 *   @result TRUE if the thread has a name, FALSE otherwise.
 */
extern boolean_t thread_has_thread_name(thread_t th);

/*! @function thread_set_thread_name
 *   @abstract Set a thread's name.
 *   @discussion This function takes two input parameters: a thread to name, and the name to apply to the thread.  The name will be copied over to the thread in order to better identify the thread.  If the name is longer than MAXTHREADNAMESIZE - 1, it will be truncated.
 *   @param th The thread to be named.
 *   @param name The name to apply to the thread.
 */
extern void thread_set_thread_name(thread_t th, const char* name);

/*! @function kernel_thread_start
 *   @abstract Create a kernel thread.
 *   @discussion This function takes three input parameters, namely reference to the function that the thread should execute, caller specified data and a reference which is used to return the newly created kernel thread. The function returns KERN_SUCCESS on success or an appropriate kernel code type indicating the error. It may be noted that the caller is responsible for explicitly releasing the reference to the created thread when no longer needed. This should be done by calling thread_deallocate(new_thread).
 *   @param continuation A C-function pointer where the thread will begin execution.
 *   @param parameter Caller specified data to be passed to the new thread.
 *   @param new_thread Reference to the new thread is returned in this parameter.
 *   @result Returns KERN_SUCCESS on success or an appropriate kernel code type.
 */

extern kern_return_t    kernel_thread_start(
	thread_continue_t       continuation,
	void                            *parameter,
	thread_t                        *new_thread);

#ifdef KERNEL_PRIVATE
void thread_set_eager_preempt(thread_t thread);
void thread_clear_eager_preempt(thread_t thread);
void thread_set_honor_qlimit(thread_t thread);
void thread_clear_honor_qlimit(thread_t thread);
extern ipc_port_t convert_thread_to_port(thread_t);
extern ipc_port_t convert_thread_inspect_to_port(thread_inspect_t);
extern boolean_t is_vm_privileged(void);
extern boolean_t set_vm_privilege(boolean_t);
extern kern_allocation_name_t thread_set_allocation_name(kern_allocation_name_t new_name);
extern void *thread_iokit_tls_get(uint32_t index);
extern void thread_iokit_tls_set(uint32_t index, void * data);
#endif /* KERNEL_PRIVATE */

__END_DECLS

#endif  /* _KERN_THREAD_H_ */
