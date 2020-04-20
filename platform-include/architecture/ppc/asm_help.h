/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
/* Copyright (c) 1996 NeXT Software, Inc.  All rights reserved.
 *
 *	File:	architecture/ppc/asm_help.h
 *	Author:	Mike DeMoney, NeXT Software, Inc.
 *
 *	This header file defines macros useful when writing assembly code
 *	for the PowerPC processors.
 *	r12 is used as the tmp register / PICIFY base.
 *
 * HISTORY
 * 20-May-97  Umesh Vaishampayan (umeshv@apple.com)
 *	Implemented Dynamic / PIC macros.
 *
 * 28-Dec-96  Umesh Vaishampayan (umeshv@NeXT.com)
 *	added ".align" directive to various macros to avoid alignment 
 *  	faults. Moved Register Usage #defines to reg_help.h as that's
 *	where they should have been in the first place.
 *	Added Dynamic / PIC macroes for routines which refernce external
 *	symbols. Not implemented fully as yet.
 *
 * 05-Nov-92  Mike DeMoney (mike@next.com)
 *	Created.
 */

#ifndef	_ARCH_PPC_ASM_HELP_H_
#define	_ARCH_PPC_ASM_HELP_H_

#include	<architecture/ppc/reg_help.h>

#ifdef	__ASSEMBLER__
/*
 * ppc stack frames look like this after procedure prolog has
 * been executed:
 *
 * Higher address:
 *			.........
 *		+-------------------------------+
 * 		| caller's LR			|
 *		+-------------------------------+
 * 		| caller's CR			|
 *		+-------------------------------+
 * Caller's SP->| caller's caller's sp		|  ^^ Caller's Frame ^^
 *		+===============================+  vv Called Rtn Frame vv
 *		|	Save Area for		| FPF 31
 *			..........
 *		| 	Caller's FPF's		| FPF n
 *		+-------------------------------+
 *		|	Save Area for		| GRF 31
 *			..........
 *		| 	Caller's GRF's		| GRF n
 *		+-------------------------------+
 *		|	alignment pad		|
 *			............
 *		|	(if necessary)		|
 *		+-------------------------------+
 *		|	Local			|
 *			........
 *		| 	Variables		|
 *		+-------------------------------+
 * SP + X ->	| aN for FUTURE call		|
 *		+-------------------------------+
 *			..........
 *		+-------------------------------+
 * SP + 28 ->	| a1 for FUTURE call		|
 *		+-------------------------------+
 * SP + 24 ->	| a0 for FUTURE call		|
 *		+-------------------------------+
 * SP + 20 ->	| caller's TOC			|
 *		+-------------------------------+
 * SP + 16 ->	| reserved			|
 *		+-------------------------------+
 * SP + 12 ->	| reserved			|
 *		+-------------------------------+
 * SP + 8 ->	| LR callee-save for FUTURE call|
 *		+-------------------------------+
 * SP + 4 ->	| CR callee-save for FUTURE call|
 *		+-------------------------------+
 * SP ->	| caller's sp			|
 *		+===============================+
 * Lower address:
 *
 * NOTE: All state with the exception of LR and CR are saved in the
 * called routines frame.  LR and CR are saved in the CALLER'S FRAME.
 *
 * ALSO NOTE: Args to the called routine are found in the caller's frame.
 */

/*
 * ARG(n) -- stack offset to n'th argument
 *
 * NOTE CAREFULLY!  These macros start numbering arguments at 1 (NOT 0)
 * The first argument is ARG(1).
 *
 * ALSO NOTE:  This stack offset is only valid if using routine
 * DOES NOT alter SP.
 *
 */
#define	ARG(n)		((((n) - 1) * 4) + 24)

/*
 * Macros for building stack frame according to C calling conventions.
 * lr, cr, and sp are saved.
 *
 * NOTE WELL: localvarsize is in bytes, maxargsout is a count of words,
 * grfsaved and fpfsaved is a count of registers.  BE SURE TO COUNT
 * BOTH FP (r31) AND sN REGISTERS IN THE COUNT OF GRF REGISTERS SAVED!
 * This will be TWO more than the N of the highest sN register you
 * save: s2 implies you are saving s2, s1, s0, and fp => grfsaved
 * should be 4!
 *
 * FURTHER NOTE: These macros do NOT SAVE GRF or FPF registers.  User
 * must do that.  GRF sN regs should be saved via
 *	stmw	sN,SAVED_GRF_S(N)(sp)
 * where N is the highest numbered s* register to be saved.  E.g. if
 * s0, s1, and s2 are to be saved use:
 *	stmw	s2,SAVED_GRF_S(2)(sp)
 * Note that this also saves fp.
 * An individual saved grf can be loaded via:
 *	lwz	s2,SAVED_GRF_S(2)(sp)
 * Analogous stuff works for fpf's.
 *
 * NOTE: these simple routines will be replaced with more complicated
 * ones once we know what the linker and gdb will require as for as 
 * register use masks and frame declarations.
 *
 * Warning: ROUND_TO_STACK is only to be used in assembly language;
 * for C usage, use ROUND_FRAME() in reg_help.h.
 */
#define	ROUND_TO_STACK(len)				\
	(((len) + STACK_INCR - 1) / STACK_INCR * STACK_INCR)

#define	BUILD_FRAME(localvarsize, maxargsout, grfsaved, fpfsaved)	\
	.set	__argoutsize, ROUND_TO_STACK((maxargsout) * 4)		@\
	.if	__argoutsize < 32					@\
	  .set	__argoutsize,32						@\
	.endif								@\
	.set	__framesize, ROUND_TO_STACK(				\
			24 + __argoutsize + (localvarsize)		\
			+ 4*(grfsaved) + 8*(fpfsaved))			@\
	.set	__grfbase,(__framesize - 4*(grfsaved) - 8*(fpfsaved))	@\
	.set	__fpfbase,(__framesize - 8*(fpfsaved))			@\
	mflr	r0							@\
	mfcr	r12							@\
	stw	r0,8(sp)						@\
	stw	r12,4(sp)						@\
	stwu	r1,-__framesize(r1)

/*
 * Macros for referencing data in stack frame.
 *
 * NOTE WELL: ARG's and VAR's start at 1, NOT 0. Why ??? (FIXME)
 */
#define	LOCAL_VAR(n)	(((n)-1)*4 + __argoutsize + 24)
#define	SAVED_GRF_S(n)	(__grfbase + ((grfsaved) - (n) - 2) * 4)
#define	SAVED_FRF_FS(n)	(__fpfbase + ((fpfsaved) - (n) - 1) * 4)
#define	ARG_IN(n)	(ARG(n) + __framesize)
#define	ARG_OUT(n)	(ARG(n) + 0)
#define	SAVED_FP	(__grfbase + ((grfsaved) - 1) * 4)
#define	SAVED_LR	(__framesize + 8)
#define	SAVED_CR	(__framesize + 4)

/*
 * Macros for unwinding stack frame.
 * NOTE: GRF's and FPF's are NOT RESTORED.  User must do this before
 * using this macro.
 */
#define	RETURN						\
	.if	__framesize				@\
	  lwz32	r0,r1,SAVED_LR				@\
	  lwz32	r12,r1,SAVED_CR				@\
	  addic	sp,r1,__framesize			@\
	  mtlr	r0					@\
	  mtcrf	0xff,r12				@\
	  blr						@\
	.else						@\
	  blr						@\
	.endif


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
 * FIXME: Document what makes a leaf a LEAF and a handler a HANDLER.
 * (E.g. leaf's have return pc in lr, NESTED's have rpc in offset off
 * sp, handlers have rpc in exception frame which is found via exception
 * link, etc etc.)
 */

/*
 * TEXT -- declare start of text segment
 */
#define	TEXT						\
	.text						@\
	.align 2

/*
 * LEAF -- declare global leaf procedure
 * NOTE: Control SHOULD NOT FLOW into a LEAF!  A LEAF should only
 * be jumped to.  (A leaf may do an align.)  Use a LABEL() if you
 * need control to flow into the label.
 */
#define	LEAF(name)					\
	.align 2					@\
	.globl	name					@\
name:							@\
	.set	__framesize,0

/*
 * X_LEAF -- declare alternate global label for leaf
 */
#define	X_LEAF(name, value)				\
	.globl	name					@\
	.set	name,value

/*
 * P_LEAF -- declare private leaf procedure
 */
#define	P_LEAF(name)					\
	.align 2					@\
name:							@\
	.set	__framesize,0

/*
 * LABEL -- declare a global code label
 * MUST be used (rather than LEAF, NESTED, etc) if control
 * "flows into" the label.
 */
#define	LABEL(name)					\
	.align 2					@\
	.globl	name					@\
name:

/*
 * NESTED -- declare procedure that invokes other procedures
 */
#define	NESTED(name, localvarsize, maxargsout, grfsaved, fpfsaved)\
	.align 2				@\
	.globl	name				@\
name:						@\
	BUILD_FRAME(localvarsize, maxargsout, grfsaved, fpfsaved)

/*
 * X_NESTED -- declare alternate global label for nested proc
 */
#define	X_NESTED(name, value)			\
	.globl	name				@\
	.set	name,value

/*
 * P_NESTED -- declare private nested procedure
 */
#define	P_NESTED(name, localvarsize, maxargsout, grfsaved, fpfsaved)\
	.align 2					@\
name:							@\
	BUILD_FRAME(locavarsize, maxargsout, grfsaved, fpfsaved)

/*
 * HANDLER -- declare procedure with exception frame rather than
 * standard C frame
 */
#define	HANDLER(name)					\
	.align 2					@\
	.globl	name					@\
name:

/*
 * X_HANDLER -- declare alternate name for exception handler
 * (Should appear immediately before a HANDLER declaration or
 * another X_HANDLER declaration)
 */
#define	X_HANDLER(name)					\
	.align 2					@\
	.globl	name					@\
name:

/*
 * P_HANDLER -- declare private handler
 */
#define	P_HANDLER(name)					\
	.align 2				@\
name:

/*
 * END -- mark end of procedure
 * FIXME: Unimplemented for now.
 */
#define	END(name)

/*
 * BL -- call procedure (relative)
 */
#define	BL(name)					\
	bl	name

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
	.globl	name					@\
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
	.align 2					@\
	.globl	name					@\
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

/*
 * dynamic/PIC macros for routines which reference external symbols
 */
#if defined(__DYNAMIC__)
#define PICIFY_REG r12

/* Assume that the lr is saved before calling any of these macros */
/* using PICIFY() */

#define PICIFY(var)				\
	mflr	r0				@\
	bl	1f				@\
1:	mflr	PICIFY_REG			@\
	mtlr	r0				@\
	addis	PICIFY_REG, PICIFY_REG, ha16(L ## var ## $non_lazy_ptr - 1b) @\
	lwz	PICIFY_REG, lo16(L ## var ## $non_lazy_ptr - 1b)(PICIFY_REG)

#define CALL_EXTERN_AGAIN(var)			\
	PICIFY(var)				@\
	mtctr	PICIFY_REG			@\
	mflr	r0				@\
	stw	r0,8(r1)			@\
	stwu	r1,-56(r1)			@\
	bctrl					@\
	addic	r1,r1,56     			@\
	lwz	r0,8(r1)			@\
	mtlr	r0

#define NON_LAZY_STUB(var)			\
	.non_lazy_symbol_pointer		@\
	.align 2				@\
L ## var ## $non_lazy_ptr:			@\
	.indirect_symbol var			@\
	.long 0					@\
	.text					@\
	.align 2

#define	BRANCH_EXTERN(var)			\
	PICIFY(var)				@\
	mtctr	PICIFY_REG			@\
	bctr					@\
	NON_LAZY_STUB(var)

#define CALL_EXTERN(var)			\
	CALL_EXTERN_AGAIN(var)			@\
	NON_LAZY_STUB(var)

#define REG_TO_EXTERN(reg, var)			\
	PICIFY(var)				@\
	stw reg, 0(PICIFY_REG)			@\
	NON_LAZY_STUB(var)

#define EXTERN_TO_REG(reg, var)			\
	PICIFY(var)				@\
	lwz	reg, 0(PICIFY_REG)		@\
	NON_LAZY_STUB(var)

#else /* ! __DYNAMIC__ */
#define TMP_REG r12
#define BRANCH_EXTERN(var)			\
	b	var

#define CALL_EXTERN(var)			\
	bl	var

#define CALL_EXTERN_AGAIN(var)			\
	CALL_EXTERN(var)

#define REG_TO_EXTERN(reg, var)			\
	lis	TMP_REG, ha16(var)		@\
	stw	reg, lo16(var)(TMP_REG)

#define EXTERN_TO_REG(reg, var)			\
	lis	reg, ha16(var)			@\
	lwz	reg, lo16(var)(reg)

#endif	/* __DYNAMIC__ */

#endif	/* __ASSEMBLER__ */
#endif	/* _ARCH_PPC_ASM_HELP_H_ */
