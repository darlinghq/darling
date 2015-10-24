/*
 * Copyright (c) 2005-2006 Apple Computer, Inc. All rights reserved.
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
#include <platfunc.h>

/* The common path for nonzero memset and the memset_pattern routines,
 * tuned for Pentium-M class processors with SSE2 and 64-byte cache lines.
 * This is used by the following functions:
 *
 *	void *memset(void *b, int c, size_t len);                   // when c!=0
 *	void memset_pattern4(void *b, const void *c4, size_t len);
 *	void memset_pattern8(void *b, const void *c8, size_t len);
 *	void memset_pattern16(void *b, const void *c16, size_t len);
 *
 * Note bzero() and memset() of 0 are handled separately.
 */

#define	kShort		63
#define	kVeryLong	(1024*1024)

// Initial entry from Libc with parameters passed in registers.  Although we
// correctly handle misaligned ptrs and short operands, they are inefficient.
// Therefore our caller should filter out short operands and exploit local
// knowledge (ie, original pattern length) to align the ptr if possible.
// When called, we expect:
//	%edi = ptr to memory to set (not necessarily aligned)
//	%edx = length (may be short or even 0)
//	%xmm0 = the pattern to store
// Return conditions:
//	%eax, %edi, %esi, %ecx, and %edx all trashed

	.align 5
	.private_extern _memset_pattern
_memset_pattern:
	cmpl    $(kShort),%edx		// long enough to bother aligning?
	ja	LNotShort		// yes
	jmp	LShort			// no

// Here for short operands or the end of long ones.
//      %edx = length
//      %edi = ptr (may not be not aligned)
//      %xmm0 = pattern

LUnalignedStore16:
	movdqu	%xmm0,(%edi)		// stuff in another 16 bytes
	subl	$16,%edx
	addl	$16,%edi
LShort:	
	cmpl	$16,%edx		// room for another vector?
	jge	LUnalignedStore16	// yes
LLessThan16:				// here at end of copy with < 16 bytes remaining
	test	$8,%dl			// 8-byte store required?
	jz	2f			// no
	movq	%xmm0,(%edi)		// pack in 8 low bytes
	psrldq	$8,%xmm0		// then shift vector down 8 bytes
	addl	$8,%edi
2:
	test	$4,%dl			// 4-byte store required?
	jz	3f			// no
	movd	%xmm0,(%edi)		// pack in 4 low bytes
	psrldq	$4,%xmm0		// then shift vector down 4 bytes
	addl	$4,%edi
3:
	andl	$3,%edx			// more to go?
	jz	5f			// no
	movd	%xmm0,%eax		// move remainders out into %eax
4:					// loop on up to three bytes
	movb	%al,(%edi)		// pack in next byte
	shrl	$8,%eax			// shift next byte into position
	inc	%edi
	dec	%edx
	jnz	4b
5:	ret

// Long enough to justify aligning ptr.  Note that we have to rotate the
// pattern to account for any alignment.  We do this by doing two unaligned
// stores, and then an aligned load from the middle of the two stores.
// This will stall on store forwarding alignment mismatch, and the unaligned
// stores can be pretty slow too, but the alternatives aren't any better.
// Fortunately, in most cases our caller has already aligned the ptr.
//      %edx = length (> kShort)
//      %edi = ptr (may not be aligned)
//      %xmm0 = pattern

LNotShort:
	movl    %edi,%ecx		// copy dest ptr
	negl    %ecx
	andl    $15,%ecx                // mask down to #bytes to 16-byte align
	jz	LAligned		// skip if already aligned
	movdqu	%xmm0,(%edi)		// store 16 unaligned bytes
	movdqu	%xmm0,16(%edi)		// and 16 more, to be sure we have an aligned chunk
	addl	%ecx,%edi		// now point to the aligned chunk
	subl	%ecx,%edx		// adjust remaining count
	movdqa	(%edi),%xmm0		// get the rotated pattern (probably stalling)
	addl	$16,%edi		// skip past the aligned chunk
	subl	$16,%edx

// Set up for 64-byte loops.
//      %edx = length remaining
//      %edi = ptr (aligned)
//      %xmm0 = rotated pattern

LAligned:
	movl	%edx,%ecx		// copy length remaining
	andl    $63,%edx                // mask down to residual length (0..63)
	andl    $-64,%ecx               // %ecx <- #bytes we will zero in by-64 loop
	jz	LNoMoreChunks		// no 64-byte chunks
	addl    %ecx,%edi               // increment ptr by length to move
	cmpl	$(kVeryLong),%ecx	// long enough to justify non-temporal stores?
	jge	LVeryLong		// yes
	negl    %ecx			// negate length to move
	jmp	1f

// Loop over 64-byte chunks, storing into cache.

	.align	4,0x90			// keep inner loops 16-byte aligned
1:
	movdqa  %xmm0,(%edi,%ecx)
	movdqa  %xmm0,16(%edi,%ecx)
	movdqa  %xmm0,32(%edi,%ecx)
	movdqa  %xmm0,48(%edi,%ecx)
	addl    $64,%ecx
	jne     1b

	jmp	LNoMoreChunks

// Very long operands: use non-temporal stores to bypass cache.

LVeryLong:
	negl    %ecx			// negate length to move
	jmp	1f

	.align	4,0x90			// keep inner loops 16-byte aligned
1:
	movntdq %xmm0,(%edi,%ecx)
	movntdq %xmm0,16(%edi,%ecx)
	movntdq %xmm0,32(%edi,%ecx)
	movntdq %xmm0,48(%edi,%ecx)
	addl    $64,%ecx
	jne     1b

	sfence                          // required by non-temporal stores
	jmp	LNoMoreChunks

// Handle leftovers: loop by 16.
//      %edx = length remaining (<64)
//      %edi = ptr (aligned)
//      %xmm0 = rotated pattern

LLoopBy16:
	movdqa	%xmm0,(%edi)		// pack in 16 more bytes
	subl	$16,%edx		// decrement count
	addl	$16,%edi		// increment ptr
LNoMoreChunks:
	cmpl	$16,%edx		// more to go?
	jge	LLoopBy16		// yes
	jmp	LLessThan16		// handle up to 15 remaining bytes
