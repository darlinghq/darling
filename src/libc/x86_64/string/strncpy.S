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

#include <machine/cpu_capabilities.h>


// *****************
// * S T R N C P Y *
// *****************
//
// char  *strncpy(const char *dst, const char *src, size_t n);
//
// We optimize the move by doing it vector parallel.  This introduces
// a complication: if we blindly did vector load/stores until finding
// a 0, we might get a spurious page fault by touching bytes past it.
// To avoid this, we never do a load that crosses a page boundary,
// and never store a byte we don't have to.
//
// We align the destination, because unaligned vector stores are slow.
//
// Recall that strncpy() zero fills the remainder of the dest buffer,
// and does not terminate the string if its length is greater than or
// equal to n.

#define	kShort	31			// too short to bother with vector loop

        .text
        .globl _strncpy

        .align 	4
_strncpy:				// char  *strncpy(const char *dst, const char *src, size_t n);
	movq	%rdi,%r8	// preserve destination pointer so we can return it
	movl	%edi,%ecx		// copy low 4 bytes of dest ptr
	negl	%ecx
	andl	$15,%ecx		// how many bytes to align dest ptr?
	jnz	LCheckShortCopy		// align destination first
	
	
// In order to avoid spurious page faults, we loop until nearing the source page
// end.  Then we revert to a byte-by-byte loop for 16 bytes until the page is crossed,
// then resume the vector loop. 
//	%rsi = source ptr (unaligned)
//	%rdi = dest ptr (aligned)
//	%rdx = buffer length remaining

LNextChunk:				// NB: can drop down to here
	movl	%esi,%eax		// copy the low 4 bytes of the source ptr
	movl	$4096,%ecx
	andl	$4095,%eax		// get offset into source page
	subl	%eax,%ecx		// get #bytes remaining in source page
	cmpq	%rdx,%rcx		// will buffer run out before the page end?
	cmova	%rdx,%rcx		// get min(length remaining, bytes to page end)
	shrl	$4,%ecx			// get #chunks till end of page
	jnz	LLoopOverChunks		// enter vector loop
	
// We can't use the chunk loop yet.  Check for short and empty buffers, then use byte loop.

LCrossPage:				// if buffer is large enough, cross source page
	movl	$16,%ecx		// move 16 bytes to cross page but keep dest aligned
LCheckShortCopy:			// we propose to copy %ecx bytes in byte loop
	cmpq	$(kShort),%rdx		// much left?
	ja	LLoopOverBytes		// yes, loop over bytes then more chunks
	movl	%edx,%ecx		// no, use the byte loop for everything
	testl	%edx,%edx		// have we filled buffer?
	jnz	LLoopOverBytes		// no
	jmp	LDone


// Loop over bytes.
//	%rsi = source ptr
//	%rdi = dest ptr
//	%rdx = buffer length remaining
//	%rcx = count of bytes to loop over (<= buffer length)

	.align	4,0x90			// align inner loops to optimize I-fetch
LLoopOverBytes:
	movzb	(%rsi),%eax		// get source byte
	addq	$1,%rsi
	subq	$1,%rdx			// decrement length
	movb	%al,(%rdi)		// pack into dest
	addq	$1,%rdi
	testl	%eax,%eax		// 0?
	jz	LZeroBuffer		// yes, we're done copying string
	subq	$1,%rcx			// more to go?
	jnz	LLoopOverBytes
	
	testq	%rdx,%rdx		// at end of buffer?
	jnz	LNextChunk		// no, xfer chunks
	jmp	LDone			// yes


// Loop over 16-byte chunks.
//	%rsi = source ptr (unaligned)
//	%rdi = dest ptr (aligned)
//	%rdx = buffer length remaining
//	%ecx = chunk count

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
	subq	$16,%rdx		// decrement length remaining
	subl	$1,%ecx			// more to go?
	jnz	LLoopOverChunks
	
	jmp	LCrossPage		// cross page but keep dest aligned
	

// Found a zero in the vector.  Figure out where it is, and store the bytes
// up to it.  It is possible that we should check to be sure (%rdx >= 16), and
// just do an aligned store of %xmm1 if so.  But if we did, we'd be doing byte
// stores into the same double quadword in bzero(), which might hit a hazard.
// Experimentation needed.
//	%rdi = dest ptr (aligned)
//	%eax = result mask
//	%rdx = buffer length remaining
//	%xmm1 = source vector

LFound0:
	bsf	%eax,%ecx		// find first 0
	subq	%rcx,%rdx		// decrement remaining buffer length
	test	$8,%cl			// 8-byte store required?
	jz	4f			// no
	movq	%xmm1,(%rdi)		// pack in 8 low bytes
	psrldq	$8,%xmm1		// then shift vector down 8 bytes
	addq	$8,%rdi
4:
	test	$4,%cl			// 4-byte store required?
	jz	3f			// no
	movd	%xmm1,(%rdi)		// pack in 4 low bytes
	psrldq	$4,%xmm1		// then shift vector down 4 bytes
	addq	$4,%rdi
3:
	andl	$3,%ecx			// more to go?
	jz	LZeroBuffer		// no
	movd	%xmm1,%eax		// move remainders out of vector into %eax
1:					// loop on up to three bytes
	movb	%al,(%rdi)		// pack in next byte
	shrl	$8,%eax			// shift next byte into position
	addq	$1,%rdi
	subl	$1,%ecx
	jnz	1b

// We've copied the string.  Now zero the rest of the buffer, using commpage bzero().
//	%rdi = dest ptr
//	%rcx = buffer length remaining

LZeroBuffer:
	movq	%rdx,%rsi		// remaining buffer size (2nd argument)
	call	_bzero

LDone:
	movq	%r8,%rax		// original dest ptr is return value
	ret
