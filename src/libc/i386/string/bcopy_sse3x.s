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
 * The bcopy/memcpy loops, tuned for Pentium-M class processors with
 * Supplemental SSE3 and 64-byte cache lines.
 *
 * The following #defines are tightly coupled to the u-architecture:
 */

#define kShort  80                      // too short to bother with SSE (must be >=80)
#define kVeryLong   (500*1024)          // large enough for non-temporal stores (must be >= 8192)
#define kFastUCode  ((16*1024)-15)      // cutoff for microcode fastpath for "rep/movsl"

// void bcopy(const void *src, void *dst, size_t len);

PLATFUNC_FUNCTION_START(bcopy, sse3x, 32, 5)
	pushl   %ebp                    // set up a frame for backtraces
	movl    %esp,%ebp
	pushl   %esi
	pushl   %edi
	pushl   %ebx
	movl    8(%ebp),%esi            // get source ptr
	movl    12(%ebp),%edi           // get dest ptr
	movl    16(%ebp),%ecx           // get length
	movl    %edi,%edx
	subl    %esi,%edx               // (dest - source)
	cmpl    %ecx,%edx               // must move in reverse if (dest - source) < length
	jb      LReverseIsland
	cmpl    $(kShort),%ecx          // long enough to bother with SSE?
	jbe     Lshort                  // no
	jmp     LNotShort

//
// void *memcpy(void *dst, const void *src, size_t len);
// void *memmove(void *dst, const void *src, size_t len);
//

PLATFUNC_FUNCTION_START(memcpy, sse3x, 32, 0)   // void *memcpy(void *dst, const void *src, size_t len)
PLATFUNC_FUNCTION_START(memmove, sse3x, 32, 0)  // void *memmove(void *dst, const void *src, size_t len)
	pushl   %ebp                            // set up a frame for backtraces
	movl    %esp,%ebp
	pushl   %esi
	pushl   %edi
	pushl   %ebx
	movl    8(%ebp),%edi            // get dest ptr
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
	jz      Lexit
4:                                      // loop copying bytes
	movb    (%esi),%al
	inc     %esi
	movb    %al,(%edi)
	inc     %edi
	dec     %edx
	jnz     4b
Lexit:
	movl    8(%ebp),%eax            // get return value (dst ptr) for memcpy/memmove
	popl    %ebx
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

// Destination is now aligned.  Dispatch to one of sixteen loops over 64-byte chunks,
// based on the alignment of the source.  All vector loads and stores are aligned.
// Even though this means we have to shift and repack vectors, doing so is much faster
// than unaligned loads.  Since kShort>=80 and we've moved at most 15 bytes already,
// there is at least one chunk.  When we enter the copy loops, the following registers
// are set up:
//      ecx = residual length (0..63)
//      edx = -(length to move), a multiple of 64
//      esi = ptr to 1st source byte not to move (unaligned)
//      edi = ptr to 1st dest byte not to move (aligned)

LDestAligned:
	movl    %ecx,%edx               // copy length
	movl    %esi,%eax               // copy source address
	andl    $63,%ecx                // get remaining bytes for Lshort
	andl    $-64,%edx               // get number of bytes we will copy in inner loop
	andl    $15,%eax                // mask to low 4 bits of source address
	addl    %edx,%esi               // point to 1st byte not copied
	addl    %edx,%edi
	negl    %edx                    // now generate offset to 1st byte to be copied
	call	1f
1:
	popl    %ebx
	movl    (LTable-1b)(%ebx,%eax,4), %eax // load jump table entry address, relative to LZero
	leal    (LTable-1b)(%ebx,%eax,1), %eax
	jmp     *%eax

	.align  2
LTable:                                 // table of copy loop addresses
	.long LMod0 -LTable
	.long LMod1 -LTable
	.long LMod2 -LTable
	.long LMod3 -LTable
	.long LMod4 -LTable
	.long LMod5 -LTable
	.long LMod6 -LTable
	.long LMod7 -LTable
	.long LMod8 -LTable
	.long LMod9 -LTable
	.long LMod10 -LTable
	.long LMod11 -LTable
	.long LMod12 -LTable
	.long LMod13 -LTable
	.long LMod14 -LTable
	.long LMod15 -LTable


// Very long forward moves.  These are at least several pages.  They are special cased
// and aggressively optimized, not so much because they are common or useful, but
// because they are subject to benchmark.  There isn't enough room for them in the
// area reserved on the platfunc for bcopy, so we put them elsewhere.  We call
// the longcopy routine using the normal ABI.

LVeryLong:
	pushl   %ecx                    // length (>= kVeryLong)
	pushl   %esi                    // source ptr
	pushl   %edi                    // dest ptr
	call    _longcopy
	addl    $12,%esp                // pop off our parameters
	jmp     Lexit


// On Pentium-M, the microcode for "rep/movsl" is faster than SSE for 8-byte
// aligned operands from about 32KB up to kVeryLong for the hot cache case, and from
// about 256 bytes up to kVeryLong for cold caches.  This is because the microcode
// avoids having to read destination cache lines that will be completely overwritten.
// The cutoff we use (ie, kFastUCode) must somehow balance the two cases, since
// we do not know if the destination is in cache or not.

Lfastpath:
	addl    %edx,%esi               // restore ptrs to 1st byte of source and dest
	addl    %edx,%edi
	negl    %edx                    // make length positive
	orl     %edx,%ecx               // restore total #bytes remaining to move
	cld                             // we'll move forward
	movl    %ecx,%edx               // copy total length to move
	shrl    $2,%ecx                 // compute #words to move
	rep                             // the u-code will optimize this
	movsl
	jmp     LLeftovers              // handle 0..3 leftover bytes


// Forward loop for medium length operands in which low four bits of %esi == 0000

LMod0:
	cmpl    $(-kFastUCode),%edx     // %edx == -length, where (length < kVeryLong)
	jle     Lfastpath               // long enough for fastpath in microcode
	jmp     1f
	.align  4,0x90                  // 16-byte align inner loops
1:                                      // loop over 64-byte chunks
	movdqa  (%esi,%edx),%xmm0
	movdqa  16(%esi,%edx),%xmm1
	movdqa  32(%esi,%edx),%xmm2
	movdqa  48(%esi,%edx),%xmm3

	movdqa  %xmm0,(%edi,%edx)
	movdqa  %xmm1,16(%edi,%edx)
	movdqa  %xmm2,32(%edi,%edx)
	movdqa  %xmm3,48(%edi,%edx)

	addl    $64,%edx
	jnz     1b

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %esi == 0001

LMod1:
	movdqa  -1(%esi,%edx),%xmm0     // prime the loop by loading 1st quadword
1:                                      // loop over 64-byte chunks
	movdqa  15(%esi,%edx),%xmm1
	movdqa  31(%esi,%edx),%xmm2
	movdqa  47(%esi,%edx),%xmm3
	movdqa  63(%esi,%edx),%xmm4

	movdqa  %xmm0,%xmm5
	movdqa  %xmm4,%xmm0

	palignr $1,%xmm3,%xmm4          // dest <- shr( dest || source, imm*8 )
	palignr $1,%xmm2,%xmm3
	palignr $1,%xmm1,%xmm2
	palignr $1,%xmm5,%xmm1

	movdqa  %xmm1,(%edi,%edx)
	movdqa  %xmm2,16(%edi,%edx)
	movdqa  %xmm3,32(%edi,%edx)
	movdqa  %xmm4,48(%edi,%edx)

	addl    $64,%edx
	jnz     1b

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %esi == 0010

LMod2:
	movdqa  -2(%esi,%edx),%xmm0     // prime the loop by loading 1st source dq
1:                                      // loop over 64-byte chunks
	movdqa  14(%esi,%edx),%xmm1
	movdqa  30(%esi,%edx),%xmm2
	movdqa  46(%esi,%edx),%xmm3
	movdqa  62(%esi,%edx),%xmm4

	movdqa  %xmm0,%xmm5
	movdqa  %xmm4,%xmm0

	palignr $2,%xmm3,%xmm4          // dest <- shr( dest || source, imm*8 )
	palignr $2,%xmm2,%xmm3
	palignr $2,%xmm1,%xmm2
	palignr $2,%xmm5,%xmm1

	movdqa  %xmm1,(%edi,%edx)
	movdqa  %xmm2,16(%edi,%edx)
	movdqa  %xmm3,32(%edi,%edx)
	movdqa  %xmm4,48(%edi,%edx)

	addl    $64,%edx
	jnz     1b

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %esi == 0011

LMod3:
	movdqa  -3(%esi,%edx),%xmm0     // prime the loop by loading 1st source dq
1:                                      // loop over 64-byte chunks
	movdqa  13(%esi,%edx),%xmm1
	movdqa  29(%esi,%edx),%xmm2
	movdqa  45(%esi,%edx),%xmm3
	movdqa  61(%esi,%edx),%xmm4

	movdqa  %xmm0,%xmm5
	movdqa  %xmm4,%xmm0

	palignr $3,%xmm3,%xmm4          // dest <- shr( dest || source, imm*8 )
	palignr $3,%xmm2,%xmm3
	palignr $3,%xmm1,%xmm2
	palignr $3,%xmm5,%xmm1

	movdqa  %xmm1,(%edi,%edx)
	movdqa  %xmm2,16(%edi,%edx)
	movdqa  %xmm3,32(%edi,%edx)
	movdqa  %xmm4,48(%edi,%edx)

	addl    $64,%edx
	jnz     1b

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %esi == 0100
// We use the float single data type in order to use "movss" to merge vectors.

LMod4:
	movaps  -4(%esi,%edx),%xmm0     // 4-byte aligned: prime the loop
	jmp     1f
	.align  4,0x90
1:                                      // loop over 64-byte chunks
	movaps  12(%esi,%edx),%xmm1
	movaps  28(%esi,%edx),%xmm2
	movss	%xmm1,%xmm0             // copy low 4 bytes of source into destination
	pshufd	$(0x39),%xmm0,%xmm0     // rotate right 4 bytes (mask -- 00 11 10 01)
	movaps  44(%esi,%edx),%xmm3
	movss	%xmm2,%xmm1
	pshufd	$(0x39),%xmm1,%xmm1
	movaps	60(%esi,%edx),%xmm4
	movss	%xmm3,%xmm2
	pshufd	$(0x39),%xmm2,%xmm2

	movaps  %xmm0,(%edi,%edx)
	movss	%xmm4,%xmm3
	pshufd	$(0x39),%xmm3,%xmm3
	movaps  %xmm1,16(%edi,%edx)
	movaps  %xmm2,32(%edi,%edx)
	movaps	%xmm4,%xmm0
	movaps  %xmm3,48(%edi,%edx)

	addl    $64,%edx
	jnz     1b

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %esi == 0101

LMod5:
	movdqa	-5(%esi,%edx),%xmm0// prime the loop by loading 1st source dq
1:				// loop over 64-byte chunks
	movdqa  11(%esi,%edx),%xmm1
	movdqa  27(%esi,%edx),%xmm2
	movdqa  43(%esi,%edx),%xmm3
	movdqa  59(%esi,%edx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$5,%xmm3,%xmm4	// dest <- shr( dest || source, imm*8 )
	palignr	$5,%xmm2,%xmm3
	palignr	$5,%xmm1,%xmm2
	palignr	$5,%xmm5,%xmm1

	movdqa  %xmm1,(%edi,%edx)
	movdqa  %xmm2,16(%edi,%edx)
	movdqa  %xmm3,32(%edi,%edx)
	movdqa  %xmm4,48(%edi,%edx)

	addl    $64,%edx
	jnz     1b

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %esi == 0110

LMod6:
	movdqa	-6(%esi,%edx),%xmm0// prime the loop by loading 1st source dq
1:				// loop over 64-byte chunks
	movdqa  10(%esi,%edx),%xmm1
	movdqa  26(%esi,%edx),%xmm2
	movdqa  42(%esi,%edx),%xmm3
	movdqa  58(%esi,%edx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$6,%xmm3,%xmm4	// dest <- shr( dest || source, imm*8 )
	palignr	$6,%xmm2,%xmm3
	palignr	$6,%xmm1,%xmm2
	palignr	$6,%xmm5,%xmm1

	movdqa  %xmm1,(%edi,%edx)
	movdqa  %xmm2,16(%edi,%edx)
	movdqa  %xmm3,32(%edi,%edx)
	movdqa  %xmm4,48(%edi,%edx)

	addl    $64,%edx
	jnz     1b

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %esi == 0111

LMod7:
	movdqa	-7(%esi,%edx),%xmm0// prime the loop by loading 1st source dq
1:				// loop over 64-byte chunks
	movdqa  9(%esi,%edx),%xmm1
	movdqa  25(%esi,%edx),%xmm2
	movdqa  41(%esi,%edx),%xmm3
	movdqa  57(%esi,%edx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$7,%xmm3,%xmm4	// dest <- shr( dest || source, imm*8 )
	palignr	$7,%xmm2,%xmm3
	palignr	$7,%xmm1,%xmm2
	palignr	$7,%xmm5,%xmm1

	movdqa  %xmm1,(%edi,%edx)
	movdqa  %xmm2,16(%edi,%edx)
	movdqa  %xmm3,32(%edi,%edx)
	movdqa  %xmm4,48(%edi,%edx)

	addl    $64,%edx
	jnz     1b

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %esi == 1000
// We use the float double data type in order to use "shufpd" to shift by 8 bytes.

LMod8:
	cmpl	$(-kFastUCode),%edx// %edx == -length, where (length < kVeryLong)
	jle	Lfastpath	// long enough for fastpath in microcode
	movapd	-8(%esi,%edx),%xmm0// 8-byte aligned: prime the loop
	jmp	1f
	.align	4,0x90
1:				// loop over 64-byte chunks
	movapd  8(%esi,%edx),%xmm1
	movapd  24(%esi,%edx),%xmm2
	shufpd	$01,%xmm1,%xmm0	// %xmm0 <- shr( %xmm0 || %xmm1, 8 bytes)
	movapd  40(%esi,%edx),%xmm3
	shufpd	$01,%xmm2,%xmm1
	movapd	56(%esi,%edx),%xmm4
	shufpd	$01,%xmm3,%xmm2

	movapd  %xmm0,(%edi,%edx)
	shufpd	$01,%xmm4,%xmm3
	movapd  %xmm1,16(%edi,%edx)
	movapd  %xmm2,32(%edi,%edx)
	movapd	%xmm4,%xmm0
	movapd  %xmm3,48(%edi,%edx)

	addl    $64,%edx
	jnz     1b

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %esi == 1001

LMod9:
	movdqa	-9(%esi,%edx),%xmm0// prime the loop by loading 1st source dq
1:				// loop over 64-byte chunks
	movdqa  7(%esi,%edx),%xmm1
	movdqa  23(%esi,%edx),%xmm2
	movdqa  39(%esi,%edx),%xmm3
	movdqa  55(%esi,%edx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$9,%xmm3,%xmm4	// dest <- shr( dest || source, imm*8 )
	palignr	$9,%xmm2,%xmm3
	palignr	$9,%xmm1,%xmm2
	palignr	$9,%xmm5,%xmm1

	movdqa  %xmm1,(%edi,%edx)
	movdqa  %xmm2,16(%edi,%edx)
	movdqa  %xmm3,32(%edi,%edx)
	movdqa  %xmm4,48(%edi,%edx)

	addl    $64,%edx
	jnz     1b

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %esi == 1010

LMod10:
	movdqa	-10(%esi,%edx),%xmm0// prime the loop by loading 1st source dq
1:				// loop over 64-byte chunks
	movdqa  6(%esi,%edx),%xmm1
	movdqa  22(%esi,%edx),%xmm2
	movdqa  38(%esi,%edx),%xmm3
	movdqa  54(%esi,%edx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$10,%xmm3,%xmm4	// dest <- shr( dest || source, imm*8 )
	palignr	$10,%xmm2,%xmm3
	palignr	$10,%xmm1,%xmm2
	palignr	$10,%xmm5,%xmm1

	movdqa  %xmm1,(%edi,%edx)
	movdqa  %xmm2,16(%edi,%edx)
	movdqa  %xmm3,32(%edi,%edx)
	movdqa  %xmm4,48(%edi,%edx)

	addl    $64,%edx
	jnz     1b

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %esi == 1011

LMod11:
	movdqa	-11(%esi,%edx),%xmm0// prime the loop by loading 1st source dq
1:				// loop over 64-byte chunks
	movdqa  5(%esi,%edx),%xmm1
	movdqa  21(%esi,%edx),%xmm2
	movdqa  37(%esi,%edx),%xmm3
	movdqa  53(%esi,%edx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$11,%xmm3,%xmm4	// dest <- shr( dest || source, imm*8 )
	palignr	$11,%xmm2,%xmm3
	palignr	$11,%xmm1,%xmm2
	palignr	$11,%xmm5,%xmm1

	movdqa  %xmm1,(%edi,%edx)
	movdqa  %xmm2,16(%edi,%edx)
	movdqa  %xmm3,32(%edi,%edx)
	movdqa  %xmm4,48(%edi,%edx)

	addl    $64,%edx
	jnz     1b

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %esi == 1100
// We use the float single data type in order to use "movss" to merge vectors.

LMod12:
	movss	(%esi,%edx),%xmm0// prefetch 1st four bytes of source, right justified
	jmp	1f
	.align	4,0x90
1:				// loop over 64-byte chunks
	pshufd	$(0x93),4(%esi,%edx),%xmm1 // load and rotate right 12 bytes (mask -- 10 01 00 11)
	pshufd	$(0x93),20(%esi,%edx),%xmm2
	pshufd	$(0x93),36(%esi,%edx),%xmm3
	pshufd	$(0x93),52(%esi,%edx),%xmm4

	movaps	%xmm4,%xmm5
	movss	%xmm3,%xmm4	// copy low 4 bytes of source into destination
	movss	%xmm2,%xmm3
	movss	%xmm1,%xmm2
	movss	%xmm0,%xmm1

	movaps  %xmm1,(%edi,%edx)
	movaps  %xmm2,16(%edi,%edx)
	movaps	%xmm5,%xmm0
	movaps  %xmm3,32(%edi,%edx)
	movaps  %xmm4,48(%edi,%edx)

	addl    $64,%edx
	jnz     1b

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %esi == 1101

LMod13:
	movdqa	-13(%esi,%edx),%xmm0// prime the loop by loading 1st source dq
1:				// loop over 64-byte chunks
	movdqa  3(%esi,%edx),%xmm1
	movdqa  19(%esi,%edx),%xmm2
	movdqa  35(%esi,%edx),%xmm3
	movdqa  51(%esi,%edx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$13,%xmm3,%xmm4	// dest <- shr( dest || source, imm*8 )
	palignr	$13,%xmm2,%xmm3
	palignr	$13,%xmm1,%xmm2
	palignr	$13,%xmm5,%xmm1

	movdqa  %xmm1,(%edi,%edx)
	movdqa  %xmm2,16(%edi,%edx)
	movdqa  %xmm3,32(%edi,%edx)
	movdqa  %xmm4,48(%edi,%edx)

	addl    $64,%edx
	jnz     1b

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %esi == 1110

LMod14:
	movdqa	-14(%esi,%edx),%xmm0// prime the loop by loading 1st source dq
1:				// loop over 64-byte chunks
	movdqa  2(%esi,%edx),%xmm1
	movdqa  18(%esi,%edx),%xmm2
	movdqa  34(%esi,%edx),%xmm3
	movdqa  50(%esi,%edx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$14,%xmm3,%xmm4	// dest <- shr( dest || source, imm*8 )
	palignr	$14,%xmm2,%xmm3
	palignr	$14,%xmm1,%xmm2
	palignr	$14,%xmm5,%xmm1

	movdqa  %xmm1,(%edi,%edx)
	movdqa  %xmm2,16(%edi,%edx)
	movdqa  %xmm3,32(%edi,%edx)
	movdqa  %xmm4,48(%edi,%edx)

	addl    $64,%edx
	jnz     1b

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %esi == 1111

LMod15:
	movdqa	-15(%esi,%edx),%xmm0// prime the loop by loading 1st source dq
1:				// loop over 64-byte chunks
	movdqa  1(%esi,%edx),%xmm1
	movdqa  17(%esi,%edx),%xmm2
	movdqa  33(%esi,%edx),%xmm3
	movdqa  49(%esi,%edx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$15,%xmm3,%xmm4	// dest <- shr( dest || source, imm*8 )
	palignr	$15,%xmm2,%xmm3
	palignr	$15,%xmm1,%xmm2
	palignr	$15,%xmm5,%xmm1

	movdqa  %xmm1,(%edi,%edx)
	movdqa  %xmm2,16(%edi,%edx)
	movdqa  %xmm3,32(%edi,%edx)
	movdqa  %xmm4,48(%edi,%edx)

	addl    $64,%edx
	jnz     1b

	jmp     Lshort                  // copy remaining 0..63 bytes and done


// Reverse moves.  These are not optimized as aggressively as their forward
// counterparts, as they are only used with destructive overlap.
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
	movl    %ecx,%edx	// copy length
	shrl	$2,%ecx		// #words
	jz	3f
1:
	subl	$4,%esi
	movl	(%esi),%eax
	subl	$4,%edi
	movl	%eax,(%edi)
	dec	%ecx
	jnz	1b
3:
	andl	$3,%edx		// bytes?
	jz	5f
4:
	dec	%esi
	movb	(%esi),%al
	dec	%edi
	movb	%al,(%edi)
	dec	%edx
	jnz	4b
5:
	movl    8(%ebp),%eax	// get return value (dst ptr) for memcpy/memmove
	popl    %ebx
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
	subl	%edx,%ecx	// adjust length
1:				// loop copying 1..15 bytes
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
	testl	$15,%esi	// is source aligned too?
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

PLATFUNC_DESCRIPTOR(bcopy,sse3x,kHasSSE2|kHasSupplementalSSE3|kCache64,kHasSSE4_2)
PLATFUNC_DESCRIPTOR(memcpy,sse3x,kHasSSE2|kHasSupplementalSSE3|kCache64,kHasSSE4_2)
PLATFUNC_DESCRIPTOR(memmove,sse3x,kHasSSE2|kHasSupplementalSSE3|kCache64,kHasSSE4_2)
