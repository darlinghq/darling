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
	pushl	%esi
	pushl	%edi
	movl	20(%esp),%ecx		// get length
	movl	12(%esp),%esi		// get LHS ptr
	movl	16(%esp),%edi		// get RHS ptr
	push	%ebx
	cmpl	$(kShort),%ecx		// worth accelerating?
	ja	LNotShort		// yes
	

// Too short to bother with parallel compares.  Loop over bytes.
//	%esi = LHS ptr
//	%edi = RHS ptr
//	%ecx = length (<= kShort)

LShort:
	testl	%ecx,%ecx		// 0-length?
	jnz	LShortLoop		// no
	jmp	LReturn0		// yes, return 0
	.align	4,0x90			// align inner loops to optimize I-fetch
LShortLoop:				// loop over bytes
	movzb	(%esi),%eax		// get LHS byte
	movzb	(%edi),%ebx		// get RHS byte
	incl	%esi
	incl	%edi
	testl	%eax,%eax		// LHS==0 ?
	jz	LNotEqual		// yes, this terminates comparison
	subl	%ebx,%eax		// compare them
	jnz	LExit			// done if not equal
	decl	%ecx			// decrement length
	jnz	LShortLoop
LReturn0:
	xorl	%eax,%eax		// all bytes equal, so return 0
LExit:					// return value is in %eax
	popl	%ebx
	popl	%edi
	popl	%esi
	ret
	
LNotEqual:				// LHS in eax, RHS in ebx
	subl	%ebx,%eax		// generate return value (nonzero)
	popl	%ebx
	popl	%edi
	popl	%esi
	ret

	
// Loop over bytes until we reach end of a page.
//	%esi = LHS ptr
//	%edi = RHS ptr
//	%ecx = length remaining after end of loop (ie, already adjusted)
//	%edx = #bytes until next page (1..15)

	.align	4,0x90			// align inner loops to optimize I-fetch
LLoopOverBytes:
	movzb	(%esi),%eax		// get LHS byte
	movzb	(%edi),%ebx		// get RHS byte
	inc	%esi
	inc	%edi
	testl	%eax,%eax		// LHS==0 ?
	jz	LNotEqual		// yes, this terminates comparison
	subl	%ebx,%eax		// compare them
	jnz	LExit			// done if not equal
	dec	%edx			// more to go?
	jnz	LLoopOverBytes
	

// Long enough to justify overhead of setting up vector compares.  In order to
// avoid spurious page faults, we loop over:
//
//	min( length, bytes_in_LHS_page, bytes_in_RHS_page) >> 4
//
// 16-byte chunks.  When we near a page end, we have to revert to a byte-by-byte
// comparison until reaching the next page, then resume the vector comparison.
//	%esi = LHS ptr
//	%edi = RHS ptr
//	%ecx = length (> kShort)

LNotShort:
	movl	%esi,%eax		// copy ptrs
	movl	%edi,%edx
	andl	$4095,%eax		// mask down to page offsets
	andl	$4095,%edx
	cmpl	%eax,%edx		// which is bigger?
	cmova	%edx,%eax		// %eax = max(LHS offset, RHS offset);
	movl	$4096,%edx
	subl	%eax,%edx		// get #bytes to next page crossing
	cmpl	%ecx,%edx		// will operand run out first?
	cmova	%ecx,%edx		// get min(length remaining, bytes to page end)
	movl	%edx,%eax
	shrl	$4,%edx			// get #chunks till end of operand or page
	jnz	LLoopOverChunks		// enter vector loop
	
// Too near page end for vectors.

	subl	%eax,%ecx		// adjust length remaining
	movl	%eax,%edx		// %edx <- #bytes to page end
	cmpl	$(kShort),%ecx		// will there be enough after we cross page for vectors?
	ja	LLoopOverBytes		// yes
	addl	%eax,%ecx		// no, restore total length remaining
	jmp	LShortLoop		// compare rest byte-by-byte (%ecx != 0)


// Loop over 16-byte chunks.
//	%esi = LHS ptr
//	%edi = RHS ptr
//	%ecx = length remaining
//	%edx = chunk count

	.align	4,0x90			// align inner loops to optimize I-fetch
LLoopOverChunks:
	movdqu	(%esi),%xmm1		// get LHS
	movdqu	(%edi),%xmm2		// get RHS
	pxor	%xmm0,%xmm0		// get some 0s in the shadow of the loads
	addl	$16,%esi
	pcmpeqb	%xmm1,%xmm2		// compare LHS to RHS
	pcmpeqb	%xmm1,%xmm0		// compare LHS to 0s
	addl	$16,%edi
	pmovmskb %xmm2,%eax		// get result mask for comparison of LHS and RHS
	pmovmskb %xmm0,%ebx		// get result mask for 0 check
	subl	$16,%ecx		// decrement length remaining
	xorl	$0xFFFF,%eax		// complement compare mask so 1 means "not equal"
	orl	%ebx,%eax		// combine the masks and check for 1-bits
	jnz	LFoundDiffOr0		// we found differing bytes or a 0-byte
	dec	%edx			// more to go?
	jnz	LLoopOverChunks		// yes
	
	cmpl	$(kShort),%ecx		// a lot more to compare?
	jbe	LShort			// no
	jmp	LNotShort		// compute distance to next page crossing etc


// Found a zero and/or a difference in vector compare.
//	%esi = LHS ptr, already advanced by 16
//	%edi = RHS ptr, already advanced by 16
//	%eax = bit n set if bytes n differed or were 0

LFoundDiffOr0:
	bsf	%eax,%edx		// which byte differed or was 0?
	subl	$16,%esi		// point to start of vectors while we wait for bit scan
	subl	$16,%edi
	movzb	(%esi,%edx),%eax	// get LHS byte
	movzb	(%edi,%edx),%ecx	// get RHS byte
	popl	%ebx
	popl	%edi
	subl	%ecx,%eax		// compute difference (ie, return value)
	popl	%esi
	ret
