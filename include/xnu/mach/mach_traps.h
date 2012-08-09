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
/*
 *	Definitions of general Mach system traps.
 *
 *	These are the definitions as seen from user-space.
 *	The kernel definitions are in <mach/syscall_sw.h>.
 *	Kernel RPC functions are defined in <mach/mach_interface.h>.
 */

#ifndef	_MACH_MACH_TRAPS_H_
#define _MACH_MACH_TRAPS_H_

#include <stdint.h>

#include <mach/std_types.h>
#include <mach/mach_types.h>
#include <mach/kern_return.h>
#include <mach/port.h>
#include <mach/vm_types.h>
#include <mach/clock_types.h>

#include <machine/endian.h>

#include <sys/cdefs.h>

__BEGIN_DECLS

#ifndef	KERNEL

#ifdef	PRIVATE

extern mach_port_name_t mach_reply_port(void);

extern mach_port_name_t thread_self_trap(void);

extern mach_port_name_t host_self_trap(void);

extern mach_msg_return_t mach_msg_trap(
				mach_msg_header_t *msg,
				mach_msg_option_t option,
				mach_msg_size_t send_size,
				mach_msg_size_t rcv_size,
				mach_port_name_t rcv_name,
				mach_msg_timeout_t timeout,
				mach_port_name_t notify);

extern mach_msg_return_t mach_msg_overwrite_trap(
				mach_msg_header_t *msg,
				mach_msg_option_t option,
				mach_msg_size_t send_size,
				mach_msg_size_t rcv_size,
				mach_port_name_t rcv_name,
				mach_msg_timeout_t timeout,
				mach_port_name_t notify,
				mach_msg_header_t *rcv_msg,
				mach_msg_size_t rcv_limit);

extern kern_return_t semaphore_signal_trap(
				mach_port_name_t signal_name);
					      
extern kern_return_t semaphore_signal_all_trap(
				mach_port_name_t signal_name);

extern kern_return_t semaphore_signal_thread_trap(
				mach_port_name_t signal_name,
				mach_port_name_t thread_name);

extern kern_return_t semaphore_wait_trap(
				mach_port_name_t wait_name);

extern kern_return_t semaphore_wait_signal_trap(
				mach_port_name_t wait_name,
				mach_port_name_t signal_name);

extern kern_return_t semaphore_timedwait_trap(
				mach_port_name_t wait_name,
				unsigned int sec,
				clock_res_t nsec);

extern kern_return_t semaphore_timedwait_signal_trap(
				mach_port_name_t wait_name,
				mach_port_name_t signal_name,
				unsigned int sec,
				clock_res_t nsec);

#endif	/* PRIVATE */

extern kern_return_t clock_sleep_trap(
				mach_port_name_t clock_name,
				sleep_type_t sleep_type,
				int sleep_sec,
				int sleep_nsec,
				mach_timespec_t	*wakeup_time);

extern kern_return_t _kernelrpc_mach_vm_allocate_trap(
				mach_port_name_t target,
				mach_vm_offset_t *addr,
				mach_vm_size_t size,
				int flags);

extern kern_return_t _kernelrpc_mach_vm_deallocate_trap(
				mach_port_name_t target,
				mach_vm_address_t address,
				mach_vm_size_t size
);

extern kern_return_t _kernelrpc_mach_vm_protect_trap(
				mach_port_name_t target,
				mach_vm_address_t address,
				mach_vm_size_t size,
				boolean_t set_maximum,
				vm_prot_t new_protection
);

extern kern_return_t _kernelrpc_mach_port_allocate_trap(
				mach_port_name_t target,
				mach_port_right_t right,
				mach_port_name_t *name
);


extern kern_return_t _kernelrpc_mach_port_destroy_trap(
				mach_port_name_t target,
				mach_port_name_t name
);

extern kern_return_t _kernelrpc_mach_port_deallocate_trap(
				mach_port_name_t target,
				mach_port_name_t name
);

extern kern_return_t _kernelrpc_mach_port_mod_refs_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_right_t right,
				mach_port_delta_t delta
);

extern kern_return_t _kernelrpc_mach_port_move_member_trap(
				mach_port_name_t target,
				mach_port_name_t member,
				mach_port_name_t after
);

extern kern_return_t _kernelrpc_mach_port_insert_right_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_name_t poly,
				mach_msg_type_name_t polyPoly
);

extern kern_return_t _kernelrpc_mach_port_insert_member_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_name_t pset
);

extern kern_return_t _kernelrpc_mach_port_extract_member_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_name_t pset
);

extern kern_return_t macx_swapon(
				uint64_t filename,
				int flags,
				int size,
				int priority);

extern kern_return_t macx_swapoff(
				uint64_t filename,
				int flags);

extern kern_return_t macx_triggers(
				int hi_water,
				int low_water,
				int flags,
				mach_port_t alert_port);

extern kern_return_t macx_backing_store_suspend(
				boolean_t suspend);

extern kern_return_t macx_backing_store_recovery(
				int pid);

extern boolean_t swtch_pri(int pri);

extern boolean_t swtch(void);

extern kern_return_t thread_switch(
				mach_port_name_t thread_name,
				int option,
				mach_msg_timeout_t option_time);

extern mach_port_name_t task_self_trap(void);

/*
 *	Obsolete interfaces.
 */

extern kern_return_t task_for_pid(
				mach_port_name_t target_tport,
				int pid,
				mach_port_name_t *t);

extern kern_return_t task_name_for_pid(
				mach_port_name_t target_tport,
				int pid,
				mach_port_name_t *tn);

extern kern_return_t pid_for_task(
				mach_port_name_t t,
				int *x);

#if		!defined(__LP64__) && !defined(__arm__)
/* these should go away altogether - so no 64 legacy please */

extern kern_return_t map_fd(
				int fd,
				vm_offset_t offset,
				vm_offset_t *va,
				boolean_t findspace,
				vm_size_t size);

#endif	/* !defined(__LP64__) && !defined(__arm__) */

#else	/* KERNEL */

#ifdef	XNU_KERNEL_PRIVATE

/* Syscall data translations routines
 *
 * The kernel may support multiple userspace ABIs, and must use
 * argument structures with elements large enough for any of them.
 */
#define	PAD_(t)	(sizeof(uint64_t) <= sizeof(t) \
 		? 0 : sizeof(uint64_t) - sizeof(t))
#define PAD_ARG_8

#if BYTE_ORDER == LITTLE_ENDIAN
#define	PADL_(t)	0
#define	PADR_(t)	PAD_(t)
#else
#define	PADL_(t)	PAD_(t)
#define	PADR_(t)	0
#endif

#define PAD_ARG_(arg_type, arg_name) \
  char arg_name##_l_[PADL_(arg_type)]; arg_type arg_name; char arg_name##_r_[PADR_(arg_type)];

/*
 * To support 32-bit clients as well as 64-bit clients, argument
 * structures may need to be munged to repack the arguments. All
 * active architectures do this inline in the code to dispatch Mach
 * traps, without calling out to the BSD system call mungers.
 */

#if 0 /* no active architectures use this */
void munge_w(const void *, void *);  
void munge_ww(const void *, void *);  
void munge_www(const void *, void *);  
void munge_wwww(const void *, void *);  
void munge_wwwww(const void *, void *);  
void munge_wwwwww(const void *, void *);  
void munge_wwwwwww(const void *, void *);  
void munge_wwwwwwww(const void *, void *);  
void munge_d(const void *, void *);  
void munge_dd(const void *, void *);  
void munge_ddd(const void *, void *);  
void munge_dddd(const void *, void *);  
void munge_ddddd(const void *, void *);  
void munge_dddddd(const void *, void *);  
void munge_ddddddd(const void *, void *);  
void munge_dddddddd(const void *, void *);
void munge_l(const void *, void *);
void munge_lw(const void *, void *);
void munge_lwww(const void *, void *);
void munge_wl(const void *, void *);  
void munge_wlw(const void *, void *);  
void munge_wwwl(const void *, void *);  
void munge_wwwwl(const void *, void *);  
void munge_wwwwwl(const void *, void *);  
#endif /* 0 */

struct kern_invalid_args {
	int32_t dummy;
};
extern kern_return_t kern_invalid(
				struct kern_invalid_args *args);

struct mach_reply_port_args {
	int32_t dummy;
};
extern mach_port_name_t mach_reply_port(
				struct mach_reply_port_args *args);

struct thread_self_trap_args {
	int32_t dummy;
};
extern mach_port_name_t thread_self_trap(
				struct thread_self_trap_args *args);

struct task_self_trap_args {
	int32_t dummy;
};
extern mach_port_name_t task_self_trap(
				struct task_self_trap_args *args);

struct host_self_trap_args {
	int32_t dummy;
};
extern mach_port_name_t host_self_trap(
				struct host_self_trap_args *args);

struct mach_msg_overwrite_trap_args {
	PAD_ARG_(user_addr_t, msg);
	PAD_ARG_(mach_msg_option_t, option);
	PAD_ARG_(mach_msg_size_t, send_size);
	PAD_ARG_(mach_msg_size_t, rcv_size);
	PAD_ARG_(mach_port_name_t, rcv_name);
	PAD_ARG_(mach_msg_timeout_t, timeout);
	PAD_ARG_(mach_port_name_t, notify);
	PAD_ARG_8
	PAD_ARG_(user_addr_t, rcv_msg);  /* Unused on mach_msg_trap */
};
extern mach_msg_return_t mach_msg_trap(
				struct mach_msg_overwrite_trap_args *args);
extern mach_msg_return_t mach_msg_overwrite_trap(
				struct mach_msg_overwrite_trap_args *args);

struct semaphore_signal_trap_args {
	PAD_ARG_(mach_port_name_t, signal_name);
};
extern kern_return_t semaphore_signal_trap(
				struct semaphore_signal_trap_args *args);
					      
struct semaphore_signal_all_trap_args {
	PAD_ARG_(mach_port_name_t, signal_name);
};
extern kern_return_t semaphore_signal_all_trap(
				struct semaphore_signal_all_trap_args *args);

struct semaphore_signal_thread_trap_args {
	PAD_ARG_(mach_port_name_t, signal_name);
	PAD_ARG_(mach_port_name_t, thread_name);
};
extern kern_return_t semaphore_signal_thread_trap(
				struct semaphore_signal_thread_trap_args *args);

struct semaphore_wait_trap_args {
	PAD_ARG_(mach_port_name_t, wait_name);
};
extern kern_return_t semaphore_wait_trap(
				struct semaphore_wait_trap_args *args);

struct semaphore_wait_signal_trap_args {
	PAD_ARG_(mach_port_name_t, wait_name);
	PAD_ARG_(mach_port_name_t, signal_name);
};
extern kern_return_t semaphore_wait_signal_trap(
				struct semaphore_wait_signal_trap_args *args);

struct semaphore_timedwait_trap_args {
	PAD_ARG_(mach_port_name_t, wait_name);
	PAD_ARG_(unsigned int, sec);
	PAD_ARG_(clock_res_t, nsec);
};
extern kern_return_t semaphore_timedwait_trap(
				struct semaphore_timedwait_trap_args *args);

struct semaphore_timedwait_signal_trap_args {
	PAD_ARG_(mach_port_name_t, wait_name);
	PAD_ARG_(mach_port_name_t, signal_name);
	PAD_ARG_(unsigned int, sec);
	PAD_ARG_(clock_res_t, nsec);
};
extern kern_return_t semaphore_timedwait_signal_trap(
				struct semaphore_timedwait_signal_trap_args *args);

#if		!defined(CONFIG_EMBEDDED)
struct map_fd_args {
	PAD_ARG_(int, fd);
	PAD_ARG_(vm_offset_t, offset);
	PAD_ARG_(vm_offset_t *, va);
	PAD_ARG_(boolean_t, findspace);
	PAD_ARG_(vm_size_t, size);
};
extern kern_return_t map_fd(
				struct map_fd_args *args);
#endif	/* !defined(CONFIG_EMBEDDED) */

struct task_for_pid_args {
	PAD_ARG_(mach_port_name_t, target_tport);
	PAD_ARG_(int, pid);
	PAD_ARG_(user_addr_t, t);
};
extern kern_return_t task_for_pid(
				struct task_for_pid_args *args);

struct task_name_for_pid_args {
	PAD_ARG_(mach_port_name_t, target_tport);
	PAD_ARG_(int, pid);
	PAD_ARG_(user_addr_t, t);
};
extern kern_return_t task_name_for_pid(
				struct task_name_for_pid_args *args);

struct pid_for_task_args {
	PAD_ARG_(mach_port_name_t, t);
	PAD_ARG_(user_addr_t, pid);
};
extern kern_return_t pid_for_task(
				struct pid_for_task_args *args);

struct macx_swapon_args {
	PAD_ARG_(uint64_t, filename);
	PAD_ARG_(int, flags);
	PAD_ARG_(int, size);
	PAD_ARG_(int, priority);
};
extern kern_return_t macx_swapon(
				struct macx_swapon_args *args);

struct macx_swapoff_args {
    PAD_ARG_(uint64_t, filename);
    PAD_ARG_(int, flags);
};
extern kern_return_t macx_swapoff(
				struct macx_swapoff_args *args);

struct macx_triggers_args {
	PAD_ARG_(int, hi_water);
	PAD_ARG_(int, low_water);
	PAD_ARG_(int, flags);
	PAD_ARG_(mach_port_t, alert_port);
};
extern kern_return_t macx_triggers(
				struct macx_triggers_args *args);

struct macx_backing_store_suspend_args {
	PAD_ARG_(boolean_t, suspend);
};
extern kern_return_t macx_backing_store_suspend(
				struct macx_backing_store_suspend_args *args);

struct macx_backing_store_recovery_args {
	PAD_ARG_(int, pid);
};
extern kern_return_t macx_backing_store_recovery(
				struct macx_backing_store_recovery_args *args);

struct swtch_pri_args {
	PAD_ARG_(int, pri);
};
extern boolean_t swtch_pri(
				struct swtch_pri_args *args);

struct pfz_exit_args {
	int32_t dummy;
};
extern kern_return_t pfz_exit(
				struct pfz_exit_args *args);

struct swtch_args {
    int32_t dummy;
};
extern boolean_t swtch(
				struct swtch_args *args);

struct clock_sleep_trap_args{
	PAD_ARG_(mach_port_name_t, clock_name);
	PAD_ARG_(sleep_type_t, sleep_type);
	PAD_ARG_(int, sleep_sec);
	PAD_ARG_(int, sleep_nsec);
	PAD_ARG_(user_addr_t, wakeup_time);
};
extern kern_return_t clock_sleep_trap(
				struct clock_sleep_trap_args *args);

struct thread_switch_args {
	PAD_ARG_(mach_port_name_t, thread_name);
	PAD_ARG_(int, option);
	PAD_ARG_(mach_msg_timeout_t, option_time);
};
extern kern_return_t thread_switch(
				struct thread_switch_args *args);

struct mach_timebase_info_trap_args {
	PAD_ARG_(user_addr_t, info);
};
extern kern_return_t mach_timebase_info_trap(
				struct mach_timebase_info_trap_args *args);

struct mach_wait_until_trap_args {
	PAD_ARG_(uint64_t, deadline);
};
extern kern_return_t mach_wait_until_trap(
				struct mach_wait_until_trap_args *args);

struct mk_timer_create_trap_args {
    int32_t dummy;
};
extern mach_port_name_t mk_timer_create_trap(
				struct mk_timer_create_trap_args *args);

struct mk_timer_destroy_trap_args {
	PAD_ARG_(mach_port_name_t, name);
};
extern kern_return_t mk_timer_destroy_trap(
				struct mk_timer_destroy_trap_args *args);

struct mk_timer_arm_trap_args {
	PAD_ARG_(mach_port_name_t, name);
	PAD_ARG_(uint64_t, expire_time);
};
extern kern_return_t mk_timer_arm_trap(
				struct mk_timer_arm_trap_args *args);

struct mk_timer_cancel_trap_args {
    PAD_ARG_(mach_port_name_t, name);
    PAD_ARG_(user_addr_t, result_time);
};
extern kern_return_t mk_timer_cancel_trap(
				struct mk_timer_cancel_trap_args *args);

struct _kernelrpc_mach_vm_allocate_trap_args {
	PAD_ARG_(mach_port_name_t, target);	/* 1 word */
	PAD_ARG_(user_addr_t, addr);		/* 1 word */
	PAD_ARG_(mach_vm_size_t, size);		/* 2 words */
	PAD_ARG_(int, flags);			/* 1 word */
};						/* Total: 5 */

extern kern_return_t _kernelrpc_mach_vm_allocate_trap(
				struct _kernelrpc_mach_vm_allocate_trap_args *args);

struct _kernelrpc_mach_vm_deallocate_args {
	PAD_ARG_(mach_port_name_t, target);	/* 1 word */
	PAD_ARG_(mach_vm_address_t, address);	/* 2 words */
	PAD_ARG_(mach_vm_size_t, size);		/* 2 words */
};						/* Total: 5 */
extern kern_return_t _kernelrpc_mach_vm_deallocate_trap(
				struct _kernelrpc_mach_vm_deallocate_args *args);

struct _kernelrpc_mach_vm_protect_args {
	PAD_ARG_(mach_port_name_t, target);	/* 1 word */
	PAD_ARG_(mach_vm_address_t, address);	/* 2 words */
	PAD_ARG_(mach_vm_size_t, size);		/* 2 words */
	PAD_ARG_(boolean_t, set_maximum);	/* 1 word */
	PAD_ARG_(vm_prot_t, new_protection);	/* 1 word */
};						/* Total: 7 */
extern kern_return_t _kernelrpc_mach_vm_protect_trap(
				struct _kernelrpc_mach_vm_protect_args *args);

struct _kernelrpc_mach_port_allocate_args {
	PAD_ARG_(mach_port_name_t, target);
	PAD_ARG_(mach_port_right_t, right);
	PAD_ARG_(user_addr_t, name);
};
extern kern_return_t _kernelrpc_mach_port_allocate_trap(
				struct _kernelrpc_mach_port_allocate_args *args);


struct _kernelrpc_mach_port_destroy_args {
	PAD_ARG_(mach_port_name_t, target);
	PAD_ARG_(mach_port_name_t, name);
};
extern kern_return_t _kernelrpc_mach_port_destroy_trap(
				struct _kernelrpc_mach_port_destroy_args *args);

struct _kernelrpc_mach_port_deallocate_args {
	PAD_ARG_(mach_port_name_t, target);
	PAD_ARG_(mach_port_name_t, name);
};
extern kern_return_t _kernelrpc_mach_port_deallocate_trap(
				struct _kernelrpc_mach_port_deallocate_args *args);

struct _kernelrpc_mach_port_mod_refs_args {
	PAD_ARG_(mach_port_name_t, target);
	PAD_ARG_(mach_port_name_t, name);
	PAD_ARG_(mach_port_right_t, right);
	PAD_ARG_(mach_port_delta_t, delta);
};
extern kern_return_t _kernelrpc_mach_port_mod_refs_trap(
				struct _kernelrpc_mach_port_mod_refs_args *args);

struct _kernelrpc_mach_port_move_member_args {
	PAD_ARG_(mach_port_name_t, target);
	PAD_ARG_(mach_port_name_t, member);
	PAD_ARG_(mach_port_name_t, after);
};
extern kern_return_t _kernelrpc_mach_port_move_member_trap(
				struct _kernelrpc_mach_port_move_member_args *args);

struct _kernelrpc_mach_port_insert_right_args {
	PAD_ARG_(mach_port_name_t, target);
	PAD_ARG_(mach_port_name_t, name);
	PAD_ARG_(mach_port_name_t, poly);
	PAD_ARG_(mach_msg_type_name_t, polyPoly);
};
extern kern_return_t _kernelrpc_mach_port_insert_right_trap(
				struct _kernelrpc_mach_port_insert_right_args *args);

struct _kernelrpc_mach_port_insert_member_args {
	PAD_ARG_(mach_port_name_t, target);
	PAD_ARG_(mach_port_name_t, name);
	PAD_ARG_(mach_port_name_t, pset);
};
extern kern_return_t _kernelrpc_mach_port_insert_member_trap(
				struct _kernelrpc_mach_port_insert_member_args *args);

struct _kernelrpc_mach_port_extract_member_args {
	PAD_ARG_(mach_port_name_t, target);
	PAD_ARG_(mach_port_name_t, name);
	PAD_ARG_(mach_port_name_t, pset);
};
extern kern_return_t _kernelrpc_mach_port_extract_member_trap(
				struct _kernelrpc_mach_port_extract_member_args *args);

/* not published to LP64 clients yet */
struct iokit_user_client_trap_args {
	PAD_ARG_(void *, userClientRef);
	PAD_ARG_(uint32_t, index);
	PAD_ARG_(void *, p1);
	PAD_ARG_(void *, p2);
	PAD_ARG_(void *, p3);
	PAD_ARG_(void *, p4);
	PAD_ARG_(void *, p5);
    	PAD_ARG_8
	PAD_ARG_(void *, p6);
};
kern_return_t iokit_user_client_trap(
				struct iokit_user_client_trap_args *args);

#undef PAD_
#undef PADL_
#undef PADR_
#undef PAD_ARG_
#undef PAD_ARG_8

#endif	/* XNU_KERNEL_PRIVATE */

#endif	/* KERNEL */

__END_DECLS

#endif	/* _MACH_MACH_TRAPS_H_ */
