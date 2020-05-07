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
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 *	File:	task.h
 *	Author:	Avadis Tevanian, Jr.
 *
 *	This file contains the structure definitions for tasks.
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
/*
 * NOTICE: This file was modified by McAfee Research in 2004 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 * Copyright (c) 2005 SPARTA, Inc.
 */

#ifndef _KERN_TASK_H_
#define _KERN_TASK_H_

#include <kern/kern_types.h>
#include <mach/mach_types.h>
#include <sys/cdefs.h>

#ifdef XNU_KERNEL_PRIVATE
#include <kern/kern_cdata.h>
#include <mach/sfi_class.h>
#include <kern/queue.h>
#include <sys/kern_sysctl.h>
#endif /* XNU_KERNEL_PRIVATE */

#ifdef  MACH_KERNEL_PRIVATE

#include <mach/boolean.h>
#include <mach/port.h>
#include <mach/time_value.h>
#include <mach/message.h>
#include <mach/mach_param.h>
#include <mach/task_info.h>
#include <mach/exception_types.h>
#include <mach/vm_statistics.h>
#include <machine/task.h>

#if MONOTONIC
#include <machine/monotonic.h>
#endif /* MONOTONIC */

#include <kern/cpu_data.h>
#include <kern/queue.h>
#include <kern/exception.h>
#include <kern/locks.h>
#include <security/_label.h>
#include <ipc/ipc_port.h>

#include <kern/thread.h>
#include <mach/coalition.h>
#include <stdatomic.h>
#include <os/refcnt.h>

#ifdef CONFIG_ATM
#include <atm/atm_internal.h>
#endif

struct _cpu_time_qos_stats {
	uint64_t cpu_time_qos_default;
	uint64_t cpu_time_qos_maintenance;
	uint64_t cpu_time_qos_background;
	uint64_t cpu_time_qos_utility;
	uint64_t cpu_time_qos_legacy;
	uint64_t cpu_time_qos_user_initiated;
	uint64_t cpu_time_qos_user_interactive;
};

struct task_writes_counters {
	uint64_t task_immediate_writes;
	uint64_t task_deferred_writes;
	uint64_t task_invalidated_writes;
	uint64_t task_metadata_writes;
};

struct task_watchports;
#include <bank/bank_internal.h>

struct task {
	/* Synchronization/destruction information */
	decl_lck_mtx_data(, lock);               /* Task's lock */
	os_refcnt_t     ref_count;      /* Number of references to me */
	boolean_t       active;         /* Task has not been terminated */
	boolean_t       halting;        /* Task is being halted */
	boolean_t       message_app_suspended;  /* Let iokit know when pidsuspended */

	/* Virtual timers */
	uint32_t                vtimers;

	/* Miscellaneous */
	vm_map_t        map;            /* Address space description */
	queue_chain_t   tasks;  /* global list of tasks */
	struct task_watchports *watchports; /* watchports passed in spawn */
	turnstile_inheritor_t returnwait_inheritor; /* inheritor for task_wait */

#if defined(CONFIG_SCHED_MULTIQ)
	sched_group_t sched_group;
#endif /* defined(CONFIG_SCHED_MULTIQ) */

	/* Threads in this task */
	queue_head_t            threads;
	struct restartable_ranges *restartable_ranges;

	processor_set_t         pset_hint;
	struct affinity_space   *affinity_space;

	int                     thread_count;
	uint32_t                active_thread_count;
	int                     suspend_count;  /* Internal scheduling only */

	/* User-visible scheduling information */
	integer_t               user_stop_count;        /* outstanding stops */
	integer_t               legacy_stop_count;      /* outstanding legacy stops */

	integer_t               priority;                       /* base priority for threads */
	integer_t               max_priority;           /* maximum priority for threads */

	integer_t               importance;             /* priority offset (BSD 'nice' value) */

	/* Task security and audit tokens */
	security_token_t sec_token;
	audit_token_t   audit_token;

	/* Statistics */
	uint64_t                total_user_time;        /* terminated threads only */
	uint64_t                total_system_time;
	uint64_t                total_ptime;
	uint64_t                total_runnable_time;

	/* IPC structures */
	decl_lck_mtx_data(, itk_lock_data);
	struct ipc_port *itk_self;      /* not a right, doesn't hold ref */
	struct ipc_port *itk_nself;     /* not a right, doesn't hold ref */
	struct ipc_port *itk_sself;     /* a send right */
	struct exception_action exc_actions[EXC_TYPES_COUNT];
	/* a send right each valid element  */
	struct ipc_port *itk_host;      /* a send right */
	struct ipc_port *itk_bootstrap; /* a send right */
	struct ipc_port *itk_seatbelt;  /* a send right */
	struct ipc_port *itk_gssd;      /* yet another send right */
	struct ipc_port *itk_debug_control; /* send right for debugmode communications */
	struct ipc_port *itk_task_access; /* and another send right */
	struct ipc_port *itk_resume;    /* a receive right to resume this task */
	struct ipc_port *itk_registered[TASK_PORT_REGISTER_MAX];
	/* all send rights */

	struct ipc_space *itk_space;

	ledger_t        ledger;
	/* Synchronizer ownership information */
	queue_head_t    semaphore_list;         /* list of owned semaphores   */
	int             semaphores_owned;       /* number of semaphores owned */

	unsigned int    priv_flags;                     /* privilege resource flags */
#define VM_BACKING_STORE_PRIV   0x1

	MACHINE_TASK

	integer_t faults;              /* faults counter */
	integer_t decompressions;      /* decompression counter */
	integer_t pageins;             /* pageins counter */
	integer_t cow_faults;          /* copy on write fault counter */
	integer_t messages_sent;       /* messages sent counter */
	integer_t messages_received;   /* messages received counter */
	integer_t syscalls_mach;       /* mach system call counter */
	integer_t syscalls_unix;       /* unix system call counter */
	uint32_t  c_switch;                                /* total context switches */
	uint32_t  p_switch;                                /* total processor switches */
	uint32_t  ps_switch;                       /* total pset switches */

#ifdef  MACH_BSD
	void *bsd_info;
#endif
	kcdata_descriptor_t             corpse_info;
	uint64_t                        crashed_thread_id;
	queue_chain_t                   corpse_tasks;
#ifdef CONFIG_MACF
	struct label *                  crash_label;
#endif
	struct vm_shared_region         *shared_region;
	volatile uint32_t t_flags;                                      /* general-purpose task flags protected by task_lock (TL) */
#define TF_NONE                 0
#define TF_64B_ADDR             0x00000001                              /* task has 64-bit addressing */
#define TF_64B_DATA             0x00000002                              /* task has 64-bit data registers */
#define TF_CPUMON_WARNING       0x00000004                              /* task has at least one thread in CPU usage warning zone */
#define TF_WAKEMON_WARNING      0x00000008                              /* task is in wakeups monitor warning zone */
#define TF_TELEMETRY            (TF_CPUMON_WARNING | TF_WAKEMON_WARNING) /* task is a telemetry participant */
#define TF_GPU_DENIED           0x00000010                              /* task is not allowed to access the GPU */
#define TF_CORPSE               0x00000020                              /* task is a corpse */
#define TF_PENDING_CORPSE       0x00000040                              /* task corpse has not been reported yet */
#define TF_CORPSE_FORK          0x00000080                              /* task is a forked corpse */
#define TF_PLATFORM             0x00000400                              /* task is a platform binary */
#define TF_CA_CLIENT_WI         0x00000800                              /* task has CA_CLIENT work interval */
#define TF_DARKWAKE_MODE        0x00001000                              /* task is in darkwake mode */
#define TF_NO_SMT               0x00002000                              /* task threads must not be paired with SMT threads */

/*
 * Task is running within a 64-bit address space.
 */
#define task_has_64Bit_addr(task)       \
	(((task)->t_flags & TF_64B_ADDR) != 0)
#define task_set_64Bit_addr(task)       \
	((task)->t_flags |= TF_64B_ADDR)
#define task_clear_64Bit_addr(task)     \
	((task)->t_flags &= ~TF_64B_ADDR)

/*
 * Task is using 64-bit machine state.
 */
#define task_has_64Bit_data(task)       \
	(((task)->t_flags & TF_64B_DATA) != 0)
#define task_set_64Bit_data(task)       \
	((task)->t_flags |= TF_64B_DATA)
#define task_clear_64Bit_data(task)     \
	((task)->t_flags &= ~TF_64B_DATA)

#define task_is_a_corpse(task)      \
	 (((task)->t_flags & TF_CORPSE) != 0)

#define task_set_corpse(task)       \
	 ((task)->t_flags |= TF_CORPSE)

#define task_corpse_pending_report(task)        \
	 (((task)->t_flags & TF_PENDING_CORPSE) != 0)

#define task_set_corpse_pending_report(task)       \
	 ((task)->t_flags |= TF_PENDING_CORPSE)

#define task_clear_corpse_pending_report(task)       \
	 ((task)->t_flags &= ~TF_PENDING_CORPSE)

#define task_is_a_corpse_fork(task)     \
	(((task)->t_flags & TF_CORPSE_FORK) != 0)

	uint32_t t_procflags;                                            /* general-purpose task flags protected by proc_lock (PL) */
#define TPF_NONE                 0
#define TPF_DID_EXEC             0x00000001                              /* task has been execed to a new task */
#define TPF_EXEC_COPY            0x00000002                              /* task is the new copy of an exec */
#ifdef CONFIG_32BIT_TELEMETRY
#define TPF_LOG_32BIT_TELEMETRY  0x00000004                              /* task should log identifying information */
#endif

#define task_did_exec_internal(task)            \
	(((task)->t_procflags & TPF_DID_EXEC) != 0)

#define task_is_exec_copy_internal(task)        \
	(((task)->t_procflags & TPF_EXEC_COPY) != 0)

	uint8_t                  t_returnwaitflags;
#define TWF_NONE                 0
#define TRW_LRETURNWAIT          0x01           /* task is waiting for fork/posix_spawn/exec to complete */
#define TRW_LRETURNWAITER        0x02           /* task is waiting for TRW_LRETURNWAIT to get cleared */

	mach_vm_address_t       all_image_info_addr; /* dyld __all_image_info     */
	mach_vm_size_t          all_image_info_size; /* section location and size */

#if KPC
#define TASK_KPC_FORCED_ALL_CTRS        0x2     /* Bit in "t_kpc" signifying this task forced all counters */
	uint32_t t_kpc; /* kpc flags */
#endif /* KPC */

	boolean_t pidsuspended; /* pid_suspend called; no threads can execute */
	boolean_t frozen;       /* frozen; private resident pages committed to swap */
	boolean_t changing_freeze_state;        /* in the process of freezing or thawing */
	uint16_t policy_ru_cpu          :4,
	    policy_ru_cpu_ext      :4,
	    applied_ru_cpu         :4,
	    applied_ru_cpu_ext     :4;
	uint8_t  rusage_cpu_flags;
	uint8_t  rusage_cpu_percentage;         /* Task-wide CPU limit percentage */
	uint8_t  rusage_cpu_perthr_percentage;  /* Per-thread CPU limit percentage */
#if MACH_ASSERT
	int8_t          suspends_outstanding;   /* suspends this task performed in excess of resumes */
#endif
	uint64_t rusage_cpu_interval;           /* Task-wide CPU limit interval */
	uint64_t rusage_cpu_perthr_interval;    /* Per-thread CPU limit interval */
	uint64_t rusage_cpu_deadline;
	thread_call_t rusage_cpu_callt;
#if CONFIG_EMBEDDED
	queue_head_t    task_watchers;          /* app state watcher threads */
	int     num_taskwatchers;
	int             watchapplying;
#endif /* CONFIG_EMBEDDED */

#if CONFIG_ATM
	struct atm_task_descriptor *atm_context;  /* pointer to per task atm descriptor */
#endif
	struct bank_task *bank_context;  /* pointer to per task bank structure */

#if IMPORTANCE_INHERITANCE
	struct ipc_importance_task  *task_imp_base;     /* Base of IPC importance chain */
#endif /* IMPORTANCE_INHERITANCE */

	vm_extmod_statistics_data_t     extmod_statistics;

	struct task_requested_policy requested_policy;
	struct task_effective_policy effective_policy;

	/*
	 * Can be merged with imp_donor bits, once the IMPORTANCE_INHERITANCE macro goes away.
	 */
	uint32_t        low_mem_notified_warn           :1,     /* warning low memory notification is sent to the task */
	    low_mem_notified_critical       :1,                 /* critical low memory notification is sent to the task */
	    purged_memory_warn              :1,                 /* purgeable memory of the task is purged for warning level pressure */
	    purged_memory_critical          :1,                 /* purgeable memory of the task is purged for critical level pressure */
	    low_mem_privileged_listener     :1,                 /* if set, task would like to know about pressure changes before other tasks on the system */
	    mem_notify_reserved             :27;                /* reserved for future use */

	uint32_t memlimit_is_active                 :1, /* if set, use active attributes, otherwise use inactive attributes */
	    memlimit_is_fatal                   :1,     /* if set, exceeding current memlimit will prove fatal to the task */
	    memlimit_active_exc_resource        :1,     /* if set, suppress exc_resource exception when task exceeds active memory limit */
	    memlimit_inactive_exc_resource      :1,     /* if set, suppress exc_resource exception when task exceeds inactive memory limit */
	    memlimit_attrs_reserved             :28;     /* reserved for future use */

	io_stat_info_t          task_io_stats;

	struct task_writes_counters task_writes_counters_internal;
	struct task_writes_counters task_writes_counters_external;

	/*
	 * The cpu_time_qos_stats fields are protected by the task lock
	 */
	struct _cpu_time_qos_stats      cpu_time_eqos_stats;
	struct _cpu_time_qos_stats      cpu_time_rqos_stats;

	/* Statistics accumulated for terminated threads from this task */
	uint32_t        task_timer_wakeups_bin_1;
	uint32_t        task_timer_wakeups_bin_2;
	uint64_t        task_gpu_ns;
	uint64_t        task_energy;

#if MONOTONIC
	/* Read and written under task_lock */
	struct mt_task task_monotonic;
#endif /* MONOTONIC */

	uint8_t         task_can_transfer_memory_ownership;
	uint8_t         task_objects_disowning;
	uint8_t         task_objects_disowned;
	/* # of purgeable volatile VM objects owned by this task: */
	int             task_volatile_objects;
	/* # of purgeable but not volatile VM objects owned by this task: */
	int             task_nonvolatile_objects;
	int             task_owned_objects;
	queue_head_t    task_objq;
	decl_lck_mtx_data(, task_objq_lock); /* protects "task_objq" */

	unsigned int    task_thread_limit:16;
#if __arm64__
	unsigned int    task_legacy_footprint:1;
	unsigned int    task_extra_footprint_limit:1;
	unsigned int    task_ios13extended_footprint_limit:1;
#endif /* __arm64__ */
	unsigned int    task_region_footprint:1;
	unsigned int    task_has_crossed_thread_limit:1;
	uint32_t        exec_token;
	/*
	 * A task's coalition set is "adopted" in task_create_internal
	 * and unset in task_deallocate_internal, so each array member
	 * can be referenced without the task lock.
	 * Note: these fields are protected by coalition->lock,
	 *       not the task lock.
	 */
	coalition_t     coalition[COALITION_NUM_TYPES];
	queue_chain_t   task_coalition[COALITION_NUM_TYPES];
	uint64_t        dispatchqueue_offset;

#if DEVELOPMENT || DEBUG
	boolean_t       task_unnested;
	int             task_disconnected_count;
#endif

#if HYPERVISOR
	void *hv_task_target; /* hypervisor virtual machine object associated with this task */
#endif /* HYPERVISOR */

#if CONFIG_SECLUDED_MEMORY
	uint8_t task_can_use_secluded_mem;
	uint8_t task_could_use_secluded_mem;
	uint8_t task_could_also_use_secluded_mem;
	uint8_t task_suppressed_secluded;
#endif /* CONFIG_SECLUDED_MEMORY */

	task_exc_guard_behavior_t task_exc_guard;

	queue_head_t    io_user_clients;

	mach_vm_address_t mach_header_vm_address;

	uint32_t loadTag; /* dext ID used for logging identity */
};

/*
 * EXC_GUARD default delivery behavior for optional Mach port and VM guards.
 * Applied to new tasks at creation time.
 */
extern task_exc_guard_behavior_t task_exc_guard_default;

extern kern_return_t
    task_violated_guard(mach_exception_code_t, mach_exception_subcode_t, void *);

#define task_lock(task)                 lck_mtx_lock(&(task)->lock)
#define task_lock_assert_owned(task)    LCK_MTX_ASSERT(&(task)->lock, LCK_MTX_ASSERT_OWNED)
#define task_lock_try(task)             lck_mtx_try_lock(&(task)->lock)
#define task_unlock(task)               lck_mtx_unlock(&(task)->lock)

#define task_objq_lock_init(task)       lck_mtx_init(&(task)->task_objq_lock, &vm_object_lck_grp, &vm_object_lck_attr)
#define task_objq_lock(task)            lck_mtx_lock(&(task)->task_objq_lock)
#define task_objq_lock_assert_owned(task)       LCK_MTX_ASSERT(&(task)->task_objq_lock, LCK_MTX_ASSERT_OWNED)
#define task_objq_lock_try(task)        lck_mtx_try_lock(&(task)->task_objq_lock)
#define task_objq_unlock(task)          lck_mtx_unlock(&(task)->task_objq_lock)

#define itk_lock_init(task)     lck_mtx_init(&(task)->itk_lock_data, &ipc_lck_grp, &ipc_lck_attr)
#define itk_lock_destroy(task)  lck_mtx_destroy(&(task)->itk_lock_data, &ipc_lck_grp)
#define itk_lock(task)          lck_mtx_lock(&(task)->itk_lock_data)
#define itk_unlock(task)        lck_mtx_unlock(&(task)->itk_lock_data)

/* task clear return wait flags */
#define TCRW_CLEAR_INITIAL_WAIT   0x1
#define TCRW_CLEAR_FINAL_WAIT     0x2
#define TCRW_CLEAR_ALL_WAIT       (TCRW_CLEAR_INITIAL_WAIT | TCRW_CLEAR_FINAL_WAIT)

#define TASK_REFERENCE_LEAK_DEBUG 0

#if TASK_REFERENCE_LEAK_DEBUG
extern void task_reference_internal(task_t task);
extern os_ref_count_t task_deallocate_internal(task_t task);
#else
#define task_reference_internal(task) os_ref_retain(&(task)->ref_count)
#define task_deallocate_internal(task) os_ref_release(&(task)->ref_count)
#endif

#define task_reference(task)                                    \
MACRO_BEGIN                                                                             \
	if ((task) != TASK_NULL)                                        \
	        task_reference_internal(task);                  \
MACRO_END

extern kern_return_t    kernel_task_create(
	task_t                  task,
	vm_offset_t             map_base,
	vm_size_t               map_size,
	task_t                  *child);

/* Initialize task module */
extern void             task_init(void);

/* coalition_init() calls this to initialize ledgers before task_init() */
extern void             init_task_ledgers(void);

#define current_task_fast()     (current_thread()->task)
#define current_task()          current_task_fast()

extern bool task_is_driver(task_t task);

extern lck_attr_t      task_lck_attr;
extern lck_grp_t       task_lck_grp;

struct task_watchport_elem {
	task_t                          twe_task;
	ipc_port_t                      twe_port;     /* (Space lock) */
};

struct task_watchports {
	os_refcnt_t                     tw_refcount;           /* (Space lock) */
	task_t                          tw_task;               /* (Space lock) & tw_refcount == 0 */
	thread_t                        tw_thread;             /* (Space lock) & tw_refcount == 0 */
	uint32_t                        tw_elem_array_count;   /* (Space lock) */
	struct task_watchport_elem      tw_elem[];             /* (Space lock) & (Portlock) & (mq lock) */
};

#define task_watchports_retain(x)   (os_ref_retain(&(x)->tw_refcount))
#define task_watchports_release(x)  (os_ref_release(&(x)->tw_refcount))

#define task_watchport_elem_init(elem, task, port) \
do {                                               \
	(elem)->twe_task = (task);                 \
	(elem)->twe_port = (port);                 \
} while(0)

#define task_watchport_elem_clear(elem) task_watchport_elem_init((elem), NULL, NULL)

extern void
task_add_turnstile_watchports(
	task_t          task,
	thread_t        thread,
	ipc_port_t      *portwatch_ports,
	uint32_t        portwatch_count);

extern void
task_watchport_elem_deallocate(
	struct          task_watchport_elem *watchport_elem);

extern boolean_t
task_has_watchports(task_t task);

#else   /* MACH_KERNEL_PRIVATE */

__BEGIN_DECLS

extern task_t   current_task(void);

extern void             task_reference(task_t   task);
extern bool task_is_driver(task_t task);

#define TF_NONE                 0

#define TWF_NONE                 0
#define TRW_LRETURNWAIT          0x01           /* task is waiting for fork/posix_spawn/exec to complete */
#define TRW_LRETURNWAITER        0x02           /* task is waiting for TRW_LRETURNWAIT to get cleared */

/* task clear return wait flags */
#define TCRW_CLEAR_INITIAL_WAIT   0x1
#define TCRW_CLEAR_FINAL_WAIT     0x2
#define TCRW_CLEAR_ALL_WAIT       (TCRW_CLEAR_INITIAL_WAIT | TCRW_CLEAR_FINAL_WAIT)


#define TPF_NONE                0
#define TPF_EXEC_COPY           0x00000002                              /* task is the new copy of an exec */


__END_DECLS

#endif  /* MACH_KERNEL_PRIVATE */

__BEGIN_DECLS

#ifdef KERNEL_PRIVATE
extern boolean_t                task_is_app_suspended(task_t task);
#endif

#ifdef  XNU_KERNEL_PRIVATE

/* Hold all threads in a task */
extern kern_return_t    task_hold(
	task_t          task);

/* Wait for task to stop running, either just to get off CPU or to cease being runnable */
extern kern_return_t    task_wait(
	task_t          task,
	boolean_t       until_not_runnable);

/* Release hold on all threads in a task */
extern kern_return_t    task_release(
	task_t          task);

/* Suspend/resume a task where the kernel owns the suspend count */
extern kern_return_t    task_suspend_internal(          task_t          task);
extern kern_return_t    task_resume_internal(           task_t          task);

/* Suspends a task by placing a hold on its threads */
extern kern_return_t    task_pidsuspend(
	task_t          task);

/* Resumes a previously paused task */
extern kern_return_t    task_pidresume(
	task_t          task);

extern kern_return_t    task_send_trace_memory(
	task_t          task,
	uint32_t        pid,
	uint64_t        uniqueid);

extern void             task_remove_turnstile_watchports(
	task_t          task);

extern void             task_transfer_turnstile_watchports(
	task_t          old_task,
	task_t          new_task,
	thread_t        new_thread);

#if DEVELOPMENT || DEBUG

extern kern_return_t    task_disconnect_page_mappings(
	task_t          task);
#endif

extern void                     tasks_system_suspend(boolean_t suspend);

#if CONFIG_FREEZE

/* Freeze a task's resident pages */
extern kern_return_t    task_freeze(
	task_t          task,
	uint32_t        *purgeable_count,
	uint32_t        *wired_count,
	uint32_t        *clean_count,
	uint32_t        *dirty_count,
	uint32_t        dirty_budget,
	uint32_t        *shared_count,
	int             *freezer_error_code,
	boolean_t       eval_only);

/* Thaw a currently frozen task */
extern kern_return_t    task_thaw(
	task_t          task);

#endif /* CONFIG_FREEZE */

/* Halt all other threads in the current task */
extern kern_return_t    task_start_halt(
	task_t          task);

/* Wait for other threads to halt and free halting task resources */
extern void             task_complete_halt(
	task_t          task);

extern kern_return_t    task_terminate_internal(
	task_t                  task);

extern kern_return_t    task_create_internal(
	task_t          parent_task,
	coalition_t     *parent_coalitions,
	boolean_t       inherit_memory,
	boolean_t       is_64bit,
	boolean_t       is_64bit_data,
	uint32_t        flags,
	uint32_t        procflags,
	uint8_t         t_returnwaitflags,
	task_t          *child_task);                                                   /* OUT */

extern kern_return_t    task_set_special_port_internal(
	task_t                  task,
	int                     which,
	ipc_port_t              port);

extern kern_return_t    task_info(
	task_t                  task,
	task_flavor_t           flavor,
	task_info_t             task_info_out,
	mach_msg_type_number_t  *task_info_count);

extern void             task_power_info_locked(
	task_t                  task,
	task_power_info_t       info,
	gpu_energy_data_t       gpu_energy,
	task_power_info_v2_t    infov2,
	uint64_t                *runnable_time);

extern uint64_t         task_gpu_utilisation(
	task_t   task);

extern uint64_t         task_energy(
	task_t   task);

extern uint64_t         task_cpu_ptime(
	task_t   task);
extern void             task_update_cpu_time_qos_stats(
	task_t   task,
	uint64_t *eqos_stats,
	uint64_t *rqos_stats);

extern void             task_vtimer_set(
	task_t          task,
	integer_t       which);

extern void             task_vtimer_clear(
	task_t          task,
	integer_t       which);

extern void             task_vtimer_update(
	task_t          task,
	integer_t       which,
	uint32_t        *microsecs);

#define TASK_VTIMER_USER                0x01
#define TASK_VTIMER_PROF                0x02
#define TASK_VTIMER_RLIM                0x04

extern void             task_set_64bit(
	task_t          task,
	boolean_t       is_64bit,
	boolean_t       is_64bit_data);

extern boolean_t        task_get_64bit_data(
	task_t task);

extern void     task_set_platform_binary(
	task_t task,
	boolean_t is_platform);
extern bool     task_set_ca_client_wi(
	task_t task,
	boolean_t ca_client_wi);

extern void             task_set_dyld_info(
	task_t          task,
	mach_vm_address_t addr,
	mach_vm_size_t size);

extern void task_set_mach_header_address(
	task_t task,
	mach_vm_address_t addr);

/* Get number of activations in a task */
extern int              get_task_numacts(
	task_t          task);

struct label;
extern kern_return_t task_collect_crash_info(
	task_t task,
#if CONFIG_MACF
	struct label *crash_label,
#endif
	int is_corpse_fork);
void task_port_notify(mach_msg_header_t *msg);
void task_wait_till_threads_terminate_locked(task_t task);

/* JMM - should just be temporary (implementation in bsd_kern still) */
extern void     set_bsdtask_info(task_t, void *);
extern uint32_t set_task_loadTag(task_t task, uint32_t loadTag);
extern vm_map_t get_task_map_reference(task_t);
extern vm_map_t swap_task_map(task_t, thread_t, vm_map_t);
extern pmap_t   get_task_pmap(task_t);
extern uint64_t get_task_resident_size(task_t);
extern uint64_t get_task_compressed(task_t);
extern uint64_t get_task_resident_max(task_t);
extern uint64_t get_task_phys_footprint(task_t);
#if CONFIG_LEDGER_INTERVAL_MAX
extern uint64_t get_task_phys_footprint_interval_max(task_t, int reset);
#endif /* CONFIG_FOOTPRINT_INTERVAL_MAX */
extern uint64_t get_task_phys_footprint_lifetime_max(task_t);
extern uint64_t get_task_phys_footprint_limit(task_t);
extern uint64_t get_task_purgeable_size(task_t);
extern uint64_t get_task_cpu_time(task_t);
extern uint64_t get_task_dispatchqueue_offset(task_t);
extern uint64_t get_task_dispatchqueue_serialno_offset(task_t);
extern uint64_t get_task_dispatchqueue_label_offset(task_t);
extern uint64_t get_task_uniqueid(task_t task);
extern int      get_task_version(task_t task);

extern uint64_t get_task_internal(task_t);
extern uint64_t get_task_internal_compressed(task_t);
extern uint64_t get_task_purgeable_nonvolatile(task_t);
extern uint64_t get_task_purgeable_nonvolatile_compressed(task_t);
extern uint64_t get_task_iokit_mapped(task_t);
extern uint64_t get_task_alternate_accounting(task_t);
extern uint64_t get_task_alternate_accounting_compressed(task_t);
extern uint64_t get_task_memory_region_count(task_t);
extern uint64_t get_task_page_table(task_t);
extern uint64_t get_task_network_nonvolatile(task_t);
extern uint64_t get_task_network_nonvolatile_compressed(task_t);
extern uint64_t get_task_wired_mem(task_t);
extern uint32_t get_task_loadTag(task_t task);

extern kern_return_t task_convert_phys_footprint_limit(int, int *);
extern kern_return_t task_set_phys_footprint_limit_internal(task_t, int, int *, boolean_t, boolean_t);
extern kern_return_t task_get_phys_footprint_limit(task_t task, int *limit_mb);

/* Jetsam memlimit attributes */
extern boolean_t task_get_memlimit_is_active(task_t task);
extern boolean_t task_get_memlimit_is_fatal(task_t task);
extern void task_set_memlimit_is_active(task_t task, boolean_t memlimit_is_active);
extern void task_set_memlimit_is_fatal(task_t task, boolean_t memlimit_is_fatal);
extern boolean_t task_has_triggered_exc_resource(task_t task, boolean_t memlimit_is_active);
extern void task_mark_has_triggered_exc_resource(task_t task, boolean_t memlimit_is_active);

extern void task_set_thread_limit(task_t task, uint16_t thread_limit);


extern boolean_t        is_kerneltask(task_t task);
extern boolean_t        is_corpsetask(task_t task);

extern kern_return_t check_actforsig(task_t task, thread_t thread, int setast);

extern kern_return_t machine_task_get_state(
	task_t task,
	int flavor,
	thread_state_t state,
	mach_msg_type_number_t *state_count);

extern kern_return_t machine_task_set_state(
	task_t task,
	int flavor,
	thread_state_t state,
	mach_msg_type_number_t state_count);

extern void machine_task_terminate(task_t task);

struct _task_ledger_indices {
	int cpu_time;
	int tkm_private;
	int tkm_shared;
	int phys_mem;
	int wired_mem;
	int internal;
	int iokit_mapped;
	int alternate_accounting;
	int alternate_accounting_compressed;
	int page_table;
	int phys_footprint;
	int internal_compressed;
	int purgeable_volatile;
	int purgeable_nonvolatile;
	int purgeable_volatile_compressed;
	int purgeable_nonvolatile_compressed;
	int tagged_nofootprint;
	int tagged_footprint;
	int tagged_nofootprint_compressed;
	int tagged_footprint_compressed;
	int network_volatile;
	int network_nonvolatile;
	int network_volatile_compressed;
	int network_nonvolatile_compressed;
	int media_nofootprint;
	int media_footprint;
	int media_nofootprint_compressed;
	int media_footprint_compressed;
	int graphics_nofootprint;
	int graphics_footprint;
	int graphics_nofootprint_compressed;
	int graphics_footprint_compressed;
	int neural_nofootprint;
	int neural_footprint;
	int neural_nofootprint_compressed;
	int neural_footprint_compressed;
	int platform_idle_wakeups;
	int interrupt_wakeups;
#if CONFIG_SCHED_SFI
	int sfi_wait_times[MAX_SFI_CLASS_ID];
#endif /* CONFIG_SCHED_SFI */
	int cpu_time_billed_to_me;
	int cpu_time_billed_to_others;
	int physical_writes;
	int logical_writes;
	int logical_writes_to_external;
	int energy_billed_to_me;
	int energy_billed_to_others;
#if DEBUG || DEVELOPMENT
	int pages_grabbed;
	int pages_grabbed_kern;
	int pages_grabbed_iopl;
	int pages_grabbed_upl;
#endif
};
extern struct _task_ledger_indices task_ledgers;

/* requires task to be unlocked, returns a referenced thread */
thread_t task_findtid(task_t task, uint64_t tid);
int pid_from_task(task_t task);

extern kern_return_t task_wakeups_monitor_ctl(task_t task, uint32_t *rate_hz, int32_t *flags);
extern kern_return_t task_cpu_usage_monitor_ctl(task_t task, uint32_t *flags);
extern void task_rollup_accounting_info(task_t new_task, task_t parent_task);
extern kern_return_t task_io_monitor_ctl(task_t task, uint32_t *flags);
extern void task_set_did_exec_flag(task_t task);
extern void task_clear_exec_copy_flag(task_t task);
extern boolean_t task_is_exec_copy(task_t);
extern boolean_t task_did_exec(task_t task);
#ifdef CONFIG_32BIT_TELEMETRY
extern boolean_t task_consume_32bit_log_flag(task_t task);
extern void task_set_32bit_log_flag(task_t task);
#endif /* CONFIG_32BIT_TELEMETRY */
extern boolean_t task_is_active(task_t task);
extern boolean_t task_is_halting(task_t task);
extern void task_clear_return_wait(task_t task, uint32_t flags);
extern void task_wait_to_return(void) __attribute__((noreturn));
extern event_t task_get_return_wait_event(task_t task);

extern void task_atm_reset(task_t task);
extern void task_bank_reset(task_t task);
extern void task_bank_init(task_t task);

#if CONFIG_ARCADE
extern void task_prep_arcade(task_t task, thread_t thread);
#endif /* CONFIG_ARCADE */

extern int task_pid(task_t task);
extern boolean_t task_has_assertions(task_t task);
/* End task_policy */

extern void      task_set_gpu_denied(task_t task, boolean_t denied);
extern boolean_t task_is_gpu_denied(task_t task);

extern queue_head_t * task_io_user_clients(task_t task);
extern void     task_set_message_app_suspended(task_t task, boolean_t enable);

extern void task_copy_fields_for_exec(task_t dst_task, task_t src_task);

extern void task_copy_vmobjects(task_t task, vm_object_query_t query, int len, int64_t* num);

#endif  /* XNU_KERNEL_PRIVATE */

#ifdef  KERNEL_PRIVATE

extern void     *get_bsdtask_info(task_t);
extern void     *get_bsdthreadtask_info(thread_t);
extern void task_bsdtask_kill(task_t);
extern vm_map_t get_task_map(task_t);
extern ledger_t get_task_ledger(task_t);

extern boolean_t get_task_pidsuspended(task_t);
extern boolean_t get_task_suspended(task_t);
extern boolean_t get_task_frozen(task_t);

/* Convert from a task to a port */
extern ipc_port_t convert_task_to_port(task_t);
extern ipc_port_t convert_task_name_to_port(task_name_t);
extern ipc_port_t convert_task_inspect_to_port(task_inspect_t);
extern ipc_port_t convert_task_suspension_token_to_port(task_suspension_token_t task);

/* Convert from a port (in this case, an SO right to a task's resume port) to a task. */
extern task_suspension_token_t convert_port_to_task_suspension_token(ipc_port_t port);

extern boolean_t task_suspension_notify(mach_msg_header_t *);

#define TASK_WRITE_IMMEDIATE                 0x1
#define TASK_WRITE_DEFERRED                  0x2
#define TASK_WRITE_INVALIDATED               0x4
#define TASK_WRITE_METADATA                  0x8
extern void     task_update_logical_writes(task_t task, uint32_t io_size, int flags, void *vp);

#if CONFIG_SECLUDED_MEMORY
extern void task_set_can_use_secluded_mem(
	task_t task,
	boolean_t can_use_secluded_mem);
extern void task_set_could_use_secluded_mem(
	task_t task,
	boolean_t could_use_secluded_mem);
extern void task_set_could_also_use_secluded_mem(
	task_t task,
	boolean_t could_also_use_secluded_mem);
extern boolean_t task_can_use_secluded_mem(
	task_t task,
	boolean_t is_allocate);
extern boolean_t task_could_use_secluded_mem(task_t task);
extern boolean_t task_could_also_use_secluded_mem(task_t task);
#endif /* CONFIG_SECLUDED_MEMORY */

extern void task_set_darkwake_mode(task_t, boolean_t);
extern boolean_t task_get_darkwake_mode(task_t);

#if __arm64__
extern void task_set_legacy_footprint(task_t task);
extern void task_set_extra_footprint_limit(task_t task);
extern void task_set_ios13extended_footprint_limit(task_t task);
#endif /* __arm64__ */

#if CONFIG_MACF
extern struct label *get_task_crash_label(task_t task);
#endif /* CONFIG_MACF */

extern int get_task_cdhash(task_t task, char cdhash[]);

#endif  /* KERNEL_PRIVATE */

extern task_t   kernel_task;

extern void             task_deallocate(
	task_t          task);

extern void             task_name_deallocate(
	task_name_t             task_name);

extern void             task_inspect_deallocate(
	task_inspect_t          task_inspect);

extern void             task_suspension_token_deallocate(
	task_suspension_token_t token);

extern boolean_t task_self_region_footprint(void);
extern void task_self_region_footprint_set(boolean_t newval);
extern void task_ledgers_footprint(ledger_t ledger,
    ledger_amount_t *ledger_resident,
    ledger_amount_t *ledger_compressed);
extern void task_set_memory_ownership_transfer(
	task_t task,
	boolean_t value);

__END_DECLS

#endif  /* _KERN_TASK_H_ */
