/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/* Copyright (c) 1996 NeXT Software, Inc.  All rights reserved.
 *
 *	File:	architecture/ppc/reg_help.h
 *	Author:	Doug Mitchell, NeXT Computer, Inc.
 *
 *	m98k-specific macros and inlines for defining machine registers.
 *
 * HISTORY
 * 05-Nov-92  Doug Mitchell at NeXT
 *	Created.
 *
 * 29-Dec-96  Umesh Vaishampayan  (umeshv@NeXT.com)
 *	Ported from m98k. Removed dependency on nrw directory.
 *	Merged code from architecture/nrw/reg_help.h.
 *	Moved Register Usage #defines from asm_help.h in here.
 */

#ifndef _ARCH_PPC_REG_HELP_H_
#define _ARCH_PPC_REG_HELP_H_

#if defined(__ASSEMBLER__)
/*
 * GRF Register Usage Aliases
 */
#define	zt		r0		// architecturally 0 for mem refs only!
					// real reg other inst, caller-saved
#define	sp		r1		// stack pointer, callee-saved
#define	toc		r2		// tbl of contents, callee-saved
#define	a0		r3		// arg 0, return value 0, caller saved
#define	a1		r4		// arg 1, return value 1, caller saved
#define	a2		r5		// ....
#define	a3		r6
#define	a4		r7
#define	a5		r8
#define	a6		r9
#define	a7		r10		// arg 7, return value 7, caller saved
#define	ep		r11		// environment ptr, caller saved
#define	at		r12		// assembler temp, caller saved
#define	s17		r13		// callee-saved 17
#define	s16		r14
#define	s15		r15
#define	s14		r16
#define	s13		r17
#define	s12		r18
#define	s11		r19
#define	s10		r20
#define	s9		r21
#define	s8		r22
#define	s7		r23
#define	s6		r24
#define	s5		r25
#define	s4		r26
#define	s3		r27
#define	s2		r28
#define	s1		r29		// ....
#define	s0		r30		// callee-saved 0
#define	fp		r31		// frame-pointer, callee-saved

/*
 * Conversion of GRF aliases to register numbers
 */
#define	GRF_ZT		0		// architecturally 0 for mem refs only!
					// real reg other inst, caller-saved
#define	GRF_SP		1		// stack pointer, callee-saved
#define	GRF_TOC		2		// tbl of contents, callee-saved
#define	GRF_A0		3		// arg 0, return value 0, caller saved
#define	GRF_A1		4		// arg 1, return value 1, caller saved
#define	GRF_A2		5		// ....
#define	GRF_A3		6
#define	GRF_A4		7
#define	GRF_A5		8
#define	GRF_A6		9
#define	GRF_A7		10		// arg 7, return value 7, caller saved
#define	GRF_EP		11		// environment ptr, caller saved
#define	GRF_AT		12		// assembler temp, caller saved
#define	GRF_S17		13		// callee-saved 17
#define	GRF_S16		14
#define	GRF_S15		15
#define	GRF_S14		16
#define	GRF_S13		17
#define	GRF_S12		18
#define	GRF_S11		19
#define	GRF_S10		20
#define	GRF_S9		21
#define	GRF_S8		22
#define	GRF_S7		23
#define	GRF_S6		24
#define	GRF_S5		25
#define	GRF_S4		26
#define	GRF_S3		27
#define	GRF_S2		28
#define	GRF_S1		29		// ....
#define	GRF_S0		30		// callee-saved 0
#define	GRF_FP		31		// frame pointer, callee-saved

/*
 * FPF Register names
 */
#define	ft0		f0		// scratch reg, caller-saved
#define	fa0		f1		// fp arg 0, return 0, caller-saved
#define	fa1		f2		// fp arg 1, caller-saved
#define	fa2		f3		// fp arg 2, caller-saved
#define	fa3		f4
#define	fa4		f5
#define	fa5		f6
#define	fa6		f7
#define	fa7		f8
#define	fa8		f9
#define	fa9		f10
#define	fa10		f11
#define	fa11		f12
#define	fa12		f13		// fp arg 12, caller-saved
#define	fs17		f14		// callee-saved 17
#define	fs16		f15
#define	fs15		f16
#define	fs14		f17
#define	fs13		f18
#define	fs12		f19
#define	fs11		f20
#define	fs10		f21
#define	fs9		f22
#define	fs8		f23
#define	fs7		f24
#define	fs6		f25
#define	fs5		f26
#define	fs4		f27
#define	fs3		f28
#define	fs2		f29
#define	fs1		f30
#define	fs0		f31		// callee-saved 0

/*
 * Conversion of FPF aliases to register numbers
 */
#define	FPF_FT0		0		// scratch reg, caller-saved
#define	FPF_FA0		1		// fp arg 0, return 0, caller-saved
#define	FPF_FA1		2		// fp arg 1, caller-saved
#define	FPF_FA2		3		// fp arg 2, caller-saved
#define	FPF_FA3		4
#define	FPF_FA4		5
#define	FPF_FA5		6
#define	FPF_FA6		7
#define	FPF_FA7		8
#define	FPF_FA8		9
#define	FPF_FA9		10
#define	FPF_FA10	11
#define	FPF_FA11	12
#define	FPF_FA12	13		// fp arg 12, caller-saved
#define	FPF_FS17	14		// callee-saved 17
#define	FPF_FS16	15
#define	FPF_FS15	16
#define	FPF_FS14	17
#define	FPF_FS13	18
#define	FPF_FS12	19
#define	FPF_FS11	20
#define	FPF_FS10	21
#define	FPF_FS9		22
#define	FPF_FS8		23
#define	FPF_FS7		24
#define	FPF_FS6		25
#define	FPF_FS5		26
#define	FPF_FS4		27
#define	FPF_FS3		28
#define	FPF_FS2		29
#define	FPF_FS1		30
#define	FPF_FS0		31		// callee-saved 0

#endif	/* __ASSEMBLER__ */


/* Bitfield definition aid */
#define	BITS_WIDTH(msb, lsb)	((msb)-(lsb)+1)
#define	BIT_WIDTH(pos)		(1)	/* mostly to record the position */

/* Mask creation */
#define	MKMASK(width, offset)	(((unsigned)-1)>>(32-(width))<<(offset))
#define	BITSMASK(msb, lsb)	MKMASK(BITS_WIDTH(msb, lsb), lsb & 0x1f)
#define	BITMASK(pos)		MKMASK(BIT_WIDTH(pos), pos & 0x1f)

/* Register addresses */
#if	__ASSEMBLER__
# define	REG_ADDR(type, addr)	(addr)
#else	/* ! __ASSEMBLER__ */
# define	REG_ADDR(type, addr)	(*(volatile type *)(addr))
#endif	/* __ASSEMBLER__ */

/* Cast a register to be an unsigned */
/* CAUTION :	non naturally aligned foo can result into alignment traps
 *		use at own risk.
 */
#define	CONTENTS(foo)	(*(unsigned *) &(foo))

/* STRINGIFY -- perform all possible substitutions, then stringify */
#define	__STR(x)	#x		/* just a helper macro */
#define	STRINGIFY(x)	__STR(x)

/* 
 * Stack pointer must always be a multiple of 16
 */
#define	STACK_INCR	16
#define	ROUND_FRAME(x)	((((unsigned)(x)) + STACK_INCR - 1) & ~(STACK_INCR-1))

#endif /* _ARCH_PPC_REG_HELP_H_ */
