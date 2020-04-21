/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
 */
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

#ifndef _MACH_ARM_SDT_ISA_H
#define	_MACH_ARM_SDT_ISA_H

/*
 * Only define when testing.  This makes the calls into actual calls to
 * test functions.
 */
/* #define DTRACE_CALL_TEST */

#define DTRACE_STRINGIFY(s) #s
#define DTRACE_TOSTRING(s) DTRACE_STRINGIFY(s)

#if defined(KERNEL)
/*
 * For the kernel, set an explicit global label so the symbol can be located
 */
#ifdef __arm__
#define DTRACE_LAB(p, n)                                                                        \
    "__dtrace_probe$" DTRACE_TOSTRING(%=__LINE__) DTRACE_STRINGIFY(_##p##___##n)

#define DTRACE_LABEL(p, n)			\
	".pushsection __DATA, __data\n\t"	\
	".p2align	2\n\t"			\
	".globl " DTRACE_LAB(p, n) "\n\t"	\
	DTRACE_LAB(p, n) ":" ".long 1f""\n\t"	\
	".popsection" "\n\t"			\
	"1:"
#else /* __arm64__ */
#define DTRACE_LAB(p, n)                                                                        \
	"__dtrace_probe$" DTRACE_TOSTRING(%=__LINE__) DTRACE_STRINGIFY(_##p##___##n)

#define DTRACE_LABEL(p, n)			\
	".pushsection __DATA, __data\n\t"	\
	".p2align	3\n\t"			\
	".globl " DTRACE_LAB(p, n) "\n\t"	\
	DTRACE_LAB(p, n) ":" ".quad 1f""\n\t"	\
	".popsection" "\n\t"			\
	"1:"
#endif
#else	/* !KERNEL */
#define DTRACE_LABEL(p, n)									\
	"__dtrace_probe$" DTRACE_TOSTRING(%=__LINE__) DTRACE_STRINGIFY(_##p##___##n) ":"	"\n\t"
#endif	/* !KERNEL */

#ifdef DTRACE_CALL_TEST

#define DTRACE_CALL(p,n)	\
	DTRACE_LABEL(p,n)	\
	DTRACE_CALL_INSN(p,n)

#else	/* !DTRACE_CALL_TEST */

#define DTRACE_CALL(p,n)	\
	DTRACE_LABEL(p,n)	\
	DTRACE_NOPS

#endif	/* !DTRACE_CALL_TEST */

#if defined(__arm__)

#define DTRACE_NOPS			\
	"nop"			"\n\t"

#define DTRACE_CALL_INSN(p,n)						\
	"blx _dtracetest" DTRACE_STRINGIFY(_##p##_##n)	"\n\t"

#ifdef __thumb__
#define DTRACE_ALLOC_STACK(n)		\
	"sub sp, #" #n		"\n\t"
#define DTRACE_DEALLOC_STACK(n)		\
	"add sp, #" #n		"\n\t"
#else
#define DTRACE_ALLOC_STACK(n)		\
	"sub sp, sp, #" #n	"\n\t"
#define DTRACE_DEALLOC_STACK(n)		\
	"add sp, sp, #" #n	"\n\t"
#endif

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

#define DTRACE_CALL0ARGS(provider, name)			\
	asm volatile (						\
		DTRACE_CALL(provider, name)			\
		"# eat trailing nl+tab from DTRACE_CALL"	\
		:						\
		:						\
	);

#define DTRACE_CALL1ARG(provider, name)				\
	asm volatile ("ldr r0, [%0]"							"\n\t"	\
		      DTRACE_CALL(provider, name)						\
		      :										\
		      : "l" (__dtrace_args)							\
		      : "memory", "r0"								\
	);

#define DTRACE_CALL2ARGS(provider, name)			\
	asm volatile ("ldr r1, [%0, #4]"						"\n\t"	\
		      "ldr r0, [%0]"							"\n\t"	\
		      DTRACE_CALL(provider, name)						\
		      :										\
		      : "l" (__dtrace_args)							\
		      : "memory", "r0", "r1"							\
	);

#define DTRACE_CALL3ARGS(provider, name)			\
	asm volatile ("ldr r2, [%0, #8]"						"\n\t"	\
		      "ldr r1, [%0, #4]"						"\n\t"	\
		      "ldr r0, [%0]"							"\n\t"	\
		      DTRACE_CALL(provider, name)						\
		      :										\
		      : "l" (__dtrace_args)							\
		      : "memory", "r0", "r1", "r2"						\
	);

#define DTRACE_CALL4ARGS(provider, name)			\
	asm volatile ("ldr r3, [%0, #12]"						"\n\t"	\
		      "ldr r2, [%0, #8]"						"\n\t"	\
		      "ldr r1, [%0, #4]"						"\n\t"	\
		      "ldr r0, [%0]"							"\n\t"	\
		      DTRACE_CALL(provider, name)						\
		      :										\
		      : "l" (__dtrace_args)							\
		      : "memory", "r0", "r1", "r2", "r3"					\
	);

/*
 * One of our ARM32 ABIs (armv7k) mandates that the stack be aligned to 16 bytes.
 * We currently apply this constraint to all ARM32 DTRACE_CALL macros; hence the
 * macros below will overallocate for some ABIs.
 */
#define DTRACE_CALL5ARGS(provider, name)			\
	asm volatile (										\
		      DTRACE_ALLOC_STACK(16)							\
		      "ldr r0, [%0, #16]"						"\n\t"	\
		      "str r0, [sp]"							"\n\t"	\
		      "ldr r3, [%0, #12]"						"\n\t"	\
		      "ldr r2, [%0, #8]"						"\n\t"	\
		      "ldr r1, [%0, #4]"						"\n\t"	\
		      "ldr r0, [%0]"							"\n\t"	\
		      DTRACE_CALL(provider, name)						\
		      DTRACE_DEALLOC_STACK(16)							\
		      :										\
		      : "l" (__dtrace_args)							\
		      : "memory", "r0", "r1", "r2", "r3"					\
	);

#define DTRACE_CALL6ARGS(provider, name)			\
	asm volatile (										\
		      DTRACE_ALLOC_STACK(16)							\
		      "ldr r1, [%0, #20]"						"\n\t"	\
		      "ldr r0, [%0, #16]"						"\n\t"	\
		      "str r1, [sp, #4]"						"\n\t"	\
		      "str r0, [sp]"							"\n\t"	\
		      "ldr r3, [%0, #12]"						"\n\t"	\
		      "ldr r2, [%0, #8]"						"\n\t"	\
		      "ldr r1, [%0, #4]"						"\n\t"	\
		      "ldr r0, [%0]"							"\n\t"	\
		      DTRACE_CALL(provider, name)						\
		      DTRACE_DEALLOC_STACK(16)							\
		      :										\
		      : "l" (__dtrace_args)							\
		      : "memory", "r0", "r1", "r2", "r3"					\
	);

#define DTRACE_CALL7ARGS(provider, name)			\
	asm volatile (										\
		      DTRACE_ALLOC_STACK(16)							\
		      "ldr r2, [%0, #24]"						"\n\t"	\
		      "ldr r1, [%0, #20]"						"\n\t"	\
		      "ldr r0, [%0, #16]"						"\n\t"	\
		      "str r2, [sp, #8]"						"\n\t"	\
		      "str r1, [sp, #4]"						"\n\t"	\
		      "str r0, [sp]"							"\n\t"	\
		      "ldr r3, [%0, #12]"						"\n\t"	\
		      "ldr r2, [%0, #8]"						"\n\t"	\
		      "ldr r1, [%0, #4]"						"\n\t"	\
		      "ldr r0, [%0]"							"\n\t"	\
		      DTRACE_CALL(provider, name)						\
		      DTRACE_DEALLOC_STACK(16)							\
		      :										\
		      : "l" (__dtrace_args)							\
		      : "memory", "r0", "r1", "r2", "r3"					\
	);

#define DTRACE_CALL8ARGS(provider, name)			\
	asm volatile (										\
		      DTRACE_ALLOC_STACK(16)							\
		      "ldr r3, [%0, #28]"						"\n\t"	\
		      "ldr r2, [%0, #24]"						"\n\t"	\
		      "ldr r1, [%0, #20]"						"\n\t"	\
		      "ldr r0, [%0, #16]"						"\n\t"	\
		      "str r3, [sp, #12]"						"\n\t"	\
		      "str r2, [sp, #8]"						"\n\t"	\
		      "str r1, [sp, #4]"						"\n\t"	\
		      "str r0, [sp]"							"\n\t"	\
		      "ldr r3, [%0, #12]"						"\n\t"	\
		      "ldr r2, [%0, #8]"						"\n\t"	\
		      "ldr r1, [%0, #4]"						"\n\t"	\
		      "ldr r0, [%0]"							"\n\t"	\
		      DTRACE_CALL(provider, name)						\
		      DTRACE_DEALLOC_STACK(16)							\
		      :										\
		      : "l" (__dtrace_args)							\
		      : "memory", "r0", "r1", "r2", "r3"					\
	);

#define DTRACE_CALL9ARGS(provider, name)			\
	asm volatile (										\
		      DTRACE_ALLOC_STACK(32)							\
		      "ldr r0, [%0, #32]"						"\n\t"	\
		      "str r0, [sp, #16]"						"\n\t"	\
		      "ldr r3, [%0, #28]"						"\n\t"	\
		      "ldr r2, [%0, #24]"						"\n\t"	\
		      "ldr r1, [%0, #20]"						"\n\t"	\
		      "ldr r0, [%0, #16]"						"\n\t"	\
		      "str r3, [sp, #12]"						"\n\t"	\
		      "str r2, [sp, #8]"						"\n\t"	\
		      "str r1, [sp, #4]"						"\n\t"	\
		      "str r0, [sp]"							"\n\t"	\
		      "ldr r3, [%0, #12]"						"\n\t"	\
		      "ldr r2, [%0, #8]"						"\n\t"	\
		      "ldr r1, [%0, #4]"						"\n\t"	\
		      "ldr r0, [%0]"							"\n\t"	\
		      DTRACE_CALL(provider, name)						\
		      DTRACE_DEALLOC_STACK(32)							\
		      :										\
		      : "l" (__dtrace_args)							\
		      : "memory", "r0", "r1", "r2", "r3"					\
	);

#define DTRACE_CALL10ARGS(provider, name)			\
	asm volatile (										\
		      DTRACE_ALLOC_STACK(32)							\
		      "ldr r1, [%0, #36]"						"\n\t"	\
		      "ldr r0, [%0, #32]"						"\n\t"	\
		      "str r1, [sp, #20]"						"\n\t"	\
		      "str r0, [sp, #16]"						"\n\t"	\
		      "ldr r3, [%0, #28]"						"\n\t"	\
		      "ldr r2, [%0, #24]"						"\n\t"	\
		      "ldr r1, [%0, #20]"						"\n\t"	\
		      "ldr r0, [%0, #16]"						"\n\t"	\
		      "str r3, [sp, #12]"						"\n\t"	\
		      "str r2, [sp, #8]"						"\n\t"	\
		      "str r1, [sp, #4]"						"\n\t"	\
		      "str r0, [sp]"							"\n\t"	\
		      "ldr r3, [%0, #12]"						"\n\t"	\
		      "ldr r2, [%0, #8]"						"\n\t"	\
		      "ldr r1, [%0, #4]"						"\n\t"	\
		      "ldr r0, [%0]"							"\n\t"	\
		      DTRACE_CALL(provider, name)						\
		      DTRACE_DEALLOC_STACK(32)							\
		      :										\
		      : "l" (__dtrace_args)							\
		      : "memory", "r0", "r1", "r2", "r3"					\
	);

#elif defined(__arm64__)

#define DTRACE_NOPS							\
	"nop"                   "\n\t"


#define DTRACE_CALL_INSN(p,n)						\
	"bl _dtracetest" DTRACE_STRINGIFY(_##p##_##n)	"\n\t"

#define DTRACE_ALLOC_STACK(n)		\
	"sub sp, sp, #" #n	"\n\t"
#define DTRACE_DEALLOC_STACK(n)		\
	"add sp, sp, #" #n	"\n\t"

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

#define DTRACE_CALL0ARGS(provider, name)			\
	asm volatile (						\
		DTRACE_CALL(provider, name)			\
		"# eat trailing nl+tab from DTRACE_CALL"	\
		:						\
		:						\
	);

#define DTRACE_CALL1ARG(provider, name)				\
	asm volatile ("ldr x0, [%0]"							"\n\t"	\
	    DTRACE_CALL(provider, name)								\
		      :										\
		      : "r" (__dtrace_args)							\
		      : "memory", "x0"								\
	);

#define DTRACE_CALL2ARGS(provider, name)			\
	asm volatile ("ldp x0, x1, [%0]"						"\n\t"	\
		      DTRACE_CALL(provider, name)						\
		      :										\
		      : "r" (__dtrace_args)							\
		      : "memory", "x0", "x1"							\
	);

#define DTRACE_CALL3ARGS(provider, name)			\
	asm volatile ("ldr x2, [%0, #16]"						"\n\t"	\
		      "ldp x0, x1, [%0]"						"\n\t"	\
		      DTRACE_CALL(provider, name)						\
		      :										\
		      : "r" (__dtrace_args)							\
		      : "memory", "x0", "x1", "x2"						\
	);

#define DTRACE_CALL4ARGS(provider, name)			\
	asm volatile ("ldp x2, x3, [%0, #16]"						"\n\t"	\
		      "ldp x0, x1, [%0]"						"\n\t"	\
		      DTRACE_CALL(provider, name)						\
		      :										\
		      : "r" (__dtrace_args)							\
		      : "memory", "x0", "x1", "x2", "x3"					\
	);

#define DTRACE_CALL5ARGS(provider, name)			\
	asm volatile ("ldr x4, [%0, #32]"						"\n\t"	\
		      "ldp x2, x3, [%0, #16]"						"\n\t"	\
		      "ldp x0, x1, [%0]"						"\n\t"	\
		      DTRACE_CALL(provider, name)						\
		      :										\
		      : "r" (__dtrace_args)							\
		      : "memory", "x0", "x1", "x2", "x3", "x4"					\
	);

#define DTRACE_CALL6ARGS(provider, name)			\
	asm volatile ("ldp x4, x5, [%0, #32]"						"\n\t"	\
		      "ldp x2, x3, [%0, #16]"						"\n\t"	\
		      "ldp x0, x1, [%0]"						"\n\t"	\
		      DTRACE_CALL(provider, name)						\
		      :										\
		      : "r" (__dtrace_args)							\
		      : "memory", "x0", "x1", "x2", "x3", "x4", "x5"				\
	);

#define DTRACE_CALL7ARGS(provider, name)			\
	asm volatile ("ldr x6, [%0, #48]"						"\n\t"	\
    		      "ldp x4, x5, [%0, #32]"						"\n\t"	\
    		      "ldp x2, x3, [%0, #16]"						"\n\t"	\
    		      "ldp x0, x1, [%0]"						"\n\t"	\
    		      DTRACE_CALL(provider, name)						\
    		      :										\
    		      : "r" (__dtrace_args)							\
    		      : "memory", "x0", "x1", "x2", "x3", "x4", "x5", "x6"			\
    	);

#define DTRACE_CALL8ARGS(provider, name)			\
	asm volatile ("ldp x6, x7, [%0, #48]"						"\n\t"	\
    		      "ldp x4, x5, [%0, #32]"						"\n\t"	\
    		      "ldp x2, x3, [%0, #16]"						"\n\t"	\
    		      "ldp x0, x1, [%0]"						"\n\t"	\
    		      DTRACE_CALL(provider, name)						\
    		      :										\
    		      : "r" (__dtrace_args)							\
    		      : "memory", "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7"		\
    	);

/* Keep stack 16 byte aligned per ABI requirements */
#define DTRACE_CALL9ARGS(provider, name)			\
	asm volatile (											\
			      DTRACE_ALLOC_STACK(16)							\
			      "ldr x0, [%0, #64]"						"\n\t"	\
			      "str x0, [sp]"							"\n\t"	\
			      "ldp x6, x7, [%0, #48]"						"\n\t"	\
			      "ldp x4, x5, [%0, #32]"						"\n\t"	\
			      "ldp x2, x3, [%0, #16]"						"\n\t"	\
			      "ldp x0, x1, [%0]"						"\n\t"	\
			      DTRACE_CALL(provider, name)						\
			      DTRACE_DEALLOC_STACK(16)							\
			      :										\
			      : "r" (__dtrace_args)							\
			      : "memory", "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7"		\
		);

#define DTRACE_CALL10ARGS(provider, name)			\
	asm volatile (										\
			      DTRACE_ALLOC_STACK(16)							\
			      "ldp x0, x1, [%0, #64]"						"\n\t"	\
			      "stp x0, x1, [sp]"						"\n\t"	\
			      "ldp x6, x7, [%0, #48]"						"\n\t"	\
			      "ldp x4, x5, [%0, #32]"						"\n\t"	\
			      "ldp x2, x3, [%0, #16]"						"\n\t"	\
			      "ldp x0, x1, [%0]"						"\n\t"	\
			      DTRACE_CALL(provider, name)						\
			      DTRACE_DEALLOC_STACK(16)							\
			      :										\
			      : "r" (__dtrace_args)							\
			      : "memory", "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7"		\
		);

#endif /* __arm__ */

#endif	/* _MACH_ARM_SDT_ISA_H */
