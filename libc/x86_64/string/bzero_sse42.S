/*
 * Copyright (c) 2008 Apple Computer, Inc. All rights reserved.
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

#include <machine/cpu_capabilities.h>
#include "platfunc.h"

/*
 * Bzero, tuned for processors with SSE4.2 and 64-byte cache lines, ie Nehalem.
 * We don't actually use SSE4.2, but rather use it to identify Nehalem.
 * This is the 64-bit version.
 *
 * We do not use nontemporal operations, but use MOVDQA in preference to REP/STOS. 
 *
 * This routine is also used for memset(p,0,n), which is a common case
 * since gcc sometimes silently maps bzero() into memset().  As a result,
 * we always load the original ptr into %eax before returning.
 */

#define kShort		80		// too short to bother with SSE (must be >=80)


// void	bzero(void *b, size_t len);

PLATFUNC_FUNCTION_START(bzero, sse42, 64, 5)
	pushq	%rbp			// set up a frame for backtraces
	movq	%rsp,%rbp
	xorl    %eax,%eax               // set fill data to 0
	movq	%rdi,%r11		// save original ptr as return value
	cmpq    $(kShort),%rsi          // long enough for SSE?
	jg	LNotShort               // yes

// Here for short operands or the end of long ones.
//      %esi = length (<= kShort)
//      %rdi = ptr
//      %eax = zero

Lshort:
	cmpl	$12,%esi		// long enough to word align?
	jge	3f			// yes
	test	%esi,%esi		// length==0?
	jz	6f
1:
	movb	%al,(%rdi)		// zero a byte
	incq	%rdi
	decl	%esi
	jnz	1b
	jmp	6f
2:
	movb	%al,(%rdi)		// zero a byte
	incq	%rdi
	decl	%esi
3:
	testl	$3,%edi			// is ptr doubleword aligned?
	jnz	2b			// no
	movl	%esi,%ecx		// copy length
	shrl	$2,%esi			// #doublewords to store
4:      
	movl	%eax,(%rdi)		// zero an aligned doubleword
	addq	$4,%rdi
	decl	%esi
	jnz	4b
	andl	$3,%ecx			// mask down to #bytes at end (0..3)
	jz	6f			// none
5:
	movb	%al,(%rdi)		// zero a byte
	incq	%rdi
	decl	%ecx
	jnz	5b
6:
	movq	%r11,%rax		// set return value in case this was a call of memset()
	popq	%rbp
	ret


// We will be using SSE, so align ptr.
//      %rsi = length (> kShort)
//      %rdi = ptr
//      %eax = zero

LNotShort:
	testl	$3,%edi			// 4-byte aligned?
	jz	2f			// yes
	movb	%al,(%rdi)		// zero another byte
	incq	%rdi
	decq	%rsi
	jmp	LNotShort
1:					// zero doublewords until 16-byte aligned
	movl	%eax,(%rdi)
	addq	$4,%rdi
	subq	$4,%rsi
2:
	testl	$15,%edi		// 16-byte aligned?
	jnz	1b			// no

// Destination is now 16-byte aligned.  Prepare to loop over 64-byte chunks.
//      %rsi = length (> (kShort-15))
//      %rdi = ptr (aligned)
//      %eax = zero

LDestAligned:
	movq    %rsi,%rcx
	andl    $63,%esi                // mask down to residual length (0..63)
	andq    $-64,%rcx               // get #bytes we will zero in this loop
	pxor    %xmm0,%xmm0             // zero an SSE register
	addq    %rcx,%rdi               // increment ptr by length to move
	negq    %rcx			// negate length to move
	jmp	1f

// Loop over 64-byte chunks, storing into cache.

	.align	4,0x90			// keep inner loops 16-byte aligned
1:
	movdqa  %xmm0,(%rdi,%rcx)
	movdqa  %xmm0,16(%rdi,%rcx)
	movdqa  %xmm0,32(%rdi,%rcx)
	movdqa  %xmm0,48(%rdi,%rcx)
	addq    $64,%rcx
	jne     1b

	jmp	Lshort


PLATFUNC_DESCRIPTOR(bzero,sse42,kHasSSE4_2,0)
