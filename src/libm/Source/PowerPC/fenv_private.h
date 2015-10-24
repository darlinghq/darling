/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*******************************************************************************
*                                                                              *
*     File:  fenv_private.h                                                    *
*                                                                              *
*     Contains: Defines for manipulating environmental settings directly.      *
*                                                                              *
*******************************************************************************/

#ifndef __FENV_PRIVATE__
#define __FENV_PRIVATE__

/*  Pick up the publicly visible enums formerly sited here */
#include "fenv.h"

/*  Macros to get or set environment flags doubleword  */
#define      FEGETENVD(x) ({ __label__ L1, L2; L1: (void)&&L1; \
					asm volatile ("mffs %0" : "=f" (x)); \
                    L2: (void)&&L2; })
					
#define		 FESETENVD(x) ({ __label__ L1, L2; L1: (void)&&L1; \
                    asm volatile("mtfsf 255,%0" : : "f" (x)); \
                    L2: (void)&&L2; })

/*  Macros to get or set environment flags doubleword in their own dispatch group  */
#define      FEGETENVD_GRP(x)     ({ __label__ L1, L2; L1: (void)&&L1; \
									asm volatile ("mffs %0" : "=f" (x)); \
									L2: (void)&&L2; __NOOP; __NOOP; __NOOP; })
									
#define      FESETENVD_GRP(x)     ({ __label__ L1, L2; __NOOP; __NOOP; __NOOP; L1: (void)&&L1; \
									asm volatile ("mtfsf 255,%0" : : "f" (x)); \
									L2: (void)&&L2;})

/*  exception flags  */
#define      FE_SET_FX            0x80000000     /*  floating-point exception summary (FX) bit  */
#define      FE_CLR_FX            0x7fffffff
#define      SET_INVALID          0x01000000

/*  the bitwise negation (one's complement) of FE_ALL_EXCEPT  */
#define      FE_NO_EXCEPT         0xc1ffffff

/*  the bitwise OR of all of the separate exception bits in the FPSCR  */
#define      FE_ALL_FLAGS         0xfff80300

/*  the bitwise negation (one's complement) of the previous macro  */
#define      FE_NO_FLAGS          0x0007fcff

/*  the bitwise OR of all of the separate invalid stickies in the FPSCR  */
#define      FE_ALL_INVALID       0x01f80300

/*  the bitwise negation (one's complement) of the previous macro  */
#define      FE_NO_INVALID        0xfe07fcff

/* an AND mask to disable all floating-point exception enables in the FPSCR  */
#define      FE_NO_ENABLES        0xffffff07

/*  rounding direction mode bits  */
#define      FE_ALL_RND           0x00000003
#define      FE_NO_RND            0xfffffffc

#define      EXCEPT_MASK          0x1ff80000

#endif      /* __FENV_PRIVATE__ */
