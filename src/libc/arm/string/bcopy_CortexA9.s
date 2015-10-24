/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
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
 *
 *  This file implements the following functions for the Cortex-A9 processor:
 *
 *  void bcopy(const void * source,
 *             void * destination,
 *             size_t length);
 *
 *  void *memmove(void * destination,
 *                const void * source,
 *                size_t n);
 *
 *  void *memcpy(void * restrict destination,
 *               const void * restrict source,
 *               size_t n);
 *
 * All copy n successive bytes from source to destination.  Memmove and memcpy
 * return destination, whereas bcopy has no return value.  Copying takes place
 * as if it were through a temporary buffer -- after return destination
 * contains exactly the bytes from source, even if the buffers overlap (this is
 * not required of memcpy by the C standard; its behavior is undefined if the
 * buffers overlap, but we are holding ourselves to the historical behavior of
 * this function on OS X and iOS).
 */

#include <arm/arch.h>
#if defined _ARM_ARCH_7 && !defined VARIANT_DYLD

/*****************************************************************************
 * Macros                                                                    *
 *****************************************************************************/

#define A9_ENTRY(name) \
	.align 2;\
	.globl _ ## name ## $VARIANT$CortexA9;\
	_ ## name ## $VARIANT$CortexA9:

#define ESTABLISH_FRAME \
	push   {r0,r4,r7,lr};\
	add     r7,     sp, #8
    
#define CLEAR_FRAME_AND_RETURN \
	pop    {r0,r4,r7,pc}
    
#define ADDITIONAL_CALLEE_SAVE_REGISTERS {r5,r6,r8,r10}

#define COPY_REGISTERS {r3,r4,r5,r6,r8,r9,r10,r12}

/*****************************************************************************
 *  entry points                                                             *
 *****************************************************************************/

.text
.syntax unified
.code 32

A9_ENTRY(bcopy)
//  Translate bcopy calls into memcpy calls by swapping the first and second
//  arguments.
	mov     r3,     r0
	mov     r0,     r1
	mov     r1,     r3

A9_ENTRY(memcpy)
A9_ENTRY(memmove)
//  Our preference is to copy the data in ascending address order, but if the
//  buffers overlap such that the beginning of the destination buffer aliases
//  the end of the source buffer, we need to copy in descending address order
//  instead to preserve the memmove semantics.  We detect this case with the
//  test:
//
//      destination - source < length    (unsigned compare)
//
//  If the address of the source buffer is higher than the address of the
//  destination buffer, this arithmetic can overflow, but the overflowed value
//  can only be smaller than length if the buffers do not overlap, so we don't
//  need to worry about false positives due to the overflow (they happen, but
//  only in cases where copying in either order is correct).
	subs    r3,     r0, r1
	bxeq    lr
	ESTABLISH_FRAME
	cmp     r3,     r2
	blo     L_descendingCopy

/*****************************************************************************
 *  ascending copy                                                           *
 *****************************************************************************/

//  The layout of the two buffers is such that we can use our preferred
//  (ascending address order) copy implementation.  Throughout this copy,
//  registers are used as follows:
//
//      r0  lowest unwritten address in the destination buffer.
//      r1  lowest unread address in the source buffer.
//      r2  number of bytes remaining to copy less an offset that varies
//          with the size of the copies that are being made.
//      r3, r4, r5, r6, r8, r9, r10, r12
//          temporary registers used to hold the data during copies.
//      r12 also used as a scratch register for alignment / length calculations

L_ascendingCopy:
//  We begin by checking if less than four bytes are to be copied; if so, we
//  branch directly to a small-buffer copy and return.  Otherwise, we copy up
//  to three bytes if needed to make the destination pointer have word (four
//  byte) alignment.
	subs    r2,         #4
	blo     L_ascendingLengthLessThanFour
	ands    ip,     r0, #0x3
	beq     L_ascendingDestinationWordAligned
	ldrb    r3,    [r1],#1
	cmp     ip,         #2
	ldrbls  r4,    [r1],#1
	strb    r3,    [r0],#1
	ldrblo  r3,    [r1],#1
	add     r2,         ip
	strbls  r4,    [r0],#1
	strblo  r3,    [r0],#1
	subs    r2,         #4
	bhs     L_ascendingDestinationWordAligned
    
L_ascendingLengthLessThanFour:
//  Conditionally copies up to three bytes, assuming no alignment.  This is
//  only used if the original length of the buffer is smaller than four.
	lsls    ip,     r2, #31
	ldrbcs  r3,    [r1],#1
	ldrbcs  ip,    [r1],#1
	ldrbmi  r4,    [r1]
	strbcs  r3,    [r0],#1
	strbcs  ip,    [r0],#1
	strbmi  r4,    [r0]
	CLEAR_FRAME_AND_RETURN
    
L_ascendingDestinationWordAligned:
//  We know that the destination has word alignment.  If the source is not
//  similarly aligned, jump to an unaligned copy loop.
	tst     r1,         #0x3
	bne		L_ascendingUnalignedCopy

/*****************************************************************************
 *  ascending copy, both buffers have word alignment                         *
 *****************************************************************************/
    
//  If less than sixty-four bytes remain to be copied, jump directly to the
//  word-aligned cleanup path.  Otherwise, we copy up to 28 bytes as needed
//  to make the destination pointer have cacheline alignment.
	subs    r2,     r2, #0x3c
	blo     L_ascendingLengthLessThanSixtyFour
0:  tst     r0,         #0x1c
	beq     L_ascendingDestinationCachelineAligned
	ldr     r3,    [r1],#4
	subs    r2,         #4
	str     r3,    [r0],#4
	bhs     0b
	b       L_ascendingLengthLessThanSixtyFour

L_ascendingDestinationCachelineAligned:
//  Unrolled main copy loop; copies two cachelines (64 bytes) per iteration.
//  Empirical testing suggests that 0x60 is the optimal lookahead for preload,
//  though anything between 0x40 and 0x100 seems to be "acceptable".
	push    ADDITIONAL_CALLEE_SAVE_REGISTERS
0:	ldm     r1!,    COPY_REGISTERS
	subs    r2,     r2, #0x40
	stm     r0!,    COPY_REGISTERS
	pld    [r1, #0x60]
	ldm     r1!,    COPY_REGISTERS
	pld    [r1, #0x60]
	stm     r0!,    COPY_REGISTERS
	bhs     0b
	pop     ADDITIONAL_CALLEE_SAVE_REGISTERS

L_ascendingLengthLessThanSixtyFour:
//  Cleanup copy of up to 63 bytes.  We can assume that both the source and
//  destination addresses have word alignment here.
    tst     r2,         #0x30
    beq     1f
0:  ldm     r1!,   {r3,r4,r9,ip}
    sub     r2,     r2, #0x10
    stm     r0!,   {r3,r4,r9,ip}
    tst     r2,         #0x30
    bne     0b
1:  tst     r2,         #0xf
    beq     2f
    lsls    ip,     r2, #29
    ldmcs   r1!,   {r3,ip}
    stmcs   r0!,   {r3,ip}
    ldrmi   r3,    [r1],#4
    strmi   r3,    [r0],#4
	lsls    ip,     r2, #31
	ldrhcs  r3,    [r1],#2
	strhcs  r3,    [r0],#2
	ldrbmi  ip,    [r1]
	strbmi  ip,    [r0]
2:  CLEAR_FRAME_AND_RETURN

/*****************************************************************************
 *  ascending copy, source buffer is not word aligned                        *
 *****************************************************************************/

L_ascendingUnalignedCopy:
//  Destination buffer is word aligned, but source buffer is not.  Copy
//  byte-by-byte until the destination buffer has eightbyte alignment.
    subs    r2,         #4
    blo     L_ascendingUnalignedByteCleanup
0:  tst     r0,         #0x7
    beq     L_ascendingUnalignedVectorCopy
    ldrb    r3,    [r1],#1
    subs    r2,         #1
    strb    r3,    [r0],#1
    bhs     0b
L_ascendingUnalignedByteCleanup:
    adds    r2,         #8
    beq     1f
0:  ldrb    r3,    [r1],#1
    subs    r2,         #1
    strb    r3,    [r0],#1
    bne     0b
1:  CLEAR_FRAME_AND_RETURN
    
L_ascendingUnalignedVectorCopy:
//  Destination buffer is eightbyte aligned.  Source buffer has unknown
//  alignment.  Use NEON to handle the misaligned copies.  We begin by copying
//  up to 24 bytes to get cacheline alignment of the destination buffer.
    subs    r2,         #0x18
    blo     L_ascendingUnalignedVectorCleanup
0:  tst     r0,         #0x18
    beq     L_ascendingUnalignedCachelineCopy
    vld1.8 {d0},   [r1]!
    subs    r2,         #8
    vst1.8 {d0},   [r0,:64]!
    bhs     0b
L_ascendingUnalignedVectorCleanup:
    adds    r2,         #0x18
    blo     L_ascendingUnalignedByteCleanup
0:  vld1.8 {d0},   [r1]!
    subs    r2,         #8
    vst1.8 {d0},   [r0,:64]!
    bhs     0b
    b       L_ascendingUnalignedByteCleanup
    
L_ascendingUnalignedCachelineCopy:
//  Main copy loop; moves 32 bytes per iteration.  Requires only byte alignment
//  of the source address.
    vld1.8 {q0,q1},[r1]!
    pld    [r1, #0x60]
    vst1.8 {q0,q1},[r0,:256]!
    subs    r2,         #0x20
    bhs     L_ascendingUnalignedCachelineCopy
    b       L_ascendingUnalignedVectorCleanup

/*****************************************************************************
 *  descending copy                                                          *
 *****************************************************************************/

//  The layout of the two buffers is such that we must copy in descending-
//  address order.  Throughout this copy, registers are used as follows:
//
//      r0  lowest address in the destination buffer that has been written to.
//      r1  lowest address in the source buffer that has been read from.
//      r2  number of bytes remaining to copy less an offset that varies
//          with the size of the copies that are being made.
//      r3, r4, r5, r6, r8, r9, r10, r12
//          temporary registers used to hold the data during copies.
//      r12 also used as a scratch register for alignment / length calculations

L_descendingCopy:
//  We begin by checking if less than four bytes are to be copied; if so, we
//  branch directly to a small-buffer copy and return.  Otherwise, we copy up
//  to three bytes if needed to make the destination pointer have word (four
//  byte) alignment.
    add     r1,     r2
    add     r0,     r2
    subs    r2,         #4
	blo     L_descendingLengthLessThanFour
	ands    ip,     r0, #0x3
	beq     L_descendingDestinationWordAligned
	ldrb    r3,    [r1, #-1]!
	cmp     ip,         #2
	ldrbhs  r4,    [r1, #-1]!
	strb    r3,    [r0, #-1]!
	ldrbhi  r3,    [r1, #-1]!
	strbhs  r4,    [r0, #-1]!
	strbhi  r3,    [r0, #-1]!
	subs    r2,         ip
	bhs     L_descendingDestinationWordAligned
        
L_descendingLengthLessThanFour:
//  Conditionally copies up to three bytes, assuming no alignment.  This is
//  only used if the original length of the buffer is smaller than four.
	lsls    ip,     r2, #31
	ldrbcs  r3,    [r1, #-1]!
	ldrbcs  ip,    [r1, #-1]!
	ldrbmi  r4,    [r1, #-1]
	strbcs  r3,    [r0, #-1]!
	strbcs  ip,    [r0, #-1]!
	strbmi  r4,    [r0, #-1]
	CLEAR_FRAME_AND_RETURN
    
L_descendingDestinationWordAligned:
//  We know that the destination has word alignment.  If the source is not
//  similarly aligned, jump to an unaligned copy loop.
	tst     r1,         #0x3
	bne		L_descendingUnalignedCopy

/*****************************************************************************
 *  descending copy, both buffers have word alignment                        *
 *****************************************************************************/
    
//  If less than sixty-four bytes remain to be copied, jump directly to the
//  word-aligned cleanup path.  Otherwise, we copy up to 28 bytes as needed
//  to make the destination pointer have cacheline alignment.
	subs    r2,     r2, #0x3c
	blo     L_descendingLengthLessThanSixtyFour
0:  tst     r0,         #0x1c
	beq     L_descendingDestinationCachelineAligned
	ldr     r3,    [r1, #-4]!
	subs    r2,         #4
	str     r3,    [r0, #-4]!
	bhs     0b
	b       L_descendingLengthLessThanSixtyFour

L_descendingDestinationCachelineAligned:
//  Unrolled main copy loop; copies two cachelines (64 bytes) per iteration.
//  Empirical testing suggests that -0x80 is the optimal lookahead for preload,
//  though anything between -0x40 and -0x100 seems to be "acceptable".
	push    ADDITIONAL_CALLEE_SAVE_REGISTERS
0:	ldmdb   r1!,    COPY_REGISTERS
	subs    r2,     r2, #0x40
	stmdb   r0!,    COPY_REGISTERS
	pld    [r1, #-0x80]
	ldmdb   r1!,    COPY_REGISTERS
	pld    [r1, #-0x80]
	stmdb   r0!,    COPY_REGISTERS
	bhs     0b
	pop     ADDITIONAL_CALLEE_SAVE_REGISTERS

L_descendingLengthLessThanSixtyFour:
//  Cleanup copy of up to 63 bytes.  We can assume that both the source and
//  destination addresses have word alignment here.
    tst     r2,         #0x30
    beq     1f
0:  ldmdb   r1!,   {r3,r4,r9,ip}
    sub     r2,     r2, #0x10
    stmdb   r0!,   {r3,r4,r9,ip}
    tst     r2,         #0x30
    bne     0b
1:  tst     r2,         #0xf
    beq     2f
    lsls    ip,     r2, #29
    ldmdbcs r1!,   {r3,ip}
    stmdbcs r0!,   {r3,ip}
    ldrmi   r3,    [r1, #-4]!
    strmi   r3,    [r0, #-4]!
	lsls    ip,     r2, #31
	ldrhcs  r3,    [r1, #-2]!
	strhcs  r3,    [r0, #-2]!
	ldrbmi  ip,    [r1, #-1]
	strbmi  ip,    [r0, #-1]
2:  CLEAR_FRAME_AND_RETURN

/*****************************************************************************
 *  descending copy, source buffer is not word aligned                       *
 *****************************************************************************/

L_descendingUnalignedCopy:
//  Destination buffer is word aligned, but source buffer is not.  Copy
//  byte-by-byte until the destination buffer has eightbyte alignment.
    subs    r2,         #4
    blo     L_descendingUnalignedByteCleanup
0:  tst     r0,         #0x7
    beq     L_descendingUnalignedVectorCopy
    ldrb    r3,    [r1, #-1]!
    subs    r2,         #1
    strb    r3,    [r0, #-1]!
    bhs     0b
L_descendingUnalignedByteCleanup:
    adds    r2,         #8
    beq     1f
0:  ldrb    r3,    [r1, #-1]!
    subs    r2,         #1
    strb    r3,    [r0, #-1]!
    bne     0b
1:  CLEAR_FRAME_AND_RETURN
    
L_descendingUnalignedVectorCopy:
//  Destination buffer is eightbyte aligned.  Source buffer has unknown
//  alignment.  Use NEON to handle the misaligned copies.  We begin by copying
//  up to 24 bytes to get cacheline alignment of the destination buffer.
    subs    r2,         #0x18
    blo     L_descendingUnalignedVectorCleanup
0:  tst     r0,         #0x18
    beq     L_descendingUnalignedCachelineCopy
    sub     r1,         #8
    vld1.8 {d0},   [r1]
    sub     r0,         #8
    vst1.8 {d0},   [r0,:64]
    subs    r2,         #8
    bhs     0b
L_descendingUnalignedVectorCleanup:
    adds    r2,         #0x18
    blo     L_descendingUnalignedByteCleanup
0:  sub     r1,         #8
    vld1.8 {d0},   [r1]
    sub     r0,         #8
    vst1.8 {d0},   [r0,:64]
    subs    r2,         #8
    bhs     0b
    b       L_descendingUnalignedByteCleanup
    
L_descendingUnalignedCachelineCopy:
//  Main copy loop; moves 32 bytes per iteration.  Requires only byte alignment
//  of the source address.
    sub     r1,         #32
    sub     r0,         #32
    mov     r4,         #-32
0:  vld1.8 {q0,q1},[r1], r4
    pld    [r1, #-0x60]
    vst1.8 {q0,q1},[r0,:256], r4
    subs    r2,         #0x20
    bhs     0b
    add     r1,         #32
    add     r0,         #32
    b       L_descendingUnalignedVectorCleanup

#endif // defined _ARM_ARCH_7 && !defined VARIANT_DYLD
