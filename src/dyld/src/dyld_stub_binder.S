/*
 * Copyright (c) 2008-2013 Apple Inc. All rights reserved.
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
 
#include <TargetConditionals.h>
 
#include <System/machine/cpu_capabilities.h>


#ifdef __i386__

#define MH_PARAM_OUT			0
#define LP_PARAM_OUT			4
#define XMMM0_SAVE			16	/* 16-byte align */
#define XMMM1_SAVE			32
#define XMMM2_SAVE			48
#define XMMM3_SAVE			64
#define EAX_SAVE			84
#define ECX_SAVE			88
#define EDX_SAVE			92
#define LP_LOCAL			96
#define MH_LOCAL			100
#define STACK_SIZE			100	/* must be 4 mod 16 so that stack winds up 16-byte aliged  */
#define LP_OLD_BP_SAVE			104

/*    
 * sp+4	lazy binding info offset
 * sp+0	address of ImageLoader cache
 */
    .text
    .align 4,0x90
    .globl dyld_stub_binder
    .globl _misaligned_stack_error
dyld_stub_binder:
	subl		$STACK_SIZE,%esp	    # makes stack 16-byte aligned
	movl		%eax,EAX_SAVE(%esp)	
	movl		LP_OLD_BP_SAVE(%esp),%eax   # get lazy-pointer meta-parameter
	movl		%eax,LP_LOCAL(%esp)	
	movl		%ebp,LP_OLD_BP_SAVE(%esp)   # store epb back chain
	movl		%esp,%ebp		    # set epb to be this frame
	add 		$LP_OLD_BP_SAVE,%ebp
	movl		%ecx,ECX_SAVE(%esp)
	movl		%edx,EDX_SAVE(%esp)
	.align 0,0x90
_misaligned_stack_error_:
	movdqa		%xmm0,XMMM0_SAVE(%esp)
	movdqa		%xmm1,XMMM1_SAVE(%esp)
	movdqa		%xmm2,XMMM2_SAVE(%esp)
	movdqa		%xmm3,XMMM3_SAVE(%esp)
dyld_stub_binder_:
	movl		MH_LOCAL(%esp),%eax	# call dyld::fastBindLazySymbol(loadercache, lazyinfo)
	movl		%eax,MH_PARAM_OUT(%esp)
	movl		LP_LOCAL(%esp),%eax
	movl		%eax,LP_PARAM_OUT(%esp)
	call		__Z21_dyld_fast_stub_entryPvl
	movdqa		XMMM0_SAVE(%esp),%xmm0	# restore registers
	movdqa		XMMM1_SAVE(%esp),%xmm1
	movdqa		XMMM2_SAVE(%esp),%xmm2
	movdqa		XMMM3_SAVE(%esp),%xmm3
	movl		ECX_SAVE(%esp),%ecx
	movl		EDX_SAVE(%esp),%edx
	movl		%eax,%ebp		# move target address to epb
	movl		EAX_SAVE(%esp),%eax	# restore eax
	addl		$STACK_SIZE+4,%esp	# cut back stack
	xchg		%ebp, (%esp)		# restore ebp and set target to top of stack
	ret					# jump to target
    

#endif /* __i386__ */


#if __x86_64__

#define RET_ADDR_RBP			24
#define LP_PARAM_RBP			16
#define MH_PARAM_RBP			8
#define OLD_RBP_RBP				0

#define RDI_SAVE_RBP			-8
#define RSI_SAVE_RBP			-16
#define RDX_SAVE_RBP			-24
#define RCX_SAVE_RBP			-32
#define RBX_SAVE_RBP			-40
#define RAX_SAVE_RBP			-48
#define R8_SAVE_RBP 			-56
#define R9_SAVE_RBP 			-64
#define STATIC_STACK_SIZE		256	// extra padding to allow it to be 64-byte aligned

#define XMM0_SAVE_RSP			0x00
#define XMM1_SAVE_RSP			0x10
#define XMM2_SAVE_RSP			0x20
#define XMM3_SAVE_RSP			0x30
#define XMM4_SAVE_RSP			0x40
#define XMM5_SAVE_RSP			0x50
#define XMM6_SAVE_RSP			0x60
#define XMM7_SAVE_RSP			0x70


 /*    
 * sp+16 return address
 * sp+8  lazy binding info offset
 * sp+0	 address of ImageLoader cache
 */
    .align 2,0x90
    .globl dyld_stub_binder
dyld_stub_binder:
	pushq		%rbp
	test		$0xF,%rsp				# at this point stack should be 16-byte aligned
	jne			_stack_not_16_byte_aligned_error
	movq		%rsp,%rbp
	subq		$STATIC_STACK_SIZE,%rsp
	movq		%rdi,RDI_SAVE_RBP(%rbp)	# save registers that might be used as parameters
	movq		%rsi,RSI_SAVE_RBP(%rbp)
	movq		%rdx,RDX_SAVE_RBP(%rbp)
	movq		%rcx,RCX_SAVE_RBP(%rbp)
	movq		%rbx,RBX_SAVE_RBP(%rbp)
	movq		%rax,RAX_SAVE_RBP(%rbp)
	movq		%r8, R8_SAVE_RBP(%rbp)
	movq		%r9, R9_SAVE_RBP(%rbp)

	cmpl		$0, _inited(%rip)
	jne			Linited
	movl		$0x01,%eax
	cpuid		# get cpu features to check on xsave instruction support
	andl		$0x08000000,%ecx		# check OSXSAVE bit
	movl		%ecx,_hasXSave(%rip)
	cmpl		$0, %ecx
	jne			LxsaveInfo
	movl		$1, _inited(%rip)
	jmp			Lsse

LxsaveInfo:
	movl		$0x0D,%eax
	movl		$0x00,%ecx
	cpuid		# get xsave parameter info
	movl		%eax,_features_lo32(%rip)
	movl		%edx,_features_hi32(%rip)
	movl		%ecx,_bufferSize32(%rip)
	movl		$1, _inited(%rip)

Linited:
	cmpl		$0, _hasXSave(%rip)
	jne			Lxsave

Lsse:
	subq		$128, %rsp
	movdqa      %xmm0, XMM0_SAVE_RSP(%rsp)
	movdqa      %xmm1, XMM1_SAVE_RSP(%rsp)
	movdqa      %xmm2, XMM2_SAVE_RSP(%rsp)
	movdqa      %xmm3, XMM3_SAVE_RSP(%rsp)
	movdqa      %xmm4, XMM4_SAVE_RSP(%rsp)
	movdqa      %xmm5, XMM5_SAVE_RSP(%rsp)
	movdqa      %xmm6, XMM6_SAVE_RSP(%rsp)
	movdqa      %xmm7, XMM7_SAVE_RSP(%rsp)
	jmp			Lbind

Lxsave:
	movl		_bufferSize32(%rip),%eax
	movq		%rsp, %rdi
	subq		%rax, %rdi				# stack alloc buffer
	andq		$-64, %rdi				# 64-byte align stack
	movq		%rdi, %rsp
	# xsave requires buffer to be zero'ed out
	movq		$0, %rcx
	movq		%rdi, %r8
	movq		%rdi, %r9
	addq		%rax, %r9
Lz:	movq		%rcx, (%r8)
	addq		$8, %r8
	cmpq		%r8,%r9
	ja			Lz

	movl		_features_lo32(%rip),%eax
	movl		_features_hi32(%rip),%edx
	# call xsave with buffer on stack and eax:edx flag bits
	# note: do not use xsaveopt, it assumes you are using the same
	# buffer as previous xsaves, and this thread is on the same cpu.
	xsave		(%rsp)

Lbind:
	movq		MH_PARAM_RBP(%rbp),%rdi	# call fastBindLazySymbol(loadercache, lazyinfo)
	movq		LP_PARAM_RBP(%rbp),%rsi
	call		__Z21_dyld_fast_stub_entryPvl
	movq		%rax,%r11		# copy jump target

	cmpl		$0, _hasXSave(%rip)
	jne			Lxrstror

	movdqa      XMM0_SAVE_RSP(%rsp),%xmm0
	movdqa      XMM1_SAVE_RSP(%rsp),%xmm1
	movdqa      XMM2_SAVE_RSP(%rsp),%xmm2
	movdqa      XMM3_SAVE_RSP(%rsp),%xmm3
	movdqa      XMM4_SAVE_RSP(%rsp),%xmm4
	movdqa      XMM5_SAVE_RSP(%rsp),%xmm5
	movdqa      XMM6_SAVE_RSP(%rsp),%xmm6
	movdqa      XMM7_SAVE_RSP(%rsp),%xmm7
	jmp			Ldone

Lxrstror:
	movl		_features_lo32(%rip),%eax
	movl		_features_hi32(%rip),%edx
	# call xsave with buffer on stack and eax:edx flag bits
	xrstor		(%rsp)

Ldone:
	movq		RDI_SAVE_RBP(%rbp),%rdi
	movq		RSI_SAVE_RBP(%rbp),%rsi
	movq		RDX_SAVE_RBP(%rbp),%rdx
	movq		RCX_SAVE_RBP(%rbp),%rcx
	movq		RBX_SAVE_RBP(%rbp),%rbx
	movq		RAX_SAVE_RBP(%rbp),%rax
	movq		R8_SAVE_RBP(%rbp),%r8
	movq		R9_SAVE_RBP(%rbp),%r9
	movq		%rbp,%rsp
	popq		%rbp
	addq		$16,%rsp		# remove meta-parameters
	jmp			*%r11			# jmp to target

_stack_not_16_byte_aligned_error:
	movdqa      %xmm0, 0(%rsp)
	int3

	.data
# Cached info from cpuid.  These must be lazily evaluated.
# You cannot initalize these from _dyld_initializer() because
# that function is called from another dylib...
_inited:			.long 0
_features_lo32:		.long 0
_features_hi32:		.long 0
_bufferSize32:		.long 0
_hasXSave:			.long 0

#endif


#if __arm__
 /*    
 * sp+4	lazy binding info offset
 * sp+0	address of ImageLoader cache
 */
  
	.text
	.align 2
	.globl	dyld_stub_binder
dyld_stub_binder:
	stmfd	sp!, {r0,r1,r2,r3,r7,lr}	// save registers
	add	r7, sp, #16			// point FP to previous FP

	ldr	r0, [sp, #24]			// move address ImageLoader cache to 1st parameter
	ldr	r1, [sp, #28]			// move lazy info offset 2nd parameter

#if __ARM_ARCH_7K__
	vpush	{d0, d1, d2, d3, d4, d5, d6, d7}
	sub	sp, sp, #8			// Align stack to 16 bytes.
#endif
	// call dyld::fastBindLazySymbol(loadercache, lazyinfo)
	bl	__Z21_dyld_fast_stub_entryPvl
	mov	ip, r0				// move the symbol`s address into ip

#if __ARM_ARCH_7K__
	add	sp, sp, #8
	vpop	{d0, d1, d2, d3, d4, d5, d6, d7}
#endif

	ldmfd	sp!, {r0,r1,r2,r3,r7,lr}	// restore registers
	add	sp, sp, #8			// remove meta-parameters

	bx	ip				// jump to the symbol`s address that was bound

#endif /* __arm__ */


#if __arm64__
 /*    
  * sp+0	lazy binding info offset
  * sp+8	address of ImageLoader cache
  */
	.text
	.align 2
	.globl	dyld_stub_binder
dyld_stub_binder:
	stp		fp, lr, [sp, #-16]!
	mov		fp, sp
	sub		sp, sp, #240
	stp		x0,x1, [fp, #-16]	; x0-x7 are int parameter registers
	stp		x2,x3, [fp, #-32]
	stp		x4,x5, [fp, #-48]
	stp		x6,x7, [fp, #-64]
	stp		x8,x9, [fp, #-80]	; x8 is used for struct returns
	stp		q0,q1, [fp, #-128]	; q0-q7 are vector/fp parameter registers
	stp		q2,q3, [fp, #-160]
	stp		q4,q5, [fp, #-192]
	stp		q6,q7, [fp, #-224]

	ldr		x0, [fp, #24]	; move address ImageLoader cache to 1st parameter
	ldr		x1, [fp, #16]	; move lazy info offset 2nd parameter
	; call dyld::fastBindLazySymbol(loadercache, lazyinfo)
	bl		__Z21_dyld_fast_stub_entryPvl
	mov		x16,x0			; save target function address in lr
	
	; restore parameter registers
	ldp		x0,x1, [fp, #-16]
	ldp		x2,x3, [fp, #-32]
	ldp		x4,x5, [fp, #-48]
	ldp		x6,x7, [fp, #-64]
	ldp		x8,x9, [fp, #-80]
	ldp		q0,q1, [fp, #-128]
	ldp		q2,q3, [fp, #-160]
	ldp		q4,q5, [fp, #-192]
	ldp		q6,q7, [fp, #-224]
	
	mov		sp, fp
	ldp		fp, lr, [sp], #16
	add		sp, sp, #16	; remove meta-parameters
#if __arm64e__
    // Note arm64e executables will never hit this line as they don't have lazy binding.
    // However, arm64 binaries running on an arm64e system will hit this line but the
    // authentication won't have any effect there.
	braaz   x16
#else
	br      x16
#endif

#endif

