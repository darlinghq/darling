/*
 * Copyright (c) 2010-2013 Apple Inc. All rights reserved.
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
 
#include <System/machine/cpu_capabilities.h>

// bool save_xxm = (*((uint32_t*)_COMM_PAGE_CPU_CAPABILITIES) & kHasAVX1_0) != 0;

#if __x86_64__

#define RDI_SAVE_RBP			-8
#define RSI_SAVE_RBP			-16
#define RDX_SAVE_RBP			-24
#define RCX_SAVE_RBP			-32
#define RBX_SAVE_RBP			-40
#define R8_SAVE_RBP 			-48
#define R9_SAVE_RBP 			-56
#define R10_SAVE_RBP			-64
#define R11_SAVE_RBP			-72
#define STATIC_STACK_SIZE		256	// extra padding to allow it to be 64-byte aligned

#define XMM0_SAVE_RSP			0x00
#define XMM1_SAVE_RSP			0x10
#define XMM2_SAVE_RSP			0x20
#define XMM3_SAVE_RSP			0x30
#define XMM4_SAVE_RSP			0x40
#define XMM5_SAVE_RSP			0x50
#define XMM6_SAVE_RSP			0x60
#define XMM7_SAVE_RSP			0x70


	// returns address of TLV in %rax, all other registers preserved
	.globl _tlv_get_addr
	.private_extern _tlv_get_addr
_tlv_get_addr:
	movq	8(%rdi),%rax			// get key from descriptor
	movq	%gs:0x0(,%rax,8),%rax	// get thread value
	testq	%rax,%rax				// if NULL, lazily allocate
	je		LlazyAllocate
	addq	16(%rdi),%rax			// add offset from descriptor
	ret
LlazyAllocate:
	pushq		%rbp
	movq		%rsp,%rbp
	subq		$STATIC_STACK_SIZE,%rsp
	movq		%rdi,RDI_SAVE_RBP(%rbp)	# save registers that might be used as parameters
	movq		%rsi,RSI_SAVE_RBP(%rbp)
	movq		%rdx,RDX_SAVE_RBP(%rbp)
	movq		%rcx,RCX_SAVE_RBP(%rbp)
	movq		%rbx,RBX_SAVE_RBP(%rbp)
	movq		%r8,  R8_SAVE_RBP(%rbp)
	movq		%r9,  R9_SAVE_RBP(%rbp)
	movq		%r10,R10_SAVE_RBP(%rbp)
	movq		%r11,R11_SAVE_RBP(%rbp)

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
	jmp			Lalloc

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

Lalloc:
	movq		RDI_SAVE_RBP(%rbp),%rdi
	movq		8(%rdi),%rdi		// get key from descriptor
	call		_tlv_allocate_and_initialize_for_key

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
	movq		%rax,%r11
	movl		_features_lo32(%rip),%eax
	movl		_features_hi32(%rip),%edx
	# call xsave with buffer on stack and eax:edx flag bits
	xrstor		(%rsp)
	movq		%r11,%rax

Ldone:
	movq		RDI_SAVE_RBP(%rbp),%rdi
	movq		RSI_SAVE_RBP(%rbp),%rsi
	movq		RDX_SAVE_RBP(%rbp),%rdx
	movq		RCX_SAVE_RBP(%rbp),%rcx
	movq		RBX_SAVE_RBP(%rbp),%rbx
	movq		R8_SAVE_RBP(%rbp),%r8
	movq		R9_SAVE_RBP(%rbp),%r9
	movq		R10_SAVE_RBP(%rbp),%r10
	movq		R11_SAVE_RBP(%rbp),%r11
	movq		%rbp,%rsp
	popq		%rbp
 	addq		16(%rdi),%rax			// result = buffer + offset
	ret

	.data
# Cached info from cpuid.
_inited:			.long 0
_features_lo32:		.long 0
_features_hi32:		.long 0
_bufferSize32:		.long 0
_hasXSave:			.long 0

#endif



#if __i386__
	// returns address of TLV in %eax, all other registers (except %ecx) preserved
	.globl _tlv_get_addr
	.private_extern _tlv_get_addr
_tlv_get_addr:
	movl	4(%eax),%ecx			// get key from descriptor
	movl	%gs:0x0(,%ecx,4),%ecx	// get thread value
	testl	%ecx,%ecx				// if NULL, lazily allocate
	je		LlazyAllocate
	movl	8(%eax),%eax			// add offset from descriptor
	addl	%ecx,%eax
	ret
LlazyAllocate:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%edx					// save edx
	subl	$548,%esp
	movl	%eax,-8(%ebp)		    // save descriptor
	lea		-528(%ebp),%ecx		    // get 512 byte buffer in frame
	and		$-16, %ecx			    // 16-byte align buffer for fxsave
	fxsave  (%ecx)
	movl	4(%eax),%ecx			// get key from descriptor
	movl	%ecx,(%esp)	            // push key parameter, also leaves stack aligned properly
	call	_tlv_allocate_and_initialize_for_key
	movl	-8(%ebp),%ecx			// get descriptor
	movl	8(%ecx),%ecx			// get offset from descriptor
	addl	%ecx,%eax				// add offset to buffer
	lea 	-528(%ebp),%ecx
	and 	$-16, %ecx              // 16-byte align buffer for fxrstor
	fxrstor (%ecx)
	addl	$548,%esp
	popl	%edx	                // restore edx
	popl	%ebp
	ret
#endif

#if __arm64__
	// Parameters: X0 = descriptor
	// Result:  X0 = address of TLV
	// Note: all registers except X0, x16, and x17 are preserved
	.align 2
	.globl _tlv_get_addr
	.private_extern _tlv_get_addr
_tlv_get_addr:
#if __LP64__
	ldr		x16, [x0, #8]			// get key from descriptor
#else
	ldr		w16, [x0, #4]			// get key from descriptor
#endif
	mrs		x17, TPIDRRO_EL0
	and		x17, x17, #-8			// clear low 3 bits???
#if __LP64__
	ldr		x17, [x17, x16, lsl #3]	// get thread allocation address for this key
#else
	ldr		w17, [x17, x16, lsl #2]	// get thread allocation address for this key
#endif
	cbz		x17, LlazyAllocate		// if NULL, lazily allocate
#if __LP64__
	ldr		x16, [x0, #16]			// get offset from descriptor
#else
	ldr		w16, [x0, #8]			// get offset from descriptor
#endif
	add		x0, x17, x16			// return allocation+offset
	ret		lr

LlazyAllocate:
	stp		fp, lr, [sp, #-16]!
	mov		fp, sp
	sub		sp, sp, #288
	stp		x1, x2, [sp, #-16]!		// save all registers that C function might trash
	stp		x3, x4, [sp, #-16]!
	stp		x5, x6, [sp, #-16]!
	stp		x7, x8, [sp, #-16]!
	stp		x9, x10,  [sp, #-16]!
	stp		x11, x12, [sp, #-16]!
	stp		x13, x14, [sp, #-16]!
	stp		x15, x16, [sp, #-16]!
	stp		q0,  q1,  [sp, #-32]!
	stp		q2,  q3,  [sp, #-32]!
	stp		q4,  q5,  [sp, #-32]!
	stp		q6,  q7,  [sp, #-32]!
	stp		x0, x17,  [sp, #-16]!	// save descriptor

	mov		x0, x16					// use key from descriptor as parameter
	bl		_tlv_allocate_and_initialize_for_key
	ldp		x16, x17, [sp], #16		// pop descriptor
#if __LP64__
	ldr		x16, [x16, #16]			// get offset from descriptor
#else
	ldr		w16, [x16, #8]			// get offset from descriptor
#endif
	add		x0, x0, x16				// return allocation+offset

	ldp		q6,  q7,  [sp], #32
	ldp		q4,  q5,  [sp], #32
	ldp		q2,  q3,  [sp], #32
	ldp		q0,  q1,  [sp], #32
	ldp		x15, x16, [sp], #16
	ldp		x13, x14, [sp], #16
	ldp		x11, x12, [sp], #16
	ldp		x9, x10,  [sp], #16
	ldp		x7, x8, [sp], #16
	ldp		x5, x6, [sp], #16
	ldp		x3, x4, [sp], #16
	ldp		x1, x2, [sp], #16

	mov		sp, fp
	ldp		fp, lr, [sp], #16
	ret		lr

#endif

#if __arm__
	// returns address of TLV in r0, all other registers preserved
	.align 2
	.globl _tlv_get_addr
	.private_extern _tlv_get_addr
_tlv_get_addr:
	push	{r1,r2,r3,r7,lr}
#if __ARM_ARCH_7K__
	sub		sp, sp, #12						// align stack to 16 bytes
#endif
	mov		r7, r0							// save descriptor in r7
	ldr		r0, [r7, #4]					// get key from descriptor
	bl		_pthread_getspecific			// get thread value
	cmp		r0, #0
	bne		L2								// if NULL, lazily allocate
#if __ARM_ARCH_7K__
	vpush	{d0, d1, d2, d3, d4, d5, d6, d7}
#endif
	ldr		r0, [r7, #4]					// get key from descriptor
	bl		_tlv_allocate_and_initialize_for_key
#if __ARM_ARCH_7K__
	vpop	{d0, d1, d2, d3, d4, d5, d6, d7}
#endif
L2:	ldr		r1, [r7, #8]					// get offset from descriptor
	add		r0, r1, r0						// add offset into allocation block
#if __ARM_ARCH_7K__
	add		sp, sp, #12
#endif
	pop		{r1,r2,r3,r7,pc}
#endif

	.subsections_via_symbols
	

