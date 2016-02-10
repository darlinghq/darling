/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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
    
#define ADDITIONAL_CALLEE_SAVE_REGISTERS {r5,r6,r8}

#define STORE_REGISTERS {r1,r3,r4,r5,r6,r8,r9,ip}

/*****************************************************************************
 *  entry points                                                             *
 *****************************************************************************/

.text
.syntax unified
.code 32

A9_ENTRY(__bzero)
A9_ENTRY(bzero)
    mov     r2,     r1
    eor     r1,     r1

A9_ENTRY(memset)
//  Early out if fewer than four bytes are to be set.  Otherwise, store up to
//  three bytes to align the destination pointer to a word boundary.
    ESTABLISH_FRAME
    and     r1,         #0xff
    subs    r2,         #4
    orr     r1,     r1, r1, lsl #8
    blo     L_lengthLessThanFour
    orr     r1,     r1, r1, lsl #16
0:  tst     r0,         #0x3
    beq     L_wordAligned
    strb    r1,    [r0],#1
    subs    r2,         #1
    bhs     0b
L_lengthLessThanFour:
    adds    r2,         #4
    beq     1f
0:  strb    r1,    [r0],#1
    subs    r2,         #1
    bne     0b
1:  CLEAR_FRAME_AND_RETURN

L_wordAligned:
//  Destination pointer has word alignment.  Early out if fewer than 64 bytes
//  are to be set.  Otherwise, store up to 28 bytes to align the destination
//  pointer to a cacheline boundary.
    mov     r3,     r1
    mov     r4,     r1
    subs    r2,         #0x3c
    mov     r9,     r1
    blo     L_lengthLessThanSixtyFour
0:  tst     r0,         #0x1c
    beq     L_cachelineAligned
    str     r1,    [r0],#4
    subs    r2,         #4
    bhs     0b
L_lengthLessThanSixtyFour:
    tst     r2,         #0x30
    beq     1f
0:  stm     r0!,   {r1,r3,r4,r9}
    sub     r2,         #0x10
    tst     r2,         #0x30
    bne     0b
1:  tst     r2,         #0xf
    beq     2f
    lsls    ip,     r2, #29
    stmcs   r0!,   {r1,r3}
    strmi   r1,    [r0],#4
    lsls    ip,     r2, #31
    strhcs  r1,    [r0],#2
    strbmi  r1,    [r0]
2:  CLEAR_FRAME_AND_RETURN

L_cachelineAligned:
//  Main unrolled loop; stores two complete cachelines per iteration.
    push    ADDITIONAL_CALLEE_SAVE_REGISTERS
    mov     r5,     r1
    mov     r6,     r1
    mov     r8,     r1
    mov     ip,     r1
.align 4
0:  stm     r0!,    STORE_REGISTERS
    subs    r2,         #0x40
    stm     r0!,    STORE_REGISTERS
    bhs     0b
    pop     ADDITIONAL_CALLEE_SAVE_REGISTERS
    b       L_lengthLessThanSixtyFour

#endif // defined _ARM_ARCH_7
