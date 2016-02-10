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
*      File erfcerf.c,                                                         *
*      Functions erf(x) and cerf(x),                                           *
*      Implementation of the error and complementary error function for the    *
*      PowerPC.                                                                *
*                                                                              *
*      Copyright c 1993 Apple Computer, Inc.  All rights reserved.             *
*                                                                              *
*      Written by Ali Sazegari, started on February 1993,                      *
*                                                                              *
*      based on FORTRAN routines in SpecFun by J. W. Cody of Applied           *
*      Mathematics Division of Argonne National Laboratory, Argonne, IL        *
*      60439.                                                                  *
*                                                                              *
*      W A R N I N G:  This routine expects a 64 bit double model.             *
*                                                                              *
*      February  22  1993: First C implementation for PowerPC.                 *
*      July      14  1993: added #pragma fenv_access, declared variables       *
*                          register automatics, changed feholdexcept to the    *
*                          internal MathLib function _feprocentry.             *
*      October   07  1993: corrected the return of signed zero in erf.         *
*      September 19  1994: replaced the environmental enquiries by __setflm.	 *
*      October   31  1995: corrected an error in the computation of erfc in    *
*                          the interval of [-0.46875,0.0]. changed the value   *
*                          of xbig to disallow flush to zero.                  *
*      November  01  1995: corrected an undeserved underfow flag in erf.       *
*                                                                              *
********************************************************************************
*                                                                              *
*                E  R  F  (  X  )     &     C  E  R  F  (  X  )                *
*                                                                              *
********************************************************************************
*                                                                              *
*      Explanation of machine-dependent constants:                             *
*                                                                              *
*      xmin    = the smallest positive floating-point number.                  *
*      xneg    = the largest negative argument acceptable to ERFCX;            *
*                the negative of the solution to the equation                  *
*                2 * exp ( x * x ) = HUGE_VAL.                                 *
*      xsmall  = argument below which erf(x) may be represented by             *
*                2 * x / sqrt ( pi )  and above which  x * x  will not         *
*                underflow.  A conservative value is the largest machine       *
*                number x such that 1.0 + x = 1.0 to machine precision.        *
*      xbig    = largest argument acceptable to erfc;  solution to             *
*                the equation:  W(x) * ( 1 - 0.5 / x ** 2 ) = xmin,  where     *
*                W(x) = exp ( - x * x ) / ( x * sqrt ( pi ) ).                 *
*      HUGE    = argument above which  1 - 1 / ( 2 * x * x ) = 1  to           *
*                machine precision.  A conservative value is                   *
*                1 / ( 2 * sqrt ( xsmall ) ) .                                 *
*      Maximum = largest acceptable argument to erfcx; the minimum             *
*                of HUGE_VAL and 1 / ( sqrt ( pi ) * xmin ).                   *
*                                                                              *
*      Approximate values for the macintosh and the powerpc are:               *
*                                                                              *
*                          xmin       xinf        xneg     xsmall              *
*                                                                              *
*      Macintosh   (E.P.)                                                      *
*      PowerPC     (D.P.)  2.23D-308   1.79D+308   -26.628  1.11D-16           *
*                                                                              *
*                          xbig       HUGE       Maximum                       *
*                                                                              *
*      Macintosh  (E.P.)                                                       *
*      PowerPC    (D.P.)  26.543      6.71D+7     2.53D+307                    *
*      if not flush to 0  27.2                                                 *
*                                                                              *
********************************************************************************
*                                                                              *
*      Functions required are:                                                 *
*                                                                              *
*      Transandental: exp;                                                     *
*      Auxiliary: trunc, fabs and __fpclassifyd;                               *
*      Environmental: feholdexcept and feupdateenv.                            *
*                                                                              *
*      Reference:                                                              *
*                                                                              *
*      The main computation evaluates near-minimax approximations              *
*      from "Rational Chebyshev approximations for the error function"         *
*      by W. J. Cody, Math. Comp., 1969, PP. 631-638.                          *
*                                                                              *
*      This program uses rational functions that theoretically approximate     *
*      erf(x)  and  erfc(x)  to at least 18 significant decimal digits.        *
*                                                                              *
*******************************************************************************/

#include      "math.h"
#include      "fenv.h"
#include      "fp_private.h"
#include      "fenv_private.h"

#define  EXCEPT_MASK    0x1ff80000

/*******************************************************************************
*            Functions needed for the computation.                             *
*******************************************************************************/

/*     the following fp.h functions are used:                                 */
/*     __fpclassifyd, copysign, trunc, fabs and exp;                          */
/*     the following environmental functions are used:                        */
/*     __setflm.											*/

/*******************************************************************************
*     Coefficients for approximation to erf in [ -0.46875, 0.46875] in         *
*     decreasing order.                                                        *
*******************************************************************************/

static const double a[5] = { 3.16112374387056560e+0,
                       1.13864154151050156e+2,
                       3.77485237685302021e+2,
                       3.20937758913846947e+3,
                       1.85777706184603153e-1 };
                       
static const double b[4] = { 2.36012909523441209e+1,
                       2.44024637934444173e+2,
                       1.28261652607737228e+3,
                       2.84423683343917062e+3 };
     
/*******************************************************************************
*     Coefficients for approximation to erfc in [-4.0,-0.46875)U(0.46875,4.0]  *
*     in decreasing order.                                                     *
*******************************************************************************/

static const double c[9] = { 5.64188496988670089e-1,
                       8.88314979438837594e+0,
                       6.61191906371416295e+1,
                       2.98635138197400131e+2,
                       8.81952221241769090e+2,
                       1.71204761263407058e+3,
                       2.05107837782607147e+3,
                       1.23033935479799725e+3,
                       2.15311535474403846e-8 };

static const double d[8] = { 1.57449261107098347e+1,
                       1.17693950891312499e+2,
                       5.37181101862009858e+2,
                       1.62138957456669019e+3,
                       3.29079923573345963e+3,
                       4.36261909014324716e+3,
                       3.43936767414372164e+3,
                       1.23033935480374942e+3 };
                       
/*******************************************************************************
*    Coefficients for approximation to  erfc in [-inf,-4.0)U(4.0,inf] in       *
*    decreasing order.                                                         *
*******************************************************************************/

static const double p[6] = { 3.05326634961232344e-1,
                       3.60344899949804439e-1,
                       1.25781726111229246e-1,
                       1.60837851487422766e-2,
                       6.58749161529837803e-4,
                       1.63153871373020978e-2 };

static const double q[5] = { 2.56852019228982242e+0,
                       1.87295284992346047e+0,
                       5.27905102951428412e-1,
                       6.05183413124413191e-2,
                       2.33520497626869185e-3 };

static const double InvSqrtPI = 5.6418958354775628695e-1;
static const double xbig      = 27.2e+0;
static const double Maximum   = 2.53e+307;
static const double _HUGE      = 6.71e+7;

#pragma STDC FENV_ACCESS ON

static double ErrFunApprox ( double arg, double result, int which );

/*******************************************************************************
*              E   R   R   O   R      F   U   N   C   T   I   O   N            *
*******************************************************************************/

double erf ( double x )
      {
      register int which = 0;
      register double result = 0.0;
	hexdouble OldEnvironment, NewEnvironment;
      
/*******************************************************************************
*     The next switch will decipher what sort of argument we have. If argument *
*     is SNaN then a QNaN has to be returned and the invalid flag signaled.    * 
*******************************************************************************/

      switch ( __fpclassifyd ( x ) )
            {
            case FP_NAN:
                  x *= 2.0;                  /*    Sets invalid & quiets NaN */
                  return x;
            case FP_ZERO:
                  return x;
             case FP_INFINITE:
                  return (x > 0.0 ? 1.0 : - 1.0);
            default:                  /*      NORMALNUM and DENORMALNUM      */
                  break;
            }

            FEGETENVD( OldEnvironment.d );               // save environment, set default
            FESETENVD( 0.0 );

      result = 1.0;
      result = ErrFunApprox ( x, result, which );

/*******************************************************************************
*      Take care of the negative argument.                                     *
*******************************************************************************/

      result = copysign ( result, x);

      FEGETENVD( NewEnvironment.d );
      OldEnvironment.i.lo |= ( NewEnvironment.i.lo & EXCEPT_MASK );  // Merge new exceptions into old environment
      FESETENVD( OldEnvironment.d );         //   restore caller's environment
       
      return ( result );
      }

/*******************************************************************************
*        C O M P L E M E N T A R Y    E R R O R    F U N C T I O N             *
*******************************************************************************/

double erfc ( double x )
      {
      register int which = 1;
      register double result = 0.0;
      hexdouble OldEnvironment, NewEnvironment;
      
      
/*******************************************************************************
*     The next switch will decipher what sort of argument we have. If argument *
*     is SNaN then a QNaN has to be returned and the invalid flag signaled.    * 
*******************************************************************************/

      switch ( __fpclassifyd ( x ) )
            {
            case FP_NAN:
                  x *= 2.0;                  /*    Sets invalid & quiets NaN */
                  return x;
            case FP_ZERO:
                  return 1.0;
            case FP_INFINITE:
                  return (x > 0.0 ? 0.0 : 2.0);
            default:                  /*      NORMALNUM and DENORMALNUM      */
                  break;
            }
            
            FEGETENVD( OldEnvironment.d );               // save environment, set default
            FESETENVD( 0.0 );
	
      result = 0.0;
      result = ErrFunApprox ( x, result, which );

/*******************************************************************************
*      Take care of the negative argument.                                     *
*******************************************************************************/

      if ( x < 0.0 )
            result = 2.0 - result;
      
      FEGETENVD( NewEnvironment.d );
      OldEnvironment.i.lo |= ( NewEnvironment.i.lo & EXCEPT_MASK );  // Merge new exceptions into old environment
      FESETENVD( OldEnvironment.d );         //   restore caller's environment
     
      return ( result );
      }


/*******************************************************************************
*            C  O  R  E    A  P  P  R  O  X  I  M  A  T  I  O  N               *
*******************************************************************************/

static double ErrFunApprox ( double arg, double result, int which )
      {
      register int i;
      register double x, y, ysquared, numerator, denominator, del; 
      
      x = arg;
      y = fabs ( x );

/*******************************************************************************
*      Evaluate  erfc  for |x| <= 0.46875.                                     *
*******************************************************************************/

      if ( y <= 0.46875e+0 )
            {
            ysquared = 0.0;
            if ( y > 1.11e-16 )
                  ysquared = y * y;
            numerator = a[4] * ysquared;
            denominator = ysquared;
            for ( i = 0; i < 3; i++ )
                  {
                  numerator = ( numerator + a[i] ) * ysquared;
                  denominator = ( denominator + b[i] ) * ysquared;
                  }
            result = y * ( numerator + a[3] ) / ( denominator + b[3] );
            if ( which ) result = 1.0 - result;
            return result;
            }

/*******************************************************************************
*      Evaluate  erfc  for 0.46875 < |x| <= 4.0                                *
*******************************************************************************/
      
      else if ( y <= 4.0 )
            {
            numerator = c[8] * y;
            denominator = y;
            for ( i = 0; i < 7; i++ )
                  {
                  numerator = ( numerator + c[i] ) * y;
                  denominator = ( denominator + d[i] ) * y;
                  }
            result = ( numerator + c[7] ) / ( denominator + d[7] );
		ysquared = trunc ( y * 16.0 ) / 16.0;
		del = ( y - ysquared ) * ( y + ysquared );
		result = exp ( - ysquared * ysquared ) * exp ( - del ) * result;
            }

/*******************************************************************************
*      Evaluate  erfc  for |x| > 4.0                                           *
*******************************************************************************/
      
      else
            {
            if ( y >= xbig )
                  {
                  if ( ( which != 2 ) || ( y >= Maximum ) )
                        {
                        if ( which == 1 )
                                {
                                hexdouble OldEnvironment;
                                FEGETENVD( OldEnvironment.d );
                                OldEnvironment.i.lo |= FE_UNDERFLOW;
                                FESETENVD( OldEnvironment.d );
                                }
                        return result;
                        }
                  if ( y >= _HUGE )
                        {
                        result = InvSqrtPI / y;
                        return result;
                        }
                  }
            ysquared = 1.0 / ( y * y );
            numerator = p[5] * ysquared;
            denominator = ysquared;
            for ( i = 0; i < 4; i++ )
                  {
                  numerator = ( numerator + p[i] ) * ysquared;
                  denominator = ( denominator + q[i] ) * ysquared;
                  }
            result = ysquared * ( numerator + p[4] ) / ( denominator + q[4] );
            result = ( InvSqrtPI - result ) / y;
            ysquared = trunc ( y * 16.0 ) / 16.0;
            del = ( y - ysquared ) * ( y + ysquared );
            result = exp ( - ysquared * ysquared ) * exp ( - del ) * result;
            }
      
/*******************************************************************************
*     if the calling function is erf instead of erfc, take care of the		 *
*     underserved underflow.  otherwise, the computation will produce the	 *
*	exception for erfc.                                                      *
*******************************************************************************/

		if ( which == 0 )
                    {
                    hexdouble OldEnvironment;
                    FEGETENVD( OldEnvironment.d );
                    OldEnvironment.i.lo &= ~FE_UNDERFLOW;
                    FESETENVD( OldEnvironment.d );
                    }
	
	return ( which ) ? result : ( 0.5 - result ) + 0.5;
      }
