/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

//  Only built for armv6 and higher.
#if defined _ARM_ARCH_6

//  If we're building for armv7, and not for DYLD, then we have a symbol
//  resolver so we need to rename these implementations.
#if defined _ARM_ARCH_7 && !defined VARIANT_DYLD
#define _memset_pattern4  _memset_pattern4$VARIANT$Generic
#define _memset_pattern8  _memset_pattern8$VARIANT$Generic
#define _memset_pattern16 _memset_pattern16$VARIANT$Generic
#endif

#include <mach/machine/asm.h>

/* 
 * This file contains the following functions: 
 * 
 * 	void memset_pattern4(void *b, const void *c4, size_t len)
 * 	void memset_pattern8(void *b, const void *c8, size_t len)
 * 	void memset_pattern16(void *b, const void *c16, size_t len)
 * 
 * The memset() is implemented in the bzero.s file. 
 * 
 * This is a reasonably well optimized version of memset_pattern* routines
 * implemented for the ARM9 and ARM11 processors using the ARMv6 instruction
 * set. These routines use the ARM's core registers.  
 *
 * The algorithm is to align the destination pointer on a 16 byte boundary 
 * and then blast data 64 bytes at a time, in two stores of 32 bytes per loop.
 * 
 */
	.text
	.align 2
	.syntax unified

/*----------------------------------------------------------------------------*/
/* void     memset_pattern4(void *ptr, const void *pattern4, size_t len);     */
/*                                                                            */
/*      r0    << destination pointer                                          */
/* 	r1    << pointer to 4-byte pattern                                    */
/* 	r2    << 'len' (length of destination buffer in bytes)                */
/*----------------------------------------------------------------------------*/
	.globl _memset_pattern4
_memset_pattern4:
	cmp	r2, #0	/* check if len is zero */
	bxeq	lr	/* return if length is zero */

	/* We need some registers, so save volatiles on stack */
	/* Avoid r7 (frame pointer) and r9 (thread register)  */
	stmfd	sp!, {r4-r7, lr} 
	add	r7, sp, #12        /* establish frame */
	stmfd	sp!, {r8, r10-r11}

	/* copy destination base pointer r0 to r12 and leave r0 alone */
	/* so that we return original pointer back to the caller */
	mov	r12, r0

	/* Check if 'len' is long enough to bother alignment of destination */
	/* pointer */
	cmp	r2, #32     /* long enough to bother aligning? */
	movlt	r3, #4      /* move pattern length into r3 */
	movlt	r10, #4     /* pattern index */
	movlt	r11, r1     /* move pattern pointer into r11 */
	blt	L_Short     /* no */

	/* move 'len' into r1, get 4-byte pattern in r2 */
	mov 	r6, r2 	/* temporarily move 'len' in to r6 */
	ldr	r2, [r1]/* load 4-byte pattern into r2 */
	mov	r1, r6 	/* move 'len' from r6 to r1 */

	mov	r3, r2  /* copy 4-byte pattern into r3, r4 and r5 registers */
	mov 	r4, r2
	mov 	r5, r2

L_NotShort: 

	/* Check for 16 or 32 byte aligned destination pointer */ 
	tst 	r12, #0x1F  /* check for 32 byte aligned */
	beq	L_Aligned
	tst 	r12, #0xF   /* check for 16 byte aligned */
	beq	L_16ByteAligned
	b	L_Unaligned /* yes */

L_Bytewise: 
	ldrb	r4, [r11], #1
	strb	r4, [r12], #1
	subs	r10, #1
	moveq	r10, r3
	moveq	r11, r1
	sub	r2, #1

L_Short: 
	cmp 	r2, #0 /* more bytes left? */
	bne	L_Bytewise
	ldm	sp!, {r8, r10-r11} /* restores registers from stack */
	ldm 	sp!, {r4-r7, pc}   /* restore & return from subroutine */

/* 'len' is long enough to justify aligning the destination pointer           */
/*                                                                            */
/* By the time we reach here, data is stored in registers as follows:         */
/* 	r1    << 'len' (length of destination buffer in bytes)                */
/* 	r2-r5 << pattern; either 4x4byte OR 2x8byte OR 1x16-byte              */
/*      r12   << destination pointer copy (scratch register)                  */
/*      r0    << destination pointer original                                 */
/*                                                                            */
/* Use r11 as scratch register to store the #bytes offset to 16-byte align    */
/*                                                                            */
/* Unaligned on 32-byte boundary, store 1-15 bytes until 16-byte aligned      */
/* As we store these bytes, we rotate the pattern stored in r2-r5 to reflect  */
/* the alignment.                                                             */

L_Unaligned: 
	mov	r11, r12, lsl #28
	rsb	r11, r11, #0
	msr	cpsr_f, r11 /* Bits[31:28] of cpsr now contain #bytes to align*/

L_Store15BytesAndRotatePattern: 
        strbvs  r2, [r12], #1   /* v is set, unaligned in the 1s column */
	andvs	r6, r2, #0xFF   /* Rotate pattern right in r2-r5 by 1-byte */
	andvs	r8, r3, #0xFF   /* Consider register r2-r5 and a contiguous */
	andvs	r10, r4, #0xFF  /* 16-byte register with r2 containing LSB */
	andvs	r11, r5, #0xFF  /* and r5 containing MSB */
	lsrvs	r2, r2, #8
	lsrvs	r3, r3, #8
	lsrvs	r4, r4, #8
	lsrvs	r5, r5, #8
	orrvs	r2, r2, r8, lsl #24
	orrvs	r3, r3, r10, lsl #24
	orrvs	r4, r4, r11, lsl #24
	orrvs	r5, r5, r6, lsl #24
	
        strhcs  r2, [r12], #2 /* c is set, unaligned in the 2s column */
	movcs	r6, r2, lsl #16   /* Rotate pattern right in r2-r5 by 2-bytes */
	movcs	r8, r3, lsl #16
	movcs	r10, r4, lsl #16
	movcs	r11, r5, lsl #16
	lsrcs	r2, r2, #16
	lsrcs	r3, r3, #16
	lsrcs	r4, r4, #16
	lsrcs	r5, r5, #16
	orrcs	r2, r2, r8
	orrcs	r3, r3, r10
	orrcs	r4, r4, r11
	orrcs	r5, r5, r6

        streq   r2, [r12], #4 /* z is set, unaligned in the 4s column */
	moveq	r6, r2 /* Rotate pattern right in r2-r5 by 4-bytes */
	moveq	r2, r3
	moveq	r3, r4
	moveq	r4, r5
	moveq	r5, r6

	stmmi	r12!, {r2-r3} /* n is set, unaligned in the 8s column */
	movmi	r6, r2 /* Rotate pattern right in r2-r5 by 4-bytes */
	movmi	r8, r3
	movmi	r2, r4
	movmi	r3, r5
	movmi	r4, r6
	movmi	r5, r8

	mrs	r11, cpsr   /*copy cpsr in to r11 */
	subs    r1, r1, r11, lsr #28
	ldmeq	sp!, {r8, r10-r11} /* restores registers from stack */
	ldmeq 	sp!, {r4-r7, pc}   /* restore & return from subroutine */

/* By the time we reach here, we are 16-byte aligned and r2-r5 contains       */
/* rotated pattern. Now lets make sure we are 32-byte aligned.                */
L_16ByteAligned: 
	tst	r12, #(1 << 4)
	stmne	r12!, {r2-r5}
	subsne	r1, r1, #16

/* By the time we reach here, data is stored in registers as follows:         */
/* 	r1    << 'len' (remaining length of destination buffer in bytes)      */
/* 	r2-r5 << rotated pattern; either 4x4byte OR 2x8byte OR 1x16-byte      */
/*      r12   << aligned destination pointer copy (scratch register)          */
L_Aligned: 
	cmp	r1, #64
	blt	L_AlignedLessThan64

/* Copy pattern in four more registers so that we can do 64 byte transfers */
        mov	r6, r2
        mov	r8, r3
        mov	r10, r4
        mov	r11, r5

/* At this point, we are 16-byte aligned and 'len' is greater than 64 bytes   */
/* Lets transfer 64 bytes at a time until len becomes less than 64 bytes      */
	sub	r1, r1, #64  /* pre-subtract to avoid extra compare in loop   */
L_Loop64:
	stm	r12!, {r2-r6, r8, r10-r11}
	subs	r1, r1, #64
	stm	r12!, {r2-r6, r8, r10-r11}
	bge	L_Loop64

	/* return if 'len' is zero */
	adds    r1, r1, #64 /* readjust length; previously subtracted extra 64*/
	ldmeq	sp!, {r8, r10-r11} /* restores registers from stack */
	ldmeq 	sp!, {r4-r7, pc}   /* restore & return from subroutine */

L_AlignedLessThan64:
        /* do we have 16 or more bytes left */
        cmp	r1, #16
        stmge	r12!, {r2-r5}
        subsge 	r1, r1, #16
        bgt	L_AlignedLessThan64
	ldmeq	sp!, {r8, r10-r11} /* restores registers from stack */
	ldmeq 	sp!, {r4-r7, pc}   /* restore & return from subroutine */

L_AlignedLessThan16:
        /* store last up-to 15 bytes */
	/* move the remaining len bits [3:0] to the flags area of cpsr */
        mov	r1, r1, lsl #28 
        msr	cpsr_f, r1

        stmmi 	r12!, {r2-r3} /* n is set, store 8 bytes */
	movmi   r2, r4        /* shift vector down 8 bytes */
	movmi   r3, r5

        streq   r2, [r12], #4 /* z is set, store 4 bytes */
	moveq   r2, r3        /* shift vector down 4 bytes */

        strhcs  r2, [r12], #2 /* c is set, store 2 bytes */
	lsrcs   r2, #16       /* shift register right 2 bytes */

        strbvs  r2, [r12], #1 /* v is set, store 1 byte */
	ldm	sp!, {r8, r10-r11} /* restores registers from stack */
	ldm 	sp!, {r4-r7, pc}   /* restore & return from subroutine */

/*----------------------------------------------------------------------------*/
/* void     memset_pattern8(void *ptr, const void *pattern8, size_t len);     */
/*                                                                            */
/*      r0    << destination pointer                                          */
/* 	r1    << pointer to 8-byte pattern                                    */
/* 	r2    << 'len' (length of destination buffer in bytes)                */
/*----------------------------------------------------------------------------*/
	.globl _memset_pattern8
_memset_pattern8:
        cmp     r2, #0  /* check if len is zero */
        bxeq    lr      /* return if length is zero */

        /* We need some registers, so save volatiles on stack */
        /* Avoid r7 (frame pointer) and r9 (thread register)  */
        stmfd   sp!, {r4-r7, lr}
        add     r7, sp, #12        /* establish frame */
        stmfd   sp!, {r8, r10-r11}

        /* copy destination base pointer r0 to r12 and leave r0 alone */
        /* so that we return original pointer back to the caller */
        mov     r12, r0

        /* Check if 'len' is long enough to bother alignment of destination */
        /* pointer */
        cmp     r2, #32     /* long enough to bother aligning? */
        movlt   r3, #8      /* move pattern length into r3 */
        movlt   r10, #8     /* pattern index */
        movlt   r11, r1     /* move pattern pointer into r11 */
        blt     L_Short     /* no */

        /* move 'len' into r1, get 8-byte pattern in r2-r3 */
        mov     r6, r2       /* temporarily move 'len' in to r6 */
        ldr     r2, [r1], #4 /* load 8-byte pattern into r2-r3 */
        ldr     r3, [r1], #4 
        mov     r1, r6  /* move 'len' from r6 to r1 */

        mov     r4, r2  /* copy 8-byte pattern into r4-r5 registers */
        mov     r5, r3                                                        
	b 	L_NotShort  /* yes */


/*----------------------------------------------------------------------------*/
/* void     memset_pattern16(void *ptr, const void *pattern16, size_t len);   */
/*                                                                            */
/*      r0    << destination pointer                                          */
/* 	r1    << pointer to 16-byte pattern                                   */
/* 	r2    << 'len' (length of destination buffer in bytes)                */
/*----------------------------------------------------------------------------*/
	.globl _memset_pattern16
_memset_pattern16:
        cmp     r2, #0  /* check if len is zero */
        bxeq    lr      /* return if length is zero */

        /* We need some registers, so save volatiles on stack */
        /* Avoid r7 (frame pointer) and r9 (thread register)  */
        stmfd   sp!, {r4-r7, lr}                                              
        add     r7, sp, #12        /* establish frame */                      
        stmfd   sp!, {r8, r10-r11}

        /* copy destination base pointer r0 to r12 and leave r0 alone */
        /* so that we return original pointer back to the caller */
        mov     r12, r0
        
        /* Check if 'len' is long enough to bother alignment of destination */
        /* pointer */
        cmp     r2, #32     /* long enough to bother aligning? */
        movlt   r3, #16      /* move pattern length into r3 */ 
        movlt   r10, #16     /* pattern index */
        movlt   r11, r1     /* move pattern pointer into r11 */
        blt     L_Short     /* no */

        /* move 'len' into r1, get 16-byte pattern in r2-r5 */
        mov     r6, r2       /* temporarily move 'len' in to r6 */
        ldr     r2, [r1], #4 /* load 16-byte pattern into r2-r5 */
        ldr     r3, [r1], #4 
        ldr     r4, [r1], #4 
        ldr     r5, [r1], #4 
        mov     r1, r6  /* move 'len' from r6 to r1 */

        b       L_NotShort  /* yes */


#endif /* _ARM_ARCH_6 */
