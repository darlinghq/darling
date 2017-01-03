/*
 * Copyright (c) 2005-2006 Apple Computer, Inc. All rights reserved.
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


// *****************
// * S T R N C M P *
// *****************
//
// int	strncmp(const char *s1, const char *s2, size_t len);
//
// We optimize the compare by doing it vector parallel.  This introduces
// a complication: if we blindly did vector loads from both sides until
// finding a difference (or 0), we might get a spurious page fault by
// reading bytes past the difference.  To avoid this, we never do a load
// that crosses a page boundary.

#define	kShort	20			// too short for vectors (must be >16)

        .text
        .globl _strncmp

        .align 	4
_strncmp:				// int strncmp(const char *s1, const char *s2, size_t len);
	cmpq	$(kShort),%rdx		// worth accelerating?
	ja	LNotShort		// yes
	

// Too short to bother with parallel compares.  Loop over bytes.
//	%rdi = LHS ptr
//	%rsi = RHS ptr
//	%edx = length (<= kShort)

LShort:
	testl	%edx,%edx		// 0-length?
	jnz	LShortLoop		// no
	jmp	LReturn0		// yes, return 0
	.align	4,0x90			// align inner loops to optimize I-fetch
LShortLoop:				// loop over bytes
	movzb	(%rdi),%eax		// get LHS byte
	movzb	(%rsi),%ecx		// get RHS byte
	incq	%rdi
	incq	%rsi
	testl	%eax,%eax		// LHS==0 ?
	jz	LNotEqual		// yes, this terminates comparison
	cmpl	%ecx,%eax		// compare them (sub won't fuse, but cmp will)
	jnz	LNotEqual		// done if not equal
	decl	%edx			// decrement length
	jnz	LShortLoop
LReturn0:
	xorl	%eax,%eax		// all bytes equal, so return 0
	ret
	
LNotEqual:				// LHS in eax, RHS in ecx
	subl	%ecx,%eax		// generate return value (nonzero)
	ret

	
// Loop over bytes until we reach end of a page.
//	%rdi = LHS ptr
//	%rsi = RHS ptr
//	%rdx = length remaining after end of loop (ie, already adjusted)
//	%r8d = #bytes until next page (1..15)

	.align	4,0x90			// align inner loops to optimize I-fetch
LLoopOverBytes:
	movzb	(%rdi),%eax		// get LHS byte
	movzb	(%rsi),%ecx		// get RHS byte
	incq	%rdi
	incq	%rsi
	testl	%eax,%eax		// LHS==0 ?
	jz	LNotEqual		// yes, this terminates comparison
	cmpl	%ecx,%eax		// compare them (sub won't fuse, but cmp will)
	jnz	LNotEqual		// done if not equal
	decl	%r8d			// more to go?
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
	movl	%edi,%eax		// copy ptrs
	movl	%esi,%r8d
	andl	$4095,%eax		// mask down to page offsets
	andl	$4095,%r8d
	cmpl	%eax,%r8d		// which is bigger?
	cmova	%r8d,%eax		// %eax = max(LHS offset, RHS offset);
	movl	$4096,%r8d
	subl	%eax,%r8d		// get #bytes to next page crossing
	cmpq	%rdx,%r8		// will operand run out first?
	cmova	%rdx,%r8		// get min(length remaining, bytes to page end)
	movl	%r8d,%eax		// copy #bytes
	shrl	$4,%r8d			// get #chunks till end of operand or page
	testl	%r8d,%r8d		// test %r8d for 0 without partial flag update stall
	jnz	LLoopOverChunks		// enter vector loop
	
// Too near page end for vectors.

	subq	%rax,%rdx		// adjust length remaining
	movl	%eax,%r8d		// %r8d <- #bytes to page end
	cmpq	$(kShort),%rdx		// will there be enough after we cross page for vectors?
	ja	LLoopOverBytes		// yes
	addq	%rax,%rdx		// no, restore total length remaining
	jmp	LShortLoop		// compare rest byte-by-byte (%rdx != 0)


// Loop over 16-byte chunks.
//	%rdi = LHS ptr
//	%rsi = RHS ptr
//	%rdx = length remaining
//	%r8d = chunk count

	.align	4,0x90			// align inner loops to optimize I-fetch
LLoopOverChunks:
	movdqu	(%rdi),%xmm1		// get LHS
	movdqu	(%rsi),%xmm2		// get RHS
	pxor	%xmm0,%xmm0		// get some 0s in the shadow of the loads
	addq	$16,%rdi
	pcmpeqb	%xmm1,%xmm2		// compare LHS to RHS
	pcmpeqb	%xmm1,%xmm0		// compare LHS to 0s
	addq	$16,%rsi
	pmovmskb %xmm2,%eax		// get result mask for comparison of LHS and RHS
	pmovmskb %xmm0,%ecx		// get result mask for 0 check
	subq	$16,%rdx		// decrement length remaining
	xorl	$0xFFFF,%eax		// complement compare mask so 1 means "not equal"
	orl	%ecx,%eax		// combine the masks and check for 1-bits
	jnz	LFoundDiffOr0		// we found differing bytes or a 0-byte
	decl	%r8d			// more to go?
	jnz	LLoopOverChunks		// yes
	
	cmpq	$(kShort),%rdx		// a lot more to compare?
	jbe	LShort			// no
	jmp	LNotShort		// compute distance to next page crossing etc


// Found a zero and/or a difference in vector compare.
//	%rdi = LHS ptr, already advanced by 16
//	%rsi = RHS ptr, already advanced by 16
//	%eax = bit n set if bytes n differed or were 0

LFoundDiffOr0:
	bsf	%eax,%edx		// which byte differed or was 0?
	movzb	-16(%rdi,%rdx),%eax	// get LHS byte
	movzb	-16(%rsi,%rdx),%edx	// get RHS byte
	subl	%edx,%eax		// compute difference (ie, return value)
	ret
