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
	pushl	%esi
	pushl	%edi
	movl	20(%esp),%ecx		// get length
	movl	12(%esp),%esi		// get LHS ptr
	movl	16(%esp),%edi		// get RHS ptr
	cmpl	$(kShort),%ecx		// worth accelerating?
	ja	LNotShort		// yes
	

// Too short to bother with parallel compares.  Loop over bytes.
//	%esi = LHS ptr
//	%edi = RHS ptr
//	%ecx = length (<= kShort)

LShort:
	testl	%ecx,%ecx		// 0-length?
	jnz	LShortLoop		// no
	xorl	%eax,%eax		// return 0
	jmp	LExit
	.align	4,0x90			// align inner loops to optimize I-fetch
LShortLoop:				// loop over bytes
	movzb	(%esi),%eax		// get LHS byte
	movzb	(%edi),%edx		// get RHS byte
	incl	%esi
	incl	%edi
	subl	%edx,%eax		// compare them
	jnz	LExit			// done if not equal
	decl	%ecx			// decrement length
	jnz	LShortLoop
LExit:					// return value is in %eax
	popl	%edi
	popl	%esi
	ret
	
LNotEqual:				// here from LLoopOverBytes with LHS in eax
	movzb	(%edi),%edx		// get RHS byte
	subl	%edx,%eax		// generate return value (nonzero)
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
	inc	%esi
	cmpb	(%edi),%al		// compare to RHS byte
	jnz	LNotEqual		// done if not equal
	inc	%edi
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
	movdqu	(%esi),%xmm0		// get LHS
	movdqu	(%edi),%xmm1		// get RHS
	addl	$16,%esi
	pcmpeqb	%xmm1,%xmm0		// compare LHS to RHS
	addl	$16,%edi
	pmovmskb %xmm0,%eax		// collect comparison result bits (1 if equal)
	subl	$16,%ecx		// adjust length remaining
	xorl	$0xFFFF,%eax		// all equal?
	jne	LDifferent		// no, we found differing bytes
	dec	%edx			// more to go?
	jnz	LLoopOverChunks
	
	cmpl	$(kShort),%ecx		// a lot more to compare?
	jbe	LShort			// no
	jmp	LNotShort		// compute distance to next page crossing etc


// Found a difference.  
//	%esi = LHS ptr, already advanced by 16
//	%edi = RHS ptr, already advanced by 16
//	%eax = complemented compare vector (ie, 0 == equal)

LDifferent:
	bsf	%eax,%edx		// which byte differed?
	subl	$16,%esi		// point to byte 0 while we wait for bit scan
	subl	$16,%edi
	movzb	(%esi,%edx),%eax	// get LHS byte
	movzb	(%edi,%edx),%ecx	// get RHS byte
	subl	%ecx,%eax		// compute difference (ie, return value)
	popl	%edi
	popl	%esi
	ret
