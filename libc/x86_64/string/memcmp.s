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


// ***************     ***********
// * M E M C M P * and * B C M P *
// ***************     ***********
//
// int	memcmp(const char *s1, const char *s2, size_t len);
// int	  bcmp(const char *s1, const char *s2, size_t len);
//
// Bcmp returns (+,0,-), whereas memcmp returns the true difference
// between the first differing bytes, but we treat them identically.
//
// We optimize the compare by doing it with SSE.  This introduces
// a complication: if we blindly did vector loads from both sides until
// finding a difference, we might get a spurious page fault by
// reading bytes past the difference.  To avoid this, we never do a load
// that crosses a page boundary.

#define	kShort	18			// too short for vectors (must be >16)

        .text
        .align 	4

        .globl _memcmp
        .globl _bcmp

_memcmp:				// int memcmp(const char *s1,const char *s2,size_t len);
_bcmp:					// int   bcmp(const char *s1,const char *s2,size_t len);
	cmpq	$(kShort),%rdx		// worth accelerating?
	ja	LNotShort		// yes
	

// Too short to bother with parallel compares.  Loop over bytes.
//	%rdi = LHS ptr
//	%rsi = RHS ptr
//	%edx = length (<= kShort)

LShort:
	testl	%edx,%edx	// 0-length?
	jnz	LShortLoop		// no
	xorq	%rax,%rax		// return 0
	jmp	LExit
	.align	4,0x90			// align inner loops to optimize I-fetch
LShortLoop:				// loop over bytes
	movzb	(%rdi),%eax		// get LHS byte
	movzb	(%rsi),%ecx		// get RHS byte
	addq	$1,%rdi
	addq	$1,%rsi
	subl	%ecx,%eax		// compare them
	jnz	LExit			// done if not equal
	subq	$1,%rdx			// decrement length
	jnz	LShortLoop
LExit:					// return value is in %eax
	ret
	
LNotEqual:				// here from LLoopOverBytes with LHS in eax
	movzb	(%rsi),%ecx		// get RHS byte
	subl	%ecx,%eax		// generate return value (nonzero)
	ret

	
// Loop over bytes until we reach end of a page.
//	%rdi = LHS ptr
//	%edi = RHS ptr
//	%rdx = length remaining after end of loop (i.e., already adjusted)
//	%ecx = #bytes until next page (1..15)

	.align	4,0x90			// align inner loops to optimize I-fetch
LLoopOverBytes:
	movzb	(%rdi),%eax		// get LHS byte
	addq	$1,%rdi
	cmpb	(%rsi),%al		// compare to RHS byte
	jnz	LNotEqual			// done if not equal
	addq	$1,%rsi
	subl	$1,%ecx			// more to go?
	jnz	LLoopOverBytes
	

// Long enough to justify overhead of setting up vector compares.  In order to
// avoid spurious page faults, we loop over:
//
//	min( length, bytes_in_LHS_page, bytes_in_RHS_page) >> 4
//
// 16-byte chunks.  When we near a page end, we have to revert to a byte-by-byte
// comparison until reaching the next page, then resume the vector comparison.
//	%rdi = LHS ptr
//	%rsi = RHS ptr
//	%rdx = length (> kShort)

LNotShort:
	movq	%rdi,%rax		// copy ptrs
	movq	%rsi,%rcx
	andq	$4095,%rax		// mask down to page offsets
	andq	$4095,%rcx
	cmpq	%rax,%rcx		// which is bigger?
	cmova	%rcx,%rax		// %eax = max(LHS offset, RHS offset);
	movl	$4096,%ecx
	subl	%eax,%ecx		// get #bytes to next page crossing
	cmpq	%rdx,%rcx		// will operand run out first?
	cmova	%edx,%ecx		// get min(length remaining, bytes to page end)
	movl	%ecx,%eax
	shrl	$4,%ecx			// get #chunks till end of operand or page
	jnz	LLoopOverChunks		// enter vector loop
	
// Too near page end for vectors.

	subq	%rax,%rdx		// adjust length remaining
	movl	%eax,%ecx		// %ecx <- #bytes to page end
	cmpq	$(kShort),%rdx		// will there be enough after we cross page for vectors?
	ja	LLoopOverBytes		// yes
	addq	%rax,%rdx		// no, restore total length remaining
	jmp	LShortLoop		// compare rest byte-by-byte (%ecx != 0)


// Loop over 16-byte chunks.
//	%rdi = LHS ptr
//	%rsi = RHS ptr
//	%rdx = length remaining
//	%ecx = chunk count

	.align	4,0x90			// align inner loops to optimize I-fetch
LLoopOverChunks:
	movdqu	(%rdi),%xmm0		// get LHS
	movdqu	(%rsi),%xmm1		// get RHS
	addq	$16,%rdi
	pcmpeqb	%xmm1,%xmm0		// compare LHS to RHS
	addq	$16,%rsi
	pmovmskb %xmm0,%eax		// collect comparison result bits (1 if equal)
	subq	$16,%rdx		// adjust length remaining
	xorl	$0xFFFF,%eax		// all equal?
	jne	LDifferent		// no, we found differing bytes
	subl	$1,%ecx			// more to go?
	jnz	LLoopOverChunks
	
	cmpq	$(kShort),%rdx		// a lot more to compare?
	jbe	LShort			// no
	jmp	LNotShort		// compute distance to next page crossing etc


// Found a difference.  
//	%rdi = LHS ptr, already advanced by 16
//	%rsi = RHS ptr, already advanced by 16
//	%eax = complemented compare vector (ie, 0 == equal)

LDifferent:
	bsf	%eax,%edx		// which byte differed?
	subq	$16,%rdi		// point to byte 0 while we wait for bit scan
	subq	$16,%rsi
	movzb	(%rdi,%rdx),%eax	// get LHS byte
	movzb	(%rsi,%rdx),%ecx	// get RHS byte
	subl	%ecx,%eax		// compute difference (ie, return value)
	ret
