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
*     File copysign.c,                                                         *
*     Function copysign for PowerPC based machines.                            *
*                                                                              *
*     Copyright © 1991-2001 Apple Computer, Inc.  All rights reserved.         *
*                                                                              *
*     Written by A. Sazegari, started on June 1991.                            *
*     Modified and ported by Robert A. Murley (ram) for Mac OS X.              *
*                                                                              *
*     A MathLib v4 file.                                                       *
*                                                                              *
*     August    26 1991: no CFront Version 1.1d17 warnings.                    *
*     September 06 1991: passes the test suite with invalid raised on          *
*                        signaling nans.  sane rom code behaves the same.      *
*     September 24 1992: took the “#include support.h” out.                    *
*     Dcember   02 1992: PowerPC port.                                         *
*     July      20 1994: __FABS added                                          *
*     July      21 1994: deleted unnecessary functions: neg, COPYSIGNnew,      *
*                        and SIGNNUMnew.                                       *
*     April     11 2001: first port to os x using gcc.                         *
*                        removed fabs and deffered to gcc for direct           *
*                        instruction generation.                               *
*     August    28 2001: replaced DblInHex typedef with hexdouble.             *
*                        added #ifdef __ppc__.                                 *
*     September 09 2001: added more comments.                                  *
*     September 10 2001: added macros to detect PowerPC and correct compiler.  *
*     October   08 2001: removed <CoreServices/CoreServices.h>.                *
*                        changed compiler errors to warnings.                  *
*     November  06 2001: commented out warning about Intel architectures.      *
*                                                                              *
*     W A R N I N G:                                                           *
*     These routines require a 64-bit double precision IEEE-754 model.         *
*     They are written for PowerPC only and are expecting the compiler         *
*     to generate the correct sequence of multiply-add fused instructions.     *
*                                                                              *
*     These routines are not intended for 32-bit Intel architectures.          *
*                                                                              *
*     A version of gcc higher than 932 is required.                            *
*                                                                              *
*     GCC compiler options:                                                    *
*            optimization level 3 (-O3)                                        *
*            -fschedule-insns -finline-functions -funroll-all-loops            *
*                                                                              *
*******************************************************************************/

#include    "fp_private.h"
#include    "math.h"

/*******************************************************************************
*     Function copysign.                                                       *
*       Produces a value with the magnitude of its first argument and sign of  *
*      its second argument.  NOTE: the order of the arguments matches the      *
*      recommendation of the IEEE-754 floating-point standard,  which is the   *
*      opposite from SANE's copysign function.                                 *
*******************************************************************************/

static inline double __copysign_MEM ( double x, double y )
{
      hexdouble hx, hy;
      register uint32_t GPR_7f, GPR_80, GPR_x, GPR_y;
      register double result;

/*******************************************************************************
*     No need to flush NaNs out.                                               *
*******************************************************************************/
      
      hx.d = x;					hy.d = y;
      GPR_7f = 0x7fffffff;			GPR_80 = 0x80000000;
      __NOOP;
      __NOOP;
      __NOOP; // gaurantees next instruction is in different issue group, so allows store-fwd.

      GPR_x = hx.i.hi;
      GPR_x &= GPR_7f;
      GPR_y = hy.i.hi;
      GPR_x |= GPR_y & GPR_80;
      hx.i.hi = GPR_x;
      __NOOP;
      __NOOP;
      __NOOP;
      
      result = hx.d;
      return result;
}

double copysign ( double x, double y )
{
        double pos_x, neg_x;
        double pos_y, neg_y, t;
		
        pos_x = __fabs(x);
        neg_x = __fnabs(x);
        pos_y = __fabs(y);
        neg_y = __fnabs(y);

        if (unlikely(pos_y == neg_y)) 
			return __copysign_MEM(x, y);
			
        if (unlikely(y != y)) 
			return __copysign_MEM(x, y);
			
		t = __fsel(y, pos_x, neg_x);
        return t;
}

static inline float __copysignf_MEM ( float x, float y )
{
      hexsingle hx, hy;
      register uint32_t GPR_7f, GPR_80, GPR_x, GPR_y;
      register float result;

/*******************************************************************************
*     No need to flush NaNs out.                                               *
*******************************************************************************/
      
      hx.fval = x;				hy.fval = y;
      GPR_7f = 0x7fffffff;			GPR_80 = 0x80000000;
      __NOOP;
      __NOOP;
      __NOOP; // gaurantees next instruction is in different issue group, so allows store-fwd.
      
      GPR_x = hx.lval;
      GPR_x &= GPR_7f;
      GPR_y = hy.lval;
      GPR_x |= GPR_y & GPR_80;
      hx.lval = GPR_x;
      __NOOP;
      __NOOP;
      __NOOP;
      
      result = hx.fval;
      return result;
}

float copysignf ( float x, float y )
{
        double pos_x, neg_x;
        double pos_y, neg_y;
		float t;
		
        pos_x = __fabs(x);
        neg_x = __fnabs(x);
        pos_y = __fabs(y);
        neg_y = __fnabs(y);

        if (unlikely(pos_y == neg_y)) 
			return __copysignf_MEM(x, y);
			
        if (unlikely(y != y)) 
			return __copysignf_MEM(x, y);
			
		t = __fsels(y, pos_x, neg_x);
        return t;
}
