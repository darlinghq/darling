/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
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
 *  This file implements the following functions for the Swift micro-arch:
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

.syntax unified
.code 16
.globl _bcopy$VARIANT$Swift
.thumb_func _bcopy$VARIANT$Swift
.globl _memmove$VARIANT$Swift
.thumb_func _memmove$VARIANT$Swift
.globl _memcpy$VARIANT$Swift
.thumb_func _memcpy$VARIANT$Swift

.text
.align 4
_bcopy$VARIANT$Swift:
//  Translate bcopy calls into memcpy calls by swapping the first and second
//  arguments.
    mov     r3,     r0
    mov     r0,     r1
    mov     r1,     r3

_memmove$VARIANT$Swift:
_memcpy$VARIANT$Swift:
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
    push    {r7,lr}
    mov     r7,         sp
    subs    r3,     r0, r1
    beq     L_exit
    mov     ip,         r0
    cmp     r3,         r2
    blo     L_descendingCopy

/*****************************************************************************
 *  Ascending copy                                                           *
 *****************************************************************************/

    subs    r3,     r2, #32     //  If length < 32, jump to a dedicated code
    blo     L_ascendingShort    //  path for short buffers.

    orr     lr,     r0, r1      //  If the length is not a multiple of 16, or
    orr     lr,         r2      //  either buffer is not 16-byte aligned, then
    ands    lr,         #0xf    //  some edging is needed; jump to a separate
    bne     L_ascendingEdging   //  branch to handle it.

/*****************************************************************************
 *  Ascending vector aligned copy                                            *
 *****************************************************************************/

0:  subs    r3,         #32     //  Copy 32 bytes at a time from src to dst,
    vld1.8  {q0,q1}, [r1,:128]! //  both of which have 16-byte alignment.
    vst1.8  {q0,q1}, [ip,:128]! //  Terminate this loop when 32 or fewer bytes
    bhi     0b                  //  remain to be copied.

    add     r1,         r3      //  Backtrack both pointers by 32 - remaining
    vld1.8  {q0,q1}, [r1,:128]  //  and copy 32 bytes from src to dst.  This
    add     ip,         r3      //  copy may overlap the previous copy, and
    vst1.8  {q0,q1}, [ip,:128]  //  takes us precisely to the end of the
    pop     {r7,pc}             //  buffer.

/*****************************************************************************
 *  Ascending vector misaligned copy                                         *
 *****************************************************************************/

L_ascendingEdging:
    tst     ip,         #0xf    //  Copy one byte at a time until the 
    itttt   ne                  //  destination pointer has 16 byte alignment.
    ldrbne  r3,    [r1],#1
    strbne  r3,    [ip],#1
    subne   r2,         #1
    bne     L_ascendingEdging

    and     lr,     r1, #0xf    //  Back the source pointer up to a 16-byte
    bic     r1,         #0xf    //  aligned location, and check if length > 32.
    subs    r3,     r2, #32
    blo     L_ascendingEdgingExit
    tbh    [pc, lr, lsl #1]     //  Otherwise, we have a jump table based on
0:                              //  the relative alignment of the buffers.
.short (L_ascendingExtract0x0-0b)/2
.short (L_ascendingExtract0x1-0b)/2
.short (L_ascendingExtract0x2-0b)/2
.short (L_ascendingExtract0x3-0b)/2
.short (L_ascendingExtract0x4-0b)/2
.short (L_ascendingExtract0x5-0b)/2
.short (L_ascendingExtract0x6-0b)/2
.short (L_ascendingExtract0x7-0b)/2
.short (L_ascendingExtract0x8-0b)/2
.short (L_ascendingExtract0x9-0b)/2
.short (L_ascendingExtract0xa-0b)/2
.short (L_ascendingExtract0xb-0b)/2
.short (L_ascendingExtract0xc-0b)/2
.short (L_ascendingExtract0xd-0b)/2
.short (L_ascendingExtract0xe-0b)/2
.short (L_ascendingExtract0xf-0b)/2

L_ascendingExtract0x0:          //  If the two buffers are similarly aligned,
    subs    r3,         #32     //  we use a slightly simpler loop that just
    vld1.8  {q0,q1}, [r1,:128]! //  copies 32 bytes at a time.
    vst1.8  {q0,q1}, [ip,:128]!
    bhs     L_ascendingExtract0x0
    b       L_ascendingEdgingExit

#define ASCENDING_EXTRACT(shift)\
L_ascendingExtract ## shift:\
    vld1.8  {q8},    [r1,:128]!;\
0:  vld1.8  {q9,q10},[r1,:128]!;\
    vext.8  q0, q8, q9, $(shift);\
    vext.8  q1, q9, q10,$(shift);\
    vmov    q8,         q10;\
    vst1.8  {q0,q1}, [ip,:128]!;\
    subs    r3,         $32;\
    bhs     0b;\
    sub     r1,         $16;\
    b       L_ascendingEdgingExit

ASCENDING_EXTRACT(0x1)          //  Otherwise, we use the loop implemented in
ASCENDING_EXTRACT(0x2)          //  the above macro.  It loads 32 bytes per
ASCENDING_EXTRACT(0x3)          //  iteration combines it with the residual
ASCENDING_EXTRACT(0x4)          //  bytes from the previous iteration, and
ASCENDING_EXTRACT(0x5)          //  uses the VEXT instruction to extract 32
ASCENDING_EXTRACT(0x6)          //  bytes that can be stored to a 16-byte
ASCENDING_EXTRACT(0x7)          //  aligned location in the destination buffer.
ASCENDING_EXTRACT(0x8)          //  This continues until 32 or fewer bytes
ASCENDING_EXTRACT(0x9)          //  remain to be copied.  This is significantly
ASCENDING_EXTRACT(0xa)          //  faster than using misaligned loads and
ASCENDING_EXTRACT(0xb)          //  stores, which are very inefficient on
ASCENDING_EXTRACT(0xc)          //  Swift.
ASCENDING_EXTRACT(0xd)
ASCENDING_EXTRACT(0xe)
ASCENDING_EXTRACT(0xf)

L_ascendingEdgingExit:
    add     r1,         lr      //  Restore the source pointer
    add     r2,     r3, #32     //  Restore the length
L_ascendingShort:
    subs    r2,         #1      //  Copy one byte at a time until the buffer
    itt     hs                  //  is exhausted, then return.
    ldrbhs  r3,    [r1],#1
    strbhs  r3,    [ip],#1
    bhi     L_ascendingShort
L_exit:
    pop     {r7,pc}

/*****************************************************************************
 *  Descending copy                                                          *
 *****************************************************************************/

L_descendingCopy:
    add     r1,         r2      //  Advance source and destination pointers to
    add     ip,         r2      //  the end of the buffer.

    subs    r3,     r2, #32     //  If length < 32, jump to a dedicated code
    blo     L_descendingShort   //  path for short buffers.

    orr     lr,     r0, r1      //  If the length is not a multiple of 16, or
    orr     lr,         r2      //  either buffer is not 16-byte aligned, then
    ands    lr,         #0xf    //  some edging is needed; jump to a separate
    bne     L_descendingEdging  //  branch to handle it.

/*****************************************************************************
 *  Descending vector aligned copy                                            *
 *****************************************************************************/

0:  sub     r1,         #32     //  Copies 32 bytes (16-byte aligned) from
    vld1.8  {q0,q1}, [r1,:128]  //  source to destination on each pass through
    sub     ip,         #32     //  the loop.  The loop ends when 32 or fewer
    vst1.8  {q0,q1}, [ip,:128]  //  bytes remain to be copied.
    subs    r3,         #32
    bhi     0b
    add     r3,         #32     //  Copy the remaining up-to-32 bytes.
    sub     r1,         r3      //  This copy may overlap the copy performed
    vld1.8  {q0,q1}, [r1,:128]  //  in the final iteration through the
    sub     ip,         r3      //  previous loop, but this is more efficient
    vst1.8  {q0,q1}, [ip,:128]  //  than figuring out exactly which bytes
    pop     {r7,pc}             //  need to be copied.

/*****************************************************************************
 *  Descending vector misaligned copy                                        *
 *****************************************************************************/

L_descendingEdging:
    tst     ip,         #0xf    //  Identical to how we handle misalignment for
    itttt   ne                  //  ascending copies.  First we move one byte
    ldrbne  r3,    [r1,#-1]!    //  at a time until the destination has 16
    strbne  r3,    [ip,#-1]!    //  byte alignment.
    subne   r2,         #1
    bne     L_descendingEdging

    and     lr,     r1, #0xf    //  Then we extract the alignment of the source
    bic     r1,         #0xf    //  buffer and use a jump table to dispatch
    subs    r3,     r2, #32     //  into code that does the appropriate
    blo     L_descendingEdgingExit  //  software alignment fixup.
    tbh    [pc, lr, lsl #1]
0:
.short (L_descendingExtract0x0-0b)/2
.short (L_descendingExtract0x1-0b)/2
.short (L_descendingExtract0x2-0b)/2
.short (L_descendingExtract0x3-0b)/2
.short (L_descendingExtract0x4-0b)/2
.short (L_descendingExtract0x5-0b)/2
.short (L_descendingExtract0x6-0b)/2
.short (L_descendingExtract0x7-0b)/2
.short (L_descendingExtract0x8-0b)/2
.short (L_descendingExtract0x9-0b)/2
.short (L_descendingExtract0xa-0b)/2
.short (L_descendingExtract0xb-0b)/2
.short (L_descendingExtract0xc-0b)/2
.short (L_descendingExtract0xd-0b)/2
.short (L_descendingExtract0xe-0b)/2
.short (L_descendingExtract0xf-0b)/2

L_descendingExtract0x0:         //  For relative alignment, we have a fast
    sub     r1,         #32     //  path identical to the aligned copy loop.
    vld1.8  {q0,q1}, [r1,:128]
    sub     ip,         #32
    vst1.8  {q0,q1}, [ip,:128]
    subs    r3,         #32
    bhs     L_descendingExtract0x0
    b       L_descendingEdgingExit

#define DESCENDING_EXTRACT(shift)\
L_descendingExtract ## shift:\
    vld1.8  {q10},    [r1,:128];\
0:  sub     r1,         #32;\
    vld1.8  {q8,q9},  [r1,:128];\
    vext.8  q1, q9, q10,$(shift);\
    vext.8  q0, q8, q9, $(shift);\
    vmov    q10,        q8;\
    sub     ip,         #32;\
    vst1.8  {q0,q1}, [ip,:128];\
    subs    r3,         $32;\
    bhs     0b;\
    b       L_descendingEdgingExit

DESCENDING_EXTRACT(0x1)         //  Otherwise, we use the loop above (almost
DESCENDING_EXTRACT(0x2)         //  identical to the one we use in the
DESCENDING_EXTRACT(0x3)         //  ascending copy case).
DESCENDING_EXTRACT(0x4)
DESCENDING_EXTRACT(0x5)
DESCENDING_EXTRACT(0x6)
DESCENDING_EXTRACT(0x7)
DESCENDING_EXTRACT(0x8)
DESCENDING_EXTRACT(0x9)
DESCENDING_EXTRACT(0xa)
DESCENDING_EXTRACT(0xb)
DESCENDING_EXTRACT(0xc)
DESCENDING_EXTRACT(0xd)
DESCENDING_EXTRACT(0xe)
DESCENDING_EXTRACT(0xf)

L_descendingEdgingExit:
    add     r1,         lr      //  Restore source pointer
    add     r2,     r3, #32     //  Restore length
L_descendingShort:
    subs    r2,         #1      //  Byte-by-byte copy loop for short overlapping
    itt     hs                  //  buffers.
    ldrbhs  r3,    [r1,#-1]!
    strbhs  r3,    [ip,#-1]!
    bhi     L_descendingShort
    pop     {r7,pc}

#endif // defined _ARM_ARCH_7 && !defined VARIANT_DYLD