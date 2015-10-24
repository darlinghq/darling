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
*      File lgamma.c,                                                          *
*      Functions lgamma(x),                                                    *
*      Implementation of the lgamma function for the PowerPC.                  *
*                                                                              *
*      Copyright c 1993 Apple Computer, Inc.  All rights reserved.             *
*                                                                              *
*      Written by Ali Sazegari, started on February 1993,                      *
*                                                                              *
*      based on FORTRAN routines in SpecFun by J. W. Cody and L. Stoltz of     *
*      Applied Mathematics Division of Argonne National Laboratory,            *
*      Argonne, IL 60439.                                                      *
*                                                                              *
*      W A R N I N G:  This routine expects a 64 bit double model.             *
*                                                                              *
*      January  29  1993: First C implementation for PowerPC.                  *
*      April    23  1993: Fixed an error in the interval [1.5,4).              *
*      July     14  1993: added #pragma fenv_access. This function is now      *
*                         using the the string oriented ÔnanÕ.  replaced       *
*                         feholdexcept by _feprocentry to guard rounding.      *
*      July     29  1993: corrected the string nan.                            *
*      October  07  1993: removed the raising of the overflow flag for arg= °. *
*      September19  1994: changed all environemental funtions to __setflm,     *
*                         changed HUGE_VAL to Huge.d for perfrormance.       *
*      January  11  1995: an error similar to the one in gamma.c corrected.    *
*                         in the interval [12,2.25e+76], the nonexistant       *
*                         array element c[7] is addressed.  it should be c[6]. *
*                         a short error analysis reveals that in double        *
*                         precision referencing c[7] instead of c[6] has no    *
*                         impact on the accuracy of the result, provided that  *
*                         the compiler assigns 0.0 to c[7], which the ibm xlc  *
*                         does.  this explains why the double precision        *
*                         lgamma function passed accuracy tests. the relative  *
*                         error in extended is at most 5.56E-17 and occurs     *
*                         for x=12.0.  the calculation is no longer affected   *
*                         for arguments x³19.                                  *
*                                                                              *
********************************************************************************
*                                                                              *
*                           L  G  A  M  M  A  (  X  )                          *
*                                                                              *
*      This routine calculates the lgamma function for a real positive         *
*      argument x.  Computation is based on an algorithm outlined in           *
*      reference 1 and 2 below.  The program uses rational functions that      *
*      approximate the gamma function to at least 18 significant decimal       *
*      digits.  The approximation for x > 12 is from reference 3, while        *
*      approximations for x < 12.0 are similar to those in reference 1,        *
*      but are unpublished.                                                    *
*                                                                              *
********************************************************************************
*                                                                              *
*      Explanation of machine-dependent constants:                             *
*                                                                              *
*      maxexp    - the smallest positive power of beta that overflows;         *
*      xbig      - the largest argument for which gamma(x) is representable    *
*                  in the machine, i.e., the solution to the equation          *
*                  Log ( gamma ( xbig ) ) = 2 ** maxexp;                       *
*      xinf      - the largest machine representable floating-point number     *
*                  approximately 2 ** maxexp;                                  *
*      eps       - the smallest positive floating-point number such that       *
*                  1.0 + eps > 1.0;                                            *
*      Root4xbig - Rough estimate of the fourth root of xbig.                  *
*                                                                              *
*      Approximate values for the macintosh and the powerpc are:               *
*                                                                              *
*                                base       maxexp        xbig                 *
*                                                                              *
*      Macintosh       (E.P.)     2         16383            ?                 *
*      PowerPC         (D.P.)     2          1024        2.55D+305             *
*                                                                              *
*                                 xinf         eps       Root4xbig             *
*                                                                              *
*      Macintosh       (E.P.)   1.19X+4932   5.42X-20        ?                 *
*      PowerPC         (D.P.)   1.79D+308    2.22D-16    2.23D-308             *
*                                                                              *
********************************************************************************
*                                                                              *
*      The program returns a quiet NaN for singularities and infinity when     * 
*      overflow occurs.  The computation is believed to be free of underflow   *
*      and overflow.                                                           *
*                                                                              *
*      References:                                                             *
*                                                                              *
*      [1] "Chebyshev Approximations for the Natural Logarithm of the Gamma    *
*           Function", W. J. Cody and K. E. Hillstrom, Math. Comp. 21, 1967,   *
*           pp. 198-203.                                                       *
*                                                                              *
*      [2] K. E. Hillstrom, ANL/AMD Program ANLC366S, DGAMMA/DLGAMA, May 1969. *
*                                                                              *
*      [3] Hart, Et. Al., Computer Approximations, Wiley and sons, New York,   *
*          1968.                                                               *
*******************************************************************************/

#ifndef _REENTRANT
    #define _REENTRANT 1
#endif

#include      "math.h"
#include      "fenv.h"
#include      "fp_private.h"
#include      "fenv_private.h"

/*******************************************************************************
*            Functions needed for the computation.                             *
*******************************************************************************/

/*     the following fp.h functions are used:                                 */
/*     __fpclassifyd, nan and log;                                            */
/*     the following environmental functions are used:                        */
/*     __setflm.                                                              */

/*******************************************************************************
*     Coefficients for P1 in lgamma approximation over [0.5,1.5) in decreasing *
*     order.                                                                   *
*******************************************************************************/

static const double d1 = -5.772156649015328605195174e-1;

static const double p1[8] = { 4.945235359296727046734888e+0,
                        2.018112620856775083915565e+2,
                        2.290838373831346393026739e+3,
                        1.131967205903380828685045e+4,
                        2.855724635671635335736389e+4,
                        3.848496228443793359990269e+4,
                        2.637748787624195437963534e+4,
                        7.225813979700288197698961e+3 };
                       
/*******************************************************************************
*     Coefficients for Q1 in gamma approximation over [0.5,1.5) in decreasing  *
*     order.                                                                   *
*******************************************************************************/

static const double q1[8] = { 6.748212550303777196073036e+1,
                        1.113332393857199323513008e+3,
                        7.738757056935398733233834e+3,
                        2.763987074403340708898585e+4,
                        5.499310206226157329794414e+4,
                        6.161122180066002127833352e+4,
                        3.635127591501940507276287e+4,
                        8.785536302431013170870835e+3 };
                        
/*******************************************************************************
*     Coefficients for P2 in lgamma approximation over [1.5,4) in decreasing   *
*     order.                                                                   *
*******************************************************************************/

static const double d2 = 4.227843350984671393993777e-1;

static const double p2[8] = { 4.974607845568932035012064e+0,
                        5.424138599891070494101986e+2,
                        1.550693864978364947665077e+4,
                        1.847932904445632425417223e+5,
                        1.088204769468828767498470e+6,
                        3.338152967987029735917223e+6,
                        5.106661678927352456275255e+6,
                        3.074109054850539556250927e+6 };

/*******************************************************************************
*     Coefficients for Q2 in gamma approximation over [1.5,4) in decreasing    *
*     order.                                                                   *
*******************************************************************************/

static const double q2[8] = { 1.830328399370592604055942e+2,
                        7.765049321445005871323047e+3,
                        1.331903827966074194402448e+5,
                        1.136705821321969608938755e+6,
                        5.267964117437946917577538e+6,
                        1.346701454311101692290052e+7,
                        1.782736530353274213975932e+7,
                        9.533095591844353613395747e+6 };

/*******************************************************************************
*     Coefficients for P4 in lgamma approximation over [4,12) in decreasing    *
*     order.                                                                   *
*******************************************************************************/

static const double d4 = 1.791759469228055000094023e+0;

static const double p4[8] = { 1.474502166059939948905062e+04,
                        2.426813369486704502836312e+06,
                        1.214755574045093227939592e+08,
                        2.663432449630976949898078e+09,
                        2.940378956634553899906876e+10,
                        1.702665737765398868392998e+11,
                        4.926125793377430887588120e+11,
                        5.606251856223951465078242e+11 };

/*******************************************************************************
*     Coefficients for Q4 in gamma approximation over [4,12) in decreasing     *
*     order.                                                                   *
*******************************************************************************/

static const double q4[8] = { 2.690530175870899333379843e+03,
                        6.393885654300092398984238e+05,
                        4.135599930241388052042842e+07,
                        1.120872109616147941376570e+09,
                        1.488613728678813811542398e+10,
                        1.016803586272438228077304e+11,
                        3.417476345507377132798597e+11,
                        4.463158187419713286462081e+11 };
                        
/*******************************************************************************
*     Coefficients for minimax approximation over [12, xbig].                  *
*******************************************************************************/

static const double c[7] = { -1.910444077728e-03,
                        8.4171387781295e-04,
                       -5.952379913043012e-04,
                        7.93650793500350248e-04,
                       -2.777777777777681622553e-03,
                        8.333333333333333331554247e-02,
                        5.7083835261e-03 };

static const double LogSqrt2pi = 0.9189385332046727417803297e+0;
static const double xbig       = 2.55e+305;
static const double Root4xbig  = 2.25e+76;
static const double eps        = 2.22e-16;
static const double pnt68      = 0.6796875e+0;
static const hexdouble Huge    = HEXDOUBLE(0x7FF00000, 0x00000000);

static const double twoTo52      = 0x1.0p+52; // 4503599627370496.0;
static const double pi  =  3.14159265358979311600e+00; /* 0x400921FB, 0x54442D18 */

/*******************************************************************************
*            Value of special function NaN.                                    *
*******************************************************************************/

#define      SET_INVALID    0x01000000
#define      GAMMA_NAN      "42"

#pragma STDC FENV_ACCESS ON

static double lgammaApprox ( double x )
      {
      register int i;
      register double y, result, numerator, denominator, ysquared, 
                      corrector, xMinus1, xMinus2, xMinus4; 
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
                  FESETENVD( OldEnvironment.d );         //   restore caller's environment
                  return x;
                  
            case FP_ZERO:
                  x = Huge.d;
                  OldEnvironment.i.lo |= FE_DIVBYZERO;
                  FESETENVD( OldEnvironment.d );
                  return x;

            case FP_INFINITE:
                  x = Huge.d;
                  FESETENVD( OldEnvironment.d );
                  return x;
                  
            default:                  /*      NORMALNUM and DENORMALNUM      */
                  break;
            }
      
/*******************************************************************************
 *      For negative x, since (G is gamma function)
 *		-x*G(-x)*G(x) = pi/sin(pi*x),
 * 	we have
 * 		G(x) = pi/(sin(pi*x)*(-x)*G(-x))
 *	since G(-x) is positive, sign(G(x)) = sign(sin(pi*x)) for x<0
 *	Hence, for x<0, signgam = sign(sin(pi*x)) and
 *		lgamma(x) = log(|Gamma(x)|)
 *			  = log(pi/(|x*sin(pi*x)|)) - lgamma(-x);
 *******************************************************************************/

      if ( x < 0.0 )
            {
            register double a, y1, IsItAnInt;
            
            if ( x <= -twoTo52 ) // big negative integer?
                {
                OldEnvironment.i.lo |= FE_DIVBYZERO;
                FESETENVD( OldEnvironment.d );
                return Huge.d;
                }
                
            y = - x;
            y1 = trunc ( y );
            IsItAnInt = y - y1; // excess over the boundary
            
            if ( IsItAnInt == 0.0 ) // negative integer?
                {
                OldEnvironment.i.lo |= FE_DIVBYZERO;
                FESETENVD( OldEnvironment.d );
                return Huge.d;
                }
            else
                a = sin ( pi * IsItAnInt );
            
            FESETENVD( OldEnvironment.d );
            return log ( pi / fabs ( a * x ) ) - lgammaApprox ( -x );
            }
      
/*******************************************************************************
*     The argument is positive, if it is bigger than xbig = 2.55e+305 then     *
*     overflow.                                                                *
*******************************************************************************/

      if ( x > xbig )
            {
            OldEnvironment.i.lo |= FE_OVERFLOW;
            FESETENVD( OldEnvironment.d );
            return Huge.d;
            }

      y = x;

/*******************************************************************************
*     x <= eps then return the approximation log(x).                           *
*******************************************************************************/

      if ( y <= eps )
            {
            FESETENVD( OldEnvironment.d );
            return ( - log ( y ) );
            }

/*******************************************************************************
*     x is in (eps,1.5] then use d1, p1 and q1 coefficients.                   *
*******************************************************************************/

      else if ( y <= 1.5 )
            {
            if ( y < pnt68 )
                  {
                  corrector = - log ( y );
                  xMinus1 = y;
                  }
            else
                  {
                  corrector = 0.0;
                  xMinus1 = ( y - 0.5 ) - 0.5;
                  }
            if ( ( y <= 0.5 ) || ( y >= pnt68 ) )
                  {
                  denominator = 1.0;
                  numerator = 0.0;
                  for ( i = 0; i < 8; i++ )
                        {
                        numerator = numerator * xMinus1 + p1[i];
                        denominator = denominator * xMinus1 + q1[i];
                        }
                  result = corrector + ( xMinus1 * ( d1 + xMinus1 * ( numerator / denominator ) ) );
                  }
            else
                  {
                  xMinus2 = ( y - 0.5 ) - 0.5;
                  denominator = 1.0;
                  numerator = 0.0;
                  for ( i = 0; i < 8; i++ )
                        {
                        numerator = numerator * xMinus2 + p2[i];
                        denominator = denominator * xMinus2 + q2[i];
                        }
                  result = corrector + ( xMinus2 * ( d2 + xMinus2 * ( numerator / denominator ) ) );
                  }
            }

/*******************************************************************************
*     x is in (1.5,4.0] then use d2, p2 and q2 coefficients.                   *
*******************************************************************************/

      else if ( y <= 4.0 )
            {
            xMinus2 = y - 2.0;
            denominator = 1.0;
            numerator = 0.0;
            for ( i = 0; i < 8; i++ )
                  {
                  numerator = numerator * xMinus2 + p2[i];
                  denominator = denominator * xMinus2 + q2[i];
                  }
            result = xMinus2 * ( d2 + xMinus2 * ( numerator / denominator ) );
            }
            
/*******************************************************************************
*     x is in (4.0,12.0] then use d4, p4 and q4 coefficients.                  *
*******************************************************************************/

      else if ( y <= 12.0 )
            {
            xMinus4 = y - 4.0;
            denominator = - 1.0;
            numerator = 0.0;
            for ( i = 0; i < 8; i++ )
                  {
                  numerator = numerator * xMinus4 + p4[i];
                  denominator = denominator * xMinus4 + q4[i];
                  }
            result = d4 + xMinus4 * ( numerator / denominator );
            }
      else  /* ( y >= 12.0 ) */
            {
            result = 0.0;
            if ( y <= Root4xbig )
                  {
                  result = c[6];
                  ysquared = y * y;
                  for ( i = 0; i < 6; i++ )
                        result = result / ysquared + c[i];
                  }
            result /= y;
            corrector = log ( y );
            result += LogSqrt2pi - 0.5 * corrector;
            result += y * ( corrector - 1.0 );
            }
      
      FESETENVD( OldEnvironment.d );
      x = rint ( x ); // INEXACT set as a side effect for non integer x  
      return result;
      }
      
double lgamma ( double x )
{
    double g = lgammaApprox ( x );

    signgam = 1;
    if ( x < 0.0 && (g == g))
    {
        double y1 = trunc ( -x );
        hexdouble OldEnvironment;
        
        FEGETENVD( OldEnvironment.d );               // save environment, set default
        FESETENVD( 0.0 );
        
        if ( y1 == trunc ( y1 * 0.5 ) * 2.0 ) 
            signgam = -1;
            
        FESETENVD( OldEnvironment.d );
    }
    
    return g;
}

double lgamma_r ( double x, int *psigngam )
{
    double g = lgammaApprox ( x );

    if (psigngam)
        *psigngam = 1;
        
    if ( x < 0.0 && (g == g))
    {
        double y1 = trunc ( -x );
        hexdouble OldEnvironment;
        
        FEGETENVD( OldEnvironment.d );               // save environment, set default
        FESETENVD( 0.0 );
        
        if ( y1 == trunc ( y1 * 0.5 ) * 2.0 && psigngam) 
            *psigngam = -1;
            
        FESETENVD( OldEnvironment.d );
    }
    
    return g;
}
