/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

/*
 * The bcopy/memcpy loops for very long operands, tuned for 64-bit
 * Pentium-M class processors with Supplemental SSE3 and 64-byte cache lines.
 * This is the 64-bit version.
 *
 * The following #defines are tightly coupled to the u-architecture:
 */

#define kBigChunk   (256*1024)          // outer loop chunk size for kVeryLong sized operands


// Very long forward moves.  These are at least several pages, so we loop over big
// chunks of memory (kBigChunk in size.)  We first prefetch the chunk, and then copy
// it using non-temporal stores.  Hopefully all the reads occur in the prefetch loop,
// so the copy loop reads from L2 and writes directly to memory (with write combining.)
// This minimizes bus turnaround and maintains good DRAM page locality.
// Note that for this scheme to work, kVeryLong must be a large fraction of L2 cache
// size.  Otherwise, it is counter-productive to bypass L2 on the stores.
//
// We are called from the platfunc bcopy loops when they encounter very long
// operands, with the standard ABI:
//      rdi = dest ptr
//      rsi = source ptr
//      rdx = length (>= 8kb, probably much bigger)

// void longcopy(const void *dest, void *sou, size_t len)

	.private_extern _longcopy
_longcopy:
	pushq	%rbp			// set up a frame for backtraces
	movq	%rsp,%rbp
	movl	%edi,%eax		// copy dest ptr
	negl	%eax
	andl	$63,%eax		// get #bytes to cache line align destination
	jz	LBigChunkLoop		// already aligned

	// Cache line align destination, so temporal stores in copy loops work right.
	// The recursive call returns with the source and dest ptrs properly updated.

	subq	%rax,%rdx		// get length remaining after dest is aligned
	pushq	%rdx			// save length remaining
	movl	%eax,%edx		// #bytes to copy to align destination
	call	_memcpy
	popq	%rdx			// recover adjusted length

// Loop over big chunks.
//      rdx = length remaining (>= 4096)
//      rdi = dest (64-byte aligned)
//      rsi = source (may be unaligned)

LBigChunkLoop:
	movl    $(kBigChunk),%r8d       // assume we can do a full chunk
	cmpq	%r8,%rdx		// do we have a full chunk left to do?
	cmovbl	%edx,%r8d		// if not, only move what we have left
	andl	$-4096,%r8d		// we work in page multiples
	xorl	%eax,%eax		// initialize chunk offset
	jmp	LTouchLoop

// Touch in the next chunk.  We try to keep the prefetch unit in "kick-start" mode,
// by touching two adjacent cache lines every 8 lines of each page, in four slices.
// Because the source may be unaligned, we use byte loads to touch.
//      rdx = length remaining (including this chunk)
//      rdi = ptr to start of dest chunk
//      rsi = ptr to start of source chunk
//	r8d = chunk length (multiples of pages, less than  2**32)
//	ecx = scratch reg used to read a byte of each cache line
//	eax = chunk offset

	.align	4,0x90			// 16-byte align inner loops
LTouchLoop:
	movzb	(%rsi,%rax),%ecx	// touch line 0, 2, 4, or 6 of page
	movzb	1*64(%rsi,%rax),%ecx	// touch line 1, 3, 5, or 7
	movzb	8*64(%rsi,%rax),%ecx	// touch line 8, 10, 12, or 14
	movzb	9*64(%rsi,%rax),%ecx	// etc

	movzb	16*64(%rsi,%rax),%ecx
	movzb	17*64(%rsi,%rax),%ecx
	movzb	24*64(%rsi,%rax),%ecx
	movzb	25*64(%rsi,%rax),%ecx

	movzb	32*64(%rsi,%rax),%ecx
	movzb	33*64(%rsi,%rax),%ecx
	movzb	40*64(%rsi,%rax),%ecx
	movzb	41*64(%rsi,%rax),%ecx

	movzb	48*64(%rsi,%rax),%ecx
	movzb	49*64(%rsi,%rax),%ecx
	movzb	56*64(%rsi,%rax),%ecx
	movzb	57*64(%rsi,%rax),%ecx

	subl	$-128,%eax		// next slice of page (adding 128 w 8-bit immediate)
	testl	$512,%eax		// done with this page?
	jz	LTouchLoop		// no, next of four slices
	addl	$(4096-512),%eax	// move on to next page
	cmpl	%eax,%r8d		// done with this chunk?
	jnz	LTouchLoop		// no, do next page

	// The chunk has been pre-fetched, now copy it using non-temporal stores.
	// There are two copy loops, depending on whether the source is 16-byte aligned
	// or not.

	movl	%r8d,%ecx		// copy chunk size to a reg that doesn't use REX prefix
	addq	%rcx,%rsi		// increment ptrs by chunk length
	addq	%rcx,%rdi
	subq	%rcx,%rdx		// adjust remaining length
	negq	%rcx			// prepare loop index (counts up to 0)
	testl	$15,%esi		// is source 16-byte aligned?
	jnz	LVeryLongUnaligned	// no
	jmp	LVeryLongAligned

	.align	4,0x90			// 16-byte align inner loops
LVeryLongAligned:			// aligned loop over 128-bytes
	movdqa  (%rsi,%rcx),%xmm0
	movdqa  16(%rsi,%rcx),%xmm1
	movdqa  32(%rsi,%rcx),%xmm2
	movdqa  48(%rsi,%rcx),%xmm3
	movdqa  64(%rsi,%rcx),%xmm4
	movdqa  80(%rsi,%rcx),%xmm5
	movdqa  96(%rsi,%rcx),%xmm6
	movdqa  112(%rsi,%rcx),%xmm7

	movntdq %xmm0,(%rdi,%rcx)
	movntdq %xmm1,16(%rdi,%rcx)
	movntdq %xmm2,32(%rdi,%rcx)
	movntdq %xmm3,48(%rdi,%rcx)
	movntdq %xmm4,64(%rdi,%rcx)
	movntdq %xmm5,80(%rdi,%rcx)
	movntdq %xmm6,96(%rdi,%rcx)
	movntdq %xmm7,112(%rdi,%rcx)

	subq    $-128,%rcx		// add 128 with an 8-bit immediate
	jnz	LVeryLongAligned
	jmp	LVeryLongChunkEnd

	.align	4,0x90			// 16-byte align inner loops
LVeryLongUnaligned:			// unaligned loop over 128-bytes
	movdqu  (%rsi,%rcx),%xmm0
	movdqu  16(%rsi,%rcx),%xmm1
	movdqu  32(%rsi,%rcx),%xmm2
	movdqu  48(%rsi,%rcx),%xmm3
	movdqu  64(%rsi,%rcx),%xmm4
	movdqu  80(%rsi,%rcx),%xmm5
	movdqu  96(%rsi,%rcx),%xmm6
	movdqu  112(%rsi,%rcx),%xmm7

	movntdq %xmm0,(%rdi,%rcx)
	movntdq %xmm1,16(%rdi,%rcx)
	movntdq %xmm2,32(%rdi,%rcx)
	movntdq %xmm3,48(%rdi,%rcx)
	movntdq %xmm4,64(%rdi,%rcx)
	movntdq %xmm5,80(%rdi,%rcx)
	movntdq %xmm6,96(%rdi,%rcx)
	movntdq %xmm7,112(%rdi,%rcx)

	subq    $-128,%rcx		// add 128 with an 8-bit immediate
	jnz     LVeryLongUnaligned

LVeryLongChunkEnd:
	cmpq	$4096,%rdx		// at least another page to go?
	jae	LBigChunkLoop		// yes

	// Done.  Call memcpy() again to handle the 0-4095 bytes at the end.
	// We still have the args in the right registers:
	//	rdi = destination ptr
	//	rsi = source ptr
	//	rdx = length remaining (0..4095)

	sfence				// required by non-temporal stores
	testl	%edx,%edx		// anything left to copy?
	jz	1f
	call	_memcpy
1:
	popq	%rbp			// restore frame ptr
	ret
