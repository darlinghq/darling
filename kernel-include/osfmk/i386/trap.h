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
 * Copyright (c) 1991,1990 Carnegie Mellon University
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

#ifndef _I386_TRAP_H_
#define _I386_TRAP_H_

/*
 * Hardware trap vectors for i386.
 */
#define T_DIVIDE_ERROR          0
#define T_DEBUG                 1
#define T_NMI                   2               /* non-maskable interrupt */
#define T_INT3                  3               /* int 3 instruction */
#define T_OVERFLOW              4               /* overflow test */
#define T_OUT_OF_BOUNDS         5               /* bounds check */
#define T_INVALID_OPCODE        6               /* invalid op code */
#define T_NO_FPU                7               /* no floating point */
#define T_DOUBLE_FAULT          8               /* double fault */
#define T_FPU_FAULT             9
#define T_INVALID_TSS           10
#define T_SEGMENT_NOT_PRESENT   11
#define T_STACK_FAULT           12
#define T_GENERAL_PROTECTION    13
#define T_PAGE_FAULT            14
/*				15 */
#define T_FLOATING_POINT_ERROR  16
#define T_WATCHPOINT            17
#define T_MACHINE_CHECK         18
#define T_SSE_FLOAT_ERROR       19
/*                          20-126 */
#define T_DTRACE_RET            127

/* The SYSENTER and SYSCALL trap numbers are software constructs.
 * These exceptions are dispatched directly to the system call handlers.
 * See also the "software interrupt codes" section of
 * osfmk/mach/i386/syscall_sw.h
 */
#define T_SYSENTER              0x84
#define T_SYSCALL               0x85

#define T_PREEMPT               255

#define TRAP_NAMES "divide error", "debug trap", "NMI", "breakpoint", \
	           "overflow", "bounds check", "invalid opcode", \
	           "no coprocessor", "double fault", "coprocessor overrun", \
	           "invalid TSS", "segment not present", "stack bounds", \
	           "general protection", "page fault", "(reserved)", \
	           "coprocessor error", "watchpoint", "machine check", "SSE floating point"

/*
 * Page-fault trap codes.
 */
#define T_PF_PROT               0x1             /* protection violation */
#define T_PF_WRITE              0x2             /* write access */
#define T_PF_USER               0x4             /* from user state */

#define T_PF_RSVD               0x8             /* reserved bit set to 1 */
#define T_PF_EXECUTE            0x10            /* instruction fetch when NX */

#if !defined(ASSEMBLER) && defined(MACH_KERNEL)

#include <i386/thread.h>

extern void             i386_exception(
	int                     exc,
	mach_exception_code_t   code,
	mach_exception_subcode_t subcode);

extern void             sync_iss_to_iks(x86_saved_state_t *regs);

extern void             sync_iss_to_iks_unconditionally(
	x86_saved_state_t       *regs);

extern void             kernel_trap(x86_saved_state_t *regs, uintptr_t *lo_spp);

extern void             user_trap(x86_saved_state_t *regs);

extern void             interrupt(x86_saved_state_t *regs);

extern void             panic_double_fault64(x86_saved_state_t *regs) __abortlike;
extern void             panic_machine_check64(x86_saved_state_t *regs) __abortlike;

typedef kern_return_t (*perfCallback)(
	int                     trapno,
	void                    *regs,
	uintptr_t               *lo_spp,
	int);

extern void             panic_i386_backtrace(void *, int, const char *, boolean_t, x86_saved_state_t *);
extern void     print_one_backtrace(pmap_t pmap, vm_offset_t topfp, const char *cur_marker, boolean_t is_64_bit);
extern void     print_thread_num_that_crashed(task_t task);
extern void     print_tasks_user_threads(task_t task);
extern void     print_threads_registers(thread_t thread);
extern void     print_uuid_info(task_t task);
extern void     print_launchd_info(void);

#if MACH_KDP
extern boolean_t        kdp_i386_trap(
	unsigned int,
	x86_saved_state64_t *,
	kern_return_t,
	vm_offset_t);
#endif /* MACH_KDP */
#endif  /* !ASSEMBLER && MACH_KERNEL */

#endif  /* _I386_TRAP_H_ */
