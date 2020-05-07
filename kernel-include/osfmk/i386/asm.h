/*
 * Copyright (c) 2000-2018 Apple Computer, Inc. All rights reserved.
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

#ifndef	_I386_ASM_H_
#define	_I386_ASM_H_

#if defined(__i386__)

#define S_PC	 (%esp)
#define S_ARG0	 4(%esp)
#define S_ARG1	 8(%esp)
#define S_ARG2	12(%esp)
#define S_ARG3	16(%esp)
#define S_ARG4	20(%esp)

#define FRAME	pushl %ebp; movl %esp, %ebp
#define EMARF	leave

#define B_LINK	 (%ebp)
#define B_PC	 4(%ebp)
#define B_ARG0	 8(%ebp)
#define B_ARG1	12(%ebp)
#define B_ARG2	16(%ebp)
#define B_ARG3	20(%ebp)

#elif defined(__x86_64__)

#define S_PC	 (%rsp)

#define FRAME	pushq %rbp; movq %rsp, %rbp
#define EMARF	leave

#define B_LINK	 (%rbp)
#define B_PC	 8(%rbp)

#else
#error unsupported architecture
#endif

/* There is another definition of ALIGN for .c sources */
#ifdef ASSEMBLER
#define ALIGN 4,0x90
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

#define SVC .byte 0x9a; .long 0; .word 0x7

#define RPC_SVC .byte 0x9a; .long 0; .word 0xf

#define String	.asciz
#define Value	.word
#define Times(a,b) (a*b)
#define Divide(a,b) (a/b)

#define INB	inb	%dx, %al
#define OUTB	outb	%al, %dx
#define INL	inl	%dx, %eax
#define OUTL	outl	%eax, %dx

#define data16	.byte 0x66
#define addr16	.byte 0x67

#define MCOUNT

#if defined(__SHARED__)
#define MCOUNT		; .data;\
			.align ALIGN;\
			LBc(x, 8) .long 0;\
			.text;\
			Gpush;\
			Gload;\
			leal Gotoff(LBb(x,8)),%edx;\
			Egaddr(%eax,_mcount_ptr);\
			Gpop;\
			call *(%eax);
#endif /* __SHARED__ */

#ifdef __ELF__
#define ELF_FUNC(x)	.type x,@function
#define ELF_DATA(x)	.type x,@object
#define ELF_SIZE(x,s)	.size x,s
#else
#define ELF_FUNC(x)
#define ELF_DATA(x)
#define ELF_SIZE(x,s)
#endif

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

/*
 * ELF shared library accessor macros.
 * Gpush saves the %ebx register used for the GOT address
 * Gpop pops %ebx if we need a GOT
 * Gload loads %ebx with the GOT address if shared libraries are used
 * Gcall calls an external function.
 * Gotoff allows you to reference local labels.
 * Gotoff2 allows you to reference local labels with an index reg.
 * Gotoff3 allows you to reference local labels with an index reg & size.
 * Gaddr loads up a register with an address of an external item.
 * Gstack is the number of bytes that Gpush pushes on the stack.
 *
 * Varients of the above with E or L prefixes do EXT(name) or LCL(name)
 * respectively.
 */

#ifndef __SHARED__
#define Gpush
#define Gpop
#define Gload
#define Gcall(func)		call func
#define Gotoff(lab)		lab
#define Gotoff2(l,r)		l(r)
#define Gotoff3(l,r,s)		l(,r,s)
#define Gaddr(to,lab)		movl $lab,to
#define Gcmp(lab,reg)		cmpl $lab,reg
#define Gmemload(lab,reg)	movl lab,reg
#define Gmemstore(reg,lab,tmp)	movl reg,lab
#define Gstack			0

#else
#ifdef __ELF__			/* ELF shared libraries */
#define Gpush			pushl %ebx
#define Gpop			popl %ebx
#define Gload			call 9f; 9: popl %ebx; addl $_GLOBAL_OFFSET_TABLE_+[.-9b],%ebx
#define Gcall(func)		call EXT(func)@PLT
#define Gotoff(lab)		lab@GOTOFF(%ebx)
#define Gotoff2(l,r)		l@GOTOFF(%ebx,r)
#define Gotoff3(l,r,s)		l@GOTOFF(%ebx,r,s)
#define Gaddr(to,lab)		movl lab@GOT(%ebx),to
#define Gcmp(lab,reg)		cmpl reg,lab@GOT(%ebx)
#define Gmemload(lab,reg)	movl lab@GOT(%ebx),reg; movl (reg),reg
#define Gmemstore(reg,lab,tmp)	movl lab@GOT(%ebx),tmp; movl reg,(tmp)
#define Gstack			4

#else				/* ROSE shared libraries */
#define Gpush
#define Gpop
#define Gload
#define Gcall(func)		call *9f; .data; .align ALIGN; 9: .long func; .text
#define Gotoff(lab)		lab
#define Gotoff2(l,r)		l(r)
#define Gotoff3(l,r,s)		l(,r,s)
#define Gaddr(to,lab)		movl 9f,to; .data; .align ALIGN; 9: .long lab; .text
#define Gcmp(lab,reg)		cmpl reg,9f; .data; .align ALIGN; 9: .long lab; .text
#define Gmemload(lab,reg)	movl 9f,reg; movl (reg),reg; .data; .align ALIGN; 9: .long lab; .text
#define Gmemstore(reg,lab,tmp)	movl 9f,tmp; movl reg,(tmp); .data; .align ALIGN; 9: .long lab; .text
#define Gstack			0
#endif	/* __ELF__ */
#endif	/* __SHARED__ */

/* Egotoff is not provided, since external symbols should not use @GOTOFF
   relocations.  */
#define Egcall(func)		Gcall(EXT(func))
#define Egaddr(to,lab)		Gaddr(to,EXT(lab))
#define Egcmp(lab,reg)		Gcmp(EXT(lab),reg)
#define Egmemload(lab,reg)	Gmemload(EXT(lab),reg)
#define Egmemstore(reg,lab,tmp)	Gmemstore(reg,EXT(lab),tmp)

#define Lgotoff(lab)		Gotoff(LCL(lab))
#define Lgotoff2(l,r)		Gotoff2(LCL(l),r)
#define Lgotoff3(l,r,s)		Gotoff3(LCL(l),r,s)
#define Lgcmp(lab,reg)		Gcmp(LCL(lab),reg)
#define Lgmemload(lab,reg)	movl Lgotoff(lab),reg
#define Lgmemstore(reg,lab,tmp)	movl reg,Lgotoff(lab)

#ifndef ASSEMBLER
/* These defines are here for .c files that wish to reference global symbols
 * within __asm__ statements. 
 */
#ifndef __NO_UNDERSCORES__
#define CC_SYM_PREFIX "_"
#else
#define CC_SYM_PREFIX ""
#endif /* __NO_UNDERSCORES__ */
#endif /* ASSEMBLER */

/*
 * The following macros make calls into C code.
 * They dynamically align the stack to 16 bytes.
 */
#if defined(__i386__)
/*
 * Arguments are moved (not pushed) onto the correctly aligned stack.
 * NOTE: ESI is destroyed in the process, and hence cannot
 * be directly used as a parameter. Users of this macro must
 * independently preserve ESI (a non-volatile) if the routine is
 * intended to be called from C, for instance.
 */

#define CCALL(fn)			\
	movl	%esp, %esi		;\
	andl	$0xFFFFFFF0, %esp	;\
	call	EXT(fn)			;\
	movl	%esi, %esp

#define CCALL1(fn, arg1)		\
	movl	%esp, %esi		;\
	subl	$4, %esp		;\
	andl	$0xFFFFFFF0, %esp	;\
	movl	arg1, (%esp)		;\
	call	EXT(fn)			;\
	movl	%esi, %esp

#define CCALL2(fn, arg1, arg2)		\
	movl	%esp, %esi		;\
	subl	$8, %esp		;\
	andl	$0xFFFFFFF0, %esp	;\
	movl	arg2, 4(%esp)		;\
	movl	arg1, (%esp)		;\
	call	EXT(fn)			;\
	movl	%esi, %esp

/* This variant exists to permit adjustment of the stack by "dtrace" */
#define CCALL1WITHSP(fn, arg1)		\
	movl	%esp, %esi		;\
	subl	$12, %esp		;\
	andl	$0xFFFFFFF0, %esp	;\
	movl	%esi, 8(%esp)		;\
	leal	8(%esp), %esi		;\
	movl	%esi, 4(%esp)		;\
	movl	arg1, (%esp)		;\
	call	EXT(fn)			;\
	movl	8(%esp), %esp

/*
 * CCALL5 is used for callee functions with 3 arguments but
 * where arg2 (a3:a2) and arg3 (a5:a4) are 64-bit values.
 */
#define CCALL5(fn, a1, a2, a3, a4, a5)	\
	movl	%esp, %esi		;\
	subl	$20, %esp		;\
	andl	$0xFFFFFFF0, %esp	;\
	movl	a5, 16(%esp)		;\
	movl	a4, 12(%esp)		;\
	movl	a3,  8(%esp)		;\
	movl	a2,  4(%esp)		;\
	movl	a1,  (%esp)		;\
	call	EXT(fn)			;\
	movl	%esi, %esp

#elif defined(__x86_64__)

/* This variant exists to permit adjustment of the stack by "dtrace" */
#define CCALLWITHSP(fn)				 \
	mov	%rsp, %r12			;\
	sub	$8, %rsp			;\
	and	$0xFFFFFFFFFFFFFFF0, %rsp	;\
	mov	%r12, (%rsp)			;\
	leaq	(%rsp), %rsi			;\
	call	EXT(fn)				;\
	mov	(%rsp), %rsp

#define CCALL(fn)				 \
	mov	%rsp, %r12			;\
	and	$0xFFFFFFFFFFFFFFF0, %rsp	;\
	call	EXT(fn)				;\
	mov	%r12, %rsp

#define CCALL1(fn, arg1) 			 \
	mov	arg1, %rdi 			;\
	CCALL(fn)

#define CCALL2(fn, arg1, arg2)		 	 \
	mov	arg1, %rdi 			;\
	mov	arg2, %rsi 			;\
	CCALL(fn)

#define CCALL3(fn, arg1, arg2, arg3) 		 \
	mov	arg1, %rdi 			;\
	mov	arg2, %rsi 			;\
	mov	arg3, %rdx 			;\
	CCALL(fn)

#else
#error unsupported architecture
#endif

#endif /* _I386_ASM_H_ */
