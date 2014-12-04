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
*     File atan.c,                                                             *
*     Function atan.                                                           *
*                                                                              *
*     Implementation of arctg based on the approximation provided by           *
*     Taligent, Inc. who based it on sources from IBM.                         *
*                                                                              *
*     Copyright © 1996-2001 Apple Computer, Inc.  All rights reserved.         *
*                                                                              *
*     Written by A. Sazegari, started on June 1996.                            *
*     Modified and ported by Robert A. Murley (ram) for Mac OS X.              *
*                                                                              *
*     A MathLib v4 file.                                                       *
*                                                                              *
*     The principal values of the inverse tangent function are:                *
*                                                                              *
*                 -¹/2 ² atan(x) ² ¹/2,         -° ² x ² +°                    *
*                                                                              *
*     August    26 1996: produced a PowerMathLib¨ version.  this routine       *
*                        does not handle edge cases and does not set the       *
*                        ieee flags.  its computation is suseptible to the     *
*                        inherited rounding direction.  it is assumed that     *
*                        the rounding direction is set to the ieee default.    *
*     October   16 1996: fixed a grave mistake by replacing x2 with xSquared.  *
*     April     14 1997: added the environmental controls for mathlib v3.      *
*                        unlike mathlib v2, v3 will honor the inherited        *
*                        rounding mode since it is easier to set the flags     *
*                        computationally.                                      *
*     July      01 1997: this function no longer honors the rounding mode.     *
*                        corrected an error for nan arguments.                 *
*     July      20 2001: replaced __setflm with FEGETENVD/FESETENVD.           *
*                        replaced DblInHex typedef with hexdouble.             *
*     September 07 2001: added #ifdef __ppc__.                                 *
*     September 09 2001: added more comments.                                  *
*     September 10 2001: added macros to detect PowerPC and correct compiler.  *
*     September 18 2001: added <CoreServices/CoreServices.h> to get to <fp.h>  *
*                        and <fenv.h>.                                         *
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

#include    "fenv_private.h"
#include    "fp_private.h"
#include    "math.h"

extern const unsigned short SqrtTable[];

static const double            c13     =  7.6018324085327799e-02;     /*  0x1.375efd7d7dcbep-4 */
static const double            c11     = -9.0904243427904791e-02;     /* -0x1.745802097294ep-4 */
static const double            c9      =  1.1111109821671356e-01;     /*  0x1.c71c6e5103cddp-4 */
static const double            c7      = -1.4285714283952728e-01;     /* -0x1.24924923f7566p-3 */
static const double            c5      =  1.9999999999998854e-01;     /*  0x1.99999999997fdp-3 */
static const double            c3      = -3.3333333333333330e-01;     /* -0x1.5555555555555p-2 */

static const double Rint        = 6.755399441055744e15;        /* 0x1.8p52              */
static const double PiOver2     = 1.570796326794896619231322;  /* head of ¹/2           */
static const double PiOver2Tail = 6.1232339957367660e-17;      /* tail of ¹/2           */
static const hexdouble Pi       = HEXDOUBLE(0x400921fb, 0x54442d18);
static const double kMinNormal  = 0x1.0p-1022;                 // 2.2250738585072014e-308;

/*******************************************************************************
********************************************************************************
*                              A   T   A   N                                   *
********************************************************************************
*******************************************************************************/

struct tableEntry                             /* tanatantable entry structure */
      {
      double p;
      double f5;
      double f4;
      double f3;
      double f2;
      double f1;
      double f0;
      };

extern const uint32_t tanatantable[];

static const hexdouble Big       = HEXDOUBLE(0x434d0297, 0x00000000);

double atan ( double x )
{
      hexdouble reducedX;
      register double fabsOfx, xSquared, xFourth, xThird, temp1, temp2, result, y, z;
      struct tableEntry *tablePointer;

      register double FPR_env, FPR_z, FPR_half, FPR_one, FPR_256, FPR_kMinNormal;
      register double FPR_PiDiv2, FPR_PiDiv2Tail, FPR_kRint;
      register double FPR_c13, FPR_c11, FPR_c9, FPR_c7, FPR_c5, FPR_c3, FPR_r, FPR_t;
      register struct tableEntry *pT;

      fabsOfx = __FABS ( x );
      
      if (unlikely(x != x))
            return x;
            
      FPR_z = 0.0;						FPR_half = 0.5;
      
      FPR_one = 1.0;						FPR_256 = 256.0;
      
      FPR_PiDiv2 = PiOver2;					FPR_PiDiv2Tail = PiOver2Tail;
      
      FPR_kRint = Rint;						FPR_t = Big.d;

/*******************************************************************************
*     initialization of table pointer.                                         *
*******************************************************************************/

      tablePointer = ( struct tableEntry * ) ( tanatantable - ( 16 * 14 ) );

/*******************************************************************************
*     |x| > 1.0 or NaN.                                                        *
*******************************************************************************/

      FEGETENVD( FPR_env );
      __ENSURE( FPR_z, FPR_kRint, FPR_256 ); __ENSURE( FPR_half, FPR_one, FPR_PiDiv2 );

      if ( fabsOfx > FPR_one ) 
      {
            __NOOP;
            y = FPR_one / fabsOfx;  // Executes in issue slot 1 hence FPU 1
            __NOOP;

/*******************************************************************************
*     |x| is nontrivial.                                                       *
*******************************************************************************/

            if (likely( fabsOfx < FPR_t ))
            {
                  register double yTail, z, resultHead, resultTail;
                  
                  xSquared = __FMUL( y, y ); 
                  FPR_t = 16.0;
                  FPR_r = __FMADD( FPR_256, y, FPR_kRint );
                  
                  __NOOP;
                  __NOOP;
                  __NOOP;
                  reducedX.d = FPR_r;

/*******************************************************************************
*     |x| > 16.0                                                               *
*******************************************************************************/
                  if ( fabsOfx > FPR_t ) 
                  {
                        xFourth = __FMUL( xSquared, xSquared );		xThird  = __FMUL( xSquared, y );
                        FPR_c13 = c13;					
                        
                        FPR_t = __FNMSUB( fabsOfx, y, FPR_one );	resultHead   = FPR_PiDiv2 - y;
                        FPR_c11 = c11;					
                        
                        FPR_c9 = c9;
                        yTail   = __FMUL( y, FPR_t ); 			resultTail   = ( resultHead - FPR_PiDiv2 ) + y;
                        
                        FPR_c7 = c7;
                        temp1   = __FMADD( xFourth, FPR_c13, FPR_c9);	temp2   = __FMADD( xFourth, FPR_c11, FPR_c7 );
                            
                        FPR_c5 = c5;					FPR_c3 = c3;
                        temp1   = __FMADD( xFourth, temp1, FPR_c5 );	temp2   = __FMADD( xFourth, temp2, FPR_c3 );
                                
                        temp1   = __FMADD( xSquared, temp1, temp2 );
                        
                        temp1   = __FNMSUB( xThird, temp1, FPR_PiDiv2Tail );    /* correction for ¹/2         */
                        
                        if ( x > FPR_z )                                     	/* adjust sign of result      */
                            result = ( ( ( temp1 - yTail ) - resultTail ) + resultHead );
                        else
                            result = ( ( ( yTail - temp1 ) + resultTail ) - resultHead );
                            
                        FESETENVD( FPR_env );                     		/* restore the environment    */
                        __PROG_INEXACT( FPR_PiDiv2 );

                        return result;
                  }
            
/*******************************************************************************
*     1 <= |x| <= 16  use table-driven approximation for arctg(y = 1/|x|).     *
*******************************************************************************/
                  
                  pT = &(tablePointer[reducedX.i.lo]);		FPR_t = __FNMSUB( fabsOfx, y, FPR_one );
                
                  FPR_c13 = pT->p;				FPR_c11 = pT->f5;
                  yTail = __FMUL( y, FPR_t );                    z = y - FPR_c13 + yTail;	/* x delta    */
                              
                  FPR_c9 = pT->f4;				FPR_c7 = pT->f3;
                  temp1 = __FMADD( FPR_c11, z, FPR_c9 );	__NOOP;
            
                  temp1 = __FMADD( temp1, z, FPR_c7 );		__NOOP;
                  FPR_c5 = pT->f2;				FPR_c3 = pT->f1;
            
                  FPR_t = pT->f0;
                  temp1 = __FMADD( temp1, z, FPR_c5 );
                  temp1 = __FMADD( temp1, z, FPR_c3 );
                                                
                  resultHead = FPR_PiDiv2 - FPR_t;   				/* zeroth order term          */
                  
                 if ( x > FPR_z )                                            	/* adjust for sign of x       */
                        result = ( __FNMSUB( z, temp1, FPR_PiDiv2Tail ) + resultHead );
                  else
                        result = ( __FMSUB( z, temp1, FPR_PiDiv2Tail ) - resultHead );  
                            
                  FESETENVD( FPR_env );                     			/* restore the environment    */
                  __PROG_INEXACT( FPR_PiDiv2 );
                  
                  return result;
            }

/*******************************************************************************
*     |x| is huge, or INF.                                                     *
*     For x = INF, then the expression ¹/2 ± ¹tail would return the round up   *
*     or down version of atan if rounding is taken into consideration.         *
*     otherwise, just ±¹/2 would be delivered.                                 *
*******************************************************************************/
            else 
            {                                                         		/* |x| is huge, or INF        */
                  FESETENVD( FPR_env );                     			/* restore the environment    */
                  FPR_t = Pi.d;
                  if ( x > FPR_z )                                       	/* positive x returns ¹/2     */
                        result = __FMADD( FPR_t, FPR_half, FPR_PiDiv2Tail );
                  else                                                      	/* negative x returns -¹/2    */
                        result =  ( - FPR_t * FPR_half - FPR_PiDiv2Tail );
                            
                  __PROG_INEXACT( FPR_PiDiv2 );
                  return result;
            }
      }
      

/*******************************************************************************
*     |x| <= 1.0.                                                              *
*******************************************************************************/

      FPR_t = .0625;
      reducedX.d = __FMADD( FPR_256, fabsOfx, FPR_kRint );
      xSquared = __FMUL( x, x );
    
/*******************************************************************************
*     1.0/16 < |x| < 1  use table-driven approximation for arctg(x).           *
*******************************************************************************/
      if ( fabsOfx > FPR_t ) 
      {
            pT = &(tablePointer[reducedX.i.lo]);		__NOOP;
            
            FPR_c13 = pT->p;					FPR_c11 = pT->f5;
            z = fabsOfx - FPR_c13;                    		__NOOP;	/* x delta */
            
            FPR_c9 = pT->f4;					FPR_c7 = pT->f3;
            temp1 = __FMADD( FPR_c11, z, FPR_c9 );		__NOOP;
            
            temp1 = __FMADD( temp1, z, FPR_c7 );		__NOOP;
            FPR_c5 = pT->f2;					FPR_c3 = pT->f1;
            
            FPR_t = pT->f0;
            temp1 = __FMADD( temp1, z, FPR_c5 );
            temp1 = __FMADD( temp1, z, FPR_c3 );
                        
            if ( x > FPR_z )                                    		/* adjust for sign of x       */
                  result = __FMADD( z, temp1, FPR_t );
            else 
                  result = - z * temp1 - FPR_t;

            FESETENVD( FPR_env );                               		/* restore the environment    */
            __PROG_INEXACT( FPR_PiDiv2 );
            
            return result;
      }

/*******************************************************************************
*     |x| <= 1.0/16 fast, simple polynomial approximation.                     *
*******************************************************************************/

      if (unlikely( fabsOfx == FPR_z ))
      {
            FESETENVD( FPR_env );                               		/* restore the environment    */
            return x; 								/* +0 or -0 preserved */
      }
            
      xFourth = __FMUL( xSquared, xSquared );			xThird  = __FMUL( xSquared, x );
      FPR_c13 = c13;						FPR_c11 = c11;
    
      FPR_c9 = c9;						FPR_c7 = c7;
      temp1   = __FMADD( xFourth, FPR_c13, FPR_c9);		temp2   = __FMADD( xFourth, FPR_c11, FPR_c7 );
        
      FPR_c5 = c5;						FPR_c3 = c3;
      temp1   = __FMADD( xFourth, temp1, FPR_c5 );		temp2   = __FMADD( xFourth, temp2, FPR_c3 );
        
      temp1   = __FMADD( xSquared, temp1, temp2 );
      FPR_kMinNormal = kMinNormal;

      result = __FMADD( xThird, temp1, x );
      
      FESETENVD( FPR_env );                                     		/* restore the environment    */
      if (likely( __FABS( result ) >= FPR_kMinNormal ))
      {
            __PROG_INEXACT( FPR_PiDiv2 );
            return result;
      }
      else
      {
            __PROG_UF_INEXACT( FPR_kMinNormal );
            return result;
      }
}

//
// For atan2(). Mean and lean.
//
double atanCore ( double fabsOfx ) // absolute value is passed by caller!
{
      hexdouble reducedX;
      register double xSquared, xFourth, xThird, temp1, temp2, result, z;
      struct tableEntry *tablePointer;

      register double FPR_z, FPR_256, FPR_kRint;
      register double FPR_c13, FPR_c11, FPR_c9, FPR_c7, FPR_c5, FPR_c3, FPR_r, FPR_s, FPR_t;
      register struct tableEntry *pT;

      
      FPR_256 = 256.0;						FPR_kRint = Rint;
      
      FPR_r = __FMADD( FPR_256, fabsOfx, FPR_kRint );
      reducedX.d = FPR_r;
      
      FPR_s = .0625;						FPR_z = 0.0;
    
/*******************************************************************************
*     initialization of table pointer.                                         *
*******************************************************************************/

      tablePointer = ( struct tableEntry * ) ( tanatantable - ( 16 * 14 ) );
      xSquared = __FMUL( fabsOfx, fabsOfx );		__ENSURE( FPR_z, FPR_z, FPR_s );
      
/*******************************************************************************
*     |x| <= 1.0.                                                              *
*******************************************************************************/
    
/*******************************************************************************
*     1.0/16 < |x| < 1  use table-driven approximation for arctg(x).           *
*******************************************************************************/

      if ( fabsOfx > FPR_s ) 
      {            
            pT = &(tablePointer[reducedX.i.lo]);
                        
            FPR_c13 = pT->p;					FPR_c11 = pT->f5;
            z = fabsOfx - FPR_c13;                    		__NOOP;		/* x delta */
            
            FPR_c9 = pT->f4;					FPR_c7 = pT->f3;
            temp1 = __FMADD( FPR_c11, z, FPR_c9 );		__NOOP;
            
            temp1 = __FMADD( temp1, z, FPR_c7 );		__NOOP;
            FPR_c5 = pT->f2;					FPR_c3 = pT->f1;
            
            FPR_t = pT->f0;
            temp1 = __FMADD( temp1, z, FPR_c5 );
            temp1 = __FMADD( temp1, z, FPR_c3 );
                        
            result = __FMADD( z, temp1, FPR_t );

            return result;
      }

/*******************************************************************************
*     |x| <= 1.0/16 fast, simple polynomial approximation.                     *
*******************************************************************************/

      if (unlikely( fabsOfx == FPR_z ))
            return fabsOfx;
            
      xFourth = __FMUL( xSquared, xSquared );			xThird  = __FMUL( xSquared, fabsOfx );
      FPR_c13 = c13;						FPR_c11 = c11;
    
      FPR_c9 = c9;						FPR_c7 = c7;
      temp1   = __FMADD( xFourth, FPR_c13, FPR_c9);		temp2   = __FMADD( xFourth, FPR_c11, FPR_c7 );
        
      FPR_c5 = c5;						FPR_c3 = c3;
      temp1   = __FMADD( xFourth, temp1, FPR_c5 );		temp2   = __FMADD( xFourth, temp2, FPR_c3 );
        
      temp1   = __FMADD( xSquared, temp1, temp2 );

      result = __FMADD( xThird, temp1, fabsOfx );
      
      return result;
}

double atanCoreInv ( double fabsOfx ) // absolute value is passed by caller!
{
      hexdouble reducedX;
      register double xSquared, xFourth, xThird, temp1, temp2, result, y;
      struct tableEntry *tablePointer;

      register double FPR_z, FPR_half, FPR_one, FPR_256;
      register double FPR_PiDiv2, FPR_PiDiv2Tail, FPR_kRint;
      register double FPR_c13, FPR_c11, FPR_c9, FPR_c7, FPR_c5, FPR_c3, FPR_r, FPR_s, FPR_t;
      register struct tableEntry *pT;

      FPR_s = Big.d; 						y = 1.0 / fabsOfx; // slot 1 hence fpu1
      
      FPR_z = 0.0;						FPR_half = 0.5;
      
      FPR_one = 1.0;						FPR_256 = 256.0;
      
      FPR_PiDiv2 = PiOver2;					FPR_PiDiv2Tail = PiOver2Tail;
      
      __ENSURE( FPR_half, FPR_one, FPR_PiDiv2 ); //slot 0 hence fpu0
      FPR_kRint = Rint;
      __ENSURE( FPR_z, FPR_kRint, FPR_256 ); //slot 3 hence fpu0
    

/*******************************************************************************
*     initialization of table pointer.                                         *
*******************************************************************************/

      tablePointer = ( struct tableEntry * ) ( tanatantable - ( 16 * 14 ) );

/*******************************************************************************
*     |x| > 1.0 or NaN.                                                        *
*******************************************************************************/
      {

/*******************************************************************************
*     |x| is nontrivial.                                                       *
*******************************************************************************/

            if (likely( fabsOfx < FPR_s ))
            {
                  register double yTail, z, resultHead, resultTail;
                  
                  FPR_r = __FMADD( FPR_256, y, FPR_kRint );  
                  FPR_s = 16.0;
                  xSquared = __FMUL( y, y ); 

                  __NOOP;
                  __NOOP;
                  __NOOP;
                  reducedX.d = FPR_r;

/*******************************************************************************
*     |x| > 16.0                                                               *
*******************************************************************************/
                  if ( fabsOfx > FPR_s ) 
                  {
                        xFourth = __FMUL( xSquared, xSquared );		xThird  = __FMUL( xSquared, y );
                        FPR_c13 = c13;					
                        
                        FPR_t = __FNMSUB( fabsOfx, y, FPR_one );	resultHead   = FPR_PiDiv2 - y;
                        FPR_c11 = c11;					
                        
                        FPR_c9 = c9;
                        yTail   = __FMUL( y, FPR_t ); 			resultTail   = ( resultHead - FPR_PiDiv2 ) + y;
                        
                        FPR_c7 = c7;
                        temp1   = __FMADD( xFourth, FPR_c13, FPR_c9);	temp2   = __FMADD( xFourth, FPR_c11, FPR_c7 );
                            
                        FPR_c5 = c5;					FPR_c3 = c3;
                        temp1   = __FMADD( xFourth, temp1, FPR_c5 );	temp2   = __FMADD( xFourth, temp2, FPR_c3 );
                                
                        temp1   = __FMADD( xSquared, temp1, temp2 );
                        
                        temp1   = __FNMSUB( xThird, temp1, FPR_PiDiv2Tail );    /* correction for ¹/2         */
                        
                        result = ( ( ( temp1 - yTail ) - resultTail ) + resultHead );
                            
                        return result;
                  }
            
/*******************************************************************************
*     1 <= |x| <= 16  use table-driven approximation for arctg(y = 1/|x|).     *
*******************************************************************************/
                 
                  pT = &(tablePointer[reducedX.i.lo]);		FPR_t = __FNMSUB( fabsOfx, y, FPR_one );
                
                  FPR_c13 = pT->p;				FPR_c11 = pT->f5;
                  yTail = __FMUL( y, FPR_t );                    z = y - FPR_c13 + yTail;	/* x delta    */
                              
                  FPR_c9 = pT->f4;				FPR_c7 = pT->f3;
                  temp1 = __FMADD( FPR_c11, z, FPR_c9 );	__NOOP;
            
                  temp1 = __FMADD( temp1, z, FPR_c7 );		__NOOP;
                  FPR_c5 = pT->f2;				FPR_c3 = pT->f1;
            
                  FPR_t = pT->f0;
                  temp1 = __FMADD( temp1, z, FPR_c5 );
                  temp1 = __FMADD( temp1, z, FPR_c3 );
                                                
                  resultHead = FPR_PiDiv2 - FPR_t;   				/* zeroth order term          */
                  
                  result = ( __FNMSUB( z, temp1, FPR_PiDiv2Tail ) + resultHead );
                            
                  return result;
            }

/*******************************************************************************
*     |x| is huge, INF, or NaN.                                                *
*     For x = INF, then the expression ¹/2 ± ¹tail would return the round up   *
*     or down version of atan if rounding is taken into consideration.         *
*     otherwise, just ±¹/2 would be delivered.                                 *
*******************************************************************************/
            else 
            {                                                         		/* |x| is huge, INF, or NaN   */
                  if ( fabsOfx != fabsOfx )                                             	/* NaN argument is returned   */
                        result = fabsOfx;
                  else 
                  {
                    /* positive x returns ¹/2     */
                    FPR_t = Pi.d;
                    result = __FMADD( FPR_t, FPR_half, FPR_PiDiv2Tail );
                  }                
                  return result;
            }
      }
}
