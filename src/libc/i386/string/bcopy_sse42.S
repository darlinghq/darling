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
#include <platfunc.h>

/*
 * The bcopy/memcpy loops, tuned for Nehalem.
 *
 * The following #defines are tightly coupled to the u-architecture:
 */

#define kShort  80			// too short to bother with SSE (must be >=80)


// void bcopy(const void *src, void *dst, size_t len);

PLATFUNC_FUNCTION_START(bcopy, sse42, 32, 5)
	pushl	%ebp			// set up a frame for backtraces
	movl	%esp,%ebp
	pushl   %esi
	pushl   %edi
	movl    8(%ebp),%esi		// get source ptr
	movl    12(%ebp),%edi           // get dest ptr
	movl    16(%ebp),%ecx           // get length
	movl    %edi,%edx
	subl    %esi,%edx               // (dest - source)
	cmpl    %ecx,%edx               // must move in reverse if (dest - source) < length
	jb      LReverseIsland
	cmpl    $(kShort),%ecx          // long enough to bother with SSE?
	jbe     Lshort			// no
	jmp	LNotShort

//
// void *memcpy(void *dst, const void *src, size_t len);
// void *memmove(void *dst, const void *src, size_t len);
//

PLATFUNC_FUNCTION_START(memcpy, sse42, 32, 0)	// void *memcpy(void *dst, const void *src, size_t len)
PLATFUNC_FUNCTION_START(memmove, sse42, 32, 0)	// void *memmove(void *dst, const void *src, size_t len)
	pushl	%ebp			// set up a frame for backtraces
	movl	%esp,%ebp
	pushl   %esi
	pushl   %edi
	movl    8(%ebp),%edi		// get dest ptr
	movl    12(%ebp),%esi           // get source ptr
	movl    16(%ebp),%ecx           // get length
	movl    %edi,%edx
	subl    %esi,%edx               // (dest - source)
	cmpl    %ecx,%edx               // must move in reverse if (dest - source) < length
	jb      LReverseIsland
	cmpl    $(kShort),%ecx          // long enough to bother with SSE?
	ja      LNotShort               // yes

// Handle short forward copies.  As the most common case, this is the fall-through path.
//      ecx = length (<= kShort)
//      esi = source ptr
//      edi = dest ptr

Lshort:
	movl    %ecx,%edx		// copy length
	shrl	$2,%ecx			// get #doublewords
	jz	3f
2:					// loop copying doublewords
	movl	(%esi),%eax
	addl	$4,%esi
	movl	%eax,(%edi)
	addl	$4,%edi
	dec	%ecx
	jnz	2b
3:					// handle leftover bytes (0..3) in last word
	andl	$3,%edx			// any leftover bytes?
	jz	Lexit
4:					// loop copying bytes
	movb	(%esi),%al
	inc	%esi
	movb	%al,(%edi)
	inc	%edi
	dec	%edx
	jnz	4b
Lexit:
	movl    8(%ebp),%eax		// get return value (dst ptr) for memcpy/memmove
	popl    %edi
	popl    %esi
	popl	%ebp
	ret


LReverseIsland:				// keep the "jb" above a short branch...
	jmp	LReverse		// ...because reverse moves are uncommon


// Handle forward moves that are long enough to justify use of SSE.
// First, 16-byte align the destination.
//      ecx = length (> kShort)
//      esi = source ptr
//      edi = dest ptr

LNotShort:
	movl    %edi,%edx               // copy destination
	negl    %edx
	andl    $15,%edx                // get #bytes to align destination
	jz	LDestAligned		// already aligned
	subl    %edx,%ecx               // decrement length
1:					// loop copying 1..15 bytes
	movb	(%esi),%al
	inc	%esi
	movb	%al,(%edi)
	inc	%edi
	dec	%edx
	jnz	1b

// Destination is now aligned.  Nehalem does a great job with unaligned SSE loads,
// so we use MOVDQU rather than aligned loads and shifts.  Since kShort>=80, we
// know there is at least one 64-byte chunk to move.
// When we enter the copy loops, the following registers are set up:
//      ecx = residual length (0..63)
//	edx = -(length to move), a multiple of 64
//      esi = ptr to 1st source byte not to move (unaligned)
//      edi = ptr to 1st dest byte not to move (aligned)

LDestAligned:
	movl    %ecx,%edx               // copy length
	andl    $63,%ecx                // get remaining bytes for Lshort
	andl    $-64,%edx               // get number of bytes we will copy in inner loop
	addl    %edx,%esi               // point to 1st byte not copied
	addl    %edx,%edi
	negl    %edx                    // now generate offset to 1st byte to be copied
	testl	$15,%esi		// source also aligned?
	jnz	LUnalignedLoop
	jmp	LAlignedLoop


// Forward loop for aligned operands.

	.align	4,0x90			// 16-byte align inner loops
LAlignedLoop:				// loop over 64-byte chunks
	movdqa  (%esi,%edx),%xmm0
	movdqa  16(%esi,%edx),%xmm1
	movdqa  32(%esi,%edx),%xmm2
	movdqa  48(%esi,%edx),%xmm3

	movdqa  %xmm0,(%edi,%edx)
	movdqa  %xmm1,16(%edi,%edx)
	movdqa  %xmm2,32(%edi,%edx)
	movdqa  %xmm3,48(%edi,%edx)

	addl    $64,%edx
	jnz     LAlignedLoop

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Forward loop for unaligned operands.

	.align	4,0x90			// 16-byte align inner loops
LUnalignedLoop:				// loop over 64-byte chunks
	movdqu  (%esi,%edx),%xmm0
	movdqu  16(%esi,%edx),%xmm1
	movdqu  32(%esi,%edx),%xmm2
	movdqu  48(%esi,%edx),%xmm3

	movdqa  %xmm0,(%edi,%edx)
	movdqa  %xmm1,16(%edi,%edx)
	movdqa  %xmm2,32(%edi,%edx)
	movdqa  %xmm3,48(%edi,%edx)

	addl    $64,%edx
	jnz     LUnalignedLoop

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Reverse moves.  They are only used with destructive overlap.
//      ecx = length
//      esi = source ptr
//      edi = dest ptr

LReverse:
	addl    %ecx,%esi               // point to end of strings
	addl    %ecx,%edi
	cmpl    $(kShort),%ecx          // long enough to bother with SSE?
	ja      LReverseNotShort        // yes

// Handle reverse short copies.
//      ecx = length
//      esi = one byte past end of source
//      edi = one byte past end of dest

LReverseShort:
	movl    %ecx,%edx		// copy length
	shrl	$2,%ecx			// #words
	jz	3f
1:
	subl	$4,%esi
	movl	(%esi),%eax
	subl	$4,%edi
	movl	%eax,(%edi)
	dec	%ecx
	jnz	1b
3:
	andl	$3,%edx			// bytes?
	jz	5f
4:
	dec	%esi
	movb	(%esi),%al
	dec	%edi
	movb	%al,(%edi)
	dec	%edx
	jnz	4b
5:
	movl    8(%ebp),%eax		// get return value (dst ptr) for memcpy/memmove
	popl    %edi
	popl    %esi
	popl	%ebp
	ret

// Handle a reverse move long enough to justify using SSE.
//      ecx = length
//      esi = one byte past end of source
//      edi = one byte past end of dest

LReverseNotShort:
	movl    %edi,%edx               // copy destination
	andl    $15,%edx                // get #bytes to align destination
	je      LReverseDestAligned     // already aligned
	subl	%edx,%ecx		// adjust length
1:					// loop copying 1..15 bytes
	dec	%esi
	movb	(%esi),%al
	dec	%edi
	movb	%al,(%edi)
	dec	%edx
	jnz	1b

// Destination is now aligned.  Prepare for reverse loops.

LReverseDestAligned:
	movl    %ecx,%edx               // copy length
	andl    $63,%ecx                // get remaining bytes for Lshort
	andl    $-64,%edx               // get number of bytes we will copy in inner loop
	subl    %edx,%esi               // point to endpoint of copy
	subl    %edx,%edi
	testl	$15,%esi		// is source aligned too?
	jnz     LReverseUnalignedLoop   // no

LReverseAlignedLoop:                    // loop over 64-byte chunks
	movdqa  -16(%esi,%edx),%xmm0
	movdqa  -32(%esi,%edx),%xmm1
	movdqa  -48(%esi,%edx),%xmm2
	movdqa  -64(%esi,%edx),%xmm3

	movdqa  %xmm0,-16(%edi,%edx)
	movdqa  %xmm1,-32(%edi,%edx)
	movdqa  %xmm2,-48(%edi,%edx)
	movdqa  %xmm3,-64(%edi,%edx)

	subl    $64,%edx
	jne     LReverseAlignedLoop

	jmp     LReverseShort           // copy remaining 0..63 bytes and done


// Reverse, unaligned loop.  LDDQU==MOVDQU on these machines.

LReverseUnalignedLoop:                  // loop over 64-byte chunks
	movdqu  -16(%esi,%edx),%xmm0
	movdqu  -32(%esi,%edx),%xmm1
	movdqu  -48(%esi,%edx),%xmm2
	movdqu  -64(%esi,%edx),%xmm3

	movdqa  %xmm0,-16(%edi,%edx)
	movdqa  %xmm1,-32(%edi,%edx)
	movdqa  %xmm2,-48(%edi,%edx)
	movdqa  %xmm3,-64(%edi,%edx)

	subl    $64,%edx
	jne     LReverseUnalignedLoop

	jmp     LReverseShort           // copy remaining 0..63 bytes and done


PLATFUNC_DESCRIPTOR(bcopy,sse42,kHasSSE4_2,0)
PLATFUNC_DESCRIPTOR(memcpy,sse42,kHasSSE4_2,0)
PLATFUNC_DESCRIPTOR(memmove,sse42,kHasSSE4_2,0)
