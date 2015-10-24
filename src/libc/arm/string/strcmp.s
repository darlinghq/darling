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

.text
.syntax unified
.code 32
.globl _strcmp
// int strcmp(const char *s1, const char *s2);
//
// Returns zero if the two NUL-terminated strings s1 and s2 are equal.
// Otherwise, returns the difference between the first two
// characters that do not match, interpreted as unsigned integers.

#define ESTABLISH_FRAME        \
    push   {r4-r7,lr}         ;\
    add     r7,     sp, #12   ;\
    push   {r8}
#define CLEAR_FRAME            \
    pop    {r8}               ;\
    pop    {r4-r7,lr}

.align 3
.long 0, 0x01010101
_strcmp:
//  Load a character from each string and advance the pointers.  If the loaded
//  characters are unequal or NUL, return their difference.
0:	ldrb    r2,    [r0],#1
    ldrb    ip,    [r1],#1
    cmp     r2,         #1
    cmphs   r2,         ip
    bne     L_earlyReturn
//  If the address of the next character from s1 does not have word alignment,
//  continue with the character-by-character comparison.  Otherwise, fall
//  through into the word-by-word comparison path.
    tst     r0,         #3
    bne     0b

//  We have not encountered a NUL or a mismatch, and s1 has word alignment.
//  Establish a frame, since we're going to need additional registers anyway.
    ESTABLISH_FRAME
    ldr     lr,    (_strcmp-4)

//  Word align s2, and place the remainder in r8.  Compute the right- and
//  left-shifts to extract each word that we will compare to the other source
//  from the aligned words that we load:
//
//      aligned s2        to be loaded on next iteration
//      |   "true" s2     |
//      v   v             v
//      +---+---+---+---+ +---+---+---+---+
//      | 0 | 1 | 2 | 3 | | 4 | 5 | 6 | 7 |
//      +---+---+---+---+ +---+---+---+---+
//          ^-----------------^
//          to be compared on next iteration
    and     r8,     r1, #3
    bic     r1,     r1, #3
    mov     r8,         r8, lsl #3
    rsb     r5,     r8, #32

//  Load the first aligned word of s2.  OR 0x01 into any bytes that preceed the
//  "true s2", to prevent our check for NUL from generating a false positive.
//  Then check for NUL, and jump to the byte-by-byte comparison loop after
//  unwinding the pointers if we enounter one.
    ldr     r6,    [r1],#4
    orr     r6,     r6, lr, lsr r5
    sub     r2,     r6, lr
    bic     r2,     r2, r6
    tst     r2,         lr, lsl #7
    mov     r4,         r6, lsr r8
    bne     L_unwindLoopPreload

.align 3
L_wordCompareLoop:
    //  Load the next aligned word of s2 and check if it contains any NUL bytes.
    //  Load the next aligned word of s1, and extract the corresponding bytes from
    //  the two words of s2 loaded in this and the previous iteration of the loop.
    //  Compare these two words.
    //  If no NUL or mismatched words have been encountered, continue the loop.
    ldr     r6,    [r1],#4
#if defined _ARM_ARCH_6
    uqsub8  r2,     lr, r6
    tst     r2,         r2
    ldr     ip,    [r0],#4
#else
    sub     r2,     r6, lr
    bic     r2,     r2, r6
    ldr     ip,    [r0],#4
    tst     r2,         lr, lsl #7
#endif
    orr     r3,     r4, r6, lsl r5
    cmpeq   ip,         r3
    mov     r4,         r6, lsr r8
    beq     L_wordCompareLoop

//  Either we have encountered a NUL, or we have found a mismatch between s1
//  and s2.  Unwind the pointers and use a byte-by-byte comparison loop.
    sub     r0,     r0, #4
    sub     r1,     r1, #4
L_unwindLoopPreload:
    sub     r1,     r1, r5, lsr #3
    CLEAR_FRAME

L_byteCompareLoop:
//  Load a character from each string and advance the pointers.  If the loaded
//  characters are unequal or NUL, return their difference.
    ldrb    r2,    [r0],#1
    ldrb    ip,    [r1],#1
    cmp     r2,         #1
    cmpcs   r2,         ip
    beq     L_byteCompareLoop

L_earlyReturn:
//  Return the difference between the last two characters loaded.
    sub     r0,     r2, ip
    bx      lr
