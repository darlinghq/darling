/*
 * Copyright (c) 2006, 2009 Apple Inc. All rights reserved.
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

/*****************************************************************************
 * ARMv5 and ARMv6 implementation, also used in dyld on later archs          *
 *****************************************************************************/
 
#include <arm/arch.h>
#if !defined _ARM_ARCH_7 || defined VARIANT_DYLD

.text
.align 2
	
	.globl _memcpy
	.globl _bcopy
	.globl _memmove

_bcopy:		/* void bcopy(const void *src, void *dest, size_t len); */
	mov		r3, r0
	mov		r0, r1
	mov		r1, r3

_memcpy:		/* void *memcpy(void *dest, const void *src, size_t len); */
_memmove: 	/* void *memmove(void *dest, const void *src, size_t len); */
	/* check for zero len or if the pointers are the same */
	cmp		r2, #0
	cmpne	r0, r1
	bxeq	lr

	/* save r0 (return value), r4 (scratch), and r5 (scratch) */
	stmfd	sp!, { r0, r4, r5, r7, lr }
	add	r7, sp, #12
	
	/* check for overlap. r3 <- distance between src & dest */
	subhs	r3, r0, r1
	sublo	r3, r1, r0
	cmp		r3, r2			/* if distance(src, dest) < len, we have overlap */
	blo		Loverlap

Lnormalforwardcopy:
	/* are src and dest dissimilarly word aligned? */
	mov		r12, r0, lsl #30
	cmp		r12, r1, lsl #30
	bne		Lnonwordaligned_forward

	/* if len < 64, do a quick forward copy */
	cmp		r2, #64
	blt		Lsmallforwardcopy

	/* check for 16 byte src/dest unalignment */
	tst		r0, #0xf
	bne		Lsimilarlyunaligned

	/* check for 32 byte dest unalignment */
	tst		r0, #(1<<4)
	bne		Lunaligned_32

Lmorethan64_aligned:
	/* save some more registers to use in the copy */
	stmfd	sp!, { r6, r8, r10, r11 }

	/* pre-subtract 64 from the len counter to avoid an extra compare in the loop */
	sub		r2, r2, #64

L64loop:
	/* copy 64 bytes at a time */
	ldmia	r1!, { r3, r4, r5, r6, r8, r10, r11, r12 }
#ifdef _ARM_ARCH_6
	pld		[r1, #32]
#endif
	stmia	r0!, { r3, r4, r5, r6, r8, r10, r11, r12 }
	ldmia	r1!, { r3, r4, r5, r6, r8, r10, r11, r12 }
	subs	r2, r2, #64
#ifdef _ARM_ARCH_6
	pld		[r1, #32]
#endif
	stmia	r0!, { r3, r4, r5, r6, r8, r10, r11, r12 }
	bge		L64loop

	/* restore the scratch registers we just saved */
	ldmfd	sp!, { r6, r8, r10, r11 }

	/* fix up the len counter (previously subtracted an extra 64 from it) and test for completion */
	adds	r2, r2, #64
	beq		Lexit

Llessthan64_aligned:
	/* copy 16 bytes at a time until we have < 16 bytes */
	cmp		r2, #16
	ldmgeia	r1!, { r3, r4, r5, r12 }
	stmgeia	r0!, { r3, r4, r5, r12 }
	subges	r2, r2, #16
	bgt		Llessthan64_aligned
	beq		Lexit
	
Llessthan16_aligned:
	mov		r2, r2, lsl #28
	msr		cpsr_f, r2

	ldmmiia	r1!, { r2, r3 }
	ldreq	r4, [r1], #4
	ldrcsh	r5, [r1], #2
	ldrvsb	r12, [r1], #1

	stmmiia	r0!, { r2, r3 }
	streq	r4, [r0], #4
	strcsh	r5, [r0], #2
	strvsb	r12, [r0], #1
	b		Lexit

Lsimilarlyunaligned:
	/* both src and dest are unaligned in similar ways, align to dest on 32 byte boundary */
	mov		r12, r0, lsl #28
	rsb		r12, r12, #0
	msr		cpsr_f, r12

	ldrvsb	r3, [r1], #1
	ldrcsh	r4, [r1], #2
	ldreq	r5, [r1], #4

	strvsb	r3, [r0], #1
	strcsh	r4, [r0], #2
	streq	r5, [r0], #4

	ldmmiia	r1!, { r3, r4 }
	stmmiia	r0!, { r3, r4 }

	subs	r2, r2, r12, lsr #28
	beq		Lexit

Lunaligned_32:
	/* bring up to dest 32 byte alignment */
	tst		r0, #(1 << 4)
	ldmneia	r1!, { r3, r4, r5, r12 }
	stmneia	r0!, { r3, r4, r5, r12 }
	subne	r2, r2, #16

	/* we should now be aligned, see what copy method we should use */
	cmp		r2, #64
	bge		Lmorethan64_aligned
	b		Llessthan64_aligned
	
Lbytewise2:
	/* copy 2 bytes at a time */
	subs	r2, r2, #2

	ldrb	r3, [r1], #1
	ldrplb	r4, [r1], #1

	strb	r3, [r0], #1
	strplb	r4, [r0], #1

	bhi		Lbytewise2
	b		Lexit

Lbytewise:
	/* simple bytewise forward copy */
	ldrb	r3, [r1], #1
	subs	r2, r2, #1
	strb	r3, [r0], #1
	bne		Lbytewise
	b		Lexit

Lsmallforwardcopy:
	/* src and dest are word aligned similarly, less than 64 bytes to copy */
	cmp		r2, #4
	blt		Lbytewise2

	/* bytewise copy until word aligned */
	tst		r1, #3
Lwordalignloop:
	ldrneb	r3, [r1], #1
	strneb	r3, [r0], #1
	subne	r2, r2, #1
	tstne	r1, #3
	bne		Lwordalignloop

	cmp		r2, #16
	bge		Llessthan64_aligned
	blt		Llessthan16_aligned

Loverlap:
	/* src and dest overlap in some way, len > 0 */
	cmp		r0, r1				/* if dest > src */
	bhi		Loverlap_srclower

Loverlap_destlower:
	/* dest < src, see if we can still do a fast forward copy or fallback to slow forward copy */
	cmp		r3, #64
	bge		Lnormalforwardcopy 	/* overlap is greater than one stride of the copy, use normal copy */

	cmp		r3, #2
	bge		Lbytewise2
	b		Lbytewise

	/* the following routines deal with having to copy in the reverse direction */
Loverlap_srclower:
	/* src < dest, with overlap */

	/* src += len; dest += len; */
	add		r0, r0, r2
	add		r1, r1, r2

	/* we have to copy in reverse no matter what, test if we can we use a large block reverse copy */
	cmp		r2, #64				/* less than 64 bytes to copy? */
	cmpgt	r3, #64				/* less than 64 bytes of nonoverlap? */
	blt		Lbytewise_reverse

	/* test of src and dest are nonword aligned differently */
	mov		r3, r0, lsl #30
	cmp		r3, r1, lsl #30
	bne		Lbytewise_reverse

	/* test if src and dest are non word aligned or dest is non 16 byte aligned */
	tst		r0, #0xf
	bne		Lunaligned_reverse_similarly

	/* test for dest 32 byte alignment */
	tst		r0, #(1<<4)
	bne		Lunaligned_32_reverse_similarly

	/* 64 byte reverse block copy, src and dest aligned */
Lmorethan64_aligned_reverse:
	/* save some more registers to use in the copy */
	stmfd	sp!, { r6, r8, r10, r11 }

	/* pre-subtract 64 from the len counter to avoid an extra compare in the loop */
	sub		r2, r2, #64

L64loop_reverse:
	/* copy 64 bytes at a time */
	ldmdb	r1!, { r3, r4, r5, r6, r8, r10, r11, r12 }
#ifdef _ARM_ARCH_6
	pld		[r1, #-32]
#endif
	stmdb	r0!, { r3, r4, r5, r6, r8, r10, r11, r12 }	
	ldmdb	r1!, { r3, r4, r5, r6, r8, r10, r11, r12 }	
	subs	r2, r2, #64
#ifdef _ARM_ARCH_6
	pld		[r1, #-32]
#endif
	stmdb	r0!, { r3, r4, r5, r6, r8, r10, r11, r12 }	
	bge		L64loop_reverse

	/* restore the scratch registers we just saved */
	ldmfd	sp!, { r6, r8, r10, r11 }

	/* fix up the len counter (previously subtracted an extra 64 from it) and test for completion */
	adds	r2, r2, #64
	beq		Lexit

Lbytewise_reverse:
	ldrb	r3, [r1, #-1]!
	strb	r3, [r0, #-1]!
	subs	r2, r2, #1
	bne		Lbytewise_reverse
	b		Lexit

Lunaligned_reverse_similarly:
	/* both src and dest are unaligned in similar ways, align to dest on 32 byte boundary */
	mov		r12, r0, lsl #28
	msr		cpsr_f, r12

	ldrvsb	r3, [r1, #-1]!
	ldrcsh	r4, [r1, #-2]!
	ldreq	r5, [r1, #-4]!

	strvsb	r3, [r0, #-1]!
	strcsh	r4, [r0, #-2]!
	streq	r5, [r0, #-4]!

	ldmmidb	r1!, { r3, r4 }
	stmmidb	r0!, { r3, r4 }

	subs	r2, r2, r12, lsr #28
	beq		Lexit

Lunaligned_32_reverse_similarly:
	/* bring up to dest 32 byte alignment */
	tst		r0, #(1 << 4)
	ldmnedb	r1!, { r3, r4, r5, r12 }
	stmnedb	r0!, { r3, r4, r5, r12 }
	subne	r2, r2, #16

	/* we should now be aligned, see what copy method we should use */
	cmp		r2, #64
	bge		Lmorethan64_aligned_reverse
	b		Lbytewise_reverse

	/* the following routines deal with non word aligned copies */
Lnonwordaligned_forward:
	cmp		r2, #8
	blt		Lbytewise2			/* not worth the effort with less than 24 bytes total */

	/* bytewise copy until src word aligned */
	tst		r1, #3
Lwordalignloop2:
	ldrneb	r3, [r1], #1
	strneb	r3, [r0], #1
	subne	r2, r2, #1
	tstne	r1, #3
	bne		Lwordalignloop2

	/* figure out how the src and dest are unaligned */
	and		r3, r0, #3
	cmp		r3, #2
	blt		Lalign1_forward
	beq		Lalign2_forward
	bgt		Lalign3_forward

Lalign1_forward:
	/* the dest pointer is 1 byte off from src */
	mov		r12, r2, lsr #2		/* number of words we should copy */
	sub		r0, r0, #1

	/* prime the copy */
	ldrb	r4, [r0]			/* load D[7:0] */

Lalign1_forward_loop:
	ldr		r3, [r1], #4		/* load S */
	orr		r4, r4, r3, lsl #8	/* D[31:8] = S[24:0] */
	str		r4, [r0], #4		/* save D */
	mov		r4, r3, lsr #24		/* D[7:0] = S[31:25] */
	subs	r12, r12, #1
	bne		Lalign1_forward_loop

	/* finish the copy off */
	strb	r4, [r0], #1		/* save D[7:0] */

	ands	r2, r2, #3
	beq		Lexit
	b		Lbytewise2

Lalign2_forward:
	/* the dest pointer is 2 bytes off from src */
	mov		r12, r2, lsr #2		/* number of words we should copy */
	sub		r0, r0, #2

	/* prime the copy */
	ldrh	r4, [r0]			/* load D[15:0] */

Lalign2_forward_loop:
	ldr		r3, [r1], #4		/* load S */
	orr		r4, r4, r3, lsl #16	/* D[31:16] = S[15:0] */
	str		r4, [r0], #4		/* save D */
	mov		r4, r3, lsr #16		/* D[15:0] = S[31:15] */
	subs	r12, r12, #1
	bne		Lalign2_forward_loop

	/* finish the copy off */
	strh	r4, [r0], #2		/* save D[15:0] */

	ands	r2, r2, #3
	beq		Lexit
	b		Lbytewise2

Lalign3_forward:
	/* the dest pointer is 3 bytes off from src */
	mov		r12, r2, lsr #2		/* number of words we should copy */
	sub		r0, r0, #3

	/* prime the copy */
	ldr		r4, [r0]
	and		r4, r4, #0x00ffffff	/* load D[24:0] */

Lalign3_forward_loop:
	ldr		r3, [r1], #4		/* load S */
	orr		r4, r4, r3, lsl #24	/* D[31:25] = S[7:0] */
	str		r4, [r0], #4		/* save D */
	mov		r4, r3, lsr #8		/* D[24:0] = S[31:8] */
	subs	r12, r12, #1
	bne		Lalign3_forward_loop

	/* finish the copy off */
	strh	r4, [r0], #2		/* save D[15:0] */
	mov		r4, r4, lsr #16
	strb	r4, [r0], #1		/* save D[23:16] */

	ands	r2, r2, #3
	beq		Lexit
	b		Lbytewise2

Lexit:
	ldmfd	sp!, {r0, r4, r5, r7, pc}

#endif // !defined _ARM_ARCH_7 || defined VARIANT_DYLD

