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

#ifndef _MACH_PPC_SDT_ISA_H
#define	_MACH_PPC_SDT_ISA_H

/* #pragma ident	"@(#)sdt.h	1.7	05/06/08 SMI" */

/*
 * Only define when testing.  This makes the calls into actual calls to
 * test functions.
 */
/* #define DTRACE_CALL_TEST */

#define DTRACE_STRINGIFY(s) #s
#define DTRACE_TOSTRING(s) DTRACE_STRINGIFY(s)

#define DTRACE_LABEL(p, n)									\
	"__dtrace_probe$" DTRACE_TOSTRING(__LINE__) DTRACE_STRINGIFY(_##p##___##n) ":"	"\n\t"

#ifdef DTRACE_CALL_TEST

#define DTRACE_CALL(p,n)	\
	DTRACE_LABEL(p,n)	\
	DTRACE_CALL_INSN(p,n)

#else	/* !DTRACE_CALL_TEST */

#define DTRACE_CALL(p,n)	\
	DTRACE_LABEL(p,n)	\
	DTRACE_NOPS

#endif	/* !DTRACE_CALL_TEST */

#ifdef __ppc__

#define DTRACE_NOPS			\
	"nop"			"\n\t"

#define DTRACE_CALL_INSN(p,n)						\
	"bl _dtracetest" DTRACE_STRINGIFY(_##p##_##n)	"\n\t"

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
	              "# eat trailing nl+tab from DTRACE_CALL"					\
	              :										\
	              :										\
	);

#define DTRACE_CALL1ARG(provider, name)								\
	asm volatile ("subi r1,r1,0x20"							"\n\t"	\
	              "lwz r3,0x0(%0)"							"\n\t"	\
	              DTRACE_CALL(provider, name)						\
                      "addi r1,r1,0x20"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3"									\
	);

#define DTRACE_CALL2ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x20"							"\n\t"	\
	              "lwz r3,0x0(%0)"							"\n\t"	\
	              "lwz r4,0x4(%0)"							"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x20"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4"								\
	);

#define DTRACE_CALL3ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x30"							"\n\t"	\
	              "lwz r3,0x0(%0)"							"\n\t"	\
	              "lwz r4,0x4(%0)"							"\n\t"	\
	              "lwz r5,0x8(%0)"							"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x30"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4", "r5"							\
	);

#define DTRACE_CALL4ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x30"							"\n\t"	\
	              "lwz r3,0x0(%0)"							"\n\t"	\
	              "lwz r4,0x4(%0)"							"\n\t"	\
	              "lwz r5,0x8(%0)"							"\n\t"	\
	              "lwz r6,0xc(%0)"							"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x30"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4", "r5", "r6"							\
	);

#define DTRACE_CALL5ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x30"							"\n\t"	\
	              "lwz r3,0x0(%0)"							"\n\t"	\
	              "lwz r4,0x4(%0)"							"\n\t"	\
	              "lwz r5,0x8(%0)"							"\n\t"	\
	              "lwz r6,0xc(%0)"							"\n\t"	\
	              "lwz r7,0x10(%0)"							"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x30"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4", "r5", "r6", "r7"						\
	);

#define DTRACE_CALL6ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x30"							"\n\t"	\
	              "lwz r3,0x0(%0)"							"\n\t"	\
	              "lwz r4,0x4(%0)"							"\n\t"	\
	              "lwz r5,0x8(%0)"							"\n\t"	\
	              "lwz r6,0xc(%0)"							"\n\t"	\
	              "lwz r7,0x10(%0)"							"\n\t"	\
	              "lwz r8,0x14(%0)"							"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x30"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4", "r5", "r6", "r7", "r8"					\
	);

#define DTRACE_CALL7ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x40"							"\n\t"	\
	              "lwz r3,0x0(%0)"							"\n\t"	\
	              "lwz r4,0x4(%0)"							"\n\t"	\
	              "lwz r5,0x8(%0)"							"\n\t"	\
	              "lwz r6,0xc(%0)"							"\n\t"	\
	              "lwz r7,0x10(%0)"							"\n\t"	\
	              "lwz r8,0x14(%0)"							"\n\t"	\
	              "lwz r9,0x18(%0)"							"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x40"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4", "r5", "r6", "r7", "r8", "r9"				\
	);

#define DTRACE_CALL8ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x40"							"\n\t"	\
	              "lwz r3,0x0(%0)"							"\n\t"	\
	              "lwz r4,0x4(%0)"							"\n\t"	\
	              "lwz r5,0x8(%0)"							"\n\t"	\
	              "lwz r6,0xc(%0)"							"\n\t"	\
	              "lwz r7,0x10(%0)"							"\n\t"	\
	              "lwz r8,0x14(%0)"							"\n\t"	\
	              "lwz r9,0x18(%0)"							"\n\t"	\
	              "lwz r10,0x1c(%0)"						"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x40"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10"				\
	);

#define DTRACE_CALL9ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x40"							"\n\t"	\
	              "lwz r3,0x0(%0)"							"\n\t"	\
	              "lwz r4,0x4(%0)"							"\n\t"	\
	              "lwz r5,0x8(%0)"							"\n\t"	\
	              "lwz r6,0xc(%0)"							"\n\t"	\
	              "lwz r7,0x10(%0)"							"\n\t"	\
	              "lwz r8,0x14(%0)"							"\n\t"	\
	              "lwz r9,0x18(%0)"							"\n\t"	\
	              "lwz r10,0x1c(%0)"						"\n\t"	\
	              "lwz r11,0x20(%0)"						"\n\t"	\
	              "stw r11,0x38(r1)"						"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x40"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11"			\
	);

#define DTRACE_CALL10ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x40"							"\n\t"	\
	              "lwz r3,0x0(%0)"							"\n\t"	\
	              "lwz r4,0x4(%0)"							"\n\t"	\
	              "lwz r5,0x8(%0)"							"\n\t"	\
	              "lwz r6,0xc(%0)"							"\n\t"	\
	              "lwz r7,0x10(%0)"							"\n\t"	\
	              "lwz r8,0x14(%0)"							"\n\t"	\
	              "lwz r9,0x18(%0)"							"\n\t"	\
	              "lwz r10,0x1c(%0)"						"\n\t"	\
	              "lwz r11,0x20(%0)"						"\n\t"	\
	              "lwz r12,0x24(%0)"						"\n\t"	\
	              "stw r11,0x38(r1)"						"\n\t"	\
	              "stw r12,0x3c(r1)"						"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x40"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12"		\
	);

#endif // __ppc__

#ifdef __ppc64__

#define DTRACE_NOPS			\
	"nop"			"\n\t"

#define DTRACE_CALL_INSN(p,n)						\
	"bl _dtracetest" DTRACE_STRINGIFY(_##p##_##n)	"\n\t"

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
	asm volatile ("subi r1,r1,0x30"							"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x30"								\
	              :										\
	              :										\
	              :										\
	);

#define DTRACE_CALL1ARG(provider, name)								\
	asm volatile ("ld r3,0x0(%0)"							"\n\t"	\
	              "subi r1,r1,0x38"							"\n\t"	\
	              DTRACE_CALL(provider, name)						\
                      "addi r1,r1,0x38"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3"									\
	);

#define DTRACE_CALL2ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x40"							"\n\t"	\
	              "ld r3,0x0(%0)"							"\n\t"	\
	              "ld r4,0x8(%0)"							"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x40"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4"								\
	);

#define DTRACE_CALL3ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x48"							"\n\t"	\
	              "ld r3,0x0(%0)"							"\n\t"	\
	              "ld r4,0x8(%0)"							"\n\t"	\
	              "ld r5,0x10(%0)"							"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x48"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4", "r5"							\
	);

#define DTRACE_CALL4ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x50"							"\n\t"	\
	              "ld r3,0x0(%0)"							"\n\t"	\
	              "ld r4,0x8(%0)"							"\n\t"	\
	              "ld r5,0x10(%0)"							"\n\t"	\
	              "ld r6,0x18(%0)"							"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x50"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4", "r5", "r6"							\
	);

#define DTRACE_CALL5ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x58"							"\n\t"	\
	              "ld r3,0x0(%0)"							"\n\t"	\
	              "ld r4,0x8(%0)"							"\n\t"	\
	              "ld r5,0x10(%0)"							"\n\t"	\
	              "ld r6,0x18(%0)"							"\n\t"	\
	              "ld r7,0x20(%0)"							"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x58"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4", "r5", "r6", "r7"						\
	);

#define DTRACE_CALL6ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x60"							"\n\t"	\
	              "ld r3,0x0(%0)"							"\n\t"	\
	              "ld r4,0x8(%0)"							"\n\t"	\
	              "ld r5,0x10(%0)"							"\n\t"	\
	              "ld r6,0x18(%0)"							"\n\t"	\
	              "ld r7,0x20(%0)"							"\n\t"	\
	              "ld r8,0x28(%0)"							"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x60"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4", "r5", "r6", "r7", "r8"					\
	);

#define DTRACE_CALL7ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x68"							"\n\t"	\
	              "ld r3,0x0(%0)"							"\n\t"	\
	              "ld r4,0x8(%0)"							"\n\t"	\
	              "ld r5,0x10(%0)"							"\n\t"	\
	              "ld r6,0x18(%0)"							"\n\t"	\
	              "ld r7,0x20(%0)"							"\n\t"	\
	              "ld r8,0x28(%0)"							"\n\t"	\
	              "ld r9,0x30(%0)"							"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x68"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4", "r5", "r6", "r7", "r8", "r9"				\
	);

#define DTRACE_CALL8ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x70"							"\n\t"	\
	              "ld r3,0x0(%0)"							"\n\t"	\
	              "ld r4,0x8(%0)"							"\n\t"	\
	              "ld r5,0x10(%0)"							"\n\t"	\
	              "ld r6,0x18(%0)"							"\n\t"	\
	              "ld r7,0x20(%0)"							"\n\t"	\
	              "ld r8,0x28(%0)"							"\n\t"	\
	              "ld r9,0x30(%0)"							"\n\t"	\
	              "ld r10,0x38(%0)"							"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x70"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10"				\
	);

#define DTRACE_CALL9ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x78"							"\n\t"	\
	              "ld r3,0x0(%0)"							"\n\t"	\
	              "ld r4,0x8(%0)"							"\n\t"	\
	              "ld r5,0x10(%0)"							"\n\t"	\
	              "ld r6,0x18(%0)"							"\n\t"	\
	              "ld r7,0x20(%0)"							"\n\t"	\
	              "ld r8,0x28(%0)"							"\n\t"	\
	              "ld r9,0x30(%0)"							"\n\t"	\
	              "ld r10,0x38(%0)"							"\n\t"	\
	              "ld r11,0x40(%0)"							"\n\t"	\
	              "std r11,0x70(r1)"						"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x78"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11"			\
	);

#define DTRACE_CALL10ARGS(provider, name)							\
	asm volatile ("subi r1,r1,0x80"							"\n\t"	\
	              "ld r3,0x0(%0)"							"\n\t"	\
	              "ld r4,0x8(%0)"							"\n\t"	\
	              "ld r5,0x10(%0)"							"\n\t"	\
	              "ld r6,0x18(%0)"							"\n\t"	\
	              "ld r7,0x20(%0)"							"\n\t"	\
	              "ld r8,0x28(%0)"							"\n\t"	\
	              "ld r9,0x30(%0)"							"\n\t"	\
	              "ld r10,0x38(%0)"							"\n\t"	\
	              "ld r11,0x40(%0)"							"\n\t"	\
	              "ld r12,0x48(%0)"							"\n\t"	\
	              "std r11,0x70(r1)"						"\n\t"	\
	              "std r12,0x78(r1)"						"\n\t"	\
	              DTRACE_CALL(provider, name)						\
	              "addi r1,r1,0x80"								\
	              :										\
	              : "b" (__dtrace_args)							\
	              : "memory", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12"		\
	);

#endif // __ppc64__

#endif	/* _MACH_PPC_SDT_ISA_H */
