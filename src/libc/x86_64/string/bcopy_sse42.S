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
 * The bcopy/memcpy loops, tuned for Nehalem.  This is the 64-bit version.
 *
 * The following #defines are tightly coupled to the u-architecture:
 */

#define kShort  80			// too short to bother with SSE (must be >=80)


// void bcopy(const void *src, void *dst, size_t len);

PLATFUNC_FUNCTION_START(bcopy, sse42, 64, 5)
	pushq	%rbp			// set up a frame for backtraces
	movq	%rsp,%rbp
	movq	%rsi,%rax		// copy dest ptr
	movq	%rdi,%rsi		// xchange source and dest ptrs
	movq	%rax,%rdi
	subq    %rsi,%rax               // (dest - source)
	cmpq    %rdx,%rax               // must move in reverse if (dest - source) < length
	jb      LReverseIsland
	cmpq    $(kShort),%rdx          // long enough to bother with SSE?
	jbe     LShort			// no
	jmp	LNotShort

//
// void *memcpy(void *dst, const void *src, size_t len);
// void *memmove(void *dst, const void *src, size_t len);
//

PLATFUNC_FUNCTION_START(memcpy, sse42, 64, 0)		// void *memcpy(void *dst, const void *src, size_t len)
PLATFUNC_FUNCTION_START(memmove, sse42, 64, 0)	// void *memmove(void *dst, const void *src, size_t len)
	pushq	%rbp			// set up a frame for backtraces
	movq	%rsp,%rbp
	movq	%rdi,%r11		// save return value here        
	movq    %rdi,%rax
	subq    %rsi,%rax               // (dest - source)
	cmpq    %rdx,%rax               // must move in reverse if (dest - source) < length
	jb      LReverseIsland
	cmpq    $(kShort),%rdx          // long enough to bother with SSE?
	ja      LNotShort               // yes

// Handle short forward copies.  As the most common case, this is the fall-through path.
//      rdx = length (<= kShort)
//      rsi = source ptr
//      rdi = dest ptr

LShort:
	movl    %edx,%ecx		// copy length using 32-bit operation
	shrl	$2,%ecx			// get #doublewords
	jz	3f
2:					// loop copying doublewords
	movl	(%rsi),%eax
	addq	$4,%rsi
	movl	%eax,(%rdi)
	addq	$4,%rdi
	decl	%ecx
	jnz	2b
3:					// handle leftover bytes (0..3) in last word
	andl	$3,%edx			// any leftover bytes?
	jz	5f
4:					// loop copying bytes
	movb	(%rsi),%al
	incq	%rsi
	movb	%al,(%rdi)
	incq	%rdi
	decl	%edx
	jnz	4b
5:
	movq	%r11,%rax		// get return value (dst ptr) for memcpy/memmove
	popq	%rbp
	ret


LReverseIsland:				// keep the "jb" above a short branch...
	jmp	LReverse		// ...because reverse moves are uncommon


// Handle forward moves that are long enough to justify use of SSE.
// First, 16-byte align the destination.
//      rdx = length (> kShort)
//      rsi = source ptr
//      rdi = dest ptr

LNotShort:
	movl    %edi,%ecx               // copy low half of destination ptr
	negl    %ecx
	andl    $15,%ecx                // get #bytes to align destination
	jz	LDestAligned		// already aligned
	subl    %ecx,%edx               // decrement length
1:					// loop copying 1..15 bytes
	movb	(%rsi),%al
	inc	%rsi
	movb	%al,(%rdi)
	inc	%rdi
	dec	%ecx
	jnz	1b


// Destination is now aligned.  Nehalem does a great job with unaligned SSE loads,
// so we use MOVDQU rather than aligned loads and shifts.  Since kShort>=80, we
// know there is at least one 64-byte chunk to move.
// When we enter the copy loops, the following registers are set up:
//      rdx = residual length (0..63)
//	rcx = -(length to move), a multiple of 64 less than 2GB
//      rsi = ptr to 1st source byte not to move (unaligned)
//      rdi = ptr to 1st dest byte not to move (aligned)

LDestAligned:
	movq    %rdx,%rcx               // copy length
	andl    $63,%edx                // get remaining bytes for LShort
	andq    $-64,%rcx               // get number of bytes we will copy in inner loop
	addq    %rcx,%rsi               // point to 1st byte not copied
	addq    %rcx,%rdi
	negq    %rcx                    // now generate offset to 1st byte to be copied
	testl	$15,%esi		// source also aligned?
	jnz	LUnalignedLoop
	jmp	LAlignedLoop


// Forward loop for aligned operands.

	.align	4,0x90			// 16-byte align inner loops
LAlignedLoop:				// loop over 64-byte chunks
	movdqa  (%rsi,%rcx),%xmm0
	movdqa  16(%rsi,%rcx),%xmm1
	movdqa  32(%rsi,%rcx),%xmm2
	movdqa  48(%rsi,%rcx),%xmm3

	movdqa  %xmm0,(%rdi,%rcx)
	movdqa  %xmm1,16(%rdi,%rcx)
	movdqa  %xmm2,32(%rdi,%rcx)
	movdqa  %xmm3,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     LAlignedLoop

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Forward loop for unaligned operands.

	.align	4,0x90			// 16-byte align inner loops
LUnalignedLoop:				// loop over 64-byte chunks
	movdqu  (%rsi,%rcx),%xmm0
	movdqu  16(%rsi,%rcx),%xmm1
	movdqu  32(%rsi,%rcx),%xmm2
	movdqu  48(%rsi,%rcx),%xmm3

	movdqa  %xmm0,(%rdi,%rcx)
	movdqa  %xmm1,16(%rdi,%rcx)
	movdqa  %xmm2,32(%rdi,%rcx)
	movdqa  %xmm3,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     LUnalignedLoop

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Reverse moves.  These are only used with destructive overlap.
//      rdx = length
//      rsi = source ptr
//      rdi = dest ptr

LReverse:
	addq    %rdx,%rsi               // point to end of strings
	addq    %rdx,%rdi
	cmpq    $(kShort),%rdx          // long enough to bother with SSE?
	ja      LReverseNotShort        // yes

// Handle reverse short copies.
//      edx = length (<= kShort)
//      rsi = one byte past end of source
//      rdi = one byte past end of dest

LReverseShort:
	movl    %edx,%ecx		// copy length
	shrl	$3,%ecx			// #quadwords
	jz	3f
1:
	subq	$8,%rsi
	movq	(%rsi),%rax
	subq	$8,%rdi
	movq	%rax,(%rdi)
	decl	%ecx
	jnz	1b
3:
	andl	$7,%edx			// bytes?
	jz	5f
4:
	decq	%rsi
	movb	(%rsi),%al
	decq	%rdi
	movb	%al,(%rdi)
	decl	%edx
	jnz	4b
5:
	movq	%r11,%rax		// get return value (dst ptr) for memcpy/memmove
	popq	%rbp
	ret

// Handle a reverse move long enough to justify using SSE.
//      rdx = length (> kShort)
//      rsi = one byte past end of source
//      rdi = one byte past end of dest

LReverseNotShort:
	movl    %edi,%ecx               // copy destination
	andl    $15,%ecx                // get #bytes to align destination
	jz      LReverseDestAligned     // already aligned
	subq	%rcx,%rdx		// adjust length
1:					// loop copying 1..15 bytes
	decq	%rsi
	movb	(%rsi),%al
	decq	%rdi
	movb	%al,(%rdi)
	decl	%ecx
	jnz	1b

// Destination is now aligned.  Prepare for reverse loops.

LReverseDestAligned:
	movq    %rdx,%rcx               // copy length
	andl    $63,%edx                // get remaining bytes for LReverseShort
	andq    $-64,%rcx               // get number of bytes we will copy in inner loop
	subq    %rcx,%rsi               // point to endpoint of copy
	subq    %rcx,%rdi
	testl	$15,%esi		// is source aligned too?
	jnz     LReverseUnalignedLoop   // no

LReverseAlignedLoop:                    // loop over 64-byte chunks
	movdqa  -16(%rsi,%rcx),%xmm0
	movdqa  -32(%rsi,%rcx),%xmm1
	movdqa  -48(%rsi,%rcx),%xmm2
	movdqa  -64(%rsi,%rcx),%xmm3

	movdqa  %xmm0,-16(%rdi,%rcx)
	movdqa  %xmm1,-32(%rdi,%rcx)
	movdqa  %xmm2,-48(%rdi,%rcx)
	movdqa  %xmm3,-64(%rdi,%rcx)

	subq    $64,%rcx
	jne     LReverseAlignedLoop

	jmp     LReverseShort           // copy remaining 0..63 bytes and done


// Reverse, unaligned loop.  LDDQU==MOVDQU on these machines.

LReverseUnalignedLoop:                  // loop over 64-byte chunks
	movdqu  -16(%rsi,%rcx),%xmm0
	movdqu  -32(%rsi,%rcx),%xmm1
	movdqu  -48(%rsi,%rcx),%xmm2
	movdqu  -64(%rsi,%rcx),%xmm3

	movdqa  %xmm0,-16(%rdi,%rcx)
	movdqa  %xmm1,-32(%rdi,%rcx)
	movdqa  %xmm2,-48(%rdi,%rcx)
	movdqa  %xmm3,-64(%rdi,%rcx)

	subq    $64,%rcx
	jne     LReverseUnalignedLoop

	jmp     LReverseShort           // copy remaining 0..63 bytes and done


PLATFUNC_DESCRIPTOR(bcopy,sse42,kHasSSE4_2,0)
PLATFUNC_DESCRIPTOR(memcpy,sse42,kHasSSE4_2,0)
PLATFUNC_DESCRIPTOR(memmove,sse42,kHasSSE4_2,0)
