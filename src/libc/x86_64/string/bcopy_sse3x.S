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
#include "platfunc.h"

/*
 * The bcopy/memcpy loops, tuned for 64-bit Pentium-M class processors with 
 * Supplemental SSE3 and 64-byte cache lines.  This is the 64-bit version.
 *
 * The following #defines are tightly coupled to the u-architecture:
 */

#define kShort  80			// too short to bother with SSE (must be >=80)
#define kVeryLong   (500*1024)          // large enough for non-temporal stores (>=8192 and <2GB)
#define kFastUCode  ((16*1024)-15)	// cutoff for microcode fastpath for "rep/movsl"

// void bcopy(const void *src, void *dst, size_t len);

PLATFUNC_FUNCTION_START_GENERIC(bcopy, sse3x, 64, 5)
LZero:
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

PLATFUNC_FUNCTION_START_GENERIC(memcpy, sse3x, 64, 0)		// void *memcpy(void *dst, const void *src, size_t len)
PLATFUNC_FUNCTION_START_GENERIC(memmove, sse3x, 64, 0)	// void *memmove(void *dst, const void *src, size_t len)
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
	jz	LLeftovers
2:					// loop copying doublewords
	movl	(%rsi),%eax
	addq	$4,%rsi
	movl	%eax,(%rdi)
	addq	$4,%rdi
	decl	%ecx
	jnz	2b
LLeftovers:				// handle leftover bytes (0..3) in last word
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
	cmpq    $(kVeryLong),%rdx       // long enough to justify heavyweight loops?
	jae     LVeryLong		// use very-long-operand path
	movl    %edi,%ecx               // copy low half of destination ptr
	negl    %ecx
	andl    $15,%ecx                // get #bytes to align destination
	jz	LDestAligned		// already aligned
	subl    %ecx,%edx               // decrement length
	rep				// align destination
	movsb


// Destination is now aligned.  Dispatch to one of sixteen loops over 64-byte chunks,
// based on the alignment of the source.  All vector loads and stores are aligned.
// Even though this means we have to shift and repack vectors, doing so is much faster
// than unaligned loads.  Since kShort>=80 and we've moved at most 15 bytes already,
// there is at least one chunk.  When we enter the copy loops, the following registers
// are set up:
//      rdx = residual length (0..63)
//	rcx = -(length to move), a multiple of 64 less than 2GB
//      rsi = ptr to 1st source byte not to move (unaligned)
//      rdi = ptr to 1st dest byte not to move (aligned)

LDestAligned:
	movq    %rdx,%rcx               // copy length
	movl	%esi,%eax		// copy low half of source address
	andl    $63,%edx                // get remaining bytes for LShort
	andl	$15,%eax		// mask to low 4 bits of source address
	andq    $-64,%rcx               // get number of bytes we will copy in inner loop
	leaq	LTable(%rip), %r8
	addq    %rcx,%rsi               // point to 1st byte not copied
	addq    %rcx,%rdi
	movl	(%r8,%rax,4),%eax	// get offset of routine
	negq    %rcx                    // now generate offset to 1st byte to be copied
	addq	%r8,%rax		// generate address of copy loop
	jmp	*%rax			// enter copy loop, selected by source alignment

	.align	2
LTable:					// table of copy loop addresses
// force generation of assembly-time constants. Otherwise assembler
// creates subtractor relocations relative to first external symbol,
// and this file has none
	.set LMod0Offset, LMod0 - LTable
	.set LMod1Offset, LMod1 - LTable
	.set LMod2Offset, LMod2 - LTable
	.set LMod3Offset, LMod3 - LTable
	.set LMod4Offset, LMod4 - LTable
	.set LMod5Offset, LMod5 - LTable
	.set LMod6Offset, LMod6 - LTable
	.set LMod7Offset, LMod7 - LTable
	.set LMod8Offset, LMod8 - LTable
	.set LMod9Offset, LMod9 - LTable
	.set LMod10Offset, LMod10 - LTable
	.set LMod11Offset, LMod11 - LTable
	.set LMod12Offset, LMod12 - LTable
	.set LMod13Offset, LMod13 - LTable
	.set LMod14Offset, LMod14 - LTable
	.set LMod15Offset, LMod15 - LTable
	.long LMod0Offset
	.long LMod1Offset
	.long LMod2Offset
	.long LMod3Offset
	.long LMod4Offset
	.long LMod5Offset
	.long LMod6Offset
	.long LMod7Offset
	.long LMod8Offset
	.long LMod9Offset
	.long LMod10Offset
	.long LMod11Offset
	.long LMod12Offset
	.long LMod13Offset
	.long LMod14Offset
	.long LMod15Offset


// Very long forward moves.  These are at least several pages.  They are special cased
// and aggressively optimized, not so much because they are common or useful, but
// because they are subject to benchmark.  There isn't enough room for them in the
// area reserved on the platfunc for bcopy, so we put them elsewhere.  We call
// the longcopy routine using the normal ABI:
//      rdi = dest
//      rsi = source
//      rdx = length (>= kVeryLong bytes)

LVeryLong:
	pushq	%r11			// save return value
	call	_longcopy		// call very long operand routine
	popq	%rax			// pop return value
	popq	%rbp
	ret	


// On Pentium-M, the microcode for "rep/movsl" is faster than SSE for 16-byte
// aligned operands from about 32KB up to kVeryLong for the hot cache case, and from
// about 256 bytes up to kVeryLong for cold caches.  This is because the microcode
// avoids having to read destination cache lines that will be completely overwritten.
// The cutoff we use (ie, kFastUCode) must somehow balance the two cases, since
// we do not know if the destination is in cache or not.

Lfastpath:
	addq    %rcx,%rsi               // restore ptrs to 1st byte of source and dest
	addq    %rcx,%rdi
	negl	%ecx			// make length positive (known to be < 2GB)
	orl	%edx,%ecx		// restore total #bytes remaining to move
	cld				// we'll move forward
	shrl	$2,%ecx			// compute #words to move
	rep				// the u-code will optimize this
	movsl
	jmp	LLeftovers		// handle 0..3 leftover bytes


// Forward loop for medium length operands in which low four bits of %rsi == 0000

LMod0:
	cmpl	$(-kFastUCode),%ecx	// %rcx == -length, where (length < kVeryLong)
	jle	Lfastpath		// long enough for fastpath in microcode
	jmp	1f
	.align	4,0x90			// 16-byte align inner loops
1:					// loop over 64-byte chunks
	movdqa  (%rsi,%rcx),%xmm0
	movdqa  16(%rsi,%rcx),%xmm1
	movdqa  32(%rsi,%rcx),%xmm2
	movdqa  48(%rsi,%rcx),%xmm3

	movdqa  %xmm0,(%rdi,%rcx)
	movdqa  %xmm1,16(%rdi,%rcx)
	movdqa  %xmm2,32(%rdi,%rcx)
	movdqa  %xmm3,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     1b

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %rsi == 0001

LMod1:
	movdqa	-1(%rsi,%rcx),%xmm0	// prime the loop by loading 1st quadword
1:					// loop over 64-byte chunks
	movdqa  15(%rsi,%rcx),%xmm1
	movdqa  31(%rsi,%rcx),%xmm2
	movdqa  47(%rsi,%rcx),%xmm3
	movdqa  63(%rsi,%rcx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$1,%xmm3,%xmm4		// dest <- shr( dest || source, imm*8 )
	palignr	$1,%xmm2,%xmm3
	palignr	$1,%xmm1,%xmm2
	palignr	$1,%xmm5,%xmm1

	movdqa  %xmm1,(%rdi,%rcx)
	movdqa  %xmm2,16(%rdi,%rcx)
	movdqa  %xmm3,32(%rdi,%rcx)
	movdqa  %xmm4,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     1b

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %rsi == 0010

LMod2:
	movdqa	-2(%rsi,%rcx),%xmm0	// prime the loop by loading 1st source dq
1:					// loop over 64-byte chunks
	movdqa  14(%rsi,%rcx),%xmm1
	movdqa  30(%rsi,%rcx),%xmm2
	movdqa  46(%rsi,%rcx),%xmm3
	movdqa  62(%rsi,%rcx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$2,%xmm3,%xmm4		// dest <- shr( dest || source, imm*8 )
	palignr	$2,%xmm2,%xmm3
	palignr	$2,%xmm1,%xmm2
	palignr	$2,%xmm5,%xmm1

	movdqa  %xmm1,(%rdi,%rcx)
	movdqa  %xmm2,16(%rdi,%rcx)
	movdqa  %xmm3,32(%rdi,%rcx)
	movdqa  %xmm4,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     1b

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %rsi == 0011

LMod3:
	movdqa	-3(%rsi,%rcx),%xmm0	// prime the loop by loading 1st source dq
1:					// loop over 64-byte chunks
	movdqa  13(%rsi,%rcx),%xmm1
	movdqa  29(%rsi,%rcx),%xmm2
	movdqa  45(%rsi,%rcx),%xmm3
	movdqa  61(%rsi,%rcx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$3,%xmm3,%xmm4		// dest <- shr( dest || source, imm*8 )
	palignr	$3,%xmm2,%xmm3
	palignr	$3,%xmm1,%xmm2
	palignr	$3,%xmm5,%xmm1

	movdqa  %xmm1,(%rdi,%rcx)
	movdqa  %xmm2,16(%rdi,%rcx)
	movdqa  %xmm3,32(%rdi,%rcx)
	movdqa  %xmm4,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     1b

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %rsi == 0100
// We use the float single data type in order to use "movss" to merge vectors.

LMod4:
	movaps	-4(%rsi,%rcx),%xmm0	// 4-byte aligned: prime the loop
	jmp	1f
	.align	4,0x90
1:					// loop over 64-byte chunks
	movaps  12(%rsi,%rcx),%xmm1
	movaps  28(%rsi,%rcx),%xmm2
	movss	%xmm1,%xmm0		// copy low 4 bytes of source into destination
	pshufd	$(0x39),%xmm0,%xmm0	// rotate right 4 bytes (mask -- 00 11 10 01)
	movaps  44(%rsi,%rcx),%xmm3
	movss	%xmm2,%xmm1
	pshufd	$(0x39),%xmm1,%xmm1
	movaps	60(%rsi,%rcx),%xmm4
	movss	%xmm3,%xmm2
	pshufd	$(0x39),%xmm2,%xmm2

	movaps  %xmm0,(%rdi,%rcx)
	movss	%xmm4,%xmm3
	pshufd	$(0x39),%xmm3,%xmm3
	movaps  %xmm1,16(%rdi,%rcx)
	movaps  %xmm2,32(%rdi,%rcx)
	movaps	%xmm4,%xmm0
	movaps  %xmm3,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     1b

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %rsi == 0101

LMod5:
	movdqa	-5(%rsi,%rcx),%xmm0	// prime the loop by loading 1st source dq
1:					// loop over 64-byte chunks
	movdqa  11(%rsi,%rcx),%xmm1
	movdqa  27(%rsi,%rcx),%xmm2
	movdqa  43(%rsi,%rcx),%xmm3
	movdqa  59(%rsi,%rcx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$5,%xmm3,%xmm4		// dest <- shr( dest || source, imm*8 )
	palignr	$5,%xmm2,%xmm3
	palignr	$5,%xmm1,%xmm2
	palignr	$5,%xmm5,%xmm1

	movdqa  %xmm1,(%rdi,%rcx)
	movdqa  %xmm2,16(%rdi,%rcx)
	movdqa  %xmm3,32(%rdi,%rcx)
	movdqa  %xmm4,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     1b

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %rsi == 0110

LMod6:
	movdqa	-6(%rsi,%rcx),%xmm0	// prime the loop by loading 1st source dq
1:					// loop over 64-byte chunks
	movdqa  10(%rsi,%rcx),%xmm1
	movdqa  26(%rsi,%rcx),%xmm2
	movdqa  42(%rsi,%rcx),%xmm3
	movdqa  58(%rsi,%rcx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$6,%xmm3,%xmm4		// dest <- shr( dest || source, imm*8 )
	palignr	$6,%xmm2,%xmm3
	palignr	$6,%xmm1,%xmm2
	palignr	$6,%xmm5,%xmm1

	movdqa  %xmm1,(%rdi,%rcx)
	movdqa  %xmm2,16(%rdi,%rcx)
	movdqa  %xmm3,32(%rdi,%rcx)
	movdqa  %xmm4,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     1b

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %rsi == 0111

LMod7:
	movdqa	-7(%rsi,%rcx),%xmm0	// prime the loop by loading 1st source dq
1:					// loop over 64-byte chunks
	movdqa  9(%rsi,%rcx),%xmm1
	movdqa  25(%rsi,%rcx),%xmm2
	movdqa  41(%rsi,%rcx),%xmm3
	movdqa  57(%rsi,%rcx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$7,%xmm3,%xmm4		// dest <- shr( dest || source, imm*8 )
	palignr	$7,%xmm2,%xmm3
	palignr	$7,%xmm1,%xmm2
	palignr	$7,%xmm5,%xmm1

	movdqa  %xmm1,(%rdi,%rcx)
	movdqa  %xmm2,16(%rdi,%rcx)
	movdqa  %xmm3,32(%rdi,%rcx)
	movdqa  %xmm4,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     1b

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %rsi == 1000
// We use the float double data type in order to use "shufpd" to shift by 8 bytes.

LMod8:
	cmpl	$(-kFastUCode),%ecx	// %rcx == -length, where (length < kVeryLong)
	jle	Lfastpath		// long enough for fastpath in microcode
	movapd	-8(%rsi,%rcx),%xmm0	// 8-byte aligned: prime the loop
	jmp	1f
	.align	4,0x90
1:					// loop over 64-byte chunks
	movapd  8(%rsi,%rcx),%xmm1
	movapd  24(%rsi,%rcx),%xmm2
	shufpd	$01,%xmm1,%xmm0		// %xmm0 <- shr( %xmm0 || %xmm1, 8 bytes)
	movapd  40(%rsi,%rcx),%xmm3
	shufpd	$01,%xmm2,%xmm1
	movapd	56(%rsi,%rcx),%xmm4
	shufpd	$01,%xmm3,%xmm2

	movapd  %xmm0,(%rdi,%rcx)
	shufpd	$01,%xmm4,%xmm3
	movapd  %xmm1,16(%rdi,%rcx)
	movapd  %xmm2,32(%rdi,%rcx)
	movapd	%xmm4,%xmm0
	movapd  %xmm3,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     1b

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %rsi == 1001

LMod9:
	movdqa	-9(%rsi,%rcx),%xmm0	// prime the loop by loading 1st source dq
1:					// loop over 64-byte chunks
	movdqa  7(%rsi,%rcx),%xmm1
	movdqa  23(%rsi,%rcx),%xmm2
	movdqa  39(%rsi,%rcx),%xmm3
	movdqa  55(%rsi,%rcx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$9,%xmm3,%xmm4		// dest <- shr( dest || source, imm*8 )
	palignr	$9,%xmm2,%xmm3
	palignr	$9,%xmm1,%xmm2
	palignr	$9,%xmm5,%xmm1

	movdqa  %xmm1,(%rdi,%rcx)
	movdqa  %xmm2,16(%rdi,%rcx)
	movdqa  %xmm3,32(%rdi,%rcx)
	movdqa  %xmm4,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     1b

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %rsi == 1010

LMod10:
	movdqa	-10(%rsi,%rcx),%xmm0	// prime the loop by loading 1st source dq
1:					// loop over 64-byte chunks
	movdqa  6(%rsi,%rcx),%xmm1
	movdqa  22(%rsi,%rcx),%xmm2
	movdqa  38(%rsi,%rcx),%xmm3
	movdqa  54(%rsi,%rcx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$10,%xmm3,%xmm4		// dest <- shr( dest || source, imm*8 )
	palignr	$10,%xmm2,%xmm3
	palignr	$10,%xmm1,%xmm2
	palignr	$10,%xmm5,%xmm1

	movdqa  %xmm1,(%rdi,%rcx)
	movdqa  %xmm2,16(%rdi,%rcx)
	movdqa  %xmm3,32(%rdi,%rcx)
	movdqa  %xmm4,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     1b

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %rsi == 1011

LMod11:
	movdqa	-11(%rsi,%rcx),%xmm0	// prime the loop by loading 1st source dq
1:					// loop over 64-byte chunks
	movdqa  5(%rsi,%rcx),%xmm1
	movdqa  21(%rsi,%rcx),%xmm2
	movdqa  37(%rsi,%rcx),%xmm3
	movdqa  53(%rsi,%rcx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$11,%xmm3,%xmm4		// dest <- shr( dest || source, imm*8 )
	palignr	$11,%xmm2,%xmm3
	palignr	$11,%xmm1,%xmm2
	palignr	$11,%xmm5,%xmm1

	movdqa  %xmm1,(%rdi,%rcx)
	movdqa  %xmm2,16(%rdi,%rcx)
	movdqa  %xmm3,32(%rdi,%rcx)
	movdqa  %xmm4,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     1b

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %rsi == 1100
// We use the float single data type in order to use "movss" to merge vectors.

LMod12:
	movss	(%rsi,%rcx),%xmm0	// prefetch 1st four bytes of source, right justified
	jmp	1f
	.align	4,0x90
1:					// loop over 64-byte chunks
	pshufd	$(0x93),4(%rsi,%rcx),%xmm1 // load and rotate right 12 bytes (mask -- 10 01 00 11)
	pshufd	$(0x93),20(%rsi,%rcx),%xmm2
	pshufd	$(0x93),36(%rsi,%rcx),%xmm3
	pshufd	$(0x93),52(%rsi,%rcx),%xmm4

	movaps	%xmm4,%xmm5
	movss	%xmm3,%xmm4		// copy low 4 bytes of source into destination
	movss	%xmm2,%xmm3
	movss	%xmm1,%xmm2
	movss	%xmm0,%xmm1

	movaps  %xmm1,(%rdi,%rcx)
	movaps  %xmm2,16(%rdi,%rcx)
	movaps	%xmm5,%xmm0
	movaps  %xmm3,32(%rdi,%rcx)
	movaps  %xmm4,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     1b

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %rsi == 1101

LMod13:
	movdqa	-13(%rsi,%rcx),%xmm0	// prime the loop by loading 1st source dq
1:					// loop over 64-byte chunks
	movdqa  3(%rsi,%rcx),%xmm1
	movdqa  19(%rsi,%rcx),%xmm2
	movdqa  35(%rsi,%rcx),%xmm3
	movdqa  51(%rsi,%rcx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$13,%xmm3,%xmm4		// dest <- shr( dest || source, imm*8 )
	palignr	$13,%xmm2,%xmm3
	palignr	$13,%xmm1,%xmm2
	palignr	$13,%xmm5,%xmm1

	movdqa  %xmm1,(%rdi,%rcx)
	movdqa  %xmm2,16(%rdi,%rcx)
	movdqa  %xmm3,32(%rdi,%rcx)
	movdqa  %xmm4,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     1b

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %rsi == 1110

LMod14:
	movdqa	-14(%rsi,%rcx),%xmm0	// prime the loop by loading 1st source dq
1:					// loop over 64-byte chunks
	movdqa  2(%rsi,%rcx),%xmm1
	movdqa  18(%rsi,%rcx),%xmm2
	movdqa  34(%rsi,%rcx),%xmm3
	movdqa  50(%rsi,%rcx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$14,%xmm3,%xmm4		// dest <- shr( dest || source, imm*8 )
	palignr	$14,%xmm2,%xmm3
	palignr	$14,%xmm1,%xmm2
	palignr	$14,%xmm5,%xmm1

	movdqa  %xmm1,(%rdi,%rcx)
	movdqa  %xmm2,16(%rdi,%rcx)
	movdqa  %xmm3,32(%rdi,%rcx)
	movdqa  %xmm4,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     1b

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Forward loop for medium length operands in which low four bits of %rsi == 1111

LMod15:
	movdqa	-15(%rsi,%rcx),%xmm0	// prime the loop by loading 1st source dq
1:					// loop over 64-byte chunks
	movdqa  1(%rsi,%rcx),%xmm1
	movdqa  17(%rsi,%rcx),%xmm2
	movdqa  33(%rsi,%rcx),%xmm3
	movdqa  49(%rsi,%rcx),%xmm4

	movdqa	%xmm0,%xmm5
	movdqa	%xmm4,%xmm0

	palignr	$15,%xmm3,%xmm4		// dest <- shr( dest || source, imm*8 )
	palignr	$15,%xmm2,%xmm3
	palignr	$15,%xmm1,%xmm2
	palignr	$15,%xmm5,%xmm1

	movdqa  %xmm1,(%rdi,%rcx)
	movdqa  %xmm2,16(%rdi,%rcx)
	movdqa  %xmm3,32(%rdi,%rcx)
	movdqa  %xmm4,48(%rdi,%rcx)

	addq    $64,%rcx
	jnz     1b

	jmp     LShort                  // copy remaining 0..63 bytes and done


// Reverse moves.  These are not optimized as aggressively as their forward
// counterparts, as they are only used with destructive overlap.
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
	je      LReverseDestAligned     // already aligned
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

PLATFUNC_DESCRIPTOR(bcopy,sse3x,kHasSSE2|kHasSupplementalSSE3|kCache64,kHasSSE4_2)
PLATFUNC_DESCRIPTOR(memcpy,sse3x,kHasSSE2|kHasSupplementalSSE3|kCache64,kHasSSE4_2)
PLATFUNC_DESCRIPTOR(memmove,sse3x,kHasSSE2|kHasSupplementalSSE3|kCache64,kHasSSE4_2)
