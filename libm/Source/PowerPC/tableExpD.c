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
*     File tableExpD.c,                                                        *
*     Functions exp, exp2 and expm1.                                           *
*     Implementation of exp(x) based on IBM/Taligent table method.             *
*                                                                              *
*     Copyright © 1997-2001 Apple Computer, Inc.  All rights reserved.         *
*                                                                              *
*     Written by Ali Sazegari, started on April 1997.                          *
*     Modified and ported by Robert A. Murley (ram) for Mac OS X.              *
*                                                                              *
*     A MathLib v4 file.                                                       *
*                                                                              *
*     November 07 2001: removed exp2 to prevent conflict with CarbonCore.      *
*     November 06 2001: commented out warning about Intel architectures.       *
*                       changed i386 stub to call abort().                     *
*     November 02 2001: added stub for i386 version of exp2.                   *
*     April    28 1997: port of the ibm/taligent exp routines.                 *
*     July     16 1997: changed the rounding direction sensitivity of          *
*                       delivered results to default rounding at all times.    *
*     August   28 2001: replaced __setflm with FEGETENVD/FESETENVD.            *
*                       replaced DblInHex typedef with hexdouble.              *
*                       used standard exception symbols from fenv.h.           *
*                       added #ifdef __ppc__.                                  *
*     September 09 2001: added more comments.                                  *
*     September 10 2001: added macros to detect PowerPC and correct compiler.  *
*     September 18 2001: added <CoreServices/CoreServices.h> to get <fenv.h>.  *
*     October   08 2001: removed <CoreServices/CoreServices.h>.                *
*                        changed compiler errors to warnings.                  *
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

#include      "math.h"
#include      "fenv_private.h"
#include      "fp_private.h"

/*******************************************************************************
*      Floating-point constants.                                               *
*******************************************************************************/

static const double rintFactor    = 6.7553994410557440000e+15;  /* 0x43380000, 0x00000000 */
static const double oneOverLn2    = 1.4426950408889633000e+00;  /* 0x3ff71547, 0x652b82fe */
static const double ln2Head       = 6.9314718055994530000e-01;  /* 0x3fe62e42, 0xfefa39ef */
static const double ln2Tail       = 2.3190468138462996000e-17;  /* 0x3c7abc9e, 0x3b39803f */
static const double maxExp        =  7.0978271289338397000e+02; /* 0x40862e42, 0xfefa39ef */
static const double denormal      =  2.9387358770557188000e-39; /* 0x37f00000, 0x00000000 */
static const hexdouble infinity   = HEXDOUBLE(0x7ff00000, 0x00000000);

/*******************************************************************************
*      Approximation coefficients.                                             *
*******************************************************************************/

static const double cm1 = 8.3333336309523691e-03;               /* 0x3f811111,0x1b4af38e */
static const double c0  = 4.1666668402777808000e-02;            /* 0x3fa55555,0x643f1505 */
static const double c1  = 1.6666666666666655000e-01;            /* 0x3fc55555,0x55555551 */
static const double c2  = 4.9999999999999955000e-01;            /* 0x3fdfffff,0xfffffff8 */

extern const uint32_t expTable[];

struct expTableEntry 
      { 
      double x;
      double f;
      };

static const double kMinNormal = 0x1.0p-1022;                 // 2.2250738585072014e-308;
static const double kMaxNormal = 1.7976931348623157e308;

#if !defined(BUILDING_FOR_CARBONCORE_LEGACY)
/*******************************************************************************
*      Floating-point constants.                                               *
*******************************************************************************/
static const double minExp        = -7.4513321910194122000e+02; /* 0xc0874910, 0xd52d3052 */
static const double oneOverDenorm =  3.402823669209384635e+38;  /* 0x47f00000, 0x00000000 */

/*******************************************************************************
*      Approximation coefficients.                                             *
*******************************************************************************/

static const double cc4 = 0.5000000000000000000;
static const double cc3 = 0.16666666666663809522820;
static const double cc2 = 0.04166666666666111110939;
static const double cc1 = 0.008333338095239329810170;
static const double cc0 = 0.001388889583333492938381;

/*******************************************************************************
*                                                                              *
*    The base e exponential function.  					       *
*                                                                              *
********************************************************************************
*                                                                              *
*    Raised exceptions are inexact, overflow & inexact and underflow & inexact.*
*                                                                              *
*******************************************************************************/

double exp ( double x ) 
{
      hexdouble scale, xInHex, yInHex;
      register double d, y, yTail, z, zTail, z2, temp1, temp2, power, result;
      register int32_t i;
      register struct expTableEntry *tablePointer, *pT;
      
      register double FPR_oneOverLn2, FPR_rintFactor, FPR_ln2Head, FPR_ln2Tail, FPR_h, FPR_z, FPR_512, FPR_scale;
      register double FPR_env, FPR_x, FPR_f, FPR_cm1, FPR_c0, FPR_c1, FPR_c2;
      
      FPR_x = __FABS( x );
      
      FPR_z = 0.0;				 	FPR_512 = 512.0;
      FPR_oneOverLn2 = oneOverLn2; 		 	FPR_rintFactor = rintFactor;
      FPR_ln2Head = ln2Head; 		 		FPR_ln2Tail = ln2Tail;
      tablePointer = ( struct expTableEntry * ) expTable + 177; 
      
      FEGETENVD ( FPR_env );                  		// save old environment, set default
      __ENSURE( FPR_z, FPR_512, FPR_oneOverLn2 );	__ENSURE( FPR_ln2Head, FPR_ln2Tail, FPR_rintFactor );
      FESETENVD ( FPR_z );
    
      // Compute FPR_h and store it to yInHex as early as possible, even for cases that don't need it
      FPR_h = __FMADD( x, FPR_oneOverLn2, FPR_rintFactor );
      yInHex.d = FPR_h; 		
      FPR_h -= FPR_rintFactor;

      if (likely( FPR_x < FPR_512 ))
      {
            if (likely( x != FPR_z ))
            {
            
                scale.i.hi = ( yInHex.i.lo + 1023 ) << 20;  scale.i.lo = 0;

                y = __FNMSUB( FPR_ln2Head, FPR_h, x );	yTail = __FMUL( FPR_ln2Tail, FPR_h );

                xInHex.d = __FMADD( FPR_512, y, FPR_rintFactor ); 

                __NOOP;
                __NOOP;
                __NOOP;
                i = xInHex.i.lo;
            
                pT = &(tablePointer[i]);
            
                FPR_x = pT->x; 				FPR_f = pT->f;
            
                d = y - FPR_x; 			

                z = d - yTail; 
                FPR_cm1 = cm1; 			 	FPR_c1 = c1;
                FPR_c0 = c0; 			 	FPR_c2 = c2;

                z2 = __FMUL( z, z );  		 	zTail = d - z - yTail;

                temp1 = __FMADD( FPR_cm1, z2, FPR_c1 ); temp2 = __FMADD( FPR_c0, z2, FPR_c2 );

                temp1 = __FMADD( temp1, z, temp2 );
                FPR_scale = scale.d;

                temp2 = __FMADD( temp1, z2, z );

                temp1 = __FMADD( zTail, temp2, zTail );	result = __FMUL( FPR_scale, FPR_f );
                    
                temp1 = temp1 + temp2;

                result = __FMADD( result, temp1, result );

                FESETENVD ( FPR_env );
                __PROG_INEXACT( FPR_cm1 );
                return result;
            } 
            else
            {
                FESETENVD ( FPR_env );
                return 1.0;
            }
      }
      
      if (likely( ( x <= maxExp ) && ( x > minExp ) )) 
      {            
            if ( x >= FPR_512 ) 
            {
                  power = 2.0;
                  scale.i.hi = ( yInHex.i.lo + 1022 ) << 20; scale.i.lo = 0;
            }
            else 
            {
                  power = denormal;
                  scale.i.hi = ( yInHex.i.lo + 1023+128 ) << 20; scale.i.lo = 0;
            }

            y = __FNMSUB( FPR_ln2Head, FPR_h, x ); 	 yTail = __FMUL( FPR_ln2Tail, FPR_h );
            FPR_scale = scale.d;

            xInHex.d = __FMADD( FPR_512, y, FPR_rintFactor ); 

            __NOOP;
            __NOOP;
            __NOOP;
            i = xInHex.i.lo;
            
            pT = &(tablePointer[i]);
            
            FPR_x = pT->x; 			 	FPR_f = pT->f;
            
            d = y - FPR_x; 			 	result = __FMUL( FPR_scale, FPR_f );

            z = d - yTail; 
            FPR_cm1 = cm1; 			 	FPR_c1 = c1;
            FPR_c0 = c0; 				 FPR_c2 = c2;

            z2 = __FMUL( z, z );  		 	zTail = d - z - yTail;

            temp1 = __FMADD( FPR_cm1, z2, FPR_c1 ); 	temp2 = __FMADD( FPR_c0, z2, FPR_c2 );

            temp1 = __FMADD( temp1, z, temp2 );

            temp2 = __FMADD( temp1, z2, z );

            temp1 = __FMADD( zTail, temp2, zTail );
            
            temp1 = temp1 + temp2;

            result = __FMADD( result, temp1, result );
            
            result = __FMUL( result, power );
            
            FESETENVD ( FPR_env );
            __PROG_INEXACT( FPR_oneOverLn2 );
            return result;
      }
      
      FESETENVD ( FPR_env );
      if ( x != x ) 
            return x;
      else if ( x == infinity.d ) 
            return infinity.d;
      else if ( x == -infinity.d ) 
            return FPR_z;
      else if ( x > maxExp )
      {
            __PROG_OF_INEXACT( kMaxNormal );
            return infinity.d;
      }
      else 
      {
            __PROG_UF_INEXACT( kMinNormal );
            return FPR_z;
      }
}

/*******************************************************************************
*                                                                              *
*    The expm1 function.  						       *
*                                                                              *
*******************************************************************************/

static const hexdouble k2M55   = HEXDOUBLE(0x3c800000, 0x00000000);

double expm1 ( double x ) 
{
      hexdouble scale, invScale, xInHex, yInHex;
      register double d, y, yTail, z, zTail, z2, temp1, temp2, power, result, f;
      register int32_t i;
      register struct expTableEntry *tablePointer, *pT;

      register double FPR_oneOverLn2, FPR_rintFactor, FPR_ln2Head, FPR_ln2Tail, FPR_h, FPR_z, FPR_512, FPR_scale;
      register double FPR_env, FPR_x, FPR_f, FPR_cc0, FPR_cc1, FPR_cc2, FPR_cc3, FPR_cc4, FPR_iscale;
      
      FPR_x = __FABS( x );
      
      FPR_z = 0.0;				 	FPR_512 = 512.0;
      FPR_oneOverLn2 = oneOverLn2; 		 	FPR_rintFactor = rintFactor;
      FPR_ln2Head = ln2Head; 		 		FPR_ln2Tail = ln2Tail;
      tablePointer = ( struct expTableEntry * ) expTable + 177;

      FEGETENVD ( FPR_env );                  // save old environment, set default
      __ENSURE( FPR_z, FPR_512, FPR_oneOverLn2 );	__ENSURE( FPR_ln2Head, FPR_ln2Tail, FPR_rintFactor ); 
      FESETENVD ( FPR_z );
    
      // Compute FPR_h and store it to yInHex as early as possible, even for cases that don't need it
      FPR_h = __FMADD( x, FPR_oneOverLn2, FPR_rintFactor );
      yInHex.d = FPR_h; 		
      FPR_h -= FPR_rintFactor;

      if (likely( FPR_x < FPR_512 ))
      {
            if (unlikely( FPR_x < k2M55.d ))
            {
                  FESETENVD ( FPR_env );
                  if ( x == FPR_z ) 
                  {
                        /* NOTHING */
                  }
                  else 
                  {
                        if ( FPR_x < kMinNormal )
                            __PROG_UF_INEXACT( kMinNormal );
                        else
                            __PROG_INEXACT( FPR_oneOverLn2 );
                  }
                  return x;
            } 
                  
            scale.i.hi = ( yInHex.i.lo + 1023 ) << 20;  scale.i.lo = 0;

            y = __FNMSUB( FPR_ln2Head, FPR_h, x ); 	yTail = __FMUL( FPR_ln2Tail, FPR_h );
            
            xInHex.d = __FMADD( FPR_512, y, FPR_rintFactor ); 
            invScale.i.hi = 0x7fe00000 - scale.i.hi; invScale.i.lo = 0;

            i = xInHex.i.lo;
            
            pT = &(tablePointer[i]);
            
            FPR_x = pT->x; 			 	FPR_f = pT->f;
            
            d = y - FPR_x; 	
            FPR_cc4 = cc4;
            
            z = d - yTail;
            FPR_cc0 = cc0; 			 	FPR_cc2 = cc2;
            FPR_cc1 = cc1; 			 	FPR_cc3 = cc3;			

            z2 = __FMUL( z, z );  		 	zTail = d - z - yTail;
            
            temp1 = __FMADD( FPR_cc0, z2, FPR_cc2 );   	temp2 = __FMADD( FPR_cc1, z2, FPR_cc3 );
            
            temp1 = __FMADD( temp1, z2, FPR_cc4 );	
            
            temp2 = __FMADD( temp2, z, temp1 );
            FPR_iscale = invScale.d;		
            
            temp1 = __FMADD( temp2, z2, z );
            
            temp2 = __FMADD( zTail, temp1, zTail );
            
            temp2 = temp1 + temp2; 			 d = FPR_f - FPR_iscale;
            FPR_scale = scale.d;
        
            temp1 = __FMADD( FPR_f, temp2, d );
            
            result = __FMUL( temp1, FPR_scale );
            
            FESETENVD ( FPR_env );
            __PROG_INEXACT( FPR_oneOverLn2 );
            return result;
      }
      
      if (likely( ( x <= maxExp ) && ( x > minExp ) )) 
      {
            if ( x >= FPR_512 ) 
            {
                  power = 2.0;
                  f = 0.5;
                  scale.i.hi = ( yInHex.i.lo + 1022 ) << 20; scale.i.lo = 0;
            }
            else 
            {
                  power = denormal;
                  f = oneOverDenorm;
                  scale.i.hi = ( yInHex.i.lo + 1023+128 ) << 20; scale.i.lo = 0;
                  if ( scale.i.hi < ( 168<<20 ) ) 
                  {
                        FESETENVD ( FPR_env );
                        __PROG_INEXACT( FPR_oneOverLn2 );
                        return -1.0;
                  }
            }
                  
            y = __FNMSUB( FPR_ln2Head, FPR_h, x ); 	yTail = __FMUL( FPR_ln2Tail, FPR_h );
            FPR_scale = scale.d;

            xInHex.d = __FMADD( FPR_512, y, FPR_rintFactor ); 
            invScale.i.hi = 0x7fe00000 - scale.i.hi; 	invScale.i.lo = 0;

            i = xInHex.i.lo;
            
            pT = &(tablePointer[i]);
            
            FPR_x = pT->x; 			 	FPR_f = pT->f;
            
            d = y - FPR_x; 	
            FPR_iscale = invScale.d;		 	FPR_cc4 = cc4;

            z = d - yTail;			
            FPR_cc0 = cc0; 			 	FPR_cc2 = cc2;
            FPR_cc1 = cc1; 			 	FPR_cc3 = cc3;			

            z2 = __FMUL( z, z );  		 	zTail = d - z - yTail;
            
            temp1 = __FMADD( FPR_cc0, z2, FPR_cc2 );   	temp2 = __FMADD( FPR_cc1, z2, FPR_cc3 );
            
            temp1 = __FMADD( temp1, z2, FPR_cc4 );	d = __FNMSUB( f, FPR_iscale, FPR_f );
            
            temp2 = __FMADD( temp2, z, temp1 );
            
            temp1 = __FMADD( temp2, z2, z );
            
            temp2 = __FMADD( zTail, temp1, zTail );
            
            temp2 = temp1 + temp2;
            
            temp1 = __FMADD( FPR_f, temp2, d );
            
            result = __FMUL( temp1, FPR_scale );

            result = __FMUL( result, power );
            
            FESETENVD ( FPR_env );
            __PROG_INEXACT( FPR_oneOverLn2 );
            return result;
      }
      
      FESETENVD ( FPR_env );
      if ( x != x ) 
            return x;
      else if ( x == infinity.d ) 
            return infinity.d;
      else if ( x == -infinity.d ) 
            return -1.0;
      else if ( x > maxExp )
      {
            __PROG_OF_INEXACT( kMaxNormal );
            return infinity.d;
      }
      else 
      {
#if 0 /* XXX scp: test vectors don't want this to underflow */
            __PROG_UF_INEXACT( kMinNormal );
#else
            __PROG_INEXACT( FPR_oneOverLn2 );
#endif
            return -1.0;
      }
}

#else /* BUILDING_FOR_CARBONCORE_LEGACY */
/*******************************************************************************
*      Floating-point constants.                                               *
*******************************************************************************/

static const double maxExp2       =  1024.0; 
static const double minNormExp2   = -1022.0;
static const double minExp2       = -1075.0;

/*******************************************************************************
*                                                                              *
*    The base 2 exponential function.  					       *
*                                                                              *
*******************************************************************************/

double exp2 ( double x ) 
{
      hexdouble OldEnvironment, scale, xInHex, yInHex;
      register double d, y, yTail, z, zTail, z2, temp1, temp2, power, result;
      register int32_t i;
      register struct expTableEntry *tablePointer, *pT;
      
      register double FPR_oneOverLn2, FPR_rintFactor, FPR_ln2Head, FPR_ln2Tail, FPR_h, FPR_z, FPR_512, FPR_scale;
      register double FPR_env, FPR_diff, FPR_x, FPR_f, FPR_cm1, FPR_c0, FPR_c1, FPR_c2;
      
      FPR_x = __FABS( x );
      
      FPR_z = 0.0;				 	FPR_512 = 512.0;
      FPR_oneOverLn2 = oneOverLn2; 		 	FPR_rintFactor = rintFactor;
      FPR_ln2Head = ln2Head; 				FPR_ln2Tail = ln2Tail;
      tablePointer = ( struct expTableEntry * ) expTable + 177; 
      
      FEGETENVD ( FPR_env );                  // save old environment, set default
      __ENSURE( FPR_z, FPR_512, FPR_oneOverLn2 );	__ENSURE( FPR_ln2Head, FPR_ln2Tail, FPR_rintFactor );
      FESETENVD ( FPR_z );
    
      // Compute FPR_h and store it to yInHex as early as possible, even for cases that don't need it
      FPR_h = x + FPR_rintFactor;
      yInHex.d = FPR_h; 		
      FPR_h -= FPR_rintFactor;
      
      FPR_diff = x - FPR_h;
      
      if (likely( FPR_x < FPR_512 ))
      {
            if (likely( FPR_x != FPR_z ))
            {            
                scale.i.hi = ( yInHex.i.lo + 1023 ) << 20;  scale.i.lo = 0;
            
                y = __FMUL( FPR_ln2Head, FPR_diff );	yTail = __FMUL( FPR_ln2Tail, FPR_diff );
            
                xInHex.d = __FMADD( FPR_512, y, FPR_rintFactor ); 
            
                __NOOP;
                __NOOP;
                __NOOP;
                i = xInHex.i.lo;
            
                pT = &(tablePointer[i]);
            
             	FPR_x = pT->x; 			 	FPR_f = pT->f;
            
                d = y - FPR_x; 			

                z = d - yTail; 
                FPR_cm1 = cm1; 			 	FPR_c1 = c1;
                FPR_c0 = c0; 			 	FPR_c2 = c2;

                z2 = __FMUL( z, z );  		 	zTail = d - z - yTail;

                temp1 = __FMADD( FPR_cm1, z2, FPR_c1 ); temp2 = __FMADD( FPR_c0, z2, FPR_c2 );

                temp1 = __FMADD( temp1, z, temp2 );

                temp2 = __FMADD( temp1, z2, z );
                FPR_scale = scale.d;

                temp1 = __FMADD( zTail, temp2, zTail );
            
                temp1 = temp1 + temp2;			 result = __FMUL( FPR_scale, FPR_f );

                result = __FMADD( result, temp1, result );
            
                FESETENVD ( FPR_env );
                if ( FPR_diff != FPR_z)
                    __PROG_INEXACT( FPR_oneOverLn2 );
                
                return result;
            } 
            else
            {
                FESETENVD ( FPR_env );
                return 1.0;
            }
      }

      if (likely( ( x < maxExp2 ) && ( x > minExp2 ) )) 
      {
            if ( x >= FPR_512 ) 
            {
                  power = 2.0;
                  scale.i.hi = ( yInHex.i.lo + 1022 ) << 20; scale.i.lo = 0;
            }
            else 
            {
                  power = denormal;
                  scale.i.hi = ( yInHex.i.lo + 1023+128 ) << 20; scale.i.lo = 0;
            }

            FPR_diff = x - FPR_h;
            FPR_scale = scale.d;
            
            y = __FMUL( FPR_ln2Head, FPR_diff );  	yTail = __FMUL( FPR_ln2Tail, FPR_diff );
            
            xInHex.d = __FMADD( FPR_512, y, FPR_rintFactor ); 

            __NOOP;
            __NOOP;
            __NOOP;
            i = xInHex.i.lo;
            
            pT = &(tablePointer[i]);
                
            FPR_x = pT->x; 			 	FPR_f = pT->f;
            
            d = y - FPR_x; 			 	result = __FMUL( FPR_scale, FPR_f );

            z = d - yTail; 
            FPR_cm1 = cm1; 			 	FPR_c1 = c1;
            FPR_c0 = c0; 			 	FPR_c2 = c2;

            z2 = __FMUL( z, z );  		 	zTail = d - z - yTail;

            temp1 = __FMADD( FPR_cm1, z2, FPR_c1 ); 	temp2 = __FMADD( FPR_c0, z2, FPR_c2 );

            temp1 = __FMADD( temp1, z, temp2 );

            temp2 = __FMADD( temp1, z2, z );

            temp1 = __FMADD( zTail, temp2, zTail );
            
            temp1 = temp1 + temp2;

            result = __FMADD( result, temp1, result );
            
            result = __FMUL( result, power );
            
            FESETENVD ( FPR_env );
            if ( x < minNormExp2 )
            {
                OldEnvironment.d = FPR_env;
				__NOOP;
				__NOOP;
				__NOOP;
				
                OldEnvironment.i.lo |= FE_UNDERFLOW;  	// set underflow flag
                
                if ( FPR_h != x )
                    OldEnvironment.i.lo |= FE_INEXACT;  // set inexact flag
                
                FESETENVD_GRP ( OldEnvironment.d );
            }
            else if ( FPR_h != x )
                __PROG_INEXACT( FPR_oneOverLn2 );

            return result;
      }
      
      FESETENVD ( FPR_env );
      if ( x != x ) 
            return x;
      else if ( x == infinity.d ) 
            return infinity.d;
      else if ( x == -infinity.d ) 
            return FPR_z;
      else if ( x > maxExp )
      {
            __PROG_OF_INEXACT( kMaxNormal );
            return infinity.d;
      }
      else 
      {
            __PROG_UF_INEXACT( kMinNormal );
            return FPR_z;
      }
}

#endif /* BUILDING_FOR_CARBONCORE_LEGACY */
