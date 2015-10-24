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

/*
 * The bcopy/memcpy loops, tuned for Pentium-M class processors with SSE2
 * and 64-byte cache lines, such as Core and Core 2.
 *
 * The following #defines are tightly coupled to the u-architecture:
 */

#define kShort  80                      // too short to bother with SSE (must be >=80)
#define kVeryLong   (500*1024)          // large enough for non-temporal stores (must be >= 8192)
#define kBigChunk   (256*1024)          // outer loop chunk size for kVeryLong sized operands
#define kFastUCode  (16*1024)           // cutoff for microcode fastpath for "rep/movsl"


// void bcopy(const void *src, void *dst, size_t len);

PLATFUNC_FUNCTION_START(bcopy, sse2, 32, 5)
	pushl   %ebp                    // set up a frame for backtraces
	movl    %esp,%ebp
	pushl   %esi
	pushl   %edi
	movl    8(%ebp),%esi            // get source ptr
	movl    12(%ebp),%edi           // get dest ptr
	jmp     Ljoin 

//
// void *memcpy(void *dst, const void *src, size_t len);
// void *memmove(void *dst, const void *src, size_t len);
//

PLATFUNC_FUNCTION_START(memcpy, sse2, 32, 0)    // void *memcpy(void *dst, const void *src, size_t len)
PLATFUNC_FUNCTION_START(memmove, sse2, 32, 0)   // void *memmove(void *dst, const void *src, size_t len)
Lmemcpy_sse2:
	pushl   %ebp                    // set up a frame for backtraces
	movl    %esp,%ebp
	pushl   %esi
	pushl   %edi
	movl    8(%ebp),%edi            // get dest ptr
	movl    12(%ebp),%esi           // get source ptr

Ljoin:                                  // here from bcopy() with esi and edi loaded
	movl    16(%ebp),%ecx           // get length
	movl    %edi,%edx
	subl    %esi,%edx               // (dest - source)
	cmpl    %ecx,%edx               // must move in reverse if (dest - source) < length
	jb      LReverseIsland
Lrejoin:                                // here from very-long-operand copies
	cmpl    $(kShort),%ecx          // long enough to bother with SSE?
	ja      LNotShort               // yes

// Handle short forward copies.  As the most common case, this is the fall-through path.
//      ecx = length (<= kShort)
//      esi = source ptr
//      edi = dest ptr

Lshort:
	movl    %ecx,%edx               // copy length
	shrl    $2,%ecx                 // get #doublewords
	jz      LLeftovers
2:                                      // loop copying doublewords
	movl    (%esi),%eax
	addl    $4,%esi
	movl    %eax,(%edi)
	addl    $4,%edi
	dec     %ecx
	jnz     2b
LLeftovers:                             // handle leftover bytes (0..3) in last word
	andl    $3,%edx                 // any leftover bytes?
	jz      5f
4:                                      // loop copying bytes
	movb    (%esi),%al
	inc     %esi
movb	%al,(%edi)
	inc     %edi
	dec     %edx
	jnz     4b
5:
	movl    8(%ebp),%eax            // get return value (dst ptr) for memcpy/memmove
	popl    %edi
	popl    %esi
	popl    %ebp
	ret


LReverseIsland:                         // keep the "jb" above a short branch...
	jmp     LReverse                // ...because reverse moves are uncommon


// Handle forward moves that are long enough to justify use of SSE3.
// First, 16-byte align the destination.
//      ecx = length (> kShort)
//      esi = source ptr
//      edi = dest ptr

LNotShort:
	cmpl    $(kVeryLong),%ecx       // long enough to justify heavyweight loops?
	movl    %edi,%edx               // copy destination
	jae     LVeryLong               // use very-long-operand path
	negl    %edx
	andl    $15,%edx                // get #bytes to align destination
	jz      LDestAligned            // already aligned
	subl    %edx,%ecx               // decrement length
1:                                      // loop copying 1..15 bytes
	movb    (%esi),%al
	inc     %esi
	movb    %al,(%edi)
	inc     %edi
	dec     %edx
	jnz     1b

// Destination is now aligned.  Prepare for forward loops over 64-byte chunks.
// Since kShort>=80 and we've moved at most 15 bytes already, there is at least one chunk.

LDestAligned:
	movl    %ecx,%edx               // copy length
	movl    %ecx,%eax               // twice
	andl    $63,%ecx                // get remaining bytes for Lshort
	andl    $-64,%edx               // get number of bytes we will copy in inner loop
	addl    %edx,%esi               // point to 1st byte not copied
	addl    %edx,%edi
	negl    %edx                    // now generate offset to 1st byte to be copied
	testl   $15,%esi                // is source aligned too?
	jnz     LUnalignedLoop          // no

	cmpl    $(kFastUCode),%eax      // long enough for the fastpath in microcode?
	jb  LAlignedLoop                // no, use SSE
	cld                             // we'll move forward
	movl    %eax,%ecx               // copy length again
	shrl    $2,%ecx                 // compute #words to move
	addl    %edx,%esi               // restore ptrs to 1st byte of source and dest
	addl    %edx,%edi
	rep                             // the u-code will optimize this
	movsl
	movl    %eax,%edx               // original length
	jmp     LLeftovers              // handle 0..3 leftover bytes


// Forward aligned loop for medium length operands (kShort < n < kVeryLong).

	.align  4,0x90              // 16-byte align inner loops
LAlignedLoop:                       // loop over 64-byte chunks
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

	jmp     Lshort              // copy remaining 0..15 bytes and done


// Forward unaligned loop for medium length operands (kShort < n < kVeryLong).
// Note that LDDQU==MOVDQU on these machines, ie we don't care when we cross
// source cache lines.

	.align  4,0x90              // 16-byte align inner loops
LUnalignedLoop:                     // loop over 64-byte chunks
	movdqu  (%esi,%edx),%xmm0   // the loads are unaligned
	movdqu  16(%esi,%edx),%xmm1
	movdqu  32(%esi,%edx),%xmm2
	movdqu  48(%esi,%edx),%xmm3

	movdqa  %xmm0,(%edi,%edx)   // we can use aligned stores
	movdqa  %xmm1,16(%edi,%edx)
	movdqa  %xmm2,32(%edi,%edx)
	movdqa  %xmm3,48(%edi,%edx)

	addl    $64,%edx
	jnz     LUnalignedLoop

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Very long forward moves.  These are at least several pages, so we loop over big
// chunks of memory (kBigChunk in size.)  We first prefetch the chunk, and then copy
// it using non-temporal stores.  Hopefully all the reads occur in the prefetch loop,
// so the copy loop reads from L2 and writes directly to memory (with write combining.)
// This minimizes bus turnaround and maintains good DRAM page locality.
// Note that for this scheme to work, kVeryLong must be a large fraction of L2 cache
// size.  Otherwise, it is counter-productive to bypass L2 on the stores.
//      ecx = length (>= kVeryLong bytes)
//      edi = dest (aligned)
//      esi = source

LVeryLong:
	pushl   %ebx            // we'll need to use this
	movl    %edi,%ebx       // copy dest ptr
	negl    %ebx
	andl    $63,%ebx        // get #bytes to cache line align destination
	jz      LBigChunkLoop   // already aligned

// Cache line align destination, so temporal stores in copy loops work right.

	pushl   %ecx            // save total length remaining
	pushl   %ebx            // arg3 - #bytes to align destination (1..63)
	pushl   %esi            // arg2 - source
	pushl   %edi            // arg1 - dest
	call    Lmemcpy_sse2    // align the destination
	movl    12(%esp),%ecx   // recover total length
	addl    $16,%esp
	addl    %ebx,%esi       // adjust ptrs and lengths past copy
	addl    %ebx,%edi
	subl    %ebx,%ecx

// Loop over big chunks.
//      ecx = length remaining (>= 4096)
//      edi = dest (64-byte aligned)
//      esi = source (may be unaligned)

LBigChunkLoop:
	movl    $(kBigChunk),%edx       // assume we can do a full chunk
	cmpl    %edx,%ecx               // do we have a full chunk left to do?
	cmovbl  %ecx,%edx               // if not, only move what we have left
	andl    $-4096,%edx             // we work in page multiples
	xor     %eax,%eax               // initialize chunk offset
	jmp     LTouchLoop

// Because the source may be unaligned, we use byte loads to touch.
//      ecx = length remaining (including this chunk)
//      edi = ptr to start of dest chunk
//      esi = ptr to start of source chunk
//      edx = chunk length (multiples of pages)
//      ebx = scratch reg used to read a byte of each cache line
//      eax = chunk offset

	.align  4,0x90                  // 16-byte align inner loops
LTouchLoop:
	movzb   (%esi,%eax),%ebx        // touch line 0, 2, 4, or 6 of page
	movzb   1*64(%esi,%eax),%ebx    // touch line 1, 3, 5, or 7
	movzb   8*64(%esi,%eax),%ebx    // touch line 8, 10, 12, or 14
	movzb   9*64(%esi,%eax),%ebx    // etc

	movzb   16*64(%esi,%eax),%ebx
	movzb   17*64(%esi,%eax),%ebx
	movzb   24*64(%esi,%eax),%ebx
	movzb   25*64(%esi,%eax),%ebx

	movzb   32*64(%esi,%eax),%ebx
	movzb   33*64(%esi,%eax),%ebx
	movzb   40*64(%esi,%eax),%ebx
	movzb   41*64(%esi,%eax),%ebx

	movzb   48*64(%esi,%eax),%ebx
	movzb   49*64(%esi,%eax),%ebx
	movzb   56*64(%esi,%eax),%ebx
	movzb   57*64(%esi,%eax),%ebx

	subl    $-128,%eax          // next slice of page (adding 128 w 8-bit immediate)
	testl   $512,%eax           // done with this page?
	jz      LTouchLoop          // no, next of four slices
	addl    $(4096-512),%eax    // move on to next page
	cmpl    %eax,%edx           // done with this chunk?
	jnz     LTouchLoop          // no, do next page

// The chunk has been pre-fetched, now copy it using non-temporal stores.
// There are two copy loops, depending on whether the source is 16-byte aligned
// or not.

	addl    %edx,%esi           // increment ptrs by chunk length
	addl    %edx,%edi
	subl    %edx,%ecx           // adjust remaining length
	negl    %edx                // prepare loop index (counts up to 0)
	testl   $15,%esi            // is source 16-byte aligned?
	jnz     LVeryLongUnaligned  // source is not aligned
	jmp     LVeryLongAligned

	.align  4,0x90              // 16-byte align inner loops
LVeryLongAligned:                   // aligned loop over 128-bytes
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

	subl    $-128,%edx      // add 128 with an 8-bit immediate
	jnz     LVeryLongAligned
	jmp     LVeryLongChunkEnd

	.align  4,0x90          // 16-byte align inner loops
LVeryLongUnaligned:             // unaligned loop over 128-bytes
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

	subl    $-128,%edx              // add 128 with an 8-bit immediate
	jnz     LVeryLongUnaligned

LVeryLongChunkEnd:
	cmpl    $4096,%ecx              // at least another page to go?
	jae     LBigChunkLoop           // yes

	sfence                          // required by non-temporal stores
	popl    %ebx
	jmp     Lrejoin                 // handle remaining (0..4095) bytes


// Reverse moves.
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
	movl    %ecx,%edx       // copy length
	shrl    $2,%ecx         // #words
	jz      3f
1:
	subl    $4,%esi
	movl    (%esi),%eax
	subl    $4,%edi
	movl    %eax,(%edi)
	dec     %ecx
	jnz     1b
3:
	andl    $3,%edx         // bytes?
	jz      5f
4:
	dec     %esi
	movb    (%esi),%al
	dec     %edi
	movb    %al,(%edi)
	dec     %edx
	jnz     4b
5:
	movl    8(%ebp),%eax    // get return value (dst ptr) for memcpy/memmove
	popl    %edi
	popl    %esi
	popl    %ebp
	ret

// Handle a reverse move long enough to justify using SSE.
//      ecx = length
//      esi = one byte past end of source
//      edi = one byte past end of dest

LReverseNotShort:
	movl    %edi,%edx               // copy destination
	andl    $15,%edx                // get #bytes to align destination
	je      LReverseDestAligned     // already aligned
	subl    %edx,%ecx               // adjust length
1:	                                // loop copying 1..15 bytes
	dec     %esi
	movb    (%esi),%al
	dec     %edi
	movb    %al,(%edi)
	dec     %edx
	jnz     1b

// Destination is now aligned.  Prepare for reverse loops.

LReverseDestAligned:
	movl    %ecx,%edx               // copy length
	andl    $63,%ecx                // get remaining bytes for Lshort
	andl    $-64,%edx               // get number of bytes we will copy in inner loop
	subl    %edx,%esi               // point to endpoint of copy
	subl    %edx,%edi
	testl   $15,%esi                // is source aligned too?
	jnz     LReverseUnalignedLoop   // no
	jmp     LReverseAlignedLoop     // use aligned loop

	.align  4,0x90                  // 16-byte align inner loops
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

	.align  4,0x90                  // 16-byte align inner loops
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

PLATFUNC_DESCRIPTOR(bcopy,sse2,kHasSSE2|kCache64,kHasSupplementalSSE3)
PLATFUNC_DESCRIPTOR(memcpy,sse2,kHasSSE2|kCache64,kHasSupplementalSSE3)
PLATFUNC_DESCRIPTOR(memmove,sse2,kHasSSE2|kCache64,kHasSupplementalSSE3)
