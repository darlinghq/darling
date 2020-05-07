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

#ifndef _KERN_COUNTERS_
#define _KERN_COUNTERS_

#include <mach_counters.h>

/*
 *	We can count various interesting events and paths.
 *
 *	Use counter() to change the counters, eg:
 *		counter(c_idle_thread_block++);
 *	Use counter_always() for non-conditional counters.
 */

#define counter_always(code)    code

#if     MACH_COUNTERS

#define counter(code)           counter_always(code)

#else   /* MACH_COUNTERS */

#define counter(code)

#endif  /* MACH_COUNTERS */

/*
 *	We define the counters with individual integers,
 *	instead of a big structure, so that ddb
 *	will know the addresses of the counters.
 */

typedef unsigned int mach_counter_t;

#if     MACH_COUNTERS
extern mach_counter_t c_action_thread_block;
extern mach_counter_t c_ast_taken_block;
extern mach_counter_t c_dev_io_blocks;
extern mach_counter_t c_dev_io_tries;
extern mach_counter_t c_idle_thread_block;
extern mach_counter_t c_idle_thread_handoff;
extern mach_counter_t c_incoming_interrupts;
extern mach_counter_t c_io_done_thread_block;
extern mach_counter_t c_ipc_mqueue_receive_block_kernel;
extern mach_counter_t c_ipc_mqueue_receive_block_user;
extern mach_counter_t c_ipc_mqueue_send_block;
extern mach_counter_t c_net_thread_block;
extern mach_counter_t c_reaper_thread_block;
extern mach_counter_t c_sched_thread_block;
extern mach_counter_t c_stacks_current;
extern mach_counter_t c_stacks_max;
extern mach_counter_t c_stacks_min;
extern mach_counter_t c_swtch_block;
extern mach_counter_t c_swtch_pri_block;
extern mach_counter_t c_syscalls_unix;
extern mach_counter_t c_syscalls_mach;
extern mach_counter_t c_thread_invoke_csw;
extern mach_counter_t c_thread_invoke_same;
extern mach_counter_t c_thread_invoke_same_cont;
extern mach_counter_t c_thread_invoke_misses;
extern mach_counter_t c_thread_invoke_hits;
extern mach_counter_t c_thread_switch_block;
extern mach_counter_t c_thread_switch_handoff;
extern mach_counter_t c_vm_fault_page_block_backoff_kernel;
extern mach_counter_t c_vm_fault_page_block_busy_kernel;
extern mach_counter_t c_vm_fault_retry_on_w_prot;
extern mach_counter_t c_vm_fault_wait_on_unlock;
extern mach_counter_t c_vm_map_simplified;
extern mach_counter_t c_vm_map_simplify_called;
extern mach_counter_t c_vm_map_simplify_entry_called;
extern mach_counter_t c_vm_page_wait_block;
extern mach_counter_t c_vm_pageout_block;
extern mach_counter_t c_vm_pageout_scan_block;
#endif  /* MACH_COUNTERS */

#endif  /* _KERN_COUNTERS_ */
