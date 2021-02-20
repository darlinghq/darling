/*
 * Copyright (c) 1999-2008 Apple Inc. All rights reserved.
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
 
 

#if __i386__
/*
 * This is the implementation of dyld_lazy_dylib_stub_binding_helper for i386
 * on versions before Macs OS X 10.6.  On entry the address of the lazy pointer
 * has been pushed on the stack.
 *
 * After the symbol has been resolved and the lazy pointer filled in, this jumps
 * to the target address.
 */
#define LP_PARAM_OUT			0
#define XMMM0_SAVE			16	/* 16-byte align */
#define XMMM1_SAVE			32
#define XMMM2_SAVE			48
#define XMMM3_SAVE			64
#define EAX_SAVE			84
#define ECX_SAVE			88
#define EDX_SAVE			92
#define LP_LOCAL			96
#define STACK_SIZE			104	/* must be 8 mod 16 so that stack winds up 16-byte aliged  */
#define LP_OLD_BP_SAVE			104

    .text
    .align 4,0x90
    .globl dyld_lazy_dylib_stub_binding_helper
    .private_extern dyld_lazy_dylib_stub_binding_helper
dyld_lazy_dylib_stub_binding_helper:
	subl		$STACK_SIZE,%esp	    # makes stack 16-byte aligned
	movl		%eax,EAX_SAVE(%esp)	
	movl		LP_OLD_BP_SAVE(%esp),%eax   # get lazy-pointer meta-parameter
	movl		%eax,LP_LOCAL(%esp)	
	movl		%ebp,LP_OLD_BP_SAVE(%esp)   # store epb back chain
	movl		%esp,%ebp		    # set epb to be this frame
	add		$LP_OLD_BP_SAVE,%ebp
	movl		%ecx,ECX_SAVE(%esp)
	movl		%edx,EDX_SAVE(%esp)
	movdqu		%xmm0,XMMM0_SAVE(%esp)
	movdqu		%xmm1,XMMM1_SAVE(%esp)
	movdqu		%xmm2,XMMM2_SAVE(%esp)
	movdqu		%xmm3,XMMM3_SAVE(%esp)
	movl		LP_LOCAL(%esp),%eax
	movl		%eax,LP_PARAM_OUT(%esp) # call lazy_load_dylib(lazy_ptr)
	call		_lazy_load_dylib
	movdqu		XMMM0_SAVE(%esp),%xmm0	# restore registers
	movdqu		XMMM1_SAVE(%esp),%xmm1
	movdqu		XMMM2_SAVE(%esp),%xmm2
	movdqu		XMMM3_SAVE(%esp),%xmm3
	movl		ECX_SAVE(%esp),%ecx
	movl		EDX_SAVE(%esp),%edx
	movl		%eax,%ebp		# move target address to epb
	movl		EAX_SAVE(%esp),%eax	# restore eax
	addl		$STACK_SIZE,%esp	# cut back stack
	xchg		%ebp, (%esp)		# restore ebp and set target to top of stack
	ret					# jump to target
    
#endif /* __i386__ */


#if __x86_64__
/*
 * This is the implementation of dyld_lazy_dylib_stub_binding_helper for x86_64
 * on versions before Macs OS X 10.6.  On entry r11 contains address of the 
 * lazy pointer.
 *
 * All parameters registers must be preserved.
 * 
 * After the symbol has been resolved and the pointer filled in this is to pop
 * these arguments off the stack and jump to the address of the defined symbol.
 */
#define RDI_SAVE			0
#define RSI_SAVE			8
#define RDX_SAVE			16
#define RCX_SAVE			24
#define R8_SAVE				32
#define R9_SAVE				40
#define RAX_SAVE			48
#define XMMM0_SAVE			64    /* 16-byte align */
#define XMMM1_SAVE			80
#define XMMM2_SAVE			96
#define XMMM3_SAVE			112
#define XMMM4_SAVE			128
#define XMMM5_SAVE			144
#define XMMM6_SAVE			160
#define XMMM7_SAVE			176
#define STACK_SIZE			192 /*  (XMMM7_SAVE+16) must be 16 byte aligned too */
    
    .text
    .align 2,0x90
    .globl dyld_lazy_dylib_stub_binding_helper
    .private_extern dyld_lazy_dylib_stub_binding_helper
dyld_lazy_dylib_stub_binding_helper:
	pushq		%rbp
	movq		%rsp,%rbp
	subq		$STACK_SIZE,%rsp
	movq		%rdi,RDI_SAVE(%rsp)	# save registers that might be used as parameters
	movq		%rsi,RSI_SAVE(%rsp)
	movq		%rdx,RDX_SAVE(%rsp)
	movq		%rcx,RCX_SAVE(%rsp)
	movq		%r8,R8_SAVE(%rsp)
	movq		%r9,R9_SAVE(%rsp)
	movq		%rax,RAX_SAVE(%rsp)
	movdqa		%xmm0,XMMM0_SAVE(%rsp)
	movdqa		%xmm1,XMMM1_SAVE(%rsp)
	movdqa		%xmm2,XMMM2_SAVE(%rsp)
	movdqa		%xmm3,XMMM3_SAVE(%rsp)
	movdqa		%xmm4,XMMM4_SAVE(%rsp)
	movdqa		%xmm5,XMMM5_SAVE(%rsp)
	movdqa		%xmm6,XMMM6_SAVE(%rsp)
	movdqa		%xmm7,XMMM7_SAVE(%rsp)
	movq		%r11,%rdi			# call lazy_load_dylib(lazy_ptr)
	call		_lazy_load_dylib
	movq		%rax,%r11			# save target
	movdqa		XMMM0_SAVE(%rsp),%xmm0	# restore registers
	movdqa		XMMM1_SAVE(%rsp),%xmm1
	movdqa		XMMM2_SAVE(%rsp),%xmm2
	movdqa		XMMM3_SAVE(%rsp),%xmm3
	movdqa		XMMM4_SAVE(%rsp),%xmm4
	movdqa		XMMM5_SAVE(%rsp),%xmm5
	movdqa		XMMM6_SAVE(%rsp),%xmm6
	movdqa		XMMM7_SAVE(%rsp),%xmm7
	movq		RDI_SAVE(%rsp),%rdi
	movq		RSI_SAVE(%rsp),%rsi
	movq		RDX_SAVE(%rsp),%rdx
	movq		RCX_SAVE(%rsp),%rcx
	movq		R8_SAVE(%rsp),%r8
	movq		R9_SAVE(%rsp),%r9
	movq		RAX_SAVE(%rsp),%rax
	addq		$STACK_SIZE,%rsp
	popq		%rbp
	jmp			*%r11			# jmp to target

#endif


#if __ppc__ || __ppc64__
#include <architecture/ppc/mode_independent_asm.h>
/*
 * This is the implementation of dyld_lazy_dylib_stub_binding_helper for ppc
 * on versions before Macs OS X 10.6.  On entry r11 contains address of the 
 * lazy pointer to be filled
 *
 * r11 address of lazy pointer
 */
#define LRSAVE		MODE_CHOICE(8,16)
#define STACK_SIZE	MODE_CHOICE(144,288)
#define R3SAVE          MODE_CHOICE(56,112)
#define R4SAVE          MODE_CHOICE(60,120)
#define R5SAVE          MODE_CHOICE(64,128)
#define R6SAVE          MODE_CHOICE(68,136)
#define R7SAVE          MODE_CHOICE(72,144)
#define R8SAVE          MODE_CHOICE(76,152)
#define R9SAVE          MODE_CHOICE(80,160)
#define R10SAVE         MODE_CHOICE(84,168)

  
	.text
	.align 2
    .globl dyld_lazy_dylib_stub_binding_helper
    .private_extern dyld_lazy_dylib_stub_binding_helper
dyld_lazy_dylib_stub_binding_helper:
	mflr	r0		    ; get link register value
	stg	r0,LRSAVE(r1)	    ; save link register value in the linkage area
	stgu	r1,-STACK_SIZE(r1)  ; save stack pointer and update it

	stg	r3,R3SAVE(r1)	; save all registers that could contain
	stg	r4,R4SAVE(r1)	;  parameters to the routine that is being
	stg	r5,R5SAVE(r1)	;  bound.
	stg	r6,R6SAVE(r1)
	stg	r7,R7SAVE(r1)
	stg	r8,R8SAVE(r1)
	stg	r9,R9SAVE(r1)
	stg	r10,R10SAVE(r1)

	mr	r3,r11		; move address of lazy pointer to 1st parameter
				; call lazy_load_dylib(lazy_ptr)
	bl	_lazy_load_dylib
	mr	r12,r3		; move the symbol`s address into r12
	mtctr	r12		; move the symbol`s address into count register

	lg	r0,STACK_SIZE+LRSAVE(r1)	; get old link register value

	lg	r3,R3SAVE(r1)	; restore all registers that could contain
	lg	r4,R4SAVE(r1)	;  parameters to the routine that was bound.
	lg	r5,R5SAVE(r1)
	lg	r6,R6SAVE(r1)
	lg	r7,R7SAVE(r1)
	lg	r8,R8SAVE(r1)
	lg	r9,R9SAVE(r1)
	lg	r10,R10SAVE(r1)

	addi	r1,r1,STACK_SIZE; restore old stack pointer
	mtlr	r0		; restore link register

	bctr			; jump to the symbol`s address that was bound

#endif /* __ppc__ */

#if __arm__
/*
 * This is the implementation of dyld_lazy_dylib_stub_binding_helper for ARM
 * The caller has pushed the address of the a lazy pointer to be filled in with
 * the value for the defined symbol
 *
 * ip        address of lazy pointer
 * 
 * After the symbol has been resolved and the pointer filled in this is to pop
 * these arguments off the stack and jump to the address of the defined symbol.
 */
	.text
	.align 2
    .globl dyld_lazy_dylib_stub_binding_helper
    .private_extern dyld_lazy_dylib_stub_binding_helper
dyld_lazy_dylib_stub_binding_helper:
	stmfd	sp!, {r0,r1,r2,r3,r7,lr}	// save registers
	add	r7, sp, #16			// point FP to previous FP

	mov	r0, ip				// move address of lazy pointer to 1st parameter
						// call lazy_load_dylib(lazy_ptr)
	bl	_lazy_load_dylib
	mov	ip, r0				// move the symbol`s address into ip

	ldmfd	sp!, {r0,r1,r2,r3,r7,lr}	// restore registers

	bx	ip				// jump to the symbol`s address that was bound

#endif /* __arm__ */




// This code has be written to allow dead code stripping
	.subsections_via_symbols





