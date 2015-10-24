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
 *  void memset_pattern4(void *b, const void *pattern4, size_t len);
 *  void memset_pattern8(void *b, const void *pattern8, size_t len);
 *  void memset_pattern16(void *b, const void *pattern16, size_t len);
 *
 *  The implementation of all three functions is fundamentally the same.
 *  memset_pattern4 is extensively commented to explain, reference that
 *  if you have any questions about the other two.
 */

#include <arm/arch.h>
#if defined _ARM_ARCH_7 && !defined VARIANT_DYLD

.syntax unified
.code 32
.text
.globl _memset_pattern4$VARIANT$Swift
.globl _memset_pattern8$VARIANT$Swift
.globl _memset_pattern16$VARIANT$Swift

/******************************************************************************/

.align 4
_memset_pattern4$VARIANT$Swift:
    push    {r7,lr}
    mov     r7,         sp

//  Load the pattern and splat it to q0, then check if the buffer is at least
//  64 bytes long.  If not, branch to a short-buffer implementation.
    ldr     r1,        [r1]
    vdup.32 q0,         r1
    subs    r3,     r2, #64
    blo     L_short4

//  We want to use aligned vector stores to fill the bulk of the buffer.  In
//  order to make that work, we need to rotate the pattern as necessary to
//  match up with aligned locations, and we also need to extract the alignment
//  of the destination pointer mod 16.
    lsl     ip,     r0, #3
    and     lr,     r0, #0xf    //  alignment of destination pointer mod 16
    rsb     ip,     ip, #32     //  low five bits contain 32 - 8*(address%4).

//  Before we start the aligned stores, we do a single unaligned store of
//  16 bytes of the pattern to the start of the buffer.  Since the buffer is
//  at least 64 bytes long, this store is known to lie entirely inside the
//  buffer:
//                              first aligned address in buffer
//                                                            v
//   ---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---
//  ... | 3 | 4 | 5 | 6 | 7 | 8 | 9 | a | b | c | d | e | f | 0 | 1 | 2 | ...
//   ---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---
//        ^
//        unaligned store starts here:
//      [ 0   1   2   3   0   1   2   3   0   1   2   3   0   1   2   3 ]
    vst1.8  {q0},      [r0]!

//  Subsequent stores will be aligned, and will start at the first aligned
//  address in the buffer.  We apply the rotation that we calculated before
//  the vector store (in the low five bits of ip) to get the pattern that
//  is to be stored starting at the aligned location.  For example, in the
//  picture above, the buffer had alignment of 3 mod 4, so the rotation to
//  be applied is 32 - 8*3 = 8.  Rotating the pattern right by 8 bits gives
//  us [ 1 2 3 0 ] (remember, we're little-endian), which we see is what
//  needs to be stored starting at the first aligned location.
//
//  Besides rotating the pattern, we also need to adjust the length (by
//  subtracting 16 - alignment mod 16), and to advance the pointer to the
//  first aligned location.
    ror     r1,         ip      //  Pattern to use for aligned memory
    add     r3,         lr
    bic     r0,         #0xf    //  destination for first aligned store
    subs    r3,         #16     //  updated length
    blo     1f

//  Splat the rotated value across q1 and q2
    vdup.32 q1,         r1
    vmov    q2,         q1

//  Main store loop.  We write the splatted aligned pattern across 64 bytes
//  per iteration, terminating the loop when the remaining length of the
//  buffer is 64 bytes or less.
0:  subs    r3,         #64
    vst1.32 {q1,q2}, [r0,:128]!
    vst1.32 {q1,q2}, [r0,:128]!
    bhi     0b

//  The remaining length of the buffer is 64 bytes or less (but the total
//  length of the buffer is at least 64 bytes; otherwise we would have
//  branched to the "short" path).  Thus, we can handle the entirety of the
//  remaining buffer with two 32-byte unaligned stores.
//
//  Again, we need to rotate the pattern to match the alignment, this time
//  by 8*(length%4), and we also need to back up the destination pointer
//  so that it points to precisely 64 bytes before the end of the buffer.
//  We accomplish this by adding r3, which contains the remaining length of
//  the buffer minus 64.
1:  lsl     ip,     r3, #3
    ror     r1,         ip
    vdup.32 q8,         r1
    vmov    q9,         q8
    add     r0,         r3
    vst1.32 {q8,q9},   [r0]!
    vst1.32 {q8,q9},   [r0]
    pop     {r7,pc}

L_short4:
//  If we branch here, the buffer is less than 64 bytes long.  At this point,
//  register contents are as follows:
//
//      r0      pointer to the buffer
//      r1      pattern
//      r2      buffer length
//      q0      splatted pattern
//
//  To begin, we store eight bytes at a time until the remaining length is
//  less than eight bytes.
    subs    r3,     r2, #8
    blo     1f
0:  subs    r3,         #8
    vst1.32 {d0},      [r0]!
    bhs     0b

//  Then we store one byte at a time, rotating the pattern to get the next
//  byte, until we reach the end of the buffer.
    add     r2,     r3, #8
1:  subs    r2,         #1
    strbhs  r1,    [r0],#1
    ror     r1,         #8
    bhi     1b
    pop     {r7,pc}

/******************************************************************************/

.align 4
_memset_pattern8$VARIANT$Swift:
//  The implementation of this function is substantially identical to that of
//  memset_pattern4.  The only differences are in how we rotate the pattern for
//  the purposes of extracting the bytes to store.  For clarity, only those
//  differences are commented here; consult memset_pattern4 (above) for
//  a detailed description of the algorithm used.
    push    {r7,lr}
    mov     r7,         sp
    vld1.8  {d0},      [r1]
    vmov    d1,         d0
    subs    r3,     r2, #64
    blo     L_short8

    bic     sp,         #0xf    //  Align stack to 16 bytes and write 32 bytes
    sub     sp,         #16     //  of pattern to the stack.  We will use 
    vst1.8  {q0},    [sp,:128]  //  unaligned loads from this scratch buffer
    sub     sp,         #16     //  to get rotated forms of the pattern.
    vst1.8  {q0},    [sp,:128]
    and     ip,     r0, #0x7    //  Now generate an unaligned pointer to the
    rsb     ip,     ip, #8      //  rotated pattern that we need to use for
    add     ip,         sp      //  aligned stores in the main loop.
    and     lr,     r0, #0xf
    vst1.8  {q0},      [r0]!
    add     r3,         lr
    bic     r0,         #0xf
    subs    r3,         #16
    blo     1f
    vld1.8  {q1},      [ip]
    vmov    q2,         q1
0:  subs    r3,         #64
    vst1.32 {q1,q2}, [r0,:128]!
    vst1.32 {q1,q2}, [r0,:128]!
    bhi     0b
1:  and     lr,     r3, #0x7    //  Generate an unaligned pointer to the
    add     ip,         lr      //  rotated pattern to use for cleanup.
    vld1.8  {q8},      [ip]
    vmov    q9,         q8
    add     r0,         r3
    vst1.32 {q8,q9},   [r0]!
    vst1.32 {q8,q9},   [r0]
    mov     sp,         r7      //  Restore stack pointer
    pop     {r7,pc}

L_short8:
    subs    r2,         #8
    blo     1f
0:  subs    r2,         #8
    vst1.32 {d0},      [r0]!
    bhs     0b
1:  adds    r2,         #8
    beq     3f
2:  vst1.8  {d0[0]},   [r0]!    //  Store one byte from NEON
    vext.8  d0,     d0, d0, #1  //  Use VEXT to rotate pattern
    subs    r2,         #1
    bhi     2b
3:  pop     {r7,pc}

/******************************************************************************/

.align 4
_memset_pattern16$VARIANT$Swift:
//  The implementation of this function is substantially identical to that of
//  memset_pattern4.  The only differences are in how we rotate the pattern for
//  the purposes of extracting the bytes to store.  For clarity, only those
//  differences are commented here; consult memset_pattern4 (above) for
//  a detailed description of the algorithm used.
    push    {r7,lr}
    mov     r7,         sp
    vld1.8  {q0},      [r1]
    subs    r3,     r2, #64
    blo     L_short16

    bic     sp,         #0xf    //  Align stack to 16 bytes and write 48 bytes
    sub     sp,         #16     //  of pattern to the stack.  We will use 
    vst1.8  {q0},    [sp,:128]  //  unaligned loads from this scratch buffer
    sub     sp,         #16     //  to get rotated forms of the pattern.
    vst1.8  {q0},    [sp,:128]
    sub     sp,         #16   
    vst1.8  {q0},    [sp,:128]
    and     lr,     r0, #0xf    //  Now generate an unaligned pointer to the
    rsb     ip,     lr, #16     //  rotated pattern that we need to use for
    add     ip,         sp      //  aligned stores in the main loop.
    vst1.8  {q0},      [r0]!
    add     r3,         lr
    bic     r0,         #0xf
    subs    r3,         #16
    blo     1f
    vld1.8  {q1},      [ip]
    vmov    q2,         q1
0:  subs    r3,         #64
    vst1.32 {q1,q2}, [r0,:128]!
    vst1.32 {q1,q2}, [r0,:128]!
    bhi     0b
1:  and     lr,     r3, #0xf    //  Generate an unaligned pointer to the
    add     ip,         lr      //  rotated pattern to use for cleanup.
    vld1.8  {q8},      [ip]
    vmov    q9,         q8
    add     r0,         r3
    vst1.32 {q8,q9},   [r0]!
    vst1.32 {q8,q9},   [r0]
    mov     sp,         r7      //  Restore stack pointer
    pop     {r7,pc}

L_short16:
    subs    r2,         #16
    blo     1f
0:  subs    r2,         #16
    vst1.32 {q0},      [r0]!
    bhs     0b
1:  adds    r2,         #16
    beq     3f
2:  vst1.8  {d0[0]},   [r0]!    //  Store one byte from NEON
    vext.8  q0,     q0, q0, #1  //  Use VEXT to rotate pattern
    subs    r2,         #1
    bhi     2b
3:  pop     {r7,pc}

#endif // defined _ARM_ARCH_7 && !defined VARIANT_DYLD
