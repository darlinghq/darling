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
 */

// ARM Assembly implementation of memcmp( ) from <string.h>
// Uses Thumb2 if it is available, otherwise generates ARM code.
//
// -- Stephen Canon, August 2009
//
// The basic idea is to use word compares instead of byte compares as long as
// at least four bytes remain to be compared.  However, because memcmp( )
// compares the buffers as though they were big-endian unsigned integers, we
// need to byte-reverse each word before comparing them.
//
// If the buffers are not word aligned, or they are shorter than four bytes,
// we just use a simple byte comparison loop instead.
//
// int   bcmp(void *src1, void *src2, size_t length);
// int memcmp(void *src1, void *src2, size_t length);

.text
.syntax unified
#if defined __thumb2__
	.code 16
	.thumb_func _bcmp
	.thumb_func _memcmp
#else
	.code 32
#endif

#define ESTABLISH_FRAME \
	push   {r7,lr};\
	mov     r7,     sp
#define CLEAR_FRAME_AND_RETURN \
	pop    {r7,pc}

#include <arm/arch.h>

#if defined _ARM_ARCH_6
	#define BYTE_REVERSE(reg,tmp) \
    rev     reg, reg
#else // defined _ARM_ARCH_6
// Prior to ARMv6, the REV instruction is not available.  We use a very cute
// software workaround instead, which needs only a single scratch register.
	#define BYTE_REVERSE(reg,tmp) \
	eor     tmp, reg, reg, ror #16;\
	bic     tmp, tmp, #0xff0000   ;\
	mov     tmp,      tmp, lsr #8 ;\
	eor     reg, tmp, reg, ror #8
#endif // defined _ARM_ARCH_6

.globl _bcmp
.globl _memcmp
.align 2
_bcmp:
_memcmp:
    // If both buffers are not word aligned, jump to a byte-comparison loop.
	ESTABLISH_FRAME
	orr     ip,     r0, r1
	tst     ip,         #3
	bne     L_useByteComparisons

    // As long as at least four bytes of length remain, load one word from each
    // buffer and check if they are equal.
0:  subs    r2,         #4
	blo     L_lessThanFourBytesRemain
	ldr     r3,    [r0],#4
	ldr     ip,    [r1],#4
	cmp     r3,     ip
	beq     0b
	
    // If words from the two buffers compared unequal, we end up here.  We need
    // to byte-swap both words, then subtract to determine the result (+/-1).
	BYTE_REVERSE(r3,r1)
	BYTE_REVERSE(ip,r2)
    mov     r0,         #1
	subs    r3,     ip
	it      lo
	movlo   r0,         #-1
	CLEAR_FRAME_AND_RETURN
	
L_lessThanFourBytesRemain:
    adds    r2,         #4
L_useByteComparisons:
	mov     r3,     r0
    // If no bytes remain to compare, the buffers are equal and we return zero.
    // Otherwise, load one byte from each buffer and check if they are equal.
0:  subs    r2,         #1
	blo     L_buffersAreEqual
	ldrb    r0,    [r3],#1
	ldrb    ip,    [r1],#1
	subs    r0,         ip
	beq     0b
	CLEAR_FRAME_AND_RETURN
    
L_buffersAreEqual:
    mov     r0,         #0
	CLEAR_FRAME_AND_RETURN
