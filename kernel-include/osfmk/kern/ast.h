/*
 * Copyright (c) 2000-2012 Apple Computer, Inc. All rights reserved.
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
 *      kern/ast.h: Definitions for Asynchronous System Traps.
 */

#ifndef _KERN_AST_H_
#define _KERN_AST_H_


#include <kern/assert.h>
#include <kern/macro_help.h>
#include <kern/spl.h>

/*
 * A processor detects an AST when it is about to return from an
 * interrupt context, and calls ast_taken_kernel or ast_taken_user
 * depending on whether it was returning from userspace or kernelspace.
 *
 * Machine-dependent code is responsible for maintaining
 * a set of reasons for an AST.
 */
typedef uint32_t ast_t;

/*
 * When returning from interrupt/trap context to kernel mode,
 * if AST_URGENT is set, then ast_taken_kernel is called, for
 * instance to effect preemption of a kernel thread by a realtime
 * thread.
 *
 * This is also done when re-enabling preemption or re-enabling
 * interrupts, since an AST may have been set while preemption
 * was disabled, and it should take effect as soon as possible.
 *
 * When returning from interrupt/trap/syscall context to user
 * mode, any and all ASTs that are pending should be handled by
 * calling ast_taken_user.
 *
 * If a thread context switches, only ASTs not in AST_PER_THREAD
 * remain active. The per-thread ASTs are stored in the thread_t
 * and re-enabled when the thread context switches back.
 *
 * Typically the preemption ASTs are set as a result of threads
 * becoming runnable, threads changing priority, or quantum
 * expiration. If a thread becomes runnable and is chosen
 * to run on another processor, cause_ast_check() may be called
 * to IPI that processor and request csw_check() be run there.
 */

/*
 *      Bits for reasons
 *      TODO: Split the context switch and return-to-user AST namespaces
 *      NOTE: Some of these are exported as the 'reason' code in scheduler tracepoints
 */
#define AST_PREEMPT             0x01
#define AST_QUANTUM             0x02
#define AST_URGENT              0x04
#define AST_HANDOFF             0x08
#define AST_YIELD               0x10
#define AST_APC                 0x20    /* migration APC hook */
#define AST_LEDGER              0x40
#define AST_BSD                 0x80
#define AST_KPERF               0x100   /* kernel profiling */
#define AST_MACF                0x200   /* MACF user ret pending */
#define AST_RESET_PCS           0x400   /* restartable ranges */
#define AST_ARCADE              0x800   /* arcade subsciption support */
#define AST_GUARD               0x1000
#define AST_TELEMETRY_USER      0x2000  /* telemetry sample requested on interrupt from userspace */
#define AST_TELEMETRY_KERNEL    0x4000  /* telemetry sample requested on interrupt from kernel */
#define AST_TELEMETRY_PMI       0x8000  /* telemetry sample requested on PMI */
#define AST_SFI                 0x10000 /* Evaluate if SFI wait is needed before return to userspace */
#define AST_DTRACE              0x20000
#define AST_TELEMETRY_IO        0x40000 /* telemetry sample requested for I/O */
#define AST_KEVENT              0x80000
#define AST_REBALANCE           0x100000 /* thread context switched due to rebalancing */
#define AST_UNQUIESCE           0x200000 /* catch unquiesced processor before returning to userspace */

#define AST_NONE                0x00
#define AST_ALL                 (~AST_NONE)

#define AST_SCHEDULING  (AST_PREEMPTION | AST_YIELD | AST_HANDOFF)
#define AST_PREEMPTION  (AST_PREEMPT | AST_QUANTUM | AST_URGENT)

#define AST_TELEMETRY_ALL (AST_TELEMETRY_USER | AST_TELEMETRY_KERNEL | \
	        AST_TELEMETRY_PMI | AST_TELEMETRY_IO)

/* Per-thread ASTs follow the thread at context-switch time. */
#define AST_PER_THREAD  (AST_APC | AST_BSD | AST_MACF | AST_RESET_PCS | \
	AST_ARCADE | AST_LEDGER | AST_GUARD | AST_TELEMETRY_ALL | AST_KEVENT)

/* Handle AST_URGENT detected while in the kernel */
extern void ast_taken_kernel(void);

/* Handle an AST flag set while returning to user mode (may continue via thread_exception_return) */
extern void ast_taken_user(void);

/* Check for pending ASTs */
extern void ast_check(processor_t processor);

/* Pending ast mask for the current processor */
extern ast_t *ast_pending(void);

/* Set AST flags on current processor */
extern void ast_on(ast_t reasons);

/* Clear AST flags on current processor */
extern void ast_off(ast_t reasons);

/* Consume specified AST flags from current processor */
extern ast_t ast_consume(ast_t reasons);

/* Read specified AST flags from current processor */
extern ast_t ast_peek(ast_t reasons);

/* Re-set current processor's per-thread AST flags to those set on thread */
extern void ast_context(thread_t thread);

/* Propagate ASTs set on a thread to the current processor */
extern void ast_propagate(thread_t thread);

/*
 *	Set an AST on a thread with thread_ast_set.
 *
 *	You can then propagate it to the current processor with ast_propagate(),
 *	or tell another processor to act on it with cause_ast_check().
 *
 *	See act_set_ast() for an example.
 */
#define thread_ast_set(act, reason)     ((void)os_atomic_or(&(act)->ast, (reason), relaxed))
#define thread_ast_clear(act, reason)   ((void)os_atomic_andnot(&(act)->ast, (reason), relaxed))

#ifdef MACH_BSD

extern void act_set_astbsd(thread_t);
extern void bsd_ast(thread_t);

#endif /* MACH_BSD */

#ifdef CONFIG_DTRACE
extern void ast_dtrace_on(void);
extern void dtrace_ast(void);
#endif /* CONFIG_DTRACE */

extern void kevent_ast(thread_t thread, uint16_t bits);
extern void act_set_astkevent(thread_t thread, uint16_t bits);
extern uint16_t act_clear_astkevent(thread_t thread, uint16_t bits);
extern void act_set_ast_reset_pcs(thread_t thread);

#endif  /* _KERN_AST_H_ */
