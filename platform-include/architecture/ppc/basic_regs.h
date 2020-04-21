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
 *	File:	architecture/ppc/basic_regs.h
 *	Author:	Doug Mitchell, NeXT Software, Inc.
 *
 *	Basic ppc registers.
 *
 * HISTORY
 * 22-May-97  Umesh Vaishampayan  (umeshv@apple.com)
 	Updated to match MPCFPE32B/AD 1/97 REV. 1 
 * 29-Dec-96  Umesh Vaishampayan  (umeshv@NeXT.com)
 *	Ported from m98k.
 * 05-Nov-92  Doug Mitchell at NeXT
 *	Created.
 */

#ifndef _ARCH_PPC_BASIC_REGS_H_
#define _ARCH_PPC_BASIC_REGS_H_

#include <architecture/ppc/reg_help.h>
#include <architecture/ppc/macro_help.h>

#if !defined(__ASSEMBLER__)

/*
 * Number of General Purpose registers.
 */
#define PPC_NGP_REGS	32

/*
 * Common half-word used in Machine State Register and in 
 * various exception frames. Defined as a macro because the compiler
 * will align a struct to a word boundary when used inside another struct.
 */
#define MSR_BITS							   \
	unsigned	ee:BIT_WIDTH(15),	/* external intr enable    */ \
			pr:BIT_WIDTH(14),	/* problem state	   */ \
			fp:BIT_WIDTH(13),	/* floating point avail	   */ \
			me:BIT_WIDTH(12),	/* machine check enable	   */ \
			fe0:BIT_WIDTH(11),	/* fp exception mode 0	   */ \
			se:BIT_WIDTH(10),	/* single step enable	   */ \
			be:BIT_WIDTH(9),	/* branch trace enable	   */ \
			fe1:BIT_WIDTH(8),	/* fp exception mode 0	   */ \
			rsvd1:BIT_WIDTH(7),	/* reserved		   */ \
			ip:BIT_WIDTH(6),	/* interrupt prefix	   */ \
			ir:BIT_WIDTH(5),	/* instruction relocate	   */ \
			dr:BIT_WIDTH(4),	/* data relocate	   */ \
			rsvd2:BITS_WIDTH(3,2),	/* reserved		   */ \
			ri:BIT_WIDTH(1),	/* recoverable exception   */ \
			le:BIT_WIDTH(0)		/* Little-endian mode	   */

/*
 * Machine state register.
 * Read and written via get_msr() and set_msr() inlines, below.
 */
typedef struct {
	unsigned	rsvd3:BITS_WIDTH(31,19),	// reserved
			pow:BIT_WIDTH(18),	// Power management enable
			rsvd0: BIT_WIDTH(17),	// reserved
			ile: BIT_WIDTH(16);	// exception little endian

			MSR_BITS;			// see above
} msr_t;

/*
 * Data Storage Interrupt Status Register (DSISR)
 */
typedef struct {
	unsigned	dse:BIT_WIDTH(31);	// direct-store error
	unsigned	tnf:BIT_WIDTH(30);	// translation not found
	unsigned	:BITS_WIDTH(29,28);
	unsigned	pe:BIT_WIDTH(27);	// protection error
	unsigned	dsr:BIT_WIDTH(26);	// lwarx/stwcx to direct-store
	unsigned	rw:BIT_WIDTH(25);	// 1 => store, 0 => load
	unsigned	:BITS_WIDTH(24,23);
	unsigned	dab:BIT_WIDTH(22);	// data address bkpt (601)
	unsigned	ssf:BIT_WIDTH(21);	// seg table search failed
	unsigned	:BITS_WIDTH(20,0);
} dsisr_t;

/*
 * Instruction Storage Interrupt Status Register (really SRR1)
 */
typedef struct {
	unsigned	:BIT_WIDTH(31);
	unsigned	tnf:BIT_WIDTH(30);	// translation not found
	unsigned	:BIT_WIDTH(29);
	unsigned	dse:BIT_WIDTH(28);	// direct-store fetch error
	unsigned	pe:BIT_WIDTH(27);	// protection error
	unsigned	:BITS_WIDTH(26,22);
	unsigned	ssf:BIT_WIDTH(21);	// seg table search failed
	unsigned	:BITS_WIDTH(20,16);
	MSR_BITS;
} isisr_t;

/*
 * Alignment Interrupt Status Register (really DSISR)
 * NOTE: bit numbers in field *names* are in IBM'ese (0 is MSB).
 * FIXME: Yuck!!! Double Yuck!!!
 */
typedef struct {
	unsigned	:BITS_WIDTH(31,20);
	unsigned	ds3031:BITS_WIDTH(19,18);// bits 30:31 if DS form
	unsigned	:BIT_WIDTH(17);
	unsigned	x2930:BITS_WIDTH(16,15); // bits 29:30 if X form
	unsigned	x25:BIT_WIDTH(14);	 // bit 25 if X form or
						 // bit 5 if D or DS form
	unsigned	x2124:BITS_WIDTH(13,10); // bits 21:24 if X form or
						 // bits 1:4 if D or DS form
	unsigned	all615:BITS_WIDTH(9,0);	 // bits 6:15 of instr
	MSR_BITS;
} aisr_t;

/*
 * Program Interrupt Status Register (really SRR1)
 */
typedef struct {
	unsigned	:BITS_WIDTH(31,21);
	unsigned	fpee:BIT_WIDTH(20);	// floating pt enable exception
	unsigned	ill:BIT_WIDTH(19);	// illegal instruction
	unsigned	priv:BIT_WIDTH(18);	// privileged instruction
	unsigned	trap:BIT_WIDTH(17);	// trap program interrupt
	unsigned	subseq:BIT_WIDTH(16);	// 1 => SRR0 points to
						// subsequent instruction
	MSR_BITS;
} pisr_t;

/*
 * Condition register. May not be useful in C, let's see...
 */
typedef struct {
	unsigned	lt:BIT_WIDTH(31),	// negative
			gt:BIT_WIDTH(30),	// positive
			eq:BIT_WIDTH(29),	// equal to zero
			so:BIT_WIDTH(28),	// summary overflow
			fx:BIT_WIDTH(27),	// floating point exception
			fex:BIT_WIDTH(26),	// fp enabled exception
			vx:BIT_WIDTH(25),	// fp invalid operation
						//    exception
			ox:BIT_WIDTH(24),	// fp overflow exception
			rsvd:BITS_WIDTH(23,0);	// reserved
} cr_t;

/*
 * Abstract values representing fe0:fe1.
 * See get_fp_exc_mode(), below.
 */
typedef enum {
	FEM_IGNORE_EXCEP,	// ignore exceptions
	FEM_IMPR_NONREC,	// imprecise nonrecoverable
	FEM_IMPR_RECOV,		// imprecise recoverable
	FEM_PRECISE
} fp_exc_mode_t;


/*
 * Special purpose registers.
 */
 
/*
 * Processor version register (special purpose register pvr).
 */
typedef struct {
	unsigned	version:BITS_WIDTH(31,16),	
			revision:BITS_WIDTH(15,0);
} pvr_t;

/*
 * Fixed point exception register (special purpose register xer)
 */
typedef struct {
	unsigned	so:BIT_WIDTH(31),	// summary overflow
			ov:BIT_WIDTH(30),	// overflow
			ca:BIT_WIDTH(29),	// carry
			rsvd1:BITS_WIDTH(28,7), // reserved
			byte_count:BITS_WIDTH(6,0);	
} xer_t;

/*
 * Inlines and macros to manipulate the above registers.
 */
 
/*
 * Get/set machine state register.
 */
static __inline__ msr_t
get_msr()
{
	msr_t	__msr_tmp;	
	__asm__ volatile ("mfmsr %0  /* mfmsr */" : "=r" (__msr_tmp));
	return __msr_tmp;
}

static __inline__ void
set_msr(msr_t msr)
{
	__asm__ volatile ("mtmsr %0 /* mtmsr */ " : : "r" (msr));	
}

/* 
 * Determine current fp_exc_mode_t given prog_mode.
 */
static __inline__ fp_exc_mode_t
get_fp_exc_mode(pmr_t pmr)
{
	if(pmr.fe0)
		return pmr.fe1 ? FEM_PRECISE : FEM_IMPR_RECOV;
	else
		return pmr.fe1 ? FEM_IMPR_NONREC : FEM_IGNORE_EXCEP;
}

/*
 * Software definitions for special purpose registers.
 * The same register is used as per_cpu data pointer and
 * vector base register. This requires that the vector
 * table be the first item in the per_cpu table.
 */
#define SR_EXCEPTION_TMP_LR	sprg0
#define SR_EXCEPTION_TMP_CR	sprg1
#define SR_EXCEPTION_TMP_AT	sprg2
#define SR_PER_CPU_DATA		sprg3
#define SR_VBR			sprg3

/*
 * Get/set special purpose registers.
 *
 * GET_SPR - get SPR by name.
 *
 * Example usage:
 *
 *   {
 *	xer_t	some_xer;
 *
 *	some_xer = GET_SPR(xer_t, xer);
 *	...
 *   }
 *
 * This is a strange one. We're creating a list of C expressions within
 * a set of curlies; the last expression ("__spr_tmp;") is the return value
 * of the statement created by the curlies.
 *
 */
 
#define GET_SPR(type, spr)					\
({								\
	unsigned	__spr_tmp;				\
	__asm__ volatile ("mfspr %0, " STRINGIFY(spr) : "=r" (__spr_tmp));				\
	*(type *)&__spr_tmp;					\
})

/* 
 * Example usage of SET_SPR:
 *
 *   {
 *	xer_t some_xer;
 *
 *	...set up some_xer...
 *	SET_SPR(xer, some_xer);
 *   }
 */
#define	SET_SPR(spr, val)					\
MACRO_BEGIN							\
	__typeof__ (val) __spr_tmp = (val);			\
	__asm__ volatile ("mtspr "STRINGIFY(spr) ", %0" : : "r" (__spr_tmp));					\
MACRO_END

/*
 * Fully synchronize instruction stream.
 */
static __inline__ void
ppc_sync()
{
	__asm__ volatile ("sync         /* sync */" : : );
}

#endif /* ! __ASSEMBLER__ */

#endif /* _ARCH_PPC_BASIC_REGS_H_ */

