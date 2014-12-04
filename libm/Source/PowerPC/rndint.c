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
*     File:   rndint.c                                                         *
*                                                                              *
*     Contains: C source code for implementations of floating-point            *
*                functions which round to integral value or format, as         *
*                defined in C99.  In particular, this file contains            *
*                implementations of the following functions:                   *
*                rint, nearbyint, rinttol, round, roundtol, trunc and modf.    *
*                                                                              *
*     Copyright © 1992-2001 by Apple Computer, Inc. All rights reserved.       *
*                                                                              *
*     Written by Jon Okada, started on December 1992.                          *
*     Modified by Paul Finlayson (PAF) for MathLib v2.                         *
*     Modified by A. Sazegari (ali) for MathLib v3.                            *
*     Modified and ported by Robert A. Murley (ram) for Mac OS X.              *
*                                                                              *
*     A MathLib v4 file.                                                       *
*                                                                              *
*     Change History (most recent first):                                      *
*                                                                              *
*     06 Nov 01  ram commented out warning about Intel architectures.          *
*                    changed i386 stubs to call abort().                       *
*     02 Nov 01  ram added stubs for i386 routines.                            *
*     08 Oct 01  ram removed <Limits.h> and <CoreServices/CoreServices.h>.     *
*                    changed compiler errors to warnings.                      *
*     05 Oct 01  ram added defines for INT32_MAX and INT32_MIN                 *
*     18 Sep 01  ali <CoreServices/CoreServices.h> replaced "fp.h" & "fenv.h". *
*     10 Sep 01  ali added more comments.                                      *
*     09 Sep 01  ali added macros to detect PowerPC and correct compiler.      *
*     28 Aug 01  ram  added #ifdef __ppc__.                                    *
*     13 Jul 01  ram  Replaced __setflm with FEGETENVD/FESETENVD.              *
*                            replaced DblInHex typedef with hexdouble.         *
*     03 Mar 01  ali  first port to os x using gcc, added the crucial          *
*                       __setflm definition.                                   *
*                            1. removed double_t, put in double for now.       *
*                            2. removed iclass from nearbyint.                 *
*                            3. removed wrong comments in trunc.               *
*     13 May 97  ali  made performance improvements in rint, rinttol,          *
*                     roundtol and trunc by folding some of the taligent       *
*                     ideas into this implementation.  nearbyint is faster     *
*                     than the one in taligent, rint is more elegant, but      *
*                     slower by %30 than the taligent one.                     *
*     09 Apr 97  ali  deleted modfl and deferred to AuxiliaryDD.c              *
*     15 Sep 94  ali  Major overhaul and performance improvements of all       *
*                     functions.                                               *
*     20 Jul 94  PAF  New faster version.                                      *
*     16 Jul 93  ali  Added the modfl function.                                *
*     18 Feb 93  ali  Changed the return value of fenv functions               *
*                     feclearexcept and feraiseexcept to their new             *
*                     NCEG X3J11.1/93-001 definitions.                         *
*     16 Dec 92  JPO  Removed __itrunc implementation to a separate file.      *
*     15 Dec 92  JPO  Added __itrunc implementation and modified rinttol       *
*                     to include conversion from double to long int format.    *       
*                     Modified roundtol to call __itrunc.                      *
*     10 Dec 92  JPO  Added modf (double) implementation.                      *
*     04 Dec 92  JPO  First created.                                           *
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

#include       "math.h"
#include       "fp_private.h"
#include       "fenv_private.h"
#include       "limits.h"


static const hexdouble TOWARDZERO = HEXDOUBLE(0x00000000, 0x00000001);

static const float twoTo23  = 0x1.0p+23f; // 8388608.0;

#if !defined(BUILDING_FOR_CARBONCORE_LEGACY)

static const double twoTo52      = 0x1.0p+52; // 4503599627370496.0;

/*******************************************************************************
*                                                                              *
*     The function rint rounds its double argument to integral value           *
*     according to the current rounding direction and returns the result in    *
*     double format.  This function signals inexact if an ordered return       * 
*     value is not equal to the operand.                                       *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*     First, an elegant implementation.                                        *
********************************************************************************
*
*double rint ( double x )
*      {
*      double y;
*      
*      y = twoTo52.fval;
*      
*      if ( fabs ( x ) >= y )                          // huge case is exact 
*            return x;
*      if ( x < 0 ) y = -y;                            // negative case 
*      y = ( x + y ) - y;                              // force rounding 
*      if ( y == 0.0 )                                 // zero results mirror sign of x 
*            y = copysign ( y, x );
*      return ( y );      
*      }
********************************************************************************
*     Now a bit twidling version that is about %30 faster.                     *
*******************************************************************************/

double rint ( double x )
{
      hexdouble argument;
      register double y;
      uint32_t xHead;
      register int target;
      
      argument.d = x;
	  __NOOP;
	  __NOOP;
	  __NOOP;

      xHead = argument.i.hi & 0x7fffffff;              // xHead <- high half of |x|
      target = ( argument.i.hi < 0x80000000 );         // flags positive sign
      
      if (likely( xHead < 0x43300000u )) 
/*******************************************************************************
*     Is |x| < 2.0^52?                                                         *
*******************************************************************************/
       {
            if ( xHead < 0x3ff00000 ) 
/*******************************************************************************
*     Is |x| < 1.0?                                                            *
*******************************************************************************/
              {
                  if ( target )
                        y = ( x + twoTo52 ) - twoTo52; // round at binary point
                  else
                        y = ( x - twoTo52 ) + twoTo52; // round at binary point
                  if ( y == 0.0 ) 
                     {                                 // fix sign of zero result
                        if ( target )
                              return ( 0.0 );
                        else
                              return ( -0.0 );
                     }
                  return y;
              }
            
/*******************************************************************************
*     Is 1.0 < |x| < 2.0^52?                                                   *
*******************************************************************************/

            if ( target )
                  return ( ( x + twoTo52 ) - twoTo52 ); // round at binary pt.
            else
                  return ( ( x - twoTo52 ) + twoTo52 );
       }
      
/*******************************************************************************
*     |x| >= 2.0^52 or x is a NaN.                                             *
*******************************************************************************/
      return ( x );
}

float rintf ( float x )
{
      hexsingle argument;
      register float y;
      uint32_t xHead;
      register int target;
      
      argument.fval = x;
	  __NOOP;
	  __NOOP;
	  __NOOP;

      xHead = argument.lval & 0x7fffffff;              // xHead <- |x|
      target = ( (uint32_t)argument.lval < 0x80000000u );         // flags positive sign
      
      if (likely( xHead < 0x4b000000u )) 
/*******************************************************************************
*     Is |x| < 2.0^23?                                                         *
*******************************************************************************/
       {
            if ( xHead < 0x3f800000 ) 
/*******************************************************************************
*     Is |x| < 1.0?                                                            *
*******************************************************************************/
              {
                  if ( target )
                        y = ( x + twoTo23 ) - twoTo23; // round at binary point
                  else
                        y = ( x - twoTo23 ) + twoTo23; // round at binary point
                  if ( y == 0.0 ) 
                     {                                 // fix sign of zero result
                        if ( target )
                              return ( 0.0f );
                        else
                             {
#if defined(__GNUC__) && (__GNUC__<3)   /* workaround gcc2.x botch of -0 return. */
                              volatile hexsingle zInHex;
                              zInHex.lval = 0x80000000;
                              return zInHex.fval;

#else                       
                              return ( -0.0f );
#endif
                              }
                     }
                  return y;
              }
            
/*******************************************************************************
*     Is 1.0 < |x| < 2.0^23?                                                   *
*******************************************************************************/

            if ( target )
                  return ( ( x + twoTo23 ) - twoTo23 ); // round at binary pt.
            else
                  return ( ( x - twoTo23 ) + twoTo23 );
       }
      
/*******************************************************************************
*     |x| >= 2.0^23 or x is a NaN.                                             *
*******************************************************************************/
      return ( x );
}

/*******************************************************************************
*                                                                              *
*     The function nearbyint rounds its double argument to integral value      *
*     according to the current rounding direction and returns the result in    *
*     double format.  This function does not signal inexact.                   *
*                                                                              *
*     Functions used in this routine:                                          *
*     fabs and copysign.                                                       *
*                                                                              *
*******************************************************************************/
   
double nearbyint ( double x )
{
       double y, OldEnvironment;
      
       if (unlikely(x != x))
            return x;
            
       y = twoTo52;
       
       FEGETENVD( OldEnvironment );              /* save the environement */

      if (unlikely( fabs ( x ) >= y ))           /* huge case is exact */
      {
            FESETENVD( OldEnvironment );              /* restore old environment */
            return x;
      }
      if ( x < 0 ) y = -y;                       /* negative case */
      y = ( x + y ) - y;                         /* force rounding */
      if ( y == 0.0 )                            /* zero results mirror sign of x */
            y = copysign ( y, x );
      FESETENVD( OldEnvironment );              /* restore old environment */
      return ( y );      
}
      
float nearbyintf ( float x )
{
    double OldEnvironment;
    float y;

    if (unlikely(x != x))
        return x;
            
    y = twoTo23;
    
    FEGETENVD( OldEnvironment );              /* save the environement */
    
    if (unlikely( fabsf ( x ) >= y ))          /* huge case is exact */
    {
        FESETENVD( OldEnvironment );              /* restore old environment */
        return x;
    }
    if ( x < 0 ) y = -y;                       /* negative case */
        y = ( x + y ) - y;                         /* force rounding */
    if ( y == 0.0 )                            /* zero results mirror sign of x */
        y = copysignf ( y, x );
    FESETENVD( OldEnvironment );              /* restore old environment */
    return ( y );      
}
  
long int lrint ( double x )
{
    if (sizeof(long int) == 4) // PPC32 ABI
    {
        hexdouble hx;
        
        asm volatile ("fctiw %0,%1" : "=f"(hx.d) : "f"(x));
        __NOOP;
        __NOOP;
        __NOOP;
        return hx.i.lo;    
    }
    else // (sizeof(long int) == 8) LP64 ABI
    {
        union { double d; uint64_t i;} hx;
        asm volatile ("fctid %0,%1" : "=f"(hx.d) : "f"(x));
        __NOOP;
        __NOOP;
        __NOOP;
        return hx.i;
    }
}

long int lrintf ( float x )
{
    if (sizeof(long int) == 4) // PPC32 ABI
    {
        hexdouble hx;
        
        asm volatile ("fctiw %0,%1" : "=f"(hx.d) : "f"(x));
        __NOOP;
        __NOOP;
        __NOOP;
        return hx.i.lo;    
    }
    else // (sizeof(long int) == 8) LP64 ABI, PPC64 ISA
    {
        union { double d; uint64_t i;} hx;
        asm volatile ("fctid %0,%1" : "=f"(hx.d) : "f"(x));
        __NOOP;
        __NOOP;
        __NOOP;
        return hx.i;
    }
}

long long int llrint ( double x )
{
	double t;
	long long int result;
	double fenv;

    if (sizeof(long int) == 8) // LP64 ABI, PPC64 ISA
    {
        union { double d; uint64_t i;} hx;
        asm volatile ("fctid %0,%1" : "=f"(hx.d) : "f"(x));
        __NOOP;
        __NOOP;
        __NOOP;
        return hx.i;
    }
    
	if (unlikely(x != x))
	{
		feraiseexcept(FE_INVALID);
		return LONG_LONG_MIN;
	}
	
	FEGETENVD(fenv);
	t = rint ( x );
	FESETENVD(fenv);
	
	if ( t < (double)LONG_LONG_MIN )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_LONG_MIN;
	}
	else if ( t > (double)LONG_LONG_MAX )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_LONG_MAX;
	}
	else if (t != x)
	{
		feraiseexcept(FE_INEXACT);
		result = (long long int) t;
	}
	else
	{
		result = (long long int) t;
	}
	
    return result;
}

long long int llrintf (float x)
{
	float t;
	long long int result;
	double fenv;
	
    if (sizeof(long int) == 8) // LP64 ABI, PPC64 ISA
    {
        union { double d; uint64_t i;} hx;
        asm volatile ("fctid %0,%1" : "=f"(hx.d) : "f"(x));
        __NOOP;
        __NOOP;
        __NOOP;
        return hx.i;
    }

	if (unlikely(x != x))
	{
		feraiseexcept(FE_INVALID);
		return LONG_LONG_MIN;
	}
	
	FEGETENVD(fenv);
	t = rintf ( x );
	FESETENVD(fenv);
	
	if ( t < (float)LONG_LONG_MIN )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_LONG_MIN;
	}
	else if ( t > (float)LONG_LONG_MAX )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_LONG_MAX;
	}
	else if (t != x)
	{
		feraiseexcept(FE_INEXACT);
		result = (long long int) t;
	}
	else
	{
		result = (long long int) t;
	}
	
    return result;
}

/*******************************************************************************
*                                                                              *
*     The function round rounds its double argument to integral value          *
*     according to the "add half to the magnitude and truncate" rounding of    *
*     Pascal's Round function and FORTRAN's ANINT function and returns the     *
*     result in double format.  This function signals inexact if an ordered    *
*     return value is not equal to the operand.                                *
*                                                                              *
*******************************************************************************/
   
double round ( double x )
{      
      hexdouble argument, OldEnvironment;
      register double y, z;
      uint32_t xHead;
      register int target;
      
      argument.d = x;
	  __NOOP;
	  __NOOP;
	  __NOOP;

      xHead = argument.i.hi & 0x7fffffff;              // xHead <- high half of |x|
      target = ( argument.i.hi < 0x80000000 );         // flag positive sign
      
      if (likely( xHead < 0x43300000u ))
/*******************************************************************************
*     Is |x| < 2.0^52?                                                        *
*******************************************************************************/
       {
            if ( xHead < 0x3ff00000 ) 
/*******************************************************************************
*     Is |x| < 1.0?                                                           *
*******************************************************************************/
              {
                     FEGETENVD( OldEnvironment.d );     // get environment
                  if ( xHead < 0x3fe00000u ) 
/*******************************************************************************
*     Is |x| < 0.5?                                                           *
*******************************************************************************/
                     {
                        if ( ( xHead | argument.i.lo ) != 0u )
                              OldEnvironment.i.lo |= FE_INEXACT;
                        FESETENVD_GRP( OldEnvironment.d );
                        if ( target ) 
                              return ( 0.0 );
                        else
                              return ( -0.0 );
                     }
/*******************************************************************************
*     Is 0.5 ² |x| < 1.0?                                                      *
*******************************************************************************/
                  OldEnvironment.i.lo |= FE_INEXACT;
                  FESETENVD_GRP ( OldEnvironment.d );
                  if ( target )
                        return ( 1.0 );
                  else
                        return ( -1.0 );
              }
/*******************************************************************************
*     Is 1.0 < |x| < 2.0^52?                                                   *
*******************************************************************************/
            if ( target ) 
              {                                         // positive x
                  y = ( x + twoTo52 ) - twoTo52;        // round at binary point
                  if ( y == x )                         // exact case
                        return ( x );
                  z = x + 0.5;                          // inexact case
                  y = ( z + twoTo52 ) - twoTo52;        // round at binary point
                  if ( y > z )
                        return ( y - 1.0 );
                  else
                        return ( y );
              }
            
/*******************************************************************************
*     Is x < 0?                                                                *
*******************************************************************************/
            else 
              {
                  y = ( x - twoTo52 ) + twoTo52;        // round at binary point
                  if ( y == x )
                        return ( x );
                  z = x - 0.5;
                  y = ( z - twoTo52 ) + twoTo52;        // round at binary point
                  if ( y < z )
                        return ( y + 1.0 );
                  else
                            return ( y );
              }
       }
/*******************************************************************************
*      |x| >= 2.0^52 or x is a NaN.                                            *
*******************************************************************************/
      return ( x );
}

float roundf ( float x )
{      
      hexdouble OldEnvironment;
      hexsingle argument;
      register float y, z;
      uint32_t xHead;
      register int target;
      
      argument.fval = x;
	  __NOOP;
	  __NOOP;
	  __NOOP;

      xHead = argument.lval & 0x7fffffff;              // xHead <- |x|
      target = ( (uint32_t)argument.lval < 0x80000000u );         // flags positive sign
      
      if (likely( xHead < 0x4b000000u )) 
/*******************************************************************************
*     Is |x| < 2.0^52?                                                        *
*******************************************************************************/
       {
            if ( xHead < 0x3f800000u ) 
/*******************************************************************************
*     Is |x| < 1.0?                                                           *
*******************************************************************************/
              {
                  FEGETENVD( OldEnvironment.d );     // get environment
                  if ( xHead < 0x3f000000u ) 
/*******************************************************************************
*     Is |x| < 0.5?                                                           *
*******************************************************************************/
                     {
                        if ( xHead != 0u )
                              OldEnvironment.i.lo |= FE_INEXACT;
                              FESETENVD_GRP( OldEnvironment.d );
                        if ( target ) 
                              return ( 0.0f );
                        else
                             {
#if defined(__GNUC__) && (__GNUC__<3)   /* workaround gcc2.x botch of -0 return. */
                              volatile hexsingle zInHex;
                              zInHex.lval = 0x80000000;
                              return zInHex.fval;

#else                       
                              return ( -0.0f );
#endif
                              }
                     }
/*******************************************************************************
*     Is 0.5 ² |x| < 1.0?                                                      *
*******************************************************************************/
                  OldEnvironment.i.lo |= FE_INEXACT;
                  FESETENVD_GRP ( OldEnvironment.d );
                  if ( target )
                        return ( 1.0f );
                  else
                        return ( -1.0f );
              }
/*******************************************************************************
*     Is 1.0 < |x| < 2.0^23?                                                   *
*******************************************************************************/
            if ( target ) 
              {                                         // positive x
                  y = ( x + twoTo23 ) - twoTo23;        // round at binary point
                  if ( y == x )                         // exact case
                        return ( x );
                  z = x + 0.5f;                          // inexact case
                  y = ( z + twoTo23 ) - twoTo23;        // round at binary point
                  if ( y > z )
                        return ( y - 1.0f );
                  else
                        return ( y );
              }
            
/*******************************************************************************
*     Is x < 0?                                                                *
*******************************************************************************/
            else 
              {
                  y = ( x - twoTo23 ) + twoTo23;        // round at binary point
                  if ( y == x )
                        return ( x );
                  z = x - 0.5f;
                  y = ( z - twoTo23 ) + twoTo23;        // round at binary point
                  if ( y < z )
                        return ( y + 1.0f );
                  else
                            return ( y );
              }
       }
/*******************************************************************************
*      |x| >= 2.0^23 or x is a NaN.                                            *
*******************************************************************************/
      return ( x );
}

/*******************************************************************************
*                                                                              *
*     The function roundtol converts its double argument to integral format    *
*     according to the "add half to the magnitude and chop" rounding mode of   *
*     Pascal's Round function and FORTRAN's NINT function.  This conversion    *
*     signals invalid if the argument is a NaN or the rounded intermediate     *
*     result is out of range of the destination long int format, and it        *
*     delivers an unspecified result in this case.  This function signals      *
*     inexact if the rounded result is within range of the long int format but *
*     unequal to the operand.                                                  *
*                                                                              *
*******************************************************************************/

// These work just as well for the LP64 ABI
long int lround ( double x )
{
	double t;
	long int result;
	double fenv;
	
	if (unlikely(x != x))
	{
		feraiseexcept(FE_INVALID);
		return LONG_MAX;
	}
	
	FEGETENVD(fenv);
	t = round ( x );
	FESETENVD(fenv);
	
	if ( t < (double)LONG_MIN )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_MIN;
	}
	else if ( t > (double)LONG_MAX )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_MAX;
	}
	else if (t != x)
	{
		feraiseexcept(FE_INEXACT);
		result = (long int) t;
	}
	else
	{
		result = (long int) t;
	}
	
    return result;
}

long int lroundf ( float x )
{       
	float t;
	long int result;
	double fenv;
	
	if (unlikely(x != x))
	{
		feraiseexcept(FE_INVALID);
		return LONG_MAX;
	}
	
	FEGETENVD(fenv);
	t = roundf ( x );
	FESETENVD(fenv);
	
	if ( t < (float)LONG_MIN )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_MIN;
	}
	else if ( t > (float)LONG_MAX )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_MAX;
	}
	else if (t != x)
	{
		feraiseexcept(FE_INEXACT);
		result = (long int) t;
	}
	else
	{
		result = (long int) t;
	}
	
    return result;
}       

long long int llround ( double x )
{       
	double t;
	long long int result;
	double fenv;
	
	if (unlikely(x != x))
	{
		feraiseexcept(FE_INVALID);
		return LONG_LONG_MAX;
	}
	
	FEGETENVD(fenv);
	t = round ( x );
	FESETENVD(fenv);
	
	if ( t < (double)LONG_LONG_MIN )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_LONG_MIN;
	}
	else if ( t > (double)LONG_LONG_MAX )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_LONG_MAX;
	}
	else if (t != x)
	{
		feraiseexcept(FE_INEXACT);
		result = (long long int) t;
	}
	else
	{
		result = (long long int) t;
	}
	
    return result;
}

long long int llroundf ( float x )
{       
	float t;
	long long int result;
	double fenv;
	
	if (unlikely(x != x))
	{
		feraiseexcept(FE_INVALID);
		return LONG_LONG_MAX;
	}
	
	FEGETENVD(fenv);
	t = roundf ( x );
	FESETENVD(fenv);
	
	if ( t < (float)LONG_LONG_MIN )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_LONG_MIN;
	}
	else if ( t > (float)LONG_LONG_MAX )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_LONG_MAX;
	}
	else if (t != x)
	{
		feraiseexcept(FE_INEXACT);
		result = (long long int) t;
	}
	else
	{
		result = (long long int) t;
	}
	
    return result;
}

/*******************************************************************************
*                                                                              *
*     The function trunc truncates its double argument to integral value       *
*     and returns the result in double format.  This function signals          *
*     inexact if an ordered return value is not equal to the operand.          *
*                                                                              *
*******************************************************************************/
   
double trunc ( double x )
{       
       hexdouble argument, OldEnvironment;
       register double y;
       uint32_t xhi;
       register int target;
       
       argument.d = x;
       __NOOP;
       __NOOP;
       __NOOP;
       xhi = argument.i.hi & 0x7fffffff;                         // xhi <- high half of |x|
       target = ( argument.i.hi < 0x80000000 );                  // flag positive sign
       
       if (likely( xhi < 0x43300000u )) 
/*******************************************************************************
*     Is |x| < 2.0^52?                                                         *
*******************************************************************************/
       {
              if ( xhi < 0x3ff00000u ) 
/*******************************************************************************
*     Is |x| < 1.0?                                                            *
*******************************************************************************/
              {
                     if ( ( xhi | argument.i.lo ) != 0u ) 
                     {                                          // raise deserved INEXACT
                            FEGETENVD_GRP( OldEnvironment.d );
                            OldEnvironment.i.lo |= FE_INEXACT;
                            FESETENVD_GRP( OldEnvironment.d );
                     }
                     if ( target )                              // return properly signed zero
                            return ( 0.0 );
                     else
                            return ( -0.0 );
              }
/*******************************************************************************
*     Is 1.0 < |x| < 2.0^52?                                                   *
*******************************************************************************/
              if ( target ) 
              {
                     y = ( x + twoTo52 ) - twoTo52;             // round at binary point
                     if ( y > x )
                            return ( y - 1.0 );
                     else
                            return ( y );
              }
              else 
              {
                     y = ( x - twoTo52 ) + twoTo52;             // round at binary point.
                     if ( y < x )
                            return ( y + 1.0 );
                     else
                            return ( y );
              }
       }
/*******************************************************************************
*      Is |x| >= 2.0^52 or x is a NaN.                                         *
*******************************************************************************/
       return ( x );
}

float truncf ( float x )
{       
       hexdouble OldEnvironment;
       hexsingle argument;
       register float y;
       uint32_t xhi;
       register int target;
       
       argument.fval = x;
	  __NOOP;
	  __NOOP;
	  __NOOP;

       xhi = argument.lval & 0x7fffffff;                         // xhi <- |x|
       target = ( (uint32_t)argument.lval < 0x80000000u );         // flags positive sign
       
       if (likely( xhi < 0x4b000000u )) 
/*******************************************************************************
*     Is |x| < 2.0^23?                                                         *
*******************************************************************************/
       {
              if ( xhi < 0x3f800000 ) 
/*******************************************************************************
*     Is |x| < 1.0?                                                            *
*******************************************************************************/
              {
                     if ( xhi != 0u ) 
                     {                                          // raise deserved INEXACT
                            FEGETENVD_GRP( OldEnvironment.d );
                            OldEnvironment.i.lo |= FE_INEXACT;
                            FESETENVD_GRP( OldEnvironment.d );
                     }
                     if ( target )                              // return properly signed zero
                            return ( 0.0f );
                     else
                             {
#if defined(__GNUC__) && (__GNUC__<3)   /* workaround gcc2.x botch of -0 return. */
                              volatile hexsingle zInHex;
                              zInHex.lval = 0x80000000;
                              return zInHex.fval;

#else                       
                              return ( -0.0f );
#endif
                              }
              }
/*******************************************************************************
*     Is 1.0 < |x| < 2.0^23?                                                   *
*******************************************************************************/
              if ( target ) 
              {
                     y = ( x + twoTo23 ) - twoTo23;             // round at binary point
                     if ( y > x )
                            return ( y - 1.0f );
                     else
                            return ( y );
              }
              else 
              {
                     y = ( x - twoTo23 ) + twoTo23;             // round at binary point.
                     if ( y < x )
                            return ( y + 1.0f );
                     else
                            return ( y );
              }
       }
/*******************************************************************************
*      Is |x| >= 2.0^23 or x is a NaN.                                         *
*******************************************************************************/
       return ( x );
}

/*******************************************************************************
*                                                                              *
*     The modf family of functions separate a floating-point number into its   *
*     fractional and integral parts, returning the fractional part and writing *
*     the integral part in floating-point format to the object pointed to by a *
*     pointer argument.  If the input argument is integral or infinite in      *
*     value, the return value is a zero with the sign of the input argument.   *
*     The modf family of functions raises no floating-point exceptions. older  *
*     implemenation set the INVALID flag due to signaling NaN input.           *
*                                                                              *
*     modf is the double implementation.                                       *                             
*                                                                              *
*******************************************************************************/

double modf ( double x, double *iptr )
{
      register double OldEnvironment, xtrunc;
      hexdouble argument;
      
      register double FPR_negZero, FPR_zero, FPR_one, FPR_Two52, FPR_TowardZero, FPR_absx;
      
      FPR_absx = __FABS( x );						FPR_Two52 = twoTo52;
      FPR_one = 1.0;								argument.d = x;

      FPR_TowardZero = TOWARDZERO.d;				FPR_zero = 0.0;	
      
      FPR_negZero = -0.0;							__NOOP;
      
      __ENSURE(FPR_zero, FPR_TowardZero, FPR_Two52); __ENSURE(FPR_zero, FPR_Two52, FPR_one);
            
/*******************************************************************************
*     Is |x| < 2.0^52?                                                         *
*******************************************************************************/
       if (likely( FPR_absx < FPR_Two52 )) 
       {
/*******************************************************************************
*     Is |x| < 1.0?                                                            *
*******************************************************************************/
              if ( FPR_absx < FPR_one )      
              {
                  if ( argument.i.hi & 0x80000000 )   		// isolate sign bit
                        *iptr = FPR_negZero;				// truncate to zero
                  else
                        *iptr = FPR_zero;					// truncate to zero
                  return ( x );
              }
/*******************************************************************************
*     Is 1.0 < |x| < 2.0^52?                                                   *
*******************************************************************************/
              FEGETENVD( OldEnvironment );                    // save environment
              // round toward zero
              FESETENVD( FPR_TowardZero );
              if ( x > FPR_zero )                             // truncate to integer
                    xtrunc = ( x + FPR_Two52 ) - FPR_Two52;
              else
                    xtrunc = ( x - FPR_Two52 ) + FPR_Two52;
              *iptr = xtrunc;                                 // store integral part
              // restore caller's env
              FESETENVD( OldEnvironment );                    // restore environment
              if ( x != xtrunc )                              // nonzero fraction
                    return ( x - xtrunc );
              else 
              {                                               // zero with x's sign
                  if ( argument.i.hi & 0x80000000 )   		  // isolate sign bit
                        return FPR_negZero;					  // truncate to zero
                  else
                        return FPR_zero;					  // truncate to zero
              }
       }
      
      *iptr = x;                                             // x is integral or NaN
      if ( x != x )                                          // NaN is returned
            return x;
      else 
      {                                                     // zero with x's sign
            if ( argument.i.hi & 0x80000000 )   			// isolate sign bit
                return FPR_negZero;							// truncate to zero
            else
                return FPR_zero;							// truncate to zero
      }
}

#else /* BUILDING_FOR_CARBONCORE_LEGACY */

float modff ( float x, float *iptr )
{
      register double OldEnvironment;
      register float xtrunc;
      register uint32_t xHead, signBit;
      hexsingle argument;
      
      argument.fval = x;
	  __NOOP;
	  __NOOP;
	  __NOOP;

      xHead = argument.lval & 0x7fffffff;                    // |x| high bit pattern
      signBit = ( argument.lval & 0x80000000 );              // isolate sign bit
      
      if (likely( xHead < 0x4b000000u )) 
/*******************************************************************************
*     Is |x| < 2.0^23?                                                         *
*******************************************************************************/
       {
            if ( xHead < 0x3f800000 )      
/*******************************************************************************
*     Is |x| < 1.0?                                                            *
*******************************************************************************/
              {
                  argument.lval = signBit;                    // truncate to zero
                  __NOOP;
                  __NOOP;
                  __NOOP;

                  *iptr = argument.fval;
                  return ( x );
              }
/*******************************************************************************
*     Is 1.0 < |x| < 2.0^23?                                                   *
*******************************************************************************/
              FEGETENVD( OldEnvironment );                    // save environment
              // round toward zero
              FESETENVD( TOWARDZERO.d );
            if ( signBit == 0u )                             // truncate to integer
                  xtrunc = ( x + twoTo23 ) - twoTo23;
            else
                  xtrunc = ( x - twoTo23 ) + twoTo23;
              // restore caller's env
              FESETENVD( OldEnvironment );                    // restore environment
            *iptr = xtrunc;                                   // store integral part
            if ( x != xtrunc )                                // nonzero fraction
                  return ( x - xtrunc );
            else 
              {                                               // zero with x's sign
                  argument.lval = signBit;
                  __NOOP;
                  __NOOP;
                  __NOOP;
                  
                  return ( argument.fval );
              }
       }
      
      *iptr = x;                                             // x is integral or NaN
      if ( x != x )                                          // NaN is returned
            return x;
      else 
       {                                                     // zero with x's sign
            argument.lval = signBit;
            __NOOP;
            __NOOP;
            __NOOP;
            
            return ( argument.fval );
       }
}

#endif /* BUILDING_FOR_CARBONCORE_LEGACY */
