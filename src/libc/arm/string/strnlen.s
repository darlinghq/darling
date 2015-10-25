/*
 * Copyright (c) 2011 Apple, Inc. All rights reserved.
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
 */

#include <arm/arch.h>
.syntax unified
.code 32
.globl _strnlen

#define addr r0
#define maxl r1
#define temp r2
#define mask r3
#define save ip
#define word lr
#define byte lr
#define indx r0

.macro IfHS_and_WordDoesntContainNUL_SetZ
#if defined _ARM_ARCH_6
//  In each word of the string, we check for NUL bytes via a saturating
//  unsigned subtraction of each byte from 0x1.  The result of this is
//  non-zero if and only if the corresponding byte in the string is NUL.
//  Simply using a TST instruction checks all four bytes for NULs in one
//  go.
    uqsub8  temp,   mask,   word
    tsths   temp,           temp
#else
//  If we're on armv5, we do not have the uqsub8 instruction, so we need
//  to use a different test for NUL.  Instead, we compute:
//
//      byte - 0x1 & ~byte
//
//  and test the high-order bit.  If it is set, then byte is NUL.  Just
//  as with the other test, this can be applied simultaneously to all
//  bytes in a word.
    sub     temp,   word,   mask
    bic     temp,   temp,   word
    tsths   temp,           mask, lsl #7
#endif
.endm

.text
.align 3
.long 0x0           // padding
.long 0x01010101    // mask for use in finding NULs
_strnlen:
//  Establish stack frame, load mask that we will use to find NUL bytes,
//  and set aside a copy of the pointer to the string.  Subtract 4 from
//  the maxlen, and jump into a byte-by-byte search if this requires a
//  borrow, as we cannot use a word-by-word search in that case.
    push    {r7,lr}
    mov     r7,     sp
    ldr     mask,   (_strnlen-4)
    add		save,   addr,   #4
    subs    maxl,   maxl,   #4
    blo     L_bytewiseSearch

//  Load the aligned word that contains the start of the string, then OR
//  0x01 into any bytes that preceed the start to prevent false positives
//  when we check for NUL bytes.  Additionally, add the number of unused
//  bytes to maxlen.
    and     temp,   addr,   #3
    bic     addr,   addr,   #3
    add     maxl,   maxl,   temp
    lsl     temp,   temp,   #3
    ldr     word,  [addr],  #4
    rsb     temp,   temp,   #32
    orr     word,   word,   mask, lsr temp

    subs    maxl,   maxl,   #4
    IfHS_and_WordDoesntContainNUL_SetZ
    bne     1f

.align 4
0:  ldr     word,  [addr],  #4
    subs    maxl,   maxl,   #4
    IfHS_and_WordDoesntContainNUL_SetZ
    beq     0b

.align 4
//  Either the last word that we loaded contained a NUL, or we will
//  exceed maxlen before we finish the next word in the string.  Determine
//  which case we are in by repeating the check for NUL, and branch if
//  there was not a NUL byte.  Padding ensures that we don't have two
//  branches in a single 16-byte fetch group, as this interferes with
//  branch prediction on Swift.
1:  tst     temp,           temp
    beq     L_bytewiseSearch

//  The last word that we loaded contained a NUL.  Subtracting the saved
//  pointer from the current pointer gives us the number of bytes from
//  the start of the string to the word containing the NUL.
    sub     indx,   addr,   save
#if defined _ARM_ARCH_6
//  To that we add the index of the first NUL byte in the word, computed
//  using REV and CLZ followed by a shift.
    rev     temp,           temp
    clz     temp,           temp
    add     indx,   indx,   temp, lsr #3
#else
//  armv5 does not have the REV instruction, so instead we find the
//  index of the NUL byte in word with a linear search.
    tst     word,           #0x000000ff
    addne   indx,           #1
    tstne   word,           #0x0000ff00
    addne   indx,           #1
    tstne   word,           #0x00ff0000
    addne   indx,           #1
#endif
    pop     {r7,pc}

.align 4
L_bytewiseSearch:
//  Restore maxlen (the last thing that happened before we branched here
//  was that we subtracted 4 from maxlen), and adjust the saved string
//  pointer.  Then we do a simple byte-by-byte search until we either
//  reach the end of the string or maxlen reaches zero, at which point
//  the length to return is simply the difference between the current
//  and saved pointers.
    adds    maxl,   maxl,   #4
    sub     save,   save,   #4
    beq     1f
0:  ldrb    byte,          [addr]
    cmp     byte,           #0
    addhi   addr,           #1
    subshi  maxl,           #1
    bhi     0b
1:  sub     indx,   addr,   save
    pop     {r7,pc}
