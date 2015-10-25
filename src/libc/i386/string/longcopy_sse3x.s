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
#include <platfunc.h>


/*
 * The bcopy/memcpy loops for very long operands, tuned for Pentium-M
 * class processors with Supplemental SSE3 and 64-byte cache lines.
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
// operands, with the standard ABI.
//
//	void longcopy(const void *dest, void *sou, size_t len)

// void longcopy(const void *dest, void *sou, size_t len)

        .text
        .private_extern _longcopy

        .align 	5
_longcopy:
	pushl	%ebp			// set up a frame for backtraces
	movl	%esp,%ebp
	pushl   %esi
	pushl   %edi
	pushl	%ebx			// we'll need to use this too
	movl    8(%ebp),%edi		// get dest ptr
	movl    12(%ebp),%esi           // get source ptr
	movl    16(%ebp),%ecx           // get length
	movl	%edi,%ebx		// copy dest ptr
	negl	%ebx
	andl	$63,%ebx		// get #bytes to cache line align destination
	jz	LBigChunkLoop		// already aligned

// Cache line align destination, so temporal stores in copy loops work right.

	pushl	%ebx			// arg3 - #bytes to align destination (1..63)
	pushl	%esi			// arg2 - source
	pushl	%edi			// arg1 - dest
	call	_memcpy			// align the destination
	addl	$12,%esp
	movl    8(%ebp),%edi		// recover dest ptr
	movl    12(%ebp),%esi           // recover source ptr
	movl    16(%ebp),%ecx           // recover length
	addl	%ebx,%esi		// adjust ptrs and lengths past copy
	addl	%ebx,%edi
	subl	%ebx,%ecx

// Loop over big chunks.
//      ecx = length remaining (>= 4096)
//      edi = dest (64-byte aligned)
//      esi = source (may be unaligned)

LBigChunkLoop:
	movl    $(kBigChunk),%edx       // assume we can do a full chunk
	cmpl	%edx,%ecx		// do we have a full chunk left to do?
	cmovbl	%ecx,%edx		// if not, only move what we have left
	andl	$-4096,%edx		// we work in page multiples
	xor	%eax,%eax		// initialize chunk offset
	jmp	LTouchLoop

// Touch in the next chunk.  We try to keep the prefetch unit in "kick-start" mode,
// by touching two adjacent cache lines every 8 lines of each page, in four slices.
// Because the source may be unaligned, we use byte loads to touch.
//      ecx = length remaining (including this chunk)
//      edi = ptr to start of dest chunk
//      esi = ptr to start of source chunk
//	edx = chunk length (multiples of pages)
//	ebx = scratch reg used to read a byte of each cache line
//	eax = chunk offset

	.align	4,0x90			// 16-byte align inner loops
LTouchLoop:
	movzb	(%esi,%eax),%ebx	// touch line 0, 2, 4, or 6 of page
	movzb	1*64(%esi,%eax),%ebx	// touch line 1, 3, 5, or 7
	movzb	8*64(%esi,%eax),%ebx	// touch line 8, 10, 12, or 14
	movzb	9*64(%esi,%eax),%ebx	// etc

	movzb	16*64(%esi,%eax),%ebx
	movzb	17*64(%esi,%eax),%ebx
	movzb	24*64(%esi,%eax),%ebx
	movzb	25*64(%esi,%eax),%ebx

	movzb	32*64(%esi,%eax),%ebx
	movzb	33*64(%esi,%eax),%ebx
	movzb	40*64(%esi,%eax),%ebx
	movzb	41*64(%esi,%eax),%ebx

	movzb	48*64(%esi,%eax),%ebx
	movzb	49*64(%esi,%eax),%ebx
	movzb	56*64(%esi,%eax),%ebx
	movzb	57*64(%esi,%eax),%ebx

	subl	$-128,%eax		// next slice of page (adding 128 w 8-bit immediate)
	testl	$512,%eax		// done with this page?
	jz	LTouchLoop		// no, next of four slices
	addl	$(4096-512),%eax	// move on to next page
	cmpl	%eax,%edx		// done with this chunk?
	jnz	LTouchLoop		// no, do next page

// The chunk has been pre-fetched, now copy it using non-temporal stores.
// There are two copy loops, depending on whether the source is 16-byte aligned
// or not.

	addl	%edx,%esi		// increment ptrs by chunk length
	addl	%edx,%edi
	subl	%edx,%ecx		// adjust remaining length
	negl	%edx			// prepare loop index (counts up to 0)
	testl	$15,%esi		// is source 16-byte aligned?
	jnz	LVeryLongUnaligned	// source is not aligned
	jmp	LVeryLongAligned

	.align	4,0x90			// 16-byte align inner loops
LVeryLongAligned:			// aligned loop over 128-bytes
	movdqa  (%esi,%edx),%xmm0
	movdqa  16(%esi,%edx),%xmm1
	movdqa  32(%esi,%edx),%xmm2
	movdqa  48(%esi,%edx),%xmm3
	movdqa  64(%esi,%edx),%xmm4
	movdqa  80(%esi,%edx),%xmm5
	movdqa  96(%esi,%edx),%xmm6
	movdqa  112(%esi,%edx),%xmm7

	movntdq %xmm0,(%edi,%edx)
	movntdq %xmm1,16(%edi,%edx)
	movntdq %xmm2,32(%edi,%edx)
	movntdq %xmm3,48(%edi,%edx)
	movntdq %xmm4,64(%edi,%edx)
	movntdq %xmm5,80(%edi,%edx)
	movntdq %xmm6,96(%edi,%edx)
	movntdq %xmm7,112(%edi,%edx)

	subl    $-128,%edx		// add 128 with an 8-bit immediate
	jnz	LVeryLongAligned
	jmp	LVeryLongChunkEnd

	.align	4,0x90			// 16-byte align inner loops
LVeryLongUnaligned:			// unaligned loop over 128-bytes
	movdqu  (%esi,%edx),%xmm0
	movdqu  16(%esi,%edx),%xmm1
	movdqu  32(%esi,%edx),%xmm2
	movdqu  48(%esi,%edx),%xmm3
	movdqu  64(%esi,%edx),%xmm4
	movdqu  80(%esi,%edx),%xmm5
	movdqu  96(%esi,%edx),%xmm6
	movdqu  112(%esi,%edx),%xmm7

	movntdq %xmm0,(%edi,%edx)
	movntdq %xmm1,16(%edi,%edx)
	movntdq %xmm2,32(%edi,%edx)
	movntdq %xmm3,48(%edi,%edx)
	movntdq %xmm4,64(%edi,%edx)
	movntdq %xmm5,80(%edi,%edx)
	movntdq %xmm6,96(%edi,%edx)
	movntdq %xmm7,112(%edi,%edx)

	subl    $-128,%edx		// add 128 with an 8-bit immediate
	jnz     LVeryLongUnaligned

LVeryLongChunkEnd:
	cmpl	$4096,%ecx		// at least another page to go?
	jae	LBigChunkLoop		// yes

// Done.  Call memcpy() again to handle the 0-4095 bytes at the end.

	sfence				// required by non-temporal stores
	testl	%ecx,%ecx		// anything left to copy?
	jz	1f
	pushl	%ecx			// arg3 - #bytes to align destination (1..63)
	pushl	%esi			// arg2 - source
	pushl	%edi			// arg1 - dest
	call	_memcpy			// align the destination
	addl	$12,%esp		// pop off arguments
1:
	popl	%ebx
	popl	%edi
	popl	%esi
	popl	%ebp
	ret
