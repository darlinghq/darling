/*
 * Copyright (c) 2000-2004 Apple Computer, Inc. All rights reserved.
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

#ifdef	PRIVATE

#ifndef	_MACH_SYSCALL_SW_H_
#define _MACH_SYSCALL_SW_H_

/*
 *	The machine-dependent "syscall_sw.h" file should
 *	define a macro for
 *		kernel_trap(trap_name, trap_number, arg_count)
 *	which will expand into assembly code for the
 *	trap.
 *
 *	N.B.: When adding calls, do not put spaces in the macros.
 */

#include <mach/machine/syscall_sw.h>

/*
 *	These trap numbers should be taken from the
 *	table in <kern/syscall_sw.c>.
 */

/* 
 * i386 and x86_64 just load of the stack or use
 * registers in order; no munging is required, 
 * and number of args is ignored.  ARM loads args
 * into registers beyond r3, unlike the normal 
 * procedure call standard; we pad for 64-bit args.
 */
kernel_trap(_kernelrpc_mach_vm_allocate_trap,-10,5) /* 4 args, +1 for mach_vm_size_t */
kernel_trap(_kernelrpc_mach_vm_deallocate_trap,-12,5) /* 3 args, +2 for mach_vm_size_t and mach_vm_address_t */
kernel_trap(_kernelrpc_mach_vm_protect_trap,-14,7) /* 5 args, +2 for mach_vm_address_t and mach_vm_size_t */
kernel_trap(_kernelrpc_mach_port_allocate_trap,-16,3)
kernel_trap(_kernelrpc_mach_port_destroy_trap,-17,2)
kernel_trap(_kernelrpc_mach_port_deallocate_trap,-18,2)
kernel_trap(_kernelrpc_mach_port_mod_refs_trap,-19,4)
kernel_trap(_kernelrpc_mach_port_move_member_trap,-20,3)
kernel_trap(_kernelrpc_mach_port_insert_right_trap,-21,4)
kernel_trap(_kernelrpc_mach_port_insert_member_trap,-22,3)
kernel_trap(_kernelrpc_mach_port_extract_member_trap,-23,3)

kernel_trap(mach_reply_port,-26,0)
kernel_trap(thread_self_trap,-27,0)
kernel_trap(task_self_trap,-28,0)
kernel_trap(host_self_trap,-29,0)

kernel_trap(mach_msg_trap,-31,7)
kernel_trap(mach_msg_overwrite_trap,-32,9)
kernel_trap(semaphore_signal_trap, -33, 1)
kernel_trap(semaphore_signal_all_trap, -34, 1)
kernel_trap(semaphore_signal_thread_trap, -35, 2)
kernel_trap(semaphore_wait_trap,-36,1)
kernel_trap(semaphore_wait_signal_trap,-37,2)
kernel_trap(semaphore_timedwait_trap,-38,3)
kernel_trap(semaphore_timedwait_signal_trap,-39,4)

#if		!defined(__LP64__) && !defined(__arm__)
kernel_trap(map_fd,-43,5)
#endif	/*!defined(__LP64__) && !defined(__arm__) */

kernel_trap(task_name_for_pid,-44,3)
kernel_trap(task_for_pid,-45,3)
kernel_trap(pid_for_task,-46,2)

#if defined(__LP64__)
kernel_trap(macx_swapon,-48, 4)
kernel_trap(macx_swapoff,-49, 2)
#else	/* __LP64__ */
kernel_trap(macx_swapon,-48, 5)
kernel_trap(macx_swapoff,-49, 3)
#endif	/* __LP64__ */
kernel_trap(macx_triggers,-51, 4)
kernel_trap(macx_backing_store_suspend,-52, 1)
kernel_trap(macx_backing_store_recovery,-53, 1)

/* These are currently used by pthreads even on LP64 */
/* But as soon as that is fixed - they will go away there */
kernel_trap(swtch_pri,-59,1)
kernel_trap(swtch,-60,0)

kernel_trap(syscall_thread_switch,-61,3)
kernel_trap(clock_sleep_trap,-62,5)

kernel_trap(mach_timebase_info,-89,1)

#if		defined(__LP64__)
/* unit64_t arguments passed in one register in LP64 */
kernel_trap(mach_wait_until,-90,1)
#else	/* __LP64__ */
kernel_trap(mach_wait_until,-90,2)
#endif	/* __LP64__ */

kernel_trap(mk_timer_create,-91,0)
kernel_trap(mk_timer_destroy,-92,1)

#if		defined(__LP64__)
/* unit64_t arguments passed in one register in LP64 */
kernel_trap(mk_timer_arm,-93,2)
#else	/* __LP64__ */
kernel_trap(mk_timer_arm,-93,3)
#endif	/* __LP64__ */

kernel_trap(mk_timer_cancel,-94,2)

/*
 * N.B: Trap #-100 is in use by IOTrap.s in the IOKit Framework
 * (iokit_user_client_trap)
 */
#endif	/* _MACH_SYSCALL_SW_H_ */

#endif	/* PRIVATE */
