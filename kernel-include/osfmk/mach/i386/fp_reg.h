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
/*
 * @OSF_COPYRIGHT@
 */
/*
 * Mach Operating System
 * Copyright (c) 1992-1989 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 */

#ifndef _I386_FP_SAVE_H_
#define _I386_FP_SAVE_H_

#ifdef  MACH_KERNEL_PRIVATE


struct  x86_fx_thread_state {
	unsigned short  fx_control;     /* control */
	unsigned short  fx_status;      /* status */
	unsigned char   fx_tag;         /* register tags */
	unsigned char   fx_bbz1;        /* better be zero when calling fxrtstor */
	unsigned short  fx_opcode;
	union {
		struct {                /* 32-bit layout: */
			unsigned int    fx_eip; /* eip  instruction */
			unsigned short  fx_cs;  /* cs instruction */
			unsigned short  fx_bbz2;/* better be zero when calling fxrtstor */
			unsigned int    fx_dp;  /* data address */
			unsigned short  fx_ds;  /* data segment */
			unsigned short  fx_bbz3;/* better be zero when calling fxrtstor */
		};
		struct {                /* 64-bit layout: */
			uint64_t        fx_rip; /* instruction pointer */
			uint64_t        fx_rdp; /* data pointer */
		};
	};
	unsigned int    fx_MXCSR;
	unsigned int    fx_MXCSR_MASK;
	unsigned short  fx_reg_word[8][8];      /* STx/MMx registers */
	unsigned short  fx_XMM_reg[8][16];      /* XMM0-XMM15 on 64 bit processors */
	                                        /* XMM0-XMM7  on 32 bit processors... unused storage reserved */

	unsigned char   fx_reserved[16 * 5];      /* reserved by intel for future
	                                           * expansion */
	unsigned int    fp_valid;
	unsigned int    fp_save_layout;
	unsigned char   fx_pad[8];
}__attribute__ ((packed));

struct  xsave_header {
	uint64_t        xstate_bv;
	uint64_t        xcomp_bv;
	uint8_t         xhrsvd[48];
};

typedef struct { uint64_t lo64, hi64;  }__attribute__ ((packed)) reg128_t;
typedef struct { reg128_t lo128, hi128; }__attribute__ ((packed)) reg256_t;
typedef struct { reg256_t lo256, hi256; }__attribute__ ((packed)) reg512_t;

struct x86_avx_thread_state {
	struct x86_fx_thread_state      fp;
	struct xsave_header             _xh;                    /* Offset 512, xsave header */
	reg128_t                        x_YMM_Hi128[16];        /* Offset 576, high YMMs `*/
	                                                        /* Offset 832, end */
}__attribute__ ((packed));

struct x86_avx512_thread_state {
	struct x86_fx_thread_state      fp;
	struct xsave_header             _xh;                    /* Offset 512, xsave header */
	reg128_t                        x_YMM_Hi128[16];        /* Offset 576, high YMMs */

	uint64_t                        x_pad[16];              /* Offset 832, unused AMD LWP */
	uint64_t                        x_BNDREGS[8];           /* Offset 960, unused MPX */
	uint64_t                        x_BNDCTL[8];            /* Offset 1024, unused MPX */

	uint64_t                        x_Opmask[8];            /* Offset 1088, K0-K7 */
	reg256_t                        x_ZMM_Hi256[16];        /* Offset 1152, ZMM0..15[511:256] */
	reg512_t                        x_Hi16_ZMM[16];         /* Offset 1664, ZMM16..31[511:0] */
	                                                        /* Offset 2688, end */
}__attribute__ ((packed));

typedef union {
	struct x86_fx_thread_state      fx;
	struct x86_avx_thread_state     avx;
#if !defined(RC_HIDE_XNU_J137)
	struct x86_avx512_thread_state  avx512;
#endif
} x86_ext_thread_state_t;

#define EVEX_PREFIX     0x62            /* AVX512's EVEX vector operation prefix */
#define VEX2_PREFIX     0xC5            /* VEX 2-byte prefix for Opmask instructions */
#define VEX3_PREFIX     0xC4            /* VEX 3-byte prefix for Opmask instructions */

#endif /* MACH_KERNEL_PRIVATE */
/*
 * Control register
 */
#define FPC_IE          0x0001          /* enable invalid operation
	                                 *  exception */
#define FPC_IM          FPC_IE
#define FPC_DE          0x0002          /* enable denormalized operation
	                                 *  exception */
#define FPC_DM          FPC_DE
#define FPC_ZE          0x0004          /* enable zero-divide exception */
#define FPC_ZM          FPC_ZE
#define FPC_OE          0x0008          /* enable overflow exception */
#define FPC_OM          FPC_OE
#define FPC_UE          0x0010          /* enable underflow exception */
#define FPC_PE          0x0020          /* enable precision exception */
#define FPC_PC          0x0300          /* precision control: */
#define FPC_PC_24       0x0000                  /* 24 bits */
#define FPC_PC_53       0x0200                  /* 53 bits */
#define FPC_PC_64       0x0300                  /* 64 bits */
#define FPC_RC          0x0c00          /* rounding control: */
#define FPC_RC_RN       0x0000                  /* round to nearest or even */
#define FPC_RC_RD       0x0400                  /* round down */
#define FPC_RC_RU       0x0800                  /* round up */
#define FPC_RC_CHOP     0x0c00                  /* chop */
#define FPC_IC          0x1000          /* infinity control (obsolete) */
#define FPC_IC_PROJ     0x0000                  /* projective infinity */
#define FPC_IC_AFF      0x1000                  /* affine infinity (std) */

/*
 * Status register
 */
#define FPS_IE          0x0001          /* invalid operation */
#define FPS_DE          0x0002          /* denormalized operand */
#define FPS_ZE          0x0004          /* divide by zero */
#define FPS_OE          0x0008          /* overflow */
#define FPS_UE          0x0010          /* underflow */
#define FPS_PE          0x0020          /* precision */
#define FPS_SF          0x0040          /* stack flag */
#define FPS_ES          0x0080          /* error summary */
#define FPS_C0          0x0100          /* condition code bit 0 */
#define FPS_C1          0x0200          /* condition code bit 1 */
#define FPS_C2          0x0400          /* condition code bit 2 */
#define FPS_TOS         0x3800          /* top-of-stack pointer */
#define FPS_TOS_SHIFT   11
#define FPS_C3          0x4000          /* condition code bit 3 */
#define FPS_BUSY        0x8000          /* FPU busy */

/*
 * Kind of floating-point support provided by kernel.
 */
#define FP_NO           0               /* no floating point */
#define FP_SOFT         1               /* software FP emulator */
#define FP_287          2               /* 80287 */
#define FP_387          3               /* 80387 or 80486 */
#define FP_FXSR         4               /* Fast save/restore SIMD Extension */

#endif  /* _I386_FP_SAVE_H_ */
