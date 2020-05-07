/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#ifdef KERNEL_PRIVATE

#ifndef _PTHREAD_SHIMS_H_
#define _PTHREAD_SHIMS_H_

#ifndef ASSEMBLER

#include <kern/block_hint.h>
#include <kern/clock.h>
#include <kern/kern_types.h>
#include <kern/kcdata.h>
#include <kern/locks.h>
#include <kern/turnstile.h>
#include <pthread/priority_private.h>
#include <sys/user.h>
#include <sys/_types.h>
#include <sys/_types/_sigset_t.h>
#include <sys/kernel_types.h>

#ifndef PTHREAD_INTERNAL
struct uthread;
struct ksyn_waitq_element;
#define M_PROC 41
#endif

#if !defined(_SCHED_CALL_T_DEFINED)
#define _SCHED_CALL_T_DEFINED
typedef void (*sched_call_t)(int type, thread_t thread);
#endif

/*
 * Increment each time new reserved slots are used. When the pthread
 * kext registers this table, it will include the version of the xnu
 * headers that it was built against.
 */
#define PTHREAD_FUNCTIONS_TABLE_VERSION 1

typedef const struct pthread_functions_s {
	int version;

	/* internal calls, kernel core -> kext */
	void (*pthread_init)(void);

	void *__unused_was_fill_procworkqueue;
	void *__unused1;
	void *__unused2;
	void *__unused_was_workqueue_exit;
	void *__unused_was_workqueue_mark_exiting;
	void *__unused_was_workqueue_thread_yielded;

	void (*pth_proc_hashinit)(proc_t p);
	void (*pth_proc_hashdelete)(proc_t p);

	/* syscall stubs */
	int (*bsdthread_create)(struct proc *p, user_addr_t user_func, user_addr_t user_funcarg, user_addr_t user_stack, user_addr_t user_pthread, uint32_t flags, user_addr_t *retval);
	int (*bsdthread_register)(struct proc *p, user_addr_t threadstart, user_addr_t wqthread, int pthsize, user_addr_t dummy_value, user_addr_t targetconc_ptr, uint64_t dispatchqueue_offset, int32_t *retval);
	int (*bsdthread_terminate)(struct proc *p, user_addr_t stackaddr, size_t size, uint32_t kthport, uint32_t sem, int32_t *retval);
	int (*thread_selfid)(struct proc *p, uint64_t *retval);
	void *__unused_was_workq_kernreturn;
	void *__unused_was_workq_open;

	/* psynch syscalls */
	int (*psynch_mutexwait)(proc_t p, user_addr_t mutex, uint32_t mgen, uint32_t  ugen, uint64_t tid, uint32_t flags, uint32_t *retval);
	int (*psynch_mutexdrop)(proc_t p, user_addr_t mutex, uint32_t mgen, uint32_t  ugen, uint64_t tid, uint32_t flags, uint32_t *retval);
	int (*psynch_cvbroad)(proc_t p, user_addr_t cv, uint64_t cvlsgen, uint64_t cvudgen, uint32_t flags, user_addr_t mutex, uint64_t mugen, uint64_t tid, uint32_t *retval);
	int (*psynch_cvsignal)(proc_t p, user_addr_t cv, uint64_t cvlsgen, uint32_t cvugen, int thread_port, user_addr_t mutex, uint64_t mugen, uint64_t tid, uint32_t flags, uint32_t *retval);
	int (*psynch_cvwait)(proc_t p, user_addr_t cv, uint64_t cvlsgen, uint32_t cvugen, user_addr_t mutex, uint64_t mugen, uint32_t flags, int64_t sec, uint32_t nsec, uint32_t * retval);
	int (*psynch_cvclrprepost)(proc_t p, user_addr_t cv, uint32_t cvgen, uint32_t cvugen, uint32_t cvsgen, uint32_t prepocnt, uint32_t preposeq, uint32_t flags, int *retval);
	int (*psynch_rw_longrdlock)(proc_t p, user_addr_t rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags, uint32_t *retval);
	int (*psynch_rw_rdlock)(proc_t p, user_addr_t rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags, uint32_t *retval);
	int (*psynch_rw_unlock)(proc_t p, user_addr_t rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags, uint32_t *retval);
	int (*psynch_rw_wrlock)(proc_t p, user_addr_t rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags, uint32_t *retval);
	int (*psynch_rw_yieldwrlock)(proc_t p, user_addr_t rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags, uint32_t *retval);

	void *__unused_was_workqueue_get_sched_callback;

	/* New register function with TSD offset */
	int (*bsdthread_register2)(struct proc *p, user_addr_t threadstart, user_addr_t wqthread, uint32_t flags, user_addr_t stack_addr_hint, user_addr_t targetconc_ptr, uint32_t dispatchqueue_offset, uint32_t tsd_offset, int32_t *retval);

	void *__unused_was_bsdthread_ctl;
	void *__unused_was_workq_reqthreads;

	void *__unused_was_thread_qos_from_pthread_priority;
	void *__unused_was_get_pwq_state_kdp;
	void *__unused3;
	void *__unused_was_pthread_priority_canonicalize2;
	void *__unused_was_workq_thread_has_been_unbound;

	void (*pthread_find_owner)(thread_t thread, struct stackshot_thread_waitinfo *waitinfo);
	void *(*pthread_get_thread_kwq)(thread_t thread);

	void *__unused_was_workq_threadreq;

	int  (*workq_handle_stack_events)(proc_t p, thread_t th, vm_map_t map,
	    user_addr_t stackaddr, mach_port_name_t kport,
	    user_addr_t events, int nevents, int upcall_flags);

	int (*workq_create_threadstack)(proc_t p, vm_map_t vmap,
	    mach_vm_offset_t *out_addr);

	int (*workq_destroy_threadstack)(proc_t p, vm_map_t vmap,
	    mach_vm_offset_t stackaddr);

	void (*workq_setup_thread)(proc_t p, thread_t th, vm_map_t map,
	    user_addr_t stackaddr, mach_port_name_t kport, int th_qos,
	    int setup_flags, int upcall_flags);

	void (*workq_markfree_threadstack)(proc_t p, thread_t, vm_map_t map,
	    user_addr_t stackaddr);

	/* padding for future */
	void * _pad[83];
} * pthread_functions_t;

typedef const struct pthread_callbacks_s {
	int version;

	/* config information */
	uint32_t config_thread_max;
	uint32_t (*get_task_threadmax)(void);

	/* proc.h accessors */
	uint64_t (*proc_get_register)(struct proc *t);
	void (*proc_set_register)(struct proc *t);

	user_addr_t (*proc_get_threadstart)(struct proc *t);
	void (*proc_set_threadstart)(struct proc *t, user_addr_t addr);
	user_addr_t (*proc_get_wqthread)(struct proc *t);
	void (*proc_set_wqthread)(struct proc *t, user_addr_t addr);
	int (*proc_get_pthsize)(struct proc *t);
	void (*proc_set_pthsize)(struct proc *t, int size);

	thread_t (*task_findtid)(task_t t, uint64_t tid);
	void (*thread_deallocate_safe)(thread_t);
	void *__unused_was_proc_get_dispatchqueue_offset;
	void (*proc_set_dispatchqueue_offset)(struct proc *t, uint64_t offset);
	void *__unused_was_proc_get_wqlockptr;
	void *__unused_was_proc_get_wqinitingptr;
	void *__unused_was_proc_get_wqptr;

	wait_result_t (*psynch_wait_prepare)(uintptr_t kwq,
	    struct turnstile **tstore, thread_t owner, block_hint_t block_hint,
	    uint64_t deadline);

	void (*psynch_wait_update_complete)(struct turnstile *turnstile);

	void (*psynch_wait_complete)(uintptr_t kwq, struct turnstile **tstore);

	void (*psynch_wait_cleanup)(void);

	kern_return_t (*psynch_wait_wakeup)(uintptr_t kwq,
	    struct ksyn_waitq_element *kwe, struct turnstile **tstore);

	void (*psynch_wait_update_owner)(uintptr_t kwq, thread_t owner,
	    struct turnstile **tstore);

	void* (*proc_get_pthhash)(struct proc *t);
	void (*proc_set_pthhash)(struct proc *t, void* ptr);

	/* bsd/sys/user.h */
	void *__unused_was_uthread_get_threadlist;
	void *__unused_was_uthread_set_threadlist;
	void *__unused_was_uthread_get_sigmask;
	void *__unused_was_uthread_set_sigmask;

	void* (*uthread_get_uukwe)(struct uthread *t);
	void *__unused_was_uthread_get_returnval;
	void (*uthread_set_returnval)(struct uthread *t, int val);
	int (*uthread_is_cancelled)(struct uthread *t);

	/* vm_protos.h calls */
	ipc_space_t (*task_get_ipcspace)(task_t t);
	mach_port_name_t (*ipc_port_copyout_send)(ipc_port_t sright, ipc_space_t space);

	/* osfmk/vm/vm_map.h */
	kern_return_t (*vm_map_page_info)(vm_map_t map, vm_map_offset_t offset, vm_page_info_flavor_t flavor, vm_page_info_t info, mach_msg_type_number_t *count);
	void *__unused_was_vm_map_switch;

	/* wq functions */
	kern_return_t (*thread_set_wq_state32)(thread_t thread, thread_state_t state);
#if !defined(__arm__)
	kern_return_t (*thread_set_wq_state64)(thread_t thread, thread_state_t state);
#endif

	/* sched_prim.h */
	void (*thread_exception_return)(void);
	void (*thread_bootstrap_return)(void);

	void *__unused_was_absolutetime_to_microtime;
	void *__unused_was_thread_set_workq_pri;
	void *__unused_was_thread_set_workq_qos;

	/* osfmk/kern/thread.h */
	struct uthread* (*get_bsdthread_info)(thread_t th);
	void *__unused_was_thread_sched_call;
	void *__unused_was_thread_static_param;
	void *__unused_was_thread_create_workq_waiting_parameter;
	kern_return_t (*thread_policy_set_internal)(thread_t t, thread_policy_flavor_t flavour, thread_policy_t info, mach_msg_type_number_t count);

	void *__unused_was_thread_affinity_set;

	/* bsd/sys/systm.h */
	void (*unix_syscall_return)(int error);

	void *__unused_was_zalloc;
	void *__unused_was_zfree;
	void *__unused_was_zinit;

	/* bsd/kern/kern_sig.c */
	void (*__pthread_testcancel)(int);

	/* calls without portfolio */
	kern_return_t (*mach_port_deallocate)(ipc_space_t space, mach_port_name_t name);
	kern_return_t (*semaphore_signal_internal_trap)(mach_port_name_t sema_name);
	vm_map_t (*current_map)(void);

	/* osfmk/kern/thread.h */
	ipc_port_t (*convert_thread_to_port)(thread_t th);

	/* mach/task.h */
	kern_return_t (*thread_create)(task_t parent_task, thread_act_t *child_act);

	/* mach/thread_act.h */
	kern_return_t (*thread_resume)(thread_act_t target_act);

	/* bsd/sys/event.h */
	int (*kevent_workq_internal)(struct proc *p,
	    user_addr_t changelist, int nchanges,
	    user_addr_t eventlist, int nevents,
	    user_addr_t data_out, user_size_t *data_available,
	    unsigned int flags, int32_t *retval);

#if defined(__arm__)
	void *__unused_was_map_is_1gb;
#endif

	void *__unused_was_proc_get_dispatchqueue_serialno_offset;
	void *__unused_was_proc_set_dispatchqueue_serialno_offset;

	void *__unused_was_proc_usynch_thread_qos_add_override_for_resource_check_owner;
	void *__unused_was_proc_set_stack_addr_hint;

	uint32_t (*proc_get_pthread_tsd_offset)(struct proc *p);
	void (*proc_set_pthread_tsd_offset)(struct proc *p, uint32_t pthread_tsd_offset);

	kern_return_t (*thread_set_tsd_base)(thread_t thread, mach_vm_offset_t tsd_base);

	int     (*proc_usynch_get_requested_thread_qos)(struct uthread *);
	uint64_t (*proc_get_mach_thread_self_tsd_offset)(struct proc *p);
	void (*proc_set_mach_thread_self_tsd_offset)(struct proc *p, uint64_t mach_thread_self_tsd_offset);

	kern_return_t (*thread_policy_get)(thread_t t, thread_policy_flavor_t flavor, thread_policy_t info, mach_msg_type_number_t *count, boolean_t *get_default);
	boolean_t (*qos_main_thread_active)(void);

	kern_return_t (*thread_set_voucher_name)(mach_port_name_t name);

	boolean_t (*proc_usynch_thread_qos_add_override_for_resource)(task_t task, struct uthread *, uint64_t tid, int override_qos, boolean_t first_override_for_resource, user_addr_t resource, int resource_type);
	boolean_t (*proc_usynch_thread_qos_remove_override_for_resource)(task_t task, struct uthread *, uint64_t tid, user_addr_t resource, int resource_type);
	void *__unused_was_proc_usynch_thread_qos_reset_override_for_resource;

	void *__unused_was_proc_init_wqptr_or_wait;

	uint16_t (*thread_set_tag)(thread_t thread, uint16_t tag);
	uint16_t (*thread_get_tag)(thread_t thread);

	void *__unused_was_proc_usynch_thread_qos_squash_override_for_resource;
	void *__unused_was_task_get_default_manager_qos;
	void *__unused_was_thread_create_workq_waiting;

	user_addr_t (*proc_get_stack_addr_hint)(struct proc *p);
	void (*proc_set_stack_addr_hint)(struct proc *p, user_addr_t stack_addr_hint);

	void *__unused_was_proc_get_return_to_kernel_offset;
	void (*proc_set_return_to_kernel_offset)(struct proc *t, uint64_t offset);

	void *__unused_was_workloop_fulfill_threadreq;
	void (*thread_will_park_or_terminate)(thread_t thread);

	void *__unused_was_qos_max_parallelism;

	/* proc_internal.h: struct proc user_stack accessor */
	user_addr_t (*proc_get_user_stack)(struct proc *p);
	void *__unused_was_proc_set_user_stack;

	/* padding for future */
	void* _pad[69];
} *pthread_callbacks_t;

void
pthread_kext_register(pthread_functions_t fns, pthread_callbacks_t *callbacks);

#ifdef BSD_KERNEL_PRIVATE
void thread_will_park_or_terminate(thread_t thread);
void pthread_init(void);
extern pthread_callbacks_t pthread_kern;
extern pthread_functions_t pthread_functions;
#endif

#endif /* ASSEMBLER */
#endif /* _PTHREAD_SHIMS_H_ */
#endif /* KERNEL_PRIVATE */
