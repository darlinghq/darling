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
*     File frexpldexp.c,                                                       *
*                                                                              *
*     Contains: Functions frexp(x) and ldexp(x),                               *
*     Implementation of frexp and ldexp functions for the PowerPC.             *
*                                                                              *
*     Copyright © 1991-2001 Apple Computer, Inc.  All rights reserved.         *
*                                                                              *
*     Written by A. Sazegari, started on January 1991,                         *
*     Modified and ported by Robert A. Murley (ram) for Mac OS X.              *
*                                                                              *
*     A MathLib v4 file.                                                       *
*                                                                              *
*     December  03 1992: first rs6000 implementation.                          *
*     October   05 1993: added special cases for NaN and ° in frexp.           *
*     May       27 1997: improved the performance of frexp by eliminating the  *
*                        switch statement.                                     *
*     June      13 2001: rewrote frexp to eliminate calls to scalb and logb.   *
*                         replaced DblInHex typedef with hexdouble.            *
*     September 06 2001: added #if __ppc__.                                    *
*                        added routine descriptions.                           *
*     September 09 2001: added macros to detect PowerPC and correct compiler.  *
*     September 10 2001: added more comments.                                  *
*     September 18 2001: added <Limits.h> and <CoreServices/CoreServices.h>.   *
*     October   08 2001: removed <Limits.h> and <CoreServices/CoreServices.h>. *
*                        added #define INT16_MAX.                               *
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
*      GCC compiler options:                                                   *
*            optimization level 3 (-O3)                                        *
*            -fschedule-insns -finline-functions -funroll-all-loops            *
*                                                                              *
*******************************************************************************/

#include      "math.h"
#include      "fp_private.h"

static const double two54 =  0x1.0p+54; // 1.80143985094819840000e+16; /* 0x43500000, 0x00000000 */
static const double two25 =  0x1.0p+25; // 33554432.0e0;

/******************************************************************************
*     Function ldexp                                                          *
*     Multiplies a floating-point number by an integer power of 2.            *
*                                                                             *
*     Functions used in this routine:                                         *
*     scalb.                                                                  *
******************************************************************************/

double ldexp ( double value, int exp ) 
{
      if (unlikely( exp > INT16_MAX ))
            exp = INT16_MAX;
      else if (unlikely( exp < -INT16_MAX )) 
            exp = -INT16_MAX;
      return scalbn ( value, exp  );
}

float ldexpf ( float value, int exp ) 
{
      if (unlikely( exp > INT16_MAX )) 
            exp = INT16_MAX;
      else if (unlikely( exp < -INT16_MAX )) 
            exp = -INT16_MAX;
      return scalbnf ( value, exp  );
}

/******************************************************************************
*     Function frexp                                                          *
*     Breaks a floating-point number into a normalized fraction and an        *
*     integral power of 2.  It stores the integer in the object pointed by    *
*     *eptr.                                                                  *
******************************************************************************/

double frexp ( double value, int *eptr )
{
      hexdouble argument;
      uint32_t valueHead;

      argument.d = value;
	  __NOOP;
	  __NOOP;
	  __NOOP;
      valueHead = argument.i.hi & 0x7fffffff;            // valueHead <- |x|

      *eptr = 0;
      if (unlikely( valueHead >= 0x7ff00000 || ( valueHead | argument.i.lo ) == 0 ))
            return value;            // 0, inf, or NaN
      
      if (unlikely( valueHead < 0x00100000 ))
      {      // denorm
            argument.d = two54 * value;
		    __NOOP;
		    __NOOP;
		    __NOOP;
            valueHead = argument.i.hi & 0x7fffffff;
            *eptr = -54;
      }
      *eptr += ( valueHead >> 20 ) - 1022;
      argument.i.hi = ( argument.i.hi & 0x800fffff ) | 0x3fe00000;
	  __NOOP;
	  __NOOP;
	  __NOOP;
      return argument.d;
}

float frexpf ( float value, int *eptr )
{
      hexsingle argument;
      uint32_t valueHead;

      argument.fval = value;
	  __NOOP;
	  __NOOP;
	  __NOOP;
      valueHead = argument.lval & 0x7fffffff;            // valueHead <- |x|

      *eptr = 0;
      if (unlikely( valueHead >= 0x7f800000 || valueHead == 0 ))
            return value;            // 0, inf, or NaN
      
      if (unlikely( valueHead < 0x00800000 ))
      {      // denorm
            argument.fval = (float) two25 * value;
		    __NOOP;
		    __NOOP;
		    __NOOP;
            valueHead = argument.lval & 0x7fffffff;
            *eptr = -25;
      }
      *eptr += ( valueHead >> 23 ) - 126;
      argument.lval = ( argument.lval & 0x807fffff ) | 0x3f000000;
	  __NOOP;
	  __NOOP;
	  __NOOP;
      return argument.fval;
}
