/*
 * Copyright (c) 2000-2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef	_ARCH_ARM_ASM_HELP_H_
#define	_ARCH_ARM_ASM_HELP_H_

#include	<architecture/arm/reg_help.h>

#ifdef	__ASSEMBLER__

#if defined(__arm__)

#define ALIGN						\
	.align	2

#define	ROUND_TO_STACK(len)				\
	(((len) + STACK_INCR - 1) / STACK_INCR * STACK_INCR)

/*
 * Macros for declaring procedures
 *
 * Use of these macros allows ctags to have a predictable way
 * to find various types of declarations.  They also simplify
 * inserting appropriate symbol table information.
 *
 * NOTE: these simple stubs will be replaced with more
 * complicated versions once we know what the linker and gdb
 * will require as far as register use masks and frame declarations.
 * These macros may also be ifdef'ed in the future to contain profiling
 * code.
 *
 */

/*
 * TEXT -- declare start of text segment
 */
#define	TEXT						\
	.text

/*
 * DATA -- declare start of data segment
 */
#define DATA						\
	.data

/*
 * LEAF -- declare global leaf procedure
 * NOTE: Control SHOULD NOT FLOW into a LEAF!  A LEAF should only
 * be jumped to.  (A leaf may do an align.)  Use a LABEL() if you
 * need control to flow into the label.
 */
#define	LEAF(name, localvarsize)			\
	.globl	name					;\
	ALIGN						;\
name:

/*
 * X_LEAF -- declare alternate global label for leaf
 */
#define	X_LEAF(name, value)				\
	.globl	name					;\
	.set	name,value

/*
 * P_LEAF -- declare private leaf procedure
 */
#define	P_LEAF(name, localvarsize)			\
	ALIGN						;\
name:

/*
 * LABEL -- declare a global code label
 * MUST be used (rather than LEAF, NESTED, etc) if control
 * "flows into" the label.
 */
#define	LABEL(name)					\
	.globl	name					;\
name:

/*
 * NESTED -- declare procedure that invokes other procedures
 */
#define	NESTED(name, localvarsize)			\
	.globl	name					;\
	ALIGN						;\
name:

/*
 * X_NESTED -- declare alternate global label for nested proc
 */
#define	X_NESTED(name, value)				\
	.globl	name					;\
	.set	name,value

/*
 * P_NESTED -- declare private nested procedure
 */
#define	P_NESTED(name, localvarsize)			\
	ALIGN						;\
name:

/*
 * END -- mark end of procedure
 */
#define	END(name)

/*
 * Storage definition macros
 * The main purpose of these is to allow an easy handle for ctags
 */

/*
 * IMPORT -- import symbol
 */
#define	IMPORT(name)					\
	.reference	name

/*
 * ABS -- declare global absolute symbol
 */
#define	ABS(name, value)				\
	.globl	name					;\
	.set	name,value

/*
 * P_ABS -- declare private absolute symbol
 */
#define	P_ABS(name, value)				\
	.set	name,value

/*
 * EXPORT -- declare global label for data
 */
#define	EXPORT(name)					\
	.globl	name					;\
name:

/*
 * BSS -- declare global zero'ed storage
 */
#define	BSS(name,size)					\
	.comm	name,size

/*
 * P_BSS -- declare private zero'ed storage
 */
#define	P_BSS(name,size)				\
	.lcomm	name,size

#if defined(__DYNAMIC__)
#define GET_ADDRESS(reg,var)		\
	ldr	reg, 4f						;\
3:	ldr	reg, [pc, reg]				;\
	b	5f							;\
4:	.long	6f - (3b + 8)			;\
5:									;\
	.non_lazy_symbol_pointer		;\
6:									;\
	.indirect_symbol var			;\
	.long 0							;\
	.text							;\
	.align 2
#else
#define GET_ADDRESS(reg,var)		\
	ldr	reg, 3f						;\
	b	4f							;\
3:	.long var						;\
4:
#endif

#if defined(__DYNAMIC__)
#define BRANCH_EXTERNAL(var)		\
	GET_ADDRESS(ip, var)			;\
	bx	ip
#else
#define BRANCH_EXTERNAL(var)		;\
	b	var
#endif

#if defined(__DYNAMIC__)
#define CALL_EXTERNAL(var)			\
	GET_ADDRESS(ip,var)				;\
	mov	lr, pc						;\
	bx	ip
#else
#define CALL_EXTERNAL(var)			\
	bl	var
#endif

#define ENTRY_POINT(name)			\
	.align 2						;\
	.globl  name					;\
	.text							;\
name:

#else /* defined(__arm___) */

#if defined(__arm64__)

#define ALIGN	.align 2

#define ROUND_TO_STACK(len) (((len) + (STACK_INCR - 1)) & ~(STACK_INCR - 1))

#define TEXT	.text

#define DATA	.data

#define LEAF(name, localvarsize)	\
	.globl	name					%%\
	ALIGN							%%\
name:

#define X_LEAF(name, value)			\
	.globl	name					%%\
	.set	name, value

#define P_LEAF(name, localvarsize)	\
	ALIGN							%%\
name:


#define LABEL(name)					\
	.globl	name					%%\
name:

#define NESTED(name, localvarsize)	\
	.globl 	name					%%\
	ALIGN							%%\
name:

#define X_NESTED(name, value)		\
	.globl 	name					%%\
	.set 	name, value

#define P_NESTED(name, localvarsize)\
	ALIGN							%%\
name:

#define END(name)

#define IMPORT(name)				\
	.reference name

#define ABS(name, value)			\
	.globl 	name					%%\
	.set 	name, value

#define P_ABS(name, value)			\
	.set 	name, value

#define EXPORT(name)				\
	.globl	name					%%\
name:

#define BSS(name, size)				\
	.comm	name, size

#define P_BSS(name, size)			\
	.lcomm	name, size


#if __LP64__
#if defined(__DYNAMIC__)
#define GET_ADDRESS(wreg, xreg, var)	\
	adrp	xreg, var@gotpage		  %%\
	ldr		xreg, [xreg, var@gotpageoff]
#else /* __DYNAMIC__ */
#define	GET_ADDRESS(wreg, xreg, var)	\
	adrp	xreg, var@page			  %%\
	add		xreg, xreg, var@pageoff
#endif /* __DYNAMIC__ */
#else /* __LP64__ */
/*
 * The adrp instruction can't encode 'w' registers which is why the 'x' register
 * is used in the below LP32 variants of GET_ADDRESS().
 */
#if defined(__DYNAMIC__)
#define GET_ADDRESS(wreg, xreg, var)	\
	adrp	xreg, var@gotpage		  %%\
	ldr		wreg, [xreg, var@gotpageoff]
#else /* __DYNAMIC__ */
#define	GET_ADDRESS(wreg, xreg, var)	\
	adrp	xreg, var@page			  %%\
	add		wreg, wreg, var@pageoff
#endif /* __DYNAMIC__ */
#endif /* __LP64__ */

/* The linker will link to stubs in the DYNAMIC case */
#define BRANCH_EXTERNAL(var) \
	b var

/* The linker will link to stubs in the DYNAMIC case */
#define CALL_EXTERNAL(var) \
	bl var

#define ENTRY_POINT(name)			\
	.align 2						%%\
	.globl name						%%\
	.text							%%\
name:
#else /* defined(__arm64__) */
#error Unknown architecture.
#endif /* defined(__arm64__) */
#endif  /* defined(__arm__) */
#endif	/* __ASSEMBLER__ */

#endif	/* _ARCH_ARM_ASM_HELP_H_ */
