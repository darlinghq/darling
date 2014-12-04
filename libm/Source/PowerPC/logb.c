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
*     File logb.c,                                                             *
*     Functions logb.                                                          *
*     Implementation of IEEE-754 logb for the PowerPC platforms.               *
*                                                                              *
*     Copyright © 1991-2001 Apple Computer, Inc.  All rights reserved.         *
*                                                                              *
*     Written by A. Sazegari, started on June 1991.                            *
*     Modified and ported by Robert A. Murley (ram) for Mac OS X.              *
*                                                                              *
*     A MathLib v4 file.                                                       *
*                                                                              *
*     Change History (most recent last):                                       *
*                                                                              *
*     August    26 1991: removed CFront Version 1.1d17 warnings.               *
*     August    27 1991: no errors reported by the test suite.                 *
*     November  11 1991: changed CLASSEXTENDED to the macro CLASSIFY and       *
*                        + or - infinity to constants.                         *
*     November  18 1991: changed the macro CLASSIFY to CLASSEXTENDEDint to     *
*                        improve performance.                                  *
*     February  07 1992: changed bit operations to macros (  object size is    *
*                        unchanged  ).                                         *
*     September 24 1992: took the "#include support.h" out.                    *
*     December  03 1992: first rs/6000 port.                                   *
*     August    30 1992: set the divide by zero for the zero argument case.    *
*     October   05 1993: corrected the environment.                            *
*     October   17 1994: replaced all environmental functions with __setflm.   *
*     May       28 1997: made speed improvements.                              *
*     April     30 2001: first mac os x port using gcc.                        *
*     July      16 2001: replaced __setflm with FEGETENVD/FESETENVD.           *
*     August    28 2001: added description of logb function.                   *
*     September 06 2001: added #if __ppc__.                                    *
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
*      GCC compiler options:                                                   *
*            optimization level 3 (-O3)                                        *
*            -fschedule-insns -finline-functions -funroll-all-loops            *
*                                                                              *
********************************************************************************
*     The C math library offers a similar function called "frexp".  It is      *
*     different in details from logb, but similar in spirit.  This current     *
*     implementation of logb follows the recommendation in IEEE Standard 854   *
*     which is different in its handling of denormalized numbers from the IEEE *
*     Standard 754.                                                            *
*******************************************************************************/

#include      "math.h"
#include      "fenv.h"
#include      "fp_private.h"
#include      "fenv_private.h"

static const double twoTo52 = 0x1.0p+52; // 4.50359962737049600e15;
static const double klTod = 4503601774854144.0;                    // 0x1.000008p52
static const hexdouble minusInf  = HEXDOUBLE(0xfff00000, 0x00000000);

static const float twoTo23 = 0x1.0p+23f; // 8388608.0e0;
static const hexsingle minusInff  = { 0xff800000 };

/*******************************************************************************
*                                    L  O  G  B                                *
********************************************************************************
*                                                                              *
*     logb extracts the exponent of its argument, as a signed integral         *
*     value. A subnormal argument is treated as though it were first           *
*     normalized. Thus                                                         *
*            1 <= x * 2^( - Logb ( x ) ) < 2                                   *
*******************************************************************************/

double logb (  double x  )
{
      hexdouble xInHex;
      int32_t shiftedExp;
      
      xInHex.d = x;
      __NOOP;
      __NOOP;
      __NOOP;
      shiftedExp = ( xInHex.i.hi & 0x7ff00000 ) >> 20;
      
      if (unlikely( shiftedExp == 2047 )) 
      {                                                  // NaN or INF
            if ( ( ( xInHex.i.hi & 0x80000000 ) == 0 ) || ( x != x ) )
                  return x;                              // NaN or +INF return x
            else
                  return -x;                             // -INF returns +INF
      }
      
      if (likely( shiftedExp != 0 ))                     // normal number
            shiftedExp -= 1023;                          // unbias exponent
      else if ( x == 0.0 ) 
      {                                                  // zero
            hexdouble OldEnvironment;
            FEGETENVD_GRP( OldEnvironment.d );		 // raise zero divide for DOMAIN error
            OldEnvironment.i.lo |= FE_DIVBYZERO;
            FESETENVD_GRP( OldEnvironment.d );
            return ( minusInf.d );			 // return -infinity
      }
      else 
      {                                                  // subnormal number
            xInHex.d *= twoTo52;                         // scale up
	    __NOOP;
	    __NOOP;
	    __NOOP;
            shiftedExp = ( xInHex.i.hi & 0x7ff00000 ) >> 20;
            shiftedExp -= 1075;                          // unbias exponent
      }
      
      if (unlikely( shiftedExp == 0 ))                  // zero result
            return ( 0.0 );
      else 
      {                                                  // nonzero result
            xInHex.d = klTod;
	    __NOOP;
	    __NOOP;
	    __NOOP;
            xInHex.i.lo += shiftedExp;
            return ( xInHex.d - klTod );
      }
}

int ilogb (  double x  )
{
      hexdouble xInHex;
      int32_t shiftedExp;
      
      xInHex.d = x;
      __NOOP;
      __NOOP;
      __NOOP;
      shiftedExp = ( xInHex.i.hi & 0x7ff00000 ) >> 20;
      
      if (unlikely( shiftedExp == 2047 )) 
      {                                                  // NaN or INF
            if (x != x)
                return FP_ILOGBNAN;
            else
	    {
		feraiseexcept( FE_INVALID );		 // raise invalid for DOMAIN error
                return INT32_MAX;
	    }
      }
      
      if (likely( shiftedExp != 0 ))                     // normal number
            shiftedExp -= 1023;                          // unbias exponent
      else if ( x == 0.0 ) 
      {                                                  // zero
            feraiseexcept( FE_INVALID );		 // raise invalid for DOMAIN error
            return FP_ILOGB0;			 	 // return -infinity
      }
      else 
      {                                                  // subnormal number
            xInHex.d *= twoTo52;                         // scale up
	    __NOOP;
	    __NOOP;
	    __NOOP;
            shiftedExp = ( xInHex.i.hi & 0x7ff00000 ) >> 20;
            shiftedExp -= 1075;                          // unbias exponent
      }
      
      return shiftedExp;
}

float logbf (  float x  )
{
      hexsingle xInHex;
      int32_t shiftedExp;
      
      xInHex.fval = x;
      __NOOP;
      __NOOP;
      __NOOP;
      shiftedExp = ( xInHex.lval & 0x7f800000 ) >> 23;
      
      if (unlikely( shiftedExp == 255 )) 
      {                                                  // NaN or INF
            if ( ( ( xInHex.lval & 0x80000000 ) == 0 ) || ( x != x ) )
                  return x;                              // NaN or +INF return x
            else
                  return -x;                             // -INF returns +INF
      }
      
      if (likely( shiftedExp != 0 ))                     // normal number
            shiftedExp -= 127;                           // unbias exponent
      else if ( x == 0.0 ) 
      {                                                  // zero
            hexdouble OldEnvironment;
            FEGETENVD_GRP( OldEnvironment.d );		 // raise zero divide for DOMAIN error
            OldEnvironment.i.lo |= FE_DIVBYZERO;
            FESETENVD_GRP( OldEnvironment.d );
            return ( minusInff.fval );			 // return -infinity
      }
      else 
      {                                                  // subnormal number
            xInHex.fval *= twoTo23;                      // scale up
	    __NOOP;
	    __NOOP;
	    __NOOP;
            shiftedExp = ( xInHex.lval & 0x7f800000 ) >> 23;
            shiftedExp -= 150;                          // unbias exponent
      }
      
      return (float)shiftedExp;
}

int ilogbf (  float x  )
{
      hexsingle xInHex;
      int32_t shiftedExp;
      
      xInHex.fval = x;
      __NOOP;
      __NOOP;
      __NOOP;
      shiftedExp = ( xInHex.lval & 0x7f800000 ) >> 23;
      
      if (unlikely( shiftedExp == 255 )) 
      {                                                  // NaN or INF
            if (x != x)
                return FP_ILOGBNAN;
            else
	    {
		feraiseexcept( FE_INVALID );		 // raise invalid for DOMAIN error
                return INT32_MAX;
	    }
      }
      
      if (likely( shiftedExp != 0 ))                     // normal number
            shiftedExp -= 127;                           // unbias exponent
      else if ( x == 0.0 ) 
      {                                                  // zero
            feraiseexcept( FE_INVALID );		 // raise invalid for DOMAIN error
            return FP_ILOGB0;			 	 // return -infinity
      }
      else 
      {                                                  // subnormal number
            xInHex.fval *= twoTo23;                      // scale up
	    __NOOP;
	    __NOOP;
	    __NOOP;
            shiftedExp = ( xInHex.lval & 0x7f800000 ) >> 23;
            shiftedExp -= 150;                          // unbias exponent
      }
      
      return shiftedExp;
}
