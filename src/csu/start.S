/*
 * Copyright (c) 1999-2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <Availability.h>

#if __ppc__ && __DYNAMIC__
//
// Force stub section next to __text section to minimize chance that
// a bl to a stub will be out of range.
//
	.text
	.symbol_stub
	.picsymbol_stub
#endif

/*
 * C runtime startup for ppc, ppc64, i386, x86_64
 *
 * Kernel sets up stack frame to look like:
 *
 *	       :
 *	| STRING AREA |
 *	+-------------+
 *	|      0      |	
 *	+-------------+	
 *	|  exec_path  | extra "apple" parameters start after NULL terminating env array
 *	+-------------+
 *	|      0      |
 *	+-------------+
 *	|    env[n]   |
 *	+-------------+
 *	       :
 *	       :
 *	+-------------+
 *	|    env[0]   |
 *	+-------------+
 *	|      0      |
 *	+-------------+
 *	| arg[argc-1] |
 *	+-------------+
 *	       :
 *	       :
 *	+-------------+
 *	|    arg[0]   |
 *	+-------------+
 *	|     argc    | argc is always 4 bytes long, even in 64-bit architectures
 *	+-------------+ <- sp
 *
 *	Where arg[i] and env[i] point into the STRING AREA
 */

	.text
	.globl start
	.align 2

#if __ppc__
start:	mr      r26,r1              ; save original stack pointer into r26
	subi	r1,r1,4		    ; make space for linkage
	clrrwi	r1,r1,5             ; align to 32 bytes (good enough for 32- and 64-bit APIs)
	li      r0,0                ; load 0 into r0
	stw     r0,0(r1)            ; terminate initial stack frame
	stwu	r1,-64(r1)	    ; allocate minimal stack frame
	lwz     r3,0(r26)           ; get argc into r3
	addi    r4,r26,4	    ; get argv into r4
	addi	r27,r3,1            ; calculate argc + 1 into r27
	slwi	r27,r27,2	    ; calculate (argc + 1) * sizeof(char *) into r27
	add     r5,r4,r27           ; get address of env[0] into r5
#if OLD_LIBSYSTEM_SUPPORT	
	bl	__start		    ; 24-bt branch to __start.  ld64 will make a branch island if needed
	trap                        ; should never return
#else
	mr	r6,r5
Lapple:	lwz	r0,0(r6)	    ; look for NULL ending env[] array
	addi	r6,r6,4
	cmpwi	r0,0
	bne	Lapple		    ; once found, next pointer is "apple" parameter now in r6
	bl	_main
	b	_exit		    ; pass result from main() to exit()
#endif
#endif // __ppc__ 


#if __ppc64__
start:	mr      r26,r1              ; save original stack pointer into r26
	subi	r1,r1,8		    ; make space for linkage
	clrrdi	r1,r1,5             ; align to 32 bytes (good enough for 32- and 64-bit APIs)
	li      r0,0                ; load 0 into r0
	std     r0,0(r1)            ; terminate initial stack frame
	stdu	r1,-128(r1)	    ; allocate minimal stack frame
	lwz     r3,0(r26)           ; get argc into r3
	addi    r4,r26,8	    ; get argv into r4
	addi	r27,r3,1            ; calculate argc + 1 into r27
	sldi	r27,r27,3	    ; calculate (argc + 1) * sizeof(char *) into r27
	add     r5,r4,r27           ; get address of env[0] into r5
#if OLD_LIBSYSTEM_SUPPORT	
	bl	__start		    ; 24-bt branch to __start.  ld64 will make a branch island if needed
	trap                        ; should never return
#else
	mr	r6,r5
Lapple:	ld	r0,0(r6)	    ; look for NULL ending env[] array
	addi	r6,r6,8
	cmpdi	r0,0
	bne	Lapple		    ; once found, next pointer is "apple" parameter now in r6
	bl	_main
	b	_exit		    ; pass result from main() to exit()
#endif
#endif	// __ppc64__


#if __i386__
start:	pushl	$0		    # push a zero for debugger end of frames marker
	movl	%esp,%ebp	    # pointer to base of kernel frame
	andl    $-16,%esp	    # force SSE alignment
	subl    $16,%esp	    # room for new argc, argv, & envp, SSE aligned
	movl	4(%ebp),%ebx	    # pickup argc in %ebx
	movl	%ebx,0(%esp)	    # argc to reserved stack word
	lea	8(%ebp),%ecx	    # addr of arg[0], argv, into %ecx
	movl	%ecx,4(%esp)	    # argv to reserved stack word
	addl	$1,%ebx		    # argc + 1 for zero word
	sall	$2,%ebx		    # * sizeof(char *)
	addl	%ecx,%ebx	    # addr of env[0], envp, into %ebx
	movl	%ebx,8(%esp)	    # envp to reserved stack word
#if OLD_LIBSYSTEM_SUPPORT	
	call	__start		    # call _start(argc, argv, envp)
	hlt			    # should never return
#else
Lapple:	movl	(%ebx),%eax	    # look for NULL ending env[] array
	add	$4,%ebx
	testl	%eax,%eax
	jne	Lapple		    # once found, next pointer is "apple" parameter now in %ebx
	movl	%ebx,12(%esp)	    # apple to reserved stack word
	call	_main
	movl	%eax, 0(%esp)	    # pass result from main() to exit() 
	call	_exit		    # need to use call to keep stack aligned
	hlt
#endif
#endif // __i386__ 



#if __x86_64__
start:	pushq	$0		    # push a zero for debugger end of frames marker
	movq	%rsp,%rbp	    # pointer to base of kernel frame
	andq    $-16,%rsp	    # force SSE alignment
	movq	8(%rbp),%rdi	    # put argc in %rdi
	leaq	16(%rbp),%rsi	    # addr of arg[0], argv, into %rsi
	movl	%edi,%edx	    # copy argc into %rdx
	addl	$1,%edx		    # argc + 1 for zero word
	sall	$3,%edx		    # * sizeof(char *)
	addq	%rsi,%rdx	    # addr of env[0], envp, into %rdx
#if OLD_LIBSYSTEM_SUPPORT	
	call	__start		    # call _start(argc, argv, envp)
	hlt			    # should never return
#else
	movq	%rdx,%rcx
	jmp	Lapple2
Lapple:	add	$8,%rcx
Lapple2:cmpq	$0,(%rcx)	    # look for NULL ending env[] array
	jne	Lapple		    
	add	$8,%rcx		    # once found, next pointer is "apple" parameter now in %rcx
	call	_main
	movl	%eax,%edi	    # pass result from main() to exit() 
	call	_exit		    # need to use call to keep stack aligned
	hlt
#endif
#endif // __x86_64__ 

#ifdef __arm__
start:
	ldr	r0, [sp]		// get argc into r0
	add	r1, sp, #4		// get argv into r1
	add	r4, r0, #1		// calculate argc + 1 into r4
	add	r2, r1, r4, lsl #2	// get address of env[0] into r2
	bic	sp, sp, #15		// force sixteen-byte alignment
#if OLD_LIBSYSTEM_SUPPORT	
	bl	__start
	.long	0xe1200070		// should never return
#else
	mov	r3, r2 
Lapple:
	ldr	r4, [r3], #4		// look for NULL ending env[] array
	cmp	r4, #0
	bne	Lapple			
					// "apple" param now in r3
#if __STATIC__ || ((__IPHONE_OS_VERSION_MIN_REQUIRED >= 30100) && !__ARM_ARCH_4T__)
	bl	_main
	b	_exit
#else
// use -mlong-branch style call sites so that main executable can be >32MB 
	ldr	ip, L4
L2:	add	ip, pc, ip
	ldr	ip, [ip, #0]
#if __ARM_ARCH_4T__
	mov lr, pc		// blx not supported, so simulate it in two steps
	bx  ip
#else
	blx	ip			// call main()
#endif
	
	ldr	ip, L5
L3:	add	ip, pc, ip
	ldr	ip, [ip, #0]
	bx	ip			// jmp exit()
	
L4:	.long	L_main$non_lazy_ptr-(L2+8)
L5:	.long	L_exit$non_lazy_ptr-(L3+8)

	.non_lazy_symbol_pointer
L_main$non_lazy_ptr:
	.indirect_symbol _main
	.long	0
L_exit$non_lazy_ptr:
	.indirect_symbol _exit
	.long	0
#endif


#endif
#endif /* __arm__ */


#if __arm64__

start:
	mov     x5, sp
	ldr     x0, [x5]            ; get argc into x0 (kernel passes 32-bit int argc as 64-bits on stack to keep alignment)
	add     x1, x5, #8          ; get argv into x1
	add     x4, x0, #1          ; argc + 1
	add     x2, x1, x4, lsl #3  ; &env[0] = (argc+1)*8
	and     sp, x5, #~15        ; force 16-byte alignment of stack
	mov     x3, x2
L1: ldr     x4, [x3], #8
	cmp     x4, #0              ; look for NULL ending env[] array
	b.ne     L1
	bl      _main               ; main(x0=argc, x1=argv, x2=envp, x3=apple)
	b       _exit

#endif /* __arm64__ */


// This code has be written to allow dead code stripping
	.subsections_via_symbols
