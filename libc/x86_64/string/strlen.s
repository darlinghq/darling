/*
 * Copyright (c) 2005-2007 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * Strlen, for processors with SSE3.
 *
 * Note that all memory references must be aligned, in order to avoid spurious
 * page faults.  Thus we have to load the aligned 16-byte chunk containing the
 * first byte of the operand, then mask out false 0s that may occur before the
 * first byte.
 *
 * We favor the fall-through (ie, short operand) path.
 */

        .text
        .globl  _strlen
        .align  4, 0x90
_strlen:				// size_t strlen(char *b);
	pxor	%xmm0,%xmm0		// zero %xmm0
	movl	%edi,%ecx		// copy low half of ptr
	movq	%rdi,%rdx		// make another full copy
	andq	$(-16),%rdi		// 16-byte align ptr
	orl	$(-1),%eax
	pcmpeqb	(%rdi),%xmm0		// check whole qw for 0s
	andl	$15,%ecx		// get #bytes in aligned dq before operand
	shl	%cl,%eax		// create mask for the bytes of aligned dq in operand
	pmovmskb %xmm0,%ecx		// collect mask of 0-bytes
	andl	%eax,%ecx		// mask out any 0s that occur before 1st byte
	jz	LEnterLoop		// no 0-bytes (ie, 1-bits), so enter by-16 loop
	
// We've found a 0-byte.
//	%rdi = aligned address of 16-byte block containing the terminating 0-byte
//	%ecx = compare bit vector

LFoundIt:
	bsf	%ecx,%eax		// find first 1-bit (ie, first 0-byte)
	subq	%rdx,%rdi		// get length to start of 16-byte block while we wait
	addq	%rdi,%rax		// add bytes in 16-byte block
	ret
	
// Loop over aligned 16-byte blocks:
//	%rdi = address of previous block

LEnterLoop:
	pxor	%xmm0,%xmm0		// get some 0-bytes
	addq	$16,%rdi		// advance ptr
LLoop:
	movdqa	(%rdi),%xmm1		// get next chunk
	addq	$16,%rdi
	pcmpeqb	%xmm0,%xmm1		// check for 0s
	pmovmskb %xmm1,%ecx		// collect mask of 0-bytes
	test	%ecx,%ecx		// any 0-bytes?
	jz	LLoop			// no 0-bytes, so get next dq

	subq	$16,%rdi		// back up ptr
	jmp	LFoundIt
