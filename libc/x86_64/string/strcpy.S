/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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


// ***************
// * S T R C P Y *
// ***************
//
// char  *strcpy(const char *dst, const char *src);
//
// We optimize the move by doing it vector parallel.  This introduces
// a complication: if we blindly did vector load/stores until finding
// a 0, we might get a spurious page fault by touching bytes past it.
// To avoid this, we never do a load that crosses a page boundary,
// and never store a byte we don't have to.
//
// We align the destination, because unaligned vector stores are slow.

        .text
        .globl _strcpy

        .align 	4
_strcpy:				// char *strcpy(const char *dst, const char *src);
	movq	%rdi,%rcx	// preserve dest ptr so we can return it
	movl	%edi,%edx		// copy low 4 bytes of dest ptr
	negl	%edx
	andl	$15,%edx		// how many bytes to align dest ptr?
	jnz	LLoopOverBytes		// not aligned, so go do so
	
	
// In order to avoid spurious page faults, we loop until nearing the source page
// end.  Then we revert to a byte-by-byte loop for 16 bytes until the page is crossed,
// then resume the vector loop. 
//	%rsi = source ptr (unaligned)
//	%rdi = dest ptr (aligned)

LNextChunk:
	movl	%esi,%eax		// copy low 4 bytes of source ptr
	movl	$4096,%edx
	andl	$4095,%eax		// get offset into source page
	subl	%eax,%edx		// get #bytes remaining in source page
	shrl	$4,%edx			// get #chunks till end of page
	jnz	LLoopOverChunks		// enter vector loop
	movl	$16,%edx		// move 16 bytes to cross page but keep dest aligned
	jmp	LLoopOverBytes


// Loop over bytes.
//	%rsi = source ptr
//	%rdi = dest ptr
//	%edx = byte count

	.align	4,0x90			// align inner loops to optimize I-fetch
LLoopOverBytes:
	movzb	(%rsi),%eax		// get source byte
	addq	$1,%rsi
	movb	%al,(%rdi)		// pack into dest
	addq	$1,%rdi
	testl	%eax,%eax		// 0?
	jz	LDone			// yes, we're done
	subl	$1,%edx			// more to go?
	jnz	LLoopOverBytes
	
	jmp	LNextChunk		// we've come to end of page


// Loop over 16-byte chunks.
//	%rsi = source ptr (unaligned)
//	%rdi = dest ptr (aligned)
//	%edx = chunk count

	.align	4,0x90			// align inner loops to optimize I-fetch
LLoopOverChunks:
	movdqu	(%rsi),%xmm1		// get source
	pxor	%xmm0,%xmm0		// get some 0s
	addq	$16,%rsi
	pcmpeqb	%xmm1,%xmm0		// compare source to 0s
	pmovmskb %xmm0,%eax		// get result mask for 0 check
	testl	%eax,%eax		// any 0s?
	jnz	LFound0			// yes, exit loop
	movdqa	%xmm1,(%rdi)		// no 0s so do aligned store into destination
	addq	$16,%rdi
	subl	$1,%edx			// more to go?
	jnz	LLoopOverChunks
	
	movl	$16,%edx		// move 16 bytes
	jmp	LLoopOverBytes		// cross page but keep dest aligned
	

// Found a zero in the vector.  Figure out where it is, and store the bytes
// up to it.
//	%rdi = dest ptr (aligned)
//	%eax = result mask
//	%xmm1 = source vector

LFound0:
	bsf	%eax,%edx		// find first 0
	addl	$1,%edx			// we need to store the 0 too
	test	$16,%dl			// was 0 last byte?
	jz	8f			// no
	movdqa	%xmm1,(%rdi)		// yes, store entire vector
	jmp	LDone
8:	
	test	$8,%dl			// 8-byte store required?
	jz	4f			// no
	movq	%xmm1,(%rdi)		// pack in 8 low bytes
	psrldq	$8,%xmm1		// then shift vector down 8 bytes
	addq	$8,%rdi
4:
	test	$4,%dl			// 4-byte store required?
	jz	3f			// no
	movd	%xmm1,(%rdi)		// pack in 4 low bytes
	psrldq	$4,%xmm1		// then shift vector down 4 bytes
	addq	$4,%rdi
3:
	andl	$3,%edx			// more to go?
	jz	LDone			// no
	movd	%xmm1,%eax		// move remainders out of vector into %eax
1:					// loop on up to three bytes
	movb	%al,(%rdi)		// pack in next byte
	shrl	$8,%eax			// shift next byte into position
	addq	$1,%rdi
	dec	%edx
	jnz	1b
	
LDone:
	movq	%rcx,%rax		// original dest ptr is return value
	ret
