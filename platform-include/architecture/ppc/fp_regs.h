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
 *	File:	architecture/ppc/fp_regs.h
 *	Author:	Doug Mitchell, NeXT Software, Inc.
 *
 *	ppc floating point registers.
 *
 * HISTORY
 * 29-Dec-96  Umesh Vaishampayan  (umeshv@NeXT.com)
 *	Ported from m98k.
 * 05-Nov-92  Doug Mitchell at NeXT
 *	Created.
 */

#ifndef _ARCH_PPC_FP_REGS_H_
#define _ARCH_PPC_FP_REGS_H_

#include <architecture/ppc/reg_help.h>

#if !defined(__ASSEMBLER__)
/*
 * Floating point status and control register.
 *
 * This struct is aligned to an 8-byte boundary because 64-bit
 * load/store instructions (lfd/stfd) are used to access it. The
 * FPSCR can only be read/written through other FP registers.
 */
typedef struct {
        unsigned        unused[1] __attribute__(( aligned(8) ));
	unsigned	fx:BIT_WIDTH(31),	// exception summary
			fex:BIT_WIDTH(30),	// enabled exception summary
			vx:BIT_WIDTH(29),	// invalid op exception
						//    summary
			ox:BIT_WIDTH(28),	// overflow exception
			ux:BIT_WIDTH(27),	// underflow exception
			zx:BIT_WIDTH(26),	// divide by zero exception
			xx:BIT_WIDTH(25),	// inexact exception
			vx_snan:BIT_WIDTH(24),	// not a number exception
			vx_isi:BIT_WIDTH(23),	// exception
			vx_idi:BIT_WIDTH(22),	// exception
			vx_zdz:BIT_WIDTH(21),	// exception
			vx_imz:BIT_WIDTH(20),	// exception
			vx_xvc:BIT_WIDTH(19),	// exception
			fr:BIT_WIDTH(18),	// fraction rounded
			fi:BIT_WIDTH(17),	// fraction inexact
			class:BIT_WIDTH(16),	// class descriptor
			fl:BIT_WIDTH(15),	// negative
			fg:BIT_WIDTH(14),	// positive
			fe:BIT_WIDTH(13),	// equal or zero
			fu:BIT_WIDTH(12),	// not a number
			rsvd1:BIT_WIDTH(11),	// reserved
			vx_soft:BIT_WIDTH(10),	// software request exception
			rsvd2:BIT_WIDTH(9),	// reserved
			vx_cvi:BIT_WIDTH(8),	// invalid integer convert
						//    exception
			ve:BIT_WIDTH(7),	// invalid op exception enable
			oe:BIT_WIDTH(6),	// overflow exception enable
			ue:BIT_WIDTH(5),	// underflow exception enable
			ze:BIT_WIDTH(4),	// divide by zero exception
						//    enable
			xe:BIT_WIDTH(3),	// inexact exception enable
			ni:BIT_WIDTH(2),	// non-IEEE exception enable
			rn:BITS_WIDTH(1,0);	// rounding control
} ppc_fp_scr_t;

/*
 * Values for fp_scr_t.rn (rounding control).
 */
typedef enum {
	RN_NEAREST = 0,
	RN_TOWARD_ZERO = 1,
	RN_TOWARD_PLUS = 2,
	RN_TOWARD_MINUS = 3
} ppc_fp_rn_t;

/*
 * ppc_fpf_t -- data types that MAY be in floating point register file
 * Actual data types supported is implementation dependent
 */
typedef union {
        float           f;              // 32 bit IEEE single
        double          d;              // 64 bit IEEE double
     
        /* 
	 * Insure compiler aligns struct appropriately 
	 */
        unsigned        x[2] __attribute__(( aligned(8) ));
} ppc_fpf_t;

/*
 * Number of FP registers.
 */
#define PPC_NFP_REGS	32

/*
 * Read/write FPSCR.
 * FIXME - these don't work, you need to go thru a fp register.
 */
typedef union {
	double 		__dbl;
	ppc_fp_scr_t 	__scr;
} __fp_un_t;

static __inline__ ppc_fp_scr_t
get_fp_scr()
{
	__fp_un_t 	__fp_un;
	
	__asm__ volatile ("mffs. %0           /* mffs */"	\
	  	: "=f" (__fp_un.__dbl));
	return (__fp_un.__scr);		
}

static __inline__ void
set_fp_scr(ppc_fp_scr_t fp_scr)
{
	__fp_un_t 	__fp_un;

	__fp_un.__scr = fp_scr;
	__asm__ volatile ("mtfsf 0xff, %0;    /* mtfsf */ "	\
	  : : "f" (__fp_un.__dbl));	
}

#endif /* ! __ASSEMBLER__ */

#endif /* _ARCH_PPC_FP_REGS_H_ */
