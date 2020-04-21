/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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

#ifndef	_ARM_ASM_H_
#define	_ARM_ASM_H_

#include <arm/arch.h>

#define FRAME	pushl %ebp; movl %esp, %ebp
#define EMARF	leave


/* There is another definition of ALIGN for .c sources */
#ifdef ASSEMBLER
#define ALIGN 2
#endif /* ASSEMBLER */

#ifndef FALIGN
#define FALIGN ALIGN
#endif

#define LB(x,n) n
#if	__STDC__
#ifndef __NO_UNDERSCORES__
#define	LCL(x)	L ## x
#define EXT(x) _ ## x
#define LEXT(x) _ ## x ## :
#else
#define	LCL(x)	.L ## x
#define EXT(x) x
#define LEXT(x) x ## :
#endif
#define LBc(x,n) n ## :
#define LBb(x,n) n ## b
#define LBf(x,n) n ## f
#else /* __STDC__ */
#ifndef __NO_UNDERSCORES__
#define LCL(x) L/**/x
#define EXT(x) _/**/x
#define LEXT(x) _/**/x/**/:
#else /* __NO_UNDERSCORES__ */
#define	LCL(x)	.L/**/x
#define EXT(x) x
#define LEXT(x) x/**/:
#endif /* __NO_UNDERSCORES__ */
#define LBc(x,n) n/**/:
#define LBb(x,n) n/**/b
#define LBf(x,n) n/**/f
#endif /* __STDC__ */

#define String	.asciz
#define Value	.word
#define Times(a,b) (a*b)
#define Divide(a,b) (a/b)

#if 0 /* TOTOJK */
#ifdef __ELF__
#define ELF_FUNC(x)	.type x,@function
#define ELF_DATA(x)	.type x,@object
#define ELF_SIZE(x,s)	.size x,s
#else
#define ELF_FUNC(x)
#define ELF_DATA(x)
#define ELF_SIZE(x,s)
#endif
#else
#define ELF_FUNC(x)
#define ELF_DATA(x)
#define ELF_SIZE(x,s)
#endif /* TODOJK */

#define	Entry(x)	.globl EXT(x); ELF_FUNC(EXT(x)); .align FALIGN; LEXT(x)
#define	ENTRY(x)	Entry(x) MCOUNT
#define	ENTRY2(x,y)	.globl EXT(x); .globl EXT(y); \
			ELF_FUNC(EXT(x)); ELF_FUNC(EXT(y)); \
			.align FALIGN; LEXT(x); LEXT(y) \
			MCOUNT
#if __STDC__
#define	ASENTRY(x) 	.globl x; .align FALIGN; x ## : ELF_FUNC(x) MCOUNT
#else
#define	ASENTRY(x) 	.globl x; .align FALIGN; x: ELF_FUNC(x) MCOUNT
#endif /* __STDC__ */

#define	DATA(x)		.globl EXT(x); ELF_DATA(EXT(x)); .align ALIGN; LEXT(x)

#define End(x)		ELF_SIZE(x,.-x)
#define END(x)		End(EXT(x))
#define ENDDATA(x)	END(x)
#define Enddata(x)	End(x)

#ifdef ASSEMBLER

#define MCOUNT

#else /* NOT ASSEMBLER */

/* These defines are here for .c files that wish to reference global symbols
 * within __asm__ statements. 
 */
#ifndef __NO_UNDERSCORES__
#define CC_SYM_PREFIX "_"
#else
#define CC_SYM_PREFIX ""
#endif /* __NO_UNDERSCORES__ */
#endif /* ASSEMBLER */

#ifdef ASSEMBLER

#if defined (_ARM_ARCH_4T)
# define RET    bx      lr
# define RETeq  bxeq    lr
# define RETne  bxne    lr
# ifdef __STDC__
#  define RETc(c) bx##c lr
# else
#  define RETc(c) bx/**/c       lr
# endif
#else
# define RET    mov     pc, lr
# define RETeq  moveq   pc, lr
# define RETne  movne   pc, lr
# ifdef __STDC__
#  define RETc(c) mov##c        pc, lr
# else
#  define RETc(c) mov/**/c      pc, lr
# endif
#endif

#if defined (__thumb__)
/* Provide a PI mechanism for thumb branching. */
# define BRANCH_EXTERN(x)	ldr	pc, [pc, #-4] ;	\
				.long	EXT(x)
#else
# define BRANCH_EXTERN(x)	b	EXT(x)
#endif

/*
 * arg0: Register for thread pointer
 */
.macro READ_THREAD
	mrc p15, 0, $0, c13, c0, 4  /* Read TPIDRPRW */
.endmacro


/* Macros for loading up addresses that are external to the .s file.
 * LOAD_ADDR:  loads the address for (label) into (reg). Not safe for
 *   loading to the PC.
 * LOAD_ADDR_PC:  Variant for loading to the PC; load the address of (label)
 *   into the pc.
 * LOAD_ADDR_GEN_DEF:  The general definition needed to support loading
 *   a label address.
 *
 * Usage:  For any label accessed, we require one (and only one) instance
 *   of LOAD_ADDR_GEN_DEF(label).
 * 
 * Example:
 *   LOAD_ADDR(r0, arm_init)
 *   LOAD_ADDR(lr, arm_init_cpu)
 *   LOAD_ADDR_PC(arm_init)
 *   ...
 *
 *   LOAD_ADDR_GEN_DEF(arm_init)
 *   LOAD_ADDR_GEN_DEF(arm_init_cpu)
 */

#if SLIDABLE
/* Definitions for a position dependent kernel using non-lazy pointers.
 */

/* TODO: Make this work with thumb .s files. */
#define PC_INC	0x8

/* We need wrapper macros in order to ensure that __LINE__ is expanded.
 *
 * There is some small potential for duplicate labels here, but because
 *   we do not export the generated labels, it should not be an issue.
 */

#define GLUE_LABEL_GUTS(label, tag) L_##label##_##tag##_glue
#define GLUE_LABEL(label, tag) GLUE_LABEL_GUTS(label, tag)

#define LOAD_ADDR(reg, label)                                                                   \
	movw	reg, :lower16:(label##$non_lazy_ptr - (GLUE_LABEL(label, __LINE__) + PC_INC)) ; \
	movt	reg, :upper16:(label##$non_lazy_ptr - (GLUE_LABEL(label, __LINE__) + PC_INC)) ; \
GLUE_LABEL(label, __LINE__): ;                                                                  \
	ldr	reg, [pc, reg]

/* Designed with the understanding that directly branching to thumb code
 *   is unreliable; this should allow for dealing with __thumb__ in
 *   assembly; the non-thumb variant still needs to provide the glue label
 *   to avoid failing to build on undefined symbols.
 *
 * TODO: Make this actually use a scratch register; this macro is convenient
 *   for translating (ldr pc, [?]) to a slidable format without the risk of
 *   clobbering registers, but it is also wasteful.
 */
#if defined(__thumb__)
#define LOAD_ADDR_PC(label)    \
	stmfd	sp!, { r0 } ;  \
	stmfd	sp!, { r0 } ;  \
	LOAD_ADDR(r0, label) ; \
	str	r0, [sp, #4] ; \
	ldmfd	sp!, { r0 } ;  \
	ldmfd	sp!, { pc }
#else
#define LOAD_ADDR_PC(label) \
	b	EXT(label)
#endif

#define LOAD_ADDR_GEN_DEF(label)                                   \
	.section __DATA,__nl_symbol_ptr,non_lazy_symbol_pointers ; \
	.align 2 ;                                                 \
label##$non_lazy_ptr: ;                                            \
	.indirect_symbol	EXT(label) ;                       \
	.long			0

#else /* !SLIDABLE */

/* Definitions for a position dependent kernel */
#define LOAD_ADDR(reg, label)  \
	ldr	reg, L_##label

#if defined(__thumb__)
#define LOAD_ADDR_PC(label)   \
	ldr	pc, L_##label
#else
#define LOAD_ADDR_PC(label) \
	b	EXT(label)
#endif

#define LOAD_ADDR_GEN_DEF(label)  \
	.text ;                   \
	.align 2 ;                \
L_##label: ;                      \
	.long	EXT(label)

#endif /* SLIDABLE */

/* The linker can deal with branching from ARM to thumb in unconditional
 *   branches, but not in conditional branches.  To support this in our
 *   assembly (which allows us to build xnu without -mno-thumb), use the
 *   following macros for branching conditionally to external symbols.
 *   These macros are used just like the corresponding conditional branch
 *   instructions.
 */

#define SHIM_LABEL_GUTS(line_num) L_cond_extern_##line_num##_shim
#define SHIM_LABEL(line_num) SHIM_LABEL_GUTS(line_num)

#define COND_EXTERN_BEQ(label)         \
	bne	SHIM_LABEL(__LINE__) ; \
	b	EXT(label) ;           \
SHIM_LABEL(__LINE__):

#define COND_EXTERN_BLNE(label)        \
	beq	SHIM_LABEL(__LINE__) ; \
	bl	EXT(label) ;           \
SHIM_LABEL(__LINE__):

#define COND_EXTERN_BLGT(label)        \
	ble	SHIM_LABEL(__LINE__) ; \
	bl	EXT(label) ;           \
SHIM_LABEL(__LINE__):

#endif /* ASSEMBLER */

#endif /* _ARM_ASM_H_ */
