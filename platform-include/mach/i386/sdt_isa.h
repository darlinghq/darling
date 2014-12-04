/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef _MACH_I386_SDT_ISA_H
#define	_MACH_I386_SDT_ISA_H

/*
 * Only define when testing.  This makes the calls into actual calls to
 * test functions.
 */
/* #define DTRACE_CALL_TEST */

#define DTRACE_STRINGIFY(s) #s
#define DTRACE_TOSTRING(s) DTRACE_STRINGIFY(s)
#define DTRACE_LABEL(p, n)									\
	"__dtrace_probe$" DTRACE_TOSTRING(%=__LINE__) DTRACE_STRINGIFY(_##p##___##n) ":"	"\n\t"

#ifdef DTRACE_CALL_TEST

#define DTRACE_CALL(p,n)	\
	DTRACE_LABEL(p,n)	\
	DTRACE_CALL_INSN(p,n)

#else

#define DTRACE_CALL(p,n)	\
	DTRACE_LABEL(p,n)	\
	DTRACE_NOPS

#endif

#ifdef __x86_64__

#define DTRACE_NOPS			\
	"nop"			"\n\t"	\
	"nop"			"\n\t"	\
	"nop"			"\n\t"	

#define DTRACE_CALL_INSN(p,n)						\
	"call _dtracetest" DTRACE_STRINGIFY(_##p##_##n)	"\n\t"

#define ARG1_EXTENT	1
#define ARGS2_EXTENT	2
#define ARGS3_EXTENT	3
#define ARGS4_EXTENT	4
#define ARGS5_EXTENT	5
#define ARGS6_EXTENT	6
#define ARGS7_EXTENT	7
#define ARGS8_EXTENT	8
#define ARGS9_EXTENT	9
#define ARGS10_EXTENT	10

#define DTRACE_CALL0ARGS(provider, name)							\
	asm volatile (										\
		      DTRACE_CALL(provider, name)						\
	              :										\
	              :										\
	);

#define DTRACE_CALL1ARG(provider, name)								\
	asm volatile ("movq\t0x0(%0),%%rdi"						"\n\t"	\
		      DTRACE_CALL(provider, name)						\
	              :										\
	              : "r" (__dtrace_args)							\
	              : "memory", "rdi"								\
	);

#define DTRACE_CALL2ARGS(provider, name)							\
	asm volatile ("movq\t0x0(%0),%%rdi"						"\n\t"	\
	              "movq\t0x8(%0),%%rsi"						"\n\t"	\
		      DTRACE_CALL(provider, name)						\
	              :										\
	              : "r" (__dtrace_args)							\
	              : "memory", "rdi", "rsi"							\
	);

#define DTRACE_CALL3ARGS(provider, name)							\
	asm volatile ("movq\t0x0(%0),%%rdi"						"\n\t"	\
	              "movq\t0x8(%0),%%rsi"						"\n\t"	\
	              "movq\t0x10(%0),%%rdx"						"\n\t"	\
		      DTRACE_CALL(provider, name)						\
	              :										\
	              : "r" (__dtrace_args)							\
	              : "memory", "rdi", "rsi", "rdx"						\
	);

#define DTRACE_CALL4ARGS(provider, name)							\
	asm volatile ("movq\t0x0(%0),%%rdi"						"\n\t"	\
	              "movq\t0x8(%0),%%rsi"						"\n\t"	\
	              "movq\t0x10(%0),%%rdx"						"\n\t"	\
	              "movq\t0x18(%0),%%rcx"						"\n\t"	\
		      DTRACE_CALL(provider, name)						\
	              :										\
	              : "r" (__dtrace_args)							\
	              : "memory", "rdi", "rsi", "rdx", "rcx"					\
	);

#define DTRACE_CALL5ARGS(provider, name)							\
	asm volatile ("movq\t0x0(%0),%%rdi"						"\n\t"	\
	              "movq\t0x8(%0),%%rsi"						"\n\t"	\
	              "movq\t0x10(%0),%%rdx"						"\n\t"	\
	              "movq\t0x18(%0),%%rcx"						"\n\t"	\
	              "movq\t0x20(%0),%%r8"						"\n\t"	\
		      DTRACE_CALL(provider, name)						\
	              :										\
	              : "r" (__dtrace_args)							\
	              : "memory", "rdi", "rsi", "rdx", "rcx", "r8"				\
	);

#define DTRACE_CALL6ARGS(provider, name)							\
	asm volatile ("movq\t0x0(%0),%%rdi"						"\n\t"	\
	              "movq\t0x8(%0),%%rsi"						"\n\t"	\
	              "movq\t0x10(%0),%%rdx"						"\n\t"	\
	              "movq\t0x18(%0),%%rcx"						"\n\t"	\
	              "movq\t0x20(%0),%%r8"						"\n\t"	\
	              "movq\t0x28(%0),%%r9"						"\n\t"	\
		      DTRACE_CALL(provider, name)						\
	              :										\
	              : "r" (__dtrace_args)							\
	              : "memory", "rdi", "rsi", "rdx", "rcx", "r8", "r9"			\
	);

#define DTRACE_CALL7ARGS(provider, name)							\
	asm volatile ("subq\t$0x8,%%rsp"						"\n\t"	\
	              "movq\t0x0(%0),%%rdi"						"\n\t"	\
	              "movq\t0x8(%0),%%rsi"						"\n\t"	\
	              "movq\t0x10(%0),%%rdx"						"\n\t"	\
	              "movq\t0x18(%0),%%rcx"						"\n\t"	\
	              "movq\t0x20(%0),%%r8"						"\n\t"	\
	              "movq\t0x28(%0),%%r9"						"\n\t"	\
	              "movq\t0x30(%0),%%rax"						"\n\t"	\
	              "movq\t%%rax,0x0(%%rsp)"						"\n\t"	\
		      DTRACE_CALL(provider, name)						\
	              "addq\t$0x8,%%rsp"						"\n\t"	\
	              :										\
	              : "r" (__dtrace_args)							\
	              : "memory", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "rax"			\
	);

#endif // __x86_64__

#ifdef __i386__

#define DTRACE_NOPS			\
	"nop"			"\n\t"	\
	"leal 0(%%esi), %%esi"	"\n\t"	

#define DTRACE_CALL_INSN(p,n)						\
	"call _dtracetest" DTRACE_STRINGIFY(_##p##_##n)	"\n\t"

#define ARG1_EXTENT	1
#define ARGS2_EXTENT	2
#define ARGS3_EXTENT	4
#define ARGS4_EXTENT	4
#define ARGS5_EXTENT	8
#define ARGS6_EXTENT	8
#define ARGS7_EXTENT	8
#define ARGS8_EXTENT	8
#define ARGS9_EXTENT	12	
#define ARGS10_EXTENT	12	

/*
 * Because this code is used in the kernel, we must not touch any floating point
 * or specialized registers. This leaves the following registers:
 *
 * eax ; volatile, safe to use
 * ebx ; PIC register, gcc error when used
 * ecx ; volatile, safe to use
 * edx ; volatile, safe to use
 * esi ; non-volatile, otherwise safe to use
 * edi ; non-volatile, otherwise safe to use
 *
 * Using any of the non volatile register causes a spill to stack which is almost
 * certainly a net performance loss. Also, note that the array ref (__dtrace_args)
 * consumes one free register. If all three of the volatile regs are used for load/store,
 * the compiler will spill a register to hold the array ref.
 *
 * The end result is that we only pipeline two loads/stores at a time. Blech.
 */

#define DTRACE_CALL0ARGS(provider, name)							\
	asm volatile (										\
	              DTRACE_CALL(provider, name)						\
	              "# eat trailing nl +tabfrom DTRACE_CALL"					\
	              :										\
	              :										\
	);

#define DTRACE_CALL1ARG(provider, name)								\
	asm volatile ("subl\t$0x10,%%esp"						"\n\t"	\
	              "movl\t0x0(%0),%%eax"						"\n\t"	\
	              "movl\t%%eax,0x0(%%esp)"						"\n\t"	\
	              DTRACE_CALL(provider, name)						\
                      "addl\t$0x10,%%esp"							\
	              :										\
	              : "r" (__dtrace_args)		       					\
	              : "memory", "eax"								\
	);

#define DTRACE_CALL2ARGS(provider, name)							\
	asm volatile ("subl\t$0x10,%%esp"						"\n\t"	\
	              "movl\t0x0(%0),%%eax"						"\n\t"	\
		      "movl\t0x4(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x0(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x4(%%esp)"						"\n\t"	\
	              DTRACE_CALL(provider, name)						\
                      "addl\t$0x10,%%esp"							\
	              :										\
	              : "r" (__dtrace_args)							\
	              : "memory", "eax", "edx"							\
	);

#define DTRACE_CALL3ARGS(provider, name)							\
	asm volatile ("subl\t$0x10,%%esp"						"\n\t"	\
	              "movl\t0x0(%0),%%eax"						"\n\t"	\
		      "movl\t0x4(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x0(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x4(%%esp)"						"\n\t"	\
	              "movl\t0x8(%0),%%eax"						"\n\t"	\
	              "movl\t%%eax,0x8(%%esp)"						"\n\t"	\
	              DTRACE_CALL(provider, name)						\
                      "addl\t$0x10,%%esp"							\
	              :										\
	              : "r" (__dtrace_args)							\
	              : "memory", "eax", "edx"							\
	);

#define DTRACE_CALL4ARGS(provider, name)							\
	asm volatile ("subl\t$0x10,%%esp"						"\n\t"	\
	              "movl\t0x0(%0),%%eax"						"\n\t"	\
		      "movl\t0x4(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x0(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x4(%%esp)"						"\n\t"	\
	              "movl\t0x8(%0),%%eax"						"\n\t"	\
		      "movl\t0xC(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x8(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0xC(%%esp)"						"\n\t"	\
	              DTRACE_CALL(provider, name)						\
                      "addl\t$0x10,%%esp"							\
	              :										\
	              : "r" (__dtrace_args)							\
	              : "memory", "eax", "edx"	       						\
	);

#define DTRACE_CALL5ARGS(provider, name)							\
	asm volatile ("subl\t$0x20,%%esp"						"\n\t"	\
	              "movl\t0x0(%0),%%eax"						"\n\t"	\
		      "movl\t0x4(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x0(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x4(%%esp)"						"\n\t"	\
	              "movl\t0x8(%0),%%eax"						"\n\t"	\
		      "movl\t0xC(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x8(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0xC(%%esp)"						"\n\t"	\
	              "movl\t0x10(%0),%%eax"						"\n\t"	\
	              "movl\t%%eax,0x10(%%esp)"						"\n\t"	\
	              DTRACE_CALL(provider, name)						\
                      "addl\t$0x20,%%esp"							\
	              :										\
	              : "r" (__dtrace_args)							\
	              : "memory", "eax", "edx"	       						\
	);

#define DTRACE_CALL6ARGS(provider, name)							\
	asm volatile ("subl\t$0x20,%%esp"						"\n\t"	\
	              "movl\t0x0(%0),%%eax"						"\n\t"	\
		      "movl\t0x4(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x0(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x4(%%esp)"						"\n\t"	\
	              "movl\t0x8(%0),%%eax"						"\n\t"	\
		      "movl\t0xC(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x8(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0xC(%%esp)"						"\n\t"	\
	              "movl\t0x10(%0),%%eax"						"\n\t"	\
		      "movl\t0x14(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x10(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x14(%%esp)"						"\n\t"	\
	              DTRACE_CALL(provider, name)						\
                      "addl\t$0x20,%%esp"							\
	              :										\
	              : "r" (__dtrace_args)							\
	              : "memory", "eax", "edx"	       						\
	);

#define DTRACE_CALL7ARGS(provider, name)							\
	asm volatile ("subl\t$0x20,%%esp"						"\n\t"	\
	              "movl\t0x0(%0),%%eax"						"\n\t"	\
		      "movl\t0x4(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x0(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x4(%%esp)"						"\n\t"	\
	              "movl\t0x8(%0),%%eax"						"\n\t"	\
		      "movl\t0xC(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x8(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0xC(%%esp)"						"\n\t"	\
	              "movl\t0x10(%0),%%eax"						"\n\t"	\
		      "movl\t0x14(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x10(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x14(%%esp)"						"\n\t"	\
	              "movl\t0x18(%0),%%eax"						"\n\t"	\
	              "movl\t%%eax,0x18(%%esp)"						"\n\t"	\
	              DTRACE_CALL(provider, name)						\
                      "addl\t$0x20,%%esp"							\
	              :										\
	              : "r" (__dtrace_args)							\
	              : "memory", "eax", "edx"	       						\
	);

#define DTRACE_CALL8ARGS(provider, name)							\
	asm volatile ("subl\t$0x20,%%esp"						"\n\t"	\
	              "movl\t0x0(%0),%%eax"						"\n\t"	\
		      "movl\t0x4(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x0(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x4(%%esp)"						"\n\t"	\
	              "movl\t0x8(%0),%%eax"						"\n\t"	\
		      "movl\t0xC(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x8(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0xC(%%esp)"						"\n\t"	\
	              "movl\t0x10(%0),%%eax"						"\n\t"	\
		      "movl\t0x14(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x10(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x14(%%esp)"						"\n\t"	\
	              "movl\t0x18(%0),%%eax"						"\n\t"	\
		      "movl\t0x1C(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x18(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x1C(%%esp)"						"\n\t"	\
	              DTRACE_CALL(provider, name)						\
                      "addl\t$0x20,%%esp"							\
	              :										\
	              : "r" (__dtrace_args)							\
	              : "memory", "eax", "edx"	       						\
	);

#define DTRACE_CALL9ARGS(provider, name)							\
	asm volatile ("subl\t$0x30,%%esp"						"\n\t"	\
	              "movl\t0x0(%0),%%eax"						"\n\t"	\
		      "movl\t0x4(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x0(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x4(%%esp)"						"\n\t"	\
	              "movl\t0x8(%0),%%eax"						"\n\t"	\
		      "movl\t0xC(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x8(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0xC(%%esp)"						"\n\t"	\
	              "movl\t0x10(%0),%%eax"						"\n\t"	\
		      "movl\t0x14(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x10(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x14(%%esp)"						"\n\t"	\
	              "movl\t0x18(%0),%%eax"						"\n\t"	\
		      "movl\t0x1C(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x18(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x1C(%%esp)"						"\n\t"	\
	              "movl\t0x20(%0),%%eax"						"\n\t"	\
	              "movl\t%%eax,0x20(%%esp)"						"\n\t"	\
	              DTRACE_CALL(provider, name)						\
                      "addl\t$0x30,%%esp"							\
	              :										\
	              : "r" (__dtrace_args)							\
	              : "memory", "eax", "edx"	       						\
	);

#define DTRACE_CALL10ARGS(provider, name)							\
	asm volatile ("subl\t$0x30,%%esp"						"\n\t"	\
	              "movl\t0x0(%0),%%eax"						"\n\t"	\
		      "movl\t0x4(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x0(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x4(%%esp)"						"\n\t"	\
	              "movl\t0x8(%0),%%eax"						"\n\t"	\
		      "movl\t0xC(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x8(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0xC(%%esp)"						"\n\t"	\
	              "movl\t0x10(%0),%%eax"						"\n\t"	\
		      "movl\t0x14(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x10(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x14(%%esp)"						"\n\t"	\
	              "movl\t0x18(%0),%%eax"						"\n\t"	\
		      "movl\t0x1C(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x18(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x1C(%%esp)"						"\n\t"	\
	              "movl\t0x20(%0),%%eax"						"\n\t"	\
		      "movl\t0x24(%0),%%edx"						"\n\t"	\
	              "movl\t%%eax,0x20(%%esp)"						"\n\t"	\
	              "movl\t%%edx,0x24(%%esp)"						"\n\t"	\
	              DTRACE_CALL(provider, name)						\
                      "addl\t$0x30,%%esp"							\
	              :										\
	              : "r" (__dtrace_args)							\
	              : "memory", "eax", "edx"	       						\
	);

#endif // __i386__

#endif	/* _MACH_I386_SDT_ISA_H */
