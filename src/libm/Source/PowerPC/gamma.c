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
*      File gamma.c,                                                           *
*      Functions gamma(x),                                                     *
*      Implementation of the gamma function for the PowerPC.                   *
*                                                                              *
*      Copyright c 1993 Apple Computer, Inc.  All rights reserved.             *
*                                                                              *
*      Written by Ali Sazegari, started on January 1993,                       *
*                                                                              *
*      based on FORTRAN routines in SpecFun by J. W. Cody and L. Stoltz of     *
*      Applied Mathematics Division of Argonne National Laboratory,            *
*      Argonne, IL 60439.                                                      *
*                                                                              *
*      W A R N I N G:  This routine expects a 64 bit double model.             *
*                                                                              *
*      January  29  1993: First C implementation for PowerPC.                  *
*      April    26  1993: fixed a few bugs in the interval [xbig,inf].         *
*      July     14  1993: added #pragma fenv_access. This function is now      *
*                         using the the string oriented ÔnanÕ.  replaced       *
*                         feholdexcept by _feprocentry to guard rounding.      *
*      July     29  1993: corrected the string nan.                            *
*      October  07  1993: removed the raising of the overflow flag for arg= °. *
*      September19  1994: changed all environemental funtions to __setflm,     *
*                         changed HUGE_VAL to Huge.d for perfrormance.       *
*      January  11  1995: a humilating error corrected.  in the interval       *
*                         [12,178], the nonexistant array element c[7] is      *
*                         addressed.  it should be c[6].                       *
*                         a short error analysis reveals that in double        *
*                         precision referencing c[7] instead of c[6] has no    *
*                         impact on the accuracy of the result, provided that  *
*                         the compiler assigns 0.0 to c[7], which the ibm xlc  *
*                         does.  this explains why the double precision        *
*                         gamma function passed accuracy tests.  the relative  *
*                         error in extended is at most 5.56E-17 and occurs     *
*                         for x=12.0.  the calculation is no longer affected   *
*                         for arguments x³19.                                  *
*                                                                              *
********************************************************************************
*                                                                              *
*                              G  A  M  M  A  (  X  )                          *
*                                                                              *
*      The gamma function is an increasing function over [2,inf].  For large   *
*      enough x, it behaves like [ e^( x ln ( x/ e ) ] / sqrt(x/pi).  It may   *
*      be more appropriate to work with the logorithm of Gamma.                *
*                                                                              *
*      This routine calculates the gamma function for a real argument x.       *
*      Computation is based on an algorithm outlined in reference 1 below.     *
*      The program uses rational functions that approximate the gamma          *
*      function to at least 20 significant decimal digits.  Coefficients       *
*      for the approximation over the interval (1,2) are unpublished.          *
*      Those for the approximation for x >= 12 are from reference 2 below.     *
*                                                                              *
********************************************************************************
*                                                                              *
*      Explanation of machine-dependent constants:                             *
*                                                                              *
*      xbig     - the largest argument for which gamma(x) is representable     *
*                 in the machine, i.e., the solution to the equation           *
*                 gamma ( xbig ) = 2 ** MaxExp, where MaxExp is the maximum    *
*                 power of 2 before infinity;                                  *
*      xinf     - the largest machine representable floating-point number      *
*                 before infinity, approximately 2 ** MaxExp;                  *
*      eps      - the smallest positive floating-point number such that        *
*                 1.0 + eps > 1.0;                                             *
*      MinimumX - the smallest positive floating-point number such that        *
*                 1/MinimumX is machine representable.                         *
*                                                                              *
*      Approximate values for the macintosh and the powerpc are:               *
*                                                                              *
*                                base       MaxExp        xbig                 *
*                                                                              *
*      Macintosh     extended     2         16382        1755.36               *
*      PowerPC        double      2          1023        171.624               *
*                                                                              *
*                                 xinf         eps        MinimumX             *
*                                                                              *
*      Macintosh     extended   1.19x+4932   5.42x-20    8.40x-4933            *
*      PowerPC        double    1.79d+308    2.22d-16    2.23d-308             *
*                                                                              *
********************************************************************************
*                                                                              *
*      The program returns a quiet NaN for singularities and infinity when     *
*      overflow occurs.  The computation is believed to be free of undeserved  *
*      underflow and overflow.                                                 *
*                                                                              *
*      References:                                                             *
*                                                                              *
*      [1] "An Overview of Software Development for Special Functions",        *
*          W. J. Cody, Lecture Notes in Mathematics, 506, Numerical Analysis   *
*          Dundee, 1975, G. A. Watson (ed.), Springer Verlag, Berlin, 1976.    *
*                                                                              *
*      [2] Computer Approximations, Hart, Et. Al., Wiley and sons, New York    *
*          1968.                                                               *
*                                                                              *
*******************************************************************************/

#include      "math.h"
#include      "fenv.h"
#include      "fp_private.h"
#include      "fenv_private.h"

/*******************************************************************************
*            Functions needed for the computation.                             *
*******************************************************************************/

/*     the following fp.h functions are used:                                 */
/*     exp, log, sin, __fpclassifyd and nan;                                  */
/*     the following environmental functions are used:                        */
/*     __setflm.                                                              */


/*******************************************************************************
*     Coefficients for P in gamma approximation over [1,2] in decreasing order.*
*******************************************************************************/

static const double p[8] = { -1.71618513886549492533811e+0,
                        2.47656508055759199108314e+1,
                       -3.79804256470945635097577e+2,
                        6.29331155312818442661052e+2,
                        8.66966202790413211295064e+2,
                       -3.14512729688483675254357e+4,
                       -3.61444134186911729807069e+4,
                        6.64561438202405440627855e+4 };

/*******************************************************************************
*     Coefficients for Q in gamma approximation over [1,2] in decreasing order.*
*******************************************************************************/

static const double q[8] = { -3.08402300119738975254353e+1,
                        3.15350626979604161529144e+2,
                       -1.01515636749021914166146e+3,
                       -3.10777167157231109440444e+3,
                        2.25381184209801510330112e+4,
                        4.75584627752788110767815e+3,
                       -1.34659959864969306392456e+5,
                       -1.15132259675553483497211e+5 };
                         
/*******************************************************************************
*     Coefficients for minimax approximation over [12, INF].                   *
*******************************************************************************/

static const double c[7] = { -1.910444077728e-03,
                        8.4171387781295e-04,
                       -5.952379913043012e-04,
                        7.93650793500350248e-04,
                       -2.777777777777681622553e-03,
                        8.333333333333333331554247e-02,
                        5.7083835261e-03 };

static const double LogSqrt2pi = 0.9189385332046727417803297e+0;
static const double pi         = 3.1415926535897932384626434e+0;
static const double xbig       = 171.624e+0;
static const double MinimumX   = 2.23e-308;
static const double eps        = 2.22e-16;
static const hexdouble Huge    = HEXDOUBLE(0x7FF00000, 0x00000000);
static const hexdouble MinusHuge    = HEXDOUBLE(0xFFF00000, 0x00000000);

#define      GAMMA_NAN      "42"
#define      SET_INVALID    0x01000000

#pragma STDC FENV_ACCESS ON

double tgamma ( double x )
      {
      register int n, parity, i;
      register double y, y1, result, fact, IsItAnInt, z, numerator, 
	                denominator, ysquared, sum; 
      hexdouble OldEnvironment;
    
      FEGETENVD( OldEnvironment.d );               // save environment, set default
      FESETENVD( 0.0 );
	
/*******************************************************************************
*     The next switch will decipher what sort of argument we have. If argument *
*     is SNaN then a QNaN has to be returned and the invalid flag signaled.    * 
*******************************************************************************/

      switch ( __fpclassifyd ( x ) )
            {
            case FP_NAN:
                  x *= 2.0;                  /* quiets NaN */
                  FESETENVD( OldEnvironment.d ); //   restore caller's environment
                  return x;
                  
            case FP_ZERO:
                  OldEnvironment.i.lo |= FE_DIVBYZERO;
                  FESETENVD( OldEnvironment.d );
                  return copysign( Huge.d, x);

             case FP_INFINITE:
                  if ( x > 0.0 )
                        x = Huge.d;
                  else
                        {
                        x = nan ( GAMMA_NAN );
                        OldEnvironment.i.lo |= SET_INVALID;
                        }

                  FESETENVD( OldEnvironment.d );
                  return x;
                  
            default:                  /*      NORMALNUM and DENORMALNUM      */
                  break;
            }
      
      parity = 0;
      fact = 1.0;
      n = 0;
      y = x;

/*******************************************************************************
*     The argument is negative.                                                *
*******************************************************************************/

      if ( y <= 0.0 )
            {
            y = - x;
			if ( y < MinimumX )
			      {
                  OldEnvironment.i.lo |= FE_OVERFLOW;
                  FESETENVD( OldEnvironment.d );
                  return MinusHuge.d;
				  }
            y1 = trunc ( y );
            IsItAnInt = y - y1;
            if ( IsItAnInt != 0.0 )                   /* is it an integer?   */
                  {                                   /* is it odd or even?  */
                  if ( y1 != trunc ( y1 * 0.5 ) * 2.0 ) parity = 1;
                  fact = - pi / sin ( pi * IsItAnInt );
                  y += 1.0;
                  }
            else
                  {
                  OldEnvironment.i.lo |= SET_INVALID;
                  FESETENVD( OldEnvironment.d );
                  return nan ( GAMMA_NAN );
                  }
            }

/*******************************************************************************
*     The argument is positive.                                                *
*******************************************************************************/

     if ( y < eps )                         /* argument is less than epsilon. */
            {
            if ( y >= MinimumX )          /* x is in [MinimumX,eps].          */
                  result = 1.0 / y;
            else                          /* othewise, x is in [0,MinimumX).  */
                  {
                  OldEnvironment.i.lo |= FE_OVERFLOW;
                  FESETENVD( OldEnvironment.d );
                  return Huge.d;
                  }
            }
      else if ( y < 12.0 )                 /* argument x is eps < x < 12.0.  */
            {
            y1 = y;
            if ( y < 1.0 )                 /* x is in (eps, 1.0).            */
                  {
                  z = y;
                  y += 1.0;
                  }
            else                           /* x is in [1.0,12.0].            */
                  {
                  n = ( int ) y - 1;
                  y -= ( double ) n;
                  z = y - 1.0;
                  }
            numerator = 0.0;
            denominator = 1.0;
            for ( i = 0; i < 8; i++ )
                  {
                  numerator = ( numerator + p[i] ) * z;
                  denominator = denominator * z + q[i];
                  }
            result = numerator / denominator + 1.0;
            if ( y1 < y )
                  result /= y1;
            else if ( y1 > y )
                  {
                  for ( i = 0; i < n; i++ )
                        {
                        result *= y;
                        y += 1.0;
                        }
                  }
            }
      else
            {
            if ( x <= xbig )
                  {
                  ysquared = y * y;
                  sum = c[6];
                  for ( i = 0; i < 6; i++ )
                        sum = sum / ysquared + c[i];
                  sum = sum / y - y + LogSqrt2pi;
                  sum += ( y - 0.5 ) * log ( y );
                  result = exp ( sum );
                  }
            else
                  {
                  OldEnvironment.i.lo |= FE_OVERFLOW;
                  FESETENVD( OldEnvironment.d );         //   restore caller's environment
                  return Huge.d;
                  }
            }
      if ( parity ) result = - result;
      if ( fact != 1.0 ) result = fact / result;
      FESETENVD( OldEnvironment.d );         //   restore caller's environment
      return result;
      }
