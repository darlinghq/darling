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
 *  void bzero(void * destination,
 *             size_t length);
 *
 *  void __bzero(void * destination,
 *               size_t length);
 *
 * zeros out a buffer length bytes long, beginning at the address destination.
 *
 *  void *memset(void * destination,
 *               int value,
 *               size_t n);
 *
 * writes value converted to an unsigned char to n successive bytes, beginning
 * at destination.
 */

#include <arm/arch.h>
#if defined _ARM_ARCH_7

.syntax unified
.code 32
.globl ___bzero$VARIANT$Swift
.globl _bzero$VARIANT$Swift
.globl _memset$VARIANT$Swift

.text
.align 4
___bzero$VARIANT$Swift:
_bzero$VARIANT$Swift:
    mov     r2,         r1      //  Set value to zero and move length to the
    eor     r1,         r1      //  correct register to match the memset API.
_memset$VARIANT$Swift:
    push    {r7,lr}             //  Establish a frame, and make a copy of the
    mov     r7,         sp      //  pointer to increment so that we can
    mov     ip,         r0      //  return the original pointer unmodified.

    vdup.8  q0,         r1      //  Splat the low byte of value across q0.
    subs    r3,     r2, #64     //  If length < 64, jump to a dedicated
    blo     L_lengthLessThan64  //  code path to handle small buffers.

    vmov    q1,         q0      //  Copy the splatted value to q1.
    orr     lr,     r2, r0      //  If the length is not a multiple of 16 or
    ands    lr,         #0xf    //  the buffer is not 16-byte aligned, then
    bne     L_edgingNeeded      //  some edging is needed; branch.

0:  subs    r3,         #64     //  Write 64 bytes at a time to the 16-byte
    vst1.8  {q0,q1}, [ip,:128]! //  aligned buffer.  Terminate this loop when
    vst1.8  {q0,q1}, [ip,:128]! //  64 or fewer bytes remain to be written.
    bhi     0b

    add     ip,         r3      //  Backtrack the destination pointer by
    vst1.8  {q0,q1}, [ip,:128]! //  64 - remaining bytes, and write 64 bytes
    vst1.8  {q0,q1}, [ip,:128]  //  to that address.  This takes us precisely
    pop     {r7,pc}             //  to the end of the buffer.

L_edgingNeeded:
    vst1.8  {q0},      [ip]     //  Write 16 bytes to the [possibly unaligned]
    and     lr,     ip, #0xf    //  buffer, then advance the pointer to the
    bic     ip,         #0xf    //  next aligned location, and adjust the
    add     r3,         lr      //  length accordingly.  Note that this means
    add     ip,         #16     //  that the first write in the loop may
    subs    r3,         #16     //  overlap with the write we just performed;
    blo     1f                  //  this is the fastest way to get alignment
    nop                         //  on Swift.

0:  subs    r3,         #64     //  Write 64 bytes at a time to the 16-byte
    vst1.8  {q0,q1}, [ip,:128]! //  aligned buffer.  Terminate this loop when
    vst1.8  {q0,q1}, [ip,:128]! //  64 or fewer bytes remain to be written.
    bhi     0b

1:  add     ip,         r3      //  Backtrack the destination pointer by
    vst1.8  {q0,q1},   [ip]!    //  64 - remaining bytes, and write 64 bytes
    vst1.8  {q0,q1},   [ip]     //  to that address.  This takes us precisely
    pop     {r7,pc}             //  to the end of the buffer.  

L_lengthLessThan64:
    subs    r3,     r2, #8      //  If the length is smaller than eight, jump
    blo     1f                  //  into a dedicated byte store loop.

0:  subs    r3,         #8      //  Write 8 bytes at a time to the destination
    vst1.8  {d0},      [ip]!    //  buffer, terminating when eight or fewer
    bhi     0b                  //  bytes remain to be written.

    add     ip,         r3      //  Backtrack the destination pointer by
    vst1.8  {d0},      [ip]     //  8 - remaining bytes, and write 8 bytes
    pop     {r7,pc}             //  to that address, then return.

1:  subs    r2,         #1      //  Store one byte at a time to the destination
    strbhs  r1,        [ip], #1 //  buffer, until we exhaust the length.
    bhi     1b
    pop     {r7,pc}    

#endif // defined _ARM_ARCH_7
