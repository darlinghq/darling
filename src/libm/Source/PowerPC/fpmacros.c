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
*     File:  fpmacros.c                                                        *
*                                                                              *
*     Contains:  C source code for PowerPC implementations of the inquiry      *
*     macros, as defined in C99.                                               *
*                                                                              *
*     Copyright � 1992-2001 Apple Computer, Inc.  All rights reserved.         *
*                                                                              *
*     Written by A. Sazegari and Jon Okada, started on November 29 1992.       *
*     Modified and ported by Robert A. Murley (ram) for Mac OS X.              *
*                                                                              *
*     fpmacros is a new file that brings all of C99 macros together.           *
*                                                                              *
*     A MathLib v4 file.                                                       *
*                                                                              *
*     Macros __isnormald, __isfinited, __isnand and __inf were previously      *
*     in fp.c.                                                                 *
*     Macros __fpclassifyf, __isnormalf, __isfinitef, __isnanf and             *
*     __signbitf were previously in fpfloatfunc.c.                             *
*     Macro __fpclassifyd was in classify.c.                                   *
*     Macro __signbitd was in sign.c.                                          *
*                                                                              *
*     Change History (most recent first):                                      *
*                                                                              *
*     06 Nov 01   ram   commented out warning about Intel architectures.       *
*                       changed i386 stubs to call abort().                    *
*     02 Nov 01   ram   added stubs for i386 routines.                         *
*     08 Oct 01   ram   removed <CoreServices/CoreServices.h>.                 *
*                       changed compiler errors to warnings.                   *
*     24 Sep 01   ram   corrected mantissa mask in fpclassifyf and isnanf.     *
*     18 Sep 01   ali   added <CoreServices/CoreServices.h> to get to <fp.h>.  *
*     10 Sep 01   ali   added macros to detect PowerPC and correct compiler.   *
*     09 Sep 01   ali   added more comments.                                   *
*     05 Sep 01   ram   added __inf routine.                                   *
*                       added #ifdef __ppc__.                                  *
*     07 Jul 01   ram   first created from fpfloatfunc.c, fp.c,                *
*                       classify.c and sign.c in MathLib v3 Mac OS9.           *
*                       replaced DblInHex typedef with hexdouble.              *
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
#include	  "float.h"
#include      "fp_private.h"
#include      "fenv.h"

#if !defined(BUILDING_FOR_CARBONCORE_LEGACY)

/******************************************************************************
*     No other functions are called by these routines outside of fpmacros.c.  *
******************************************************************************/

int __math_errhandling ( void )
{
    return (MATH_ERREXCEPT); // return the bitmask indicating the error discipline(s) in use.
}

/**************************************************************************
   Function __fpclassifyf
   Returns the classification code of the argument float x, as defined in 
   C99.
**************************************************************************/

int __fpclassifyf ( float x )
{
   uint32_t iexp;
   hexsingle      z;
   
   z.fval = x;
   __NOOP;
   __NOOP;
   __NOOP;
   iexp = z.lval & 0x7f800000;             // isolate float exponent
   
   if (iexp == 0x7f800000) {               // NaN or INF case
      if ((z.lval & 0x007fffff) == 0)
         return FP_INFINITE;
      else if ((z.lval & fQuietNan) != 0)
         return FP_QNAN;
      else
         return FP_SNAN;
   }
   
   if (iexp != 0)                             // normal float
      return FP_NORMAL;
      
   if ((z.lval & 0x007fffff) == 0)
      return FP_ZERO;             // zero
   else
      return FP_SUBNORMAL;        //must be subnormal
}
   

/*************************************************************************
      Function __fpclassifyd                                               
      Returns the classification code of the argument double x, as 
      defined in C99.
*************************************************************************/

int __fpclassifyd ( double arg )
{
      uint32_t exponent;
      hexdouble      x;
            
      x.d = arg;
	  __NOOP;
	  __NOOP;
	  __NOOP;
      
      exponent = x.i.hi & 0x7ff00000;
      if ( exponent == 0x7ff00000 )
      {
            if ( ( ( x.i.hi & 0x000fffff ) | x.i.lo ) == 0 )
                  return FP_INFINITE;
            else
                  return ( x.i.hi & dQuietNan ) ? FP_QNAN : FP_SNAN; 
      }
      else if ( exponent != 0)
            return FP_NORMAL;
      else
      {
            if ( ( ( x.i.hi & 0x000fffff ) | x.i.lo ) == 0 )
                  return FP_ZERO;
            else
                  return FP_SUBNORMAL;
      }
}

static const double twoTo52      = 0x1.0p+52;                  // 2^52
static const double mtwoTo52	 = -0x1.0p+52;

static inline double fastfloor( double f ) __attribute__((always_inline));
static inline double fastfloor( double f )
{
	double b, c, d, e, g, h, t;
	
	c = __fsel( f, mtwoTo52, twoTo52 );				b = fabs( f );
	d = (f - c) + c;								e = b - twoTo52;
	g = f - d;
	h = __fsel( g, 0.0, 1.0 );
	t = d - h;
	return __fsel( e, f, t );
}

// Called from fpclassify macro iff sizeof(double) != sizeof(long double)
int __fpclassify ( long double arg )
{
      uint32_t exponent;
	  int e;
      hexdouble      x, xx;
	  hexdbldbl u;
            
	  u.dd.head = 0.0;
	  u.dd.tail = 0.0;
	  
	  u.ld = arg;
	  __NOOP;
	  __NOOP;
	  __NOOP;
      
      x.d = u.dd.head;
      xx.d = fabs( u.dd.tail );
	  __NOOP;
	  __NOOP;
	  __NOOP;

      exponent = x.i.hi & 0x7ff00000;
	  e = (exponent >> 20) - 1023; // unbiased exponent

      if ( exponent == 0x7ff00000 )
      {
            if ( ( ( x.i.hi & 0x000fffff ) | x.i.lo ) == 0 )
                  return FP_INFINITE;
            else
                  return ( x.i.hi & dQuietNan ) ? FP_QNAN : FP_SNAN; 
      }
	  else if ( exponent == 0 )
	  {
            if ( ( ( x.i.hi & 0x000fffff ) | x.i.lo ) == 0 )
                  return FP_ZERO;
            else
                  return FP_SUBNORMAL;
	  } 
	  else if ( e < (LDBL_MIN_EXP - 1) )
	  {
	  		return FP_SUBNORMAL;
	  }
	  else if ( xx.d == 0.0 )
	  {
	  		return FP_NORMAL;
	  }
	  else /* xx.d > 0 */
	  {
	  		double t, u;
			int n = LDBL_MANT_DIG - e; // scaling arg by 2^n puts LDBL_MANT_DIG bits left of the binary point
			
			t = ldexp( xx.d, n ); // t holds the tails contribution to the last LDBL_MANT_DIG bits
			if ( t == 0.0 )
				return FP_SUPERNORMAL; // contribution of xx.d entirely beyond LDBL_MANT_DIG places to the right 

			u = fastfloor( t ); // chop off any "SUPERNORMAL" bits

			if ( t == u )
				return FP_NORMAL; // no "SUPERNORMAL" bits detected
			else
				return FP_SUPERNORMAL;
	  }
}

int __issupernormal ( long double x )
{
	return (FP_SUPERNORMAL == __fpclassify( x ));
}

/***********************************************************************
   Function __isnormalf
   Returns nonzero if and only if x is a normalized float number and 
   zero otherwise.
***********************************************************************/

int __isnormalf ( float x )
{
   uint32_t iexp;
   hexsingle      z;
   
   z.fval = x;
   __NOOP;
   __NOOP;
   __NOOP;
   iexp = z.lval & 0x7f800000;                 /* isolate float exponent */
   return ((iexp != 0x7f800000) && (iexp != 0));
}
   

/***********************************************************************
   Function __isnormald
   Returns nonzero if and only if x is a normalized double number and 
   zero otherwise.
***********************************************************************/

int __isnormald ( double x )
{
   uint32_t iexp;
   hexdouble      z;
   
   z.d = x;
   __NOOP;
   __NOOP;
   __NOOP;
   iexp = z.i.hi & 0x7ff00000;                 /* isolate float exponent */
   return ((iexp != 0x7ff00000) && (iexp != 0));
}

int __isnormal ( long double x )
{
	return (FP_NORMAL == __fpclassify( x ));
}

/***********************************************************************
   Function __isfinitef
   Returns nonzero if and only if x is a finite (normal, subnormal, 
   or zero) float number and zero otherwise.
***********************************************************************/

int __isfinitef ( float x )
{   
   hexsingle      z;
   
   z.fval = x;
   __NOOP;
   __NOOP;
   __NOOP;
   return ((z.lval & 0x7f800000) != 0x7f800000);
}
   

/***********************************************************************
   Function __isfinited
   Returns nonzero if and only if x is a finite (normal, subnormal, 
   or zero) double number and zero otherwise.
***********************************************************************/
int __isfinited ( double x )
{
   hexdouble      z;
   
   z.d = x;
   __NOOP;
   __NOOP;
   __NOOP;
   return ((z.i.hi & 0x7ff00000) != 0x7ff00000);
}

int __isfinite ( long double x )
{
   hexdbldbl u;
	
   u.ld = x;
   __NOOP;
   __NOOP;
   __NOOP;
   return ((u.hh.hexhead.i.hi & 0x7ff00000) != 0x7ff00000);
}

/***********************************************************************
   Function __isinff
   Returns nonzero if and only if x is an infinite float number and zero 
   otherwise.
***********************************************************************/

int __isinff ( float x )
{   
   hexsingle      z;
   
   z.fval = x;
   __NOOP;
   __NOOP;
   __NOOP;
   return (((z.lval&0x7f800000) == 0x7f800000) && ((z.lval&0x007fffff) == 0));
}
   

/***********************************************************************
   Function __isinfd
   Returns nonzero if and only if x is an infinite double number and zero 
   otherwise.
***********************************************************************/
int __isinfd ( double x )
{
   hexdouble      z;
   
   z.d = x;
   __NOOP;
   __NOOP;
   __NOOP;
   return (((z.i.hi&0x7ff00000) == 0x7ff00000) && (((z.i.hi&0x000fffff) | z.i.lo) == 0));
}

int __isinf ( long double x )
{
   hexdbldbl u;
	
   u.ld = x;
   __NOOP;
   __NOOP;
   __NOOP;
   return (((u.hh.hexhead.i.hi&0x7ff00000) == 0x7ff00000) && (((u.hh.hexhead.i.hi&0x000fffff) | u.hh.hexhead.i.lo) == 0));
}

// Retain the following for legacy ABI compatability as it shipped in 10.2 and 10.3.
int __isinfl ( long double x )
{
   return __isinf( x );	
}

/***********************************************************************
   Function __isnanf
   Returns nonzero if and only if x is a float NaN and zero otherwise.
***********************************************************************/

int __isnanf ( float x )
{   
   hexsingle      z;
   
   z.fval = x;
   __NOOP;
   __NOOP;
   __NOOP;
   return (((z.lval&0x7f800000) == 0x7f800000) && ((z.lval&0x007fffff) != 0));
}


/***********************************************************************
   Function __isnand
   Returns nonzero if and only if x is a double NaN and zero otherwise.
***********************************************************************/
int __isnand ( double x )
{
   hexdouble      z;
   
   z.d = x;
   __NOOP;
   __NOOP;
   __NOOP;
   return (((z.i.hi&0x7ff00000) == 0x7ff00000) && (((z.i.hi&0x000fffff) | z.i.lo) != 0));
}

int __isnan ( long double x )
{
   hexdbldbl u;
	
   u.ld = x;
   __NOOP;
   __NOOP;
   __NOOP;
   return (((u.hh.hexhead.i.hi&0x7ff00000) == 0x7ff00000) && (((u.hh.hexhead.i.hi&0x000fffff) | u.hh.hexhead.i.lo) != 0));
}

/***********************************************************************
   Function __signbitf
   Returns nonzero if and only if the sign bit of the float number x is 
   set and zero otherwise.
***********************************************************************/

int __signbitf ( float x )
{   
   hexsingle      z;
   
   z.fval = x;
   __NOOP;
   __NOOP;
   __NOOP;
   return (((int32_t)z.lval) < 0);
}


/***********************************************************************
   Function __signbitd
   Returns nonzero if and only if the sign bit of the double number x is 
   set and zero otherwise.
***********************************************************************/

int __signbitd ( double arg )
{
      hexdouble z;

      z.d = arg;
     __NOOP;
     __NOOP;
     __NOOP;
      return (((int32_t)z.i.hi) < 0);
}

int __signbitl ( long double arg )
{
	   hexdbldbl u;
		
	   u.ld = arg;
     __NOOP;
     __NOOP;
     __NOOP;
      return (((int32_t)u.hh.hexhead.i.hi) < 0);
}

// Retain the following for legacy ABI compatability as it shipped in 10.2 and 10.3.
int __signbit ( long double arg )
{
	  return __signbitl( arg );
}

float __nan ( void )
{
      static const hexsingle aQuietNAN  = { 0x7fc00000 };
      return ( aQuietNAN.fval );
}

/***********************************************************************
   Function __inf
   Returns the value of positive infinity for a double.
***********************************************************************/

float __inff ( void )
{
      static const hexsingle PosINF  = { 0x7f800000 };
      return ( PosINF.fval );
}

#else /* BUILDING_FOR_CARBONCORE_LEGACY */
 
double __inf ( void )
{
      static const hexdouble PosINF  = HEXDOUBLE(0x7ff00000, 0x00000000);
      return ( PosINF.d );
}

#endif /* BUILDING_FOR_CARBONCORE_LEGACY */
