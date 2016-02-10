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
.globl _strlen

#define addr r0
#define word r1
#define temp r2
#define mask r3
#define save ip
#define indx r0

.macro IfWordDoesntContainNUL_SetZ
#if defined _ARM_ARCH_6
//  In each word of the string, we check for NUL bytes via a saturating
//  unsigned subtraction of each byte from 0x1.  The result of this is
//  non-zero if and only if the corresponding byte in the string is NUL.
//  Simply using a TST instruction checks all four bytes for NULs in one
//  go.
    uqsub8  temp,   mask,   word
    tst     temp,           temp
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
    tst     temp,           mask, lsl #7
#endif
.endm

.text
.align 4
.long 0x0           // padding
.long 0x01010101    // mask for use in finding NULs
_strlen:
//  Establish stack frame, load mask that we will use to find NUL bytes,
//  and set aside a copy of the pointer to the string.
    push    {r7,lr}
    mov     r7,     sp
    ldr     mask,   (_strlen-4)
    add		save,   addr,   #4

//  Load the aligned word that contains the start of the string, then OR
//  0x01 into any bytes that preceed the start of the string to prevent
//  false positives when we check for NUL bytes.
    and     temp,   addr,   #3
    bic     addr,   addr,   #3
    lsl     temp,   temp,   #3
    ldr     word,  [addr],  #4
    rsb     temp,   temp,   #32
    orr     word,   word,   mask, lsr temp

//  Check if the string ends in the first word.  If so, don't load any
//  more of the string; instead jump directly to the cleanup code.
    IfWordDoesntContainNUL_SetZ
    bne     1f

.align 4
//  Load one word of the string on each iteration, and check it for NUL
//  bytes.  If a NUL is found, fall through into the cleanup code.
0:  ldr     word,  [addr],  #4
    IfWordDoesntContainNUL_SetZ
    beq		0b

//  The last word that we loaded contained a NUL.  Subtracting the saved
//  pointer from the current pointer gives us the number of bytes from
//  the start of the string to the word containing the NUL.
1:  sub     indx,   addr,   save
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

