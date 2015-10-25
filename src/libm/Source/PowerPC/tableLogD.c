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
*     File tableLogD.c,                                                        *
*     Functions log, log1p and log10.                                          *
*     Implementation of log( x ) based on IBM/Taligent table method.           *
*                                                                              *
*     Copyright © 1997-2001 Apple Computer, Inc.  All rights reserved.         *
*                                                                              *
*     Written by Ali Sazegari, started on April 1997.                          *
*     Modified and ported by Robert A. Murley (ram) for Mac OS X.              *
*                                                                              *
*     A MathLib v4 file.                                                       *
*                                                                              *
*     Nov   07 2001: removed log2 to prevent conflict with CarbonCore.         *
*     Nov   06 2001: commented out warning about Intel architectures.          *
*                    changed i386 stub to call abort().                        *
*     Nov   02 2001: added stub for i386 version of log2.                      *
*     April 28 1997: port of the ibm/taligent log routines.                    *
*     Sept  06 2001: replaced __setflm with FEGETENVD/FESETENVD.               *
*                    replaced DblInHex typedef with hexdouble.                 *
*                    used standard exception symbols from fenv.h.              *
*                    added #ifdef __ppc__.                                     *
*                    used faster fixed-point instructions to calculate 'n'     *
*     Sept  09 2001: added more comments.                                      *
*     Sept  10 2001: added macros to detect PowerPC and correct compiler.      *
*     Sept  18 2001: added <CoreServices/CoreServices.h> to get <fp.h> and     *
*                    <fenv.h>.                                                 *
*     Oct   08 2001: removed <CoreServices/CoreServices.h>.                    *
*                    changed compiler errors to warnings.                      *
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
#include      "fp_private.h"
#include      "fenv_private.h"

#define  LOGORITHMIC_NAN  "36"

/*******************************************************************************
*      Floating-point constants.                                               *
*******************************************************************************/

static const double ln2            = 6.931471805599452862e-1;
static const double twoTo128       = 0x1.0p+128; // 3.402823669209384635e+38;
static const double largestDenorm  = 2.2250738585072009e-308;   // { 0x000fffff, 0xffffffff }
static const hexdouble infinity     = HEXDOUBLE(0x7ff00000, 0x00000000);

/*******************************************************************************
*      Approximation coefficients.                                             *
*******************************************************************************/

static const double d2 = -0.5000000000000000000000;             // { 0xBFE00000, 0x00000000 }
static const double d3 =  0.3333333333333360968212;             // { 0x3FD55555, 0x55555587 }
static const double d4 = -0.2500000000000056849516;             // { 0xBFD00000, 0x00000066 }
static const double d5 =  0.1999999996377879912068;             // { 0x3FC99999, 0x98D278CB }
static const double d6 = -0.1666666662009609743117;             // { 0xBFC55555, 0x54554F15 }
static const double d7 =  0.1428690115662276259345;             // { 0x3FC24988, 0x2224F72F }
static const double d8 = -0.1250122079043555957999;             // { 0xBFC00066, 0x68466517 }

extern const uint32_t logTable[];

struct logTableEntry 
       {
       double X;
       double G;
       hexdouble F;
       };

static const hexdouble kConvDouble     = HEXDOUBLE(0x43300000, 0x80000000);

#if !defined(BUILDING_FOR_CARBONCORE_LEGACY)

/*******************************************************************************
*      Floating-point constants.                                               *
*******************************************************************************/

static const double ln2Tail        = 2.319046813846299558e-17;
static const double log10e         = 4.342944819032518200e-01;  // head of log10 of e { 0x3fdbcb7b, 0x1526e50e }
static const double log10eTail     = 1.098319650216765200e-17;  // tail of log10 of e { 0x3c695355, 0xbaaafad4 }

/*******************************************************************************
*      Approximation coefficients.                                             *
*******************************************************************************/

static const double c2 = -0.5000000000000000042725;
static const double c3 =  0.3333333333296328456505;
static const double c4 = -0.2499999999949632195759;
static const double c5 =  0.2000014541571685319141;
static const double c6 = -0.1666681511199968257150;

/*******************************************************************************
*                                                                              *
*    The base e logorithm function.  Caller’s rounding direction is honored.   *
*                                                                              *
********************************************************************************
*                                                                              *
*    calls the intrinsic functions __setflm, __FABS.                           *
*    raised exceptions are inexact, divide by zero and invalid.                *
*                                                                              *
*******************************************************************************/

double log ( double x ) 
{
       hexdouble yInHex, xInHex, OldEnvironment;
       register double n, zTail, high, low, z, z2, temp1, temp2, temp3, result;
       register uint32_t i;
       struct logTableEntry *tablePointer = ( struct logTableEntry * ) logTable;
       
       register double FPR_env, FPR_z, FPR_half, FPR_one, FPR_twoTo128, FPR_ln2, FPR_kConvDouble;
       register double FPR_r, FPR_s, FPR_t, FPR_u, FPR_y;
       register struct logTableEntry *pT;
       register int32_t nLong; // converted to double in two widely separated steps, avoiding LSU hazards
       hexdouble nInHex;
       
       xInHex.d = x;						nInHex.i.hi = 0x43300000; // store upper half

       FPR_z = 0.0;						FPR_half = 0.5;
       FPR_one = 1.0;						FPR_u = 1.0;
       FPR_twoTo128 = twoTo128;					FPR_ln2 = ln2;
       
       FEGETENVD( FPR_env );
       __ENSURE( FPR_z, FPR_twoTo128, FPR_ln2 );		__ENSURE( FPR_u, FPR_half, FPR_one );		
       FESETENVD( FPR_z );
              
       if (likely( FPR_z <= x && x < infinity.d )) 
       {                                             // x is finite and non-negative
              if (likely( x > largestDenorm ))  
              {                                      // x is normal
                     i = xInHex.i.hi & 0x000fffff;
                     yInHex.i.lo = xInHex.i.lo;
                     yInHex.i.hi = i | 0x3ff00000;          // now 1.0 <= y < 2.0
                     if ( yInHex.i.hi & 0x00080000 ) 
                     {                               // 1.5 <= y < 2.0
                            nLong = ((int32_t) ( xInHex.i.hi >> 20 ) - 1022);
                            nLong ^= 0x80000000; // flip sign bit
                            nInHex.i.lo = nLong; // store lower half
                            i = ( i >> 13 ) & 0x3f;         // table lookup index
                            FPR_u = FPR_half;
                     }
                     else 
                     {                               // 1.0 <= y < 1.5
                            nLong = ((int32_t) ( xInHex.i.hi >> 20 ) - 1023);
                            nLong ^= 0x80000000; // flip sign bit
                            nInHex.i.lo = nLong; // store lower half
                            i = ( i >> 12 ) + 64;           // table lookup index
                            // FPR_u = FPR_one; via initialization above
                     }
              }
              else if (likely( x != FPR_z )) 
              {                                      // x is nonzero, denormal
                     xInHex.d = __FMUL( x, FPR_twoTo128 );
                    __NOOP;
                    __NOOP;
                    __NOOP;
                     i = xInHex.i.hi & 0x000fffff;
                     yInHex.i.lo = xInHex.i.lo;
                     yInHex.i.hi = i | 0x3ff00000;          // now 1.0 <= y < 2.0
                     if ( yInHex.i.hi & 0x00080000 ) 
                     {                               // 1.5 <= y < 2.0
                            nLong = ((int32_t) ( xInHex.i.hi >> 20 ) - 1150);
                            nLong ^= 0x80000000; // flip sign bit
                            nInHex.i.lo = nLong; // store lower half
                            i = ( i >> 13 ) & 0x3f;         // table lookup index
                            FPR_u = FPR_half;
                     }
                     else 
                     {                               // 1.0 <= y < 1.5
                            nLong = ((int32_t) ( xInHex.i.hi >> 20 ) - 1151);
                            nLong ^= 0x80000000; // flip sign bit
                            nInHex.i.lo = nLong; // store lower half
                            i = ( i >> 12 ) + 64;           // table lookup index
                            // FPR_u = FPR_one; via initialization above
                     }
              }
              else                                          // x is 0.0
              {
                     OldEnvironment.d = FPR_env;
					 __NOOP;
					 __NOOP;
					 __NOOP;
                     OldEnvironment.i.lo |= FE_DIVBYZERO;
                     result = -infinity.d;
                     FESETENVD_GRP( OldEnvironment.d );
                     return result;
              }
              
              pT = &(tablePointer[i]);
              FPR_r = pT->X;					FPR_t = pT->G;
              
              FPR_y = yInHex.d;					
              FPR_s = __FMSUB( FPR_u, FPR_y, FPR_r );		__NOOP;
              
              z = __FMUL( FPR_s, FPR_t );			
              FPR_u = __FNMSUB( z, FPR_r, FPR_s );		z2 = __FMUL( z, z );
              zTail = __FMUL( FPR_u, FPR_t);
                            
              if ( (uint32_t)nLong == 0x80000000u /* iff n == 0.0 */ ) 
              {                                     // 0.75 <= x < 1.5
                     register double FPR_d2, FPR_d3, FPR_d4, FPR_d5, FPR_d6, FPR_d7, FPR_d8;
                     
                     if ( x == FPR_one ) 
                     {
                            FESETENVD( FPR_env );
                            return FPR_z;
                     }
                     
                     FPR_d8 = d8;				FPR_d6 = d6;
                     
                     FPR_d7 = d7;				FPR_d5 = d5;
                     
                     temp1 = __FMADD( FPR_d8, z2, FPR_d6 );	temp2 = __FMADD( FPR_d7, z2, FPR_d5);
                     FPR_d4 = d4;				
                     
                     FPR_d3 = d3;
                     temp1 = __FMADD( temp1, z2, FPR_d4 );	temp2 = __FMADD( temp2, z2, FPR_d3 );

                     FPR_d2 = d2;				
                     FPR_t = pT->F.d;				__NOOP;
                     
                     temp1 = __FMADD( temp1, z, temp2 );	temp3 = z + FPR_t;
                     
                     temp2 = __FMADD( temp1, z, FPR_d2 );	FPR_u = FPR_t - temp3;
                     
                     FPR_s = FPR_u + z;				FPR_r = __FNMSUB( z, zTail, zTail );
                     
                     low = FPR_s + FPR_r;
                     
                     result = __FMADD( temp2, z2, low );
                     
                     result += temp3;
                     
                     FESETENVD( FPR_env );
                     __PROG_INEXACT( FPR_ln2 );
                     
                     return result;
              }
              else if ( pT->F.i.hi != 0 ) 
              {                                     // n != 0, and y not close to 1
                     register double FPR_c2, FPR_c3, FPR_c4, FPR_c5, FPR_c6;
                     
                     FPR_c6 = c6;				FPR_c4 = c4;
                     
                     FPR_c5 = c5;				FPR_c3 = c3;
                     
                     temp3 = __FMADD( FPR_c6, z2, FPR_c4 );	temp2 = __FMADD( FPR_c5, z2, FPR_c3 );
                     FPR_kConvDouble = kConvDouble.d;	
                     
                     FPR_s = ln2Tail;
                     n = nInHex.d;			// float load double 				
                     n -= FPR_kConvDouble; 		// subtract magic value			
                     
                     __NOOP;				FPR_t = pT->F.d; 
                     low = __FMADD( n, FPR_s, zTail );		high = z + FPR_t;
                     
                     temp3 = __FMUL( temp3, z2 );		FPR_u = FPR_t - high;
                     		
                     temp2 = __FMADD( temp2, z, temp3 );	zTail = FPR_u + z;

                     FPR_c2 = c2;
                     temp2 = temp2 + FPR_c2;			temp1 = __FMADD( n, FPR_ln2, low );
                     
                     FPR_t = __FMSUB( n, FPR_ln2, temp1 );	temp3 = high + temp1;
                     
                     FPR_s = temp1 - temp3;			low = FPR_t + low;				
                     
                     temp1 = FPR_s + high;			FPR_r = __FMADD( temp2, z2, low );
                     
                     result = ( FPR_r + temp1 ) + zTail;
                     
                     result += temp3;
                     
                     FESETENVD( FPR_env );
                     __PROG_INEXACT( FPR_ln2 );
                     
                     return result;
              }
              else 
              {                                     // n != 0 and y close to 1                     
                     register double FPR_d2, FPR_d3, FPR_d4, FPR_d5, FPR_d6, FPR_d7, FPR_d8;
                    
                     FPR_d8 = d8;				FPR_d6 = d6;
                     
                     FPR_d7 = d7;				FPR_d5 = d5;
                     
                     temp1 = __FMADD( FPR_d8, z2, FPR_d6 );	temp2 = __FMADD( FPR_d7, z2, FPR_d5);
                     FPR_kConvDouble = kConvDouble.d;	
                                          
                     FPR_t = ln2Tail;
                     n = nInHex.d; 				
                     n -= FPR_kConvDouble;			
                     
                     low = __FMADD( n, FPR_t, zTail );		__NOOP;
                     FPR_d2 = d2;
                     
                     FPR_d4 = d4;				FPR_d3 = d3;
                     
                     temp1 = __FMADD( temp1, z2, FPR_d4 );	temp2 = __FMADD( temp2, z2, FPR_d3 );
                     temp3 = __FMADD( n, FPR_ln2, low );	__NOOP;
                     				
                     temp1 = __FMADD( temp1, z, temp2 );	FPR_s = __FMSUB( n, FPR_ln2, temp3 );
                     
                     temp2 = __FMADD( temp1, z, FPR_d2 );	low = FPR_s + low;
                                          
                     result = __FMADD( temp2, z2, low ) + z;
                     result += temp3;
                     
                     FESETENVD( FPR_env );
                     __PROG_INEXACT( FPR_ln2 );
                     
                     return result;
              }
       }
       
       OldEnvironment.d = FPR_env;
       if ( x == FPR_z )
       {
              OldEnvironment.i.lo |= FE_DIVBYZERO;
              x = -infinity.d;
       }
       else if ( x < FPR_z )
       {                                           // x < 0.0
              OldEnvironment.i.lo |= SET_INVALID;
              x = nan ( LOGORITHMIC_NAN );
       }
              
       FESETENVD_GRP( OldEnvironment.d );
       return x;
       
}

/*******************************************************************************
*                                                                              *
*    The base 10 logorithm function.  Caller’s rounding direction is honored.  *
*                                                                              *
*******************************************************************************/

double log10 ( double x ) 
{
       hexdouble yInHex, xInHex, OldEnvironment;
       register double n, zTail, high, low, z, z2, temp1, temp2, temp3, result, resultLow;
       register uint32_t i;
       struct logTableEntry *tablePointer = ( struct logTableEntry * ) logTable;
       
       register double FPR_env, FPR_z, FPR_half, FPR_one, FPR_twoTo128, FPR_ln2, FPR_kConvDouble;
       register double FPR_r, FPR_s, FPR_t, FPR_u, FPR_y;
       register struct logTableEntry *pT;
       register int32_t nLong; // converted to double in two widely separated steps, avoiding LSU hazards
       hexdouble nInHex;
       
       xInHex.d = x;						nInHex.i.hi = 0x43300000; // store upper half

       FPR_z = 0.0;						FPR_half = 0.5;
       FPR_one = 1.0;						FPR_u = 1.0;
       FPR_twoTo128 = twoTo128;					FPR_ln2 = ln2;
       
       FEGETENVD( FPR_env );
       __ENSURE( FPR_z, FPR_twoTo128, FPR_ln2 );		__ENSURE( FPR_u, FPR_half, FPR_one );		
       FESETENVD( FPR_z );
              
       if (likely( FPR_z <= x && x < infinity.d )) 
       {                                             // x is finite and non-negative
              if (likely( x > largestDenorm ))  
              {                                      // x is normal
                     i = xInHex.i.hi & 0x000fffff;
                     yInHex.i.lo = xInHex.i.lo;
                     yInHex.i.hi = i | 0x3ff00000;          // now 1.0 <= y < 2.0
                     if ( yInHex.i.hi & 0x00080000 ) 
                     {                               // 1.5 <= y < 2.0
                            nLong = ((int32_t) ( xInHex.i.hi >> 20 ) - 1022);
                            nLong ^= 0x80000000; // flip sign bit
                            nInHex.i.lo = nLong; // store lower half
                            i = ( i >> 13 ) & 0x3f;         // table lookup index
                            FPR_u = FPR_half;
                     }
                     else 
                     {                               // 1.0 <= y < 1.5
                            nLong = ((int32_t) ( xInHex.i.hi >> 20 ) - 1023);
                            nLong ^= 0x80000000; // flip sign bit
                            nInHex.i.lo = nLong; // store lower half
                            i = ( i >> 12 ) + 64;           // table lookup index
                            // FPR_u = FPR_one; via initialization above
                     }
              }
              else if (likely( x != FPR_z )) 
              {                                      // x is nonzero, denormal
                     xInHex.d = __FMUL( x, FPR_twoTo128 );
					 __NOOP;
					 __NOOP;
					 __NOOP;
                     i = xInHex.i.hi & 0x000fffff;
                     yInHex.i.lo = xInHex.i.lo;
                     yInHex.i.hi = i | 0x3ff00000;          // now 1.0 <= y < 2.0
                     if ( yInHex.i.hi & 0x00080000 ) 
                     {                               // 1.5 <= y < 2.0
                            nLong = ((int32_t) ( xInHex.i.hi >> 20 ) - 1150);
                            nLong ^= 0x80000000; // flip sign bit
                            nInHex.i.lo = nLong; // store lower half
                            i = ( i >> 13 ) & 0x3f;         // table lookup index
                            FPR_u = FPR_half;
                     }
                     else 
                     {                               // 1.0 <= y < 1.5
                            nLong = ((int32_t) ( xInHex.i.hi >> 20 ) - 1151);
                            nLong ^= 0x80000000; // flip sign bit
                            nInHex.i.lo = nLong; // store lower half
                            i = ( i >> 12 ) + 64;           // table lookup index
                            // FPR_u = FPR_one; via initialization above
                     }
              }
              else                                          // x is 0.0
              {
                     OldEnvironment.d = FPR_env;
					 __NOOP;
					 __NOOP;
					 __NOOP;
                     OldEnvironment.i.lo |= FE_DIVBYZERO;
                     result = -infinity.d;
                     FESETENVD_GRP( OldEnvironment.d );
                     return result;
              }
              
              pT = &(tablePointer[i]);
              FPR_r = pT->X;					FPR_t = pT->G;
              
              FPR_y = yInHex.d;					
              FPR_s = __FMSUB( FPR_u, FPR_y, FPR_r );		__NOOP;
              
              z = __FMUL( FPR_s, FPR_t );			
              FPR_u = __FNMSUB( z, FPR_r, FPR_s );		z2 = __FMUL( z, z );
              zTail = __FMUL( FPR_u, FPR_t);
                            
              if ( (uint32_t)nLong == 0x80000000u /* iff n == 0.0 */ ) 
              {                                     // 0.75 <= x < 1.5
                     register double FPR_d2, FPR_d3, FPR_d4, FPR_d5, FPR_d6, FPR_d7, FPR_d8;
                     
                     if ( x == FPR_one ) 
                     {
                            FESETENVD( FPR_env );
                            return FPR_z;
                     }
                     
                     FPR_d8 = d8;				FPR_d6 = d6;
                     
                     FPR_d7 = d7;				FPR_d5 = d5;
                     
                     temp1 = __FMADD( FPR_d8, z2, FPR_d6 );	temp2 = __FMADD( FPR_d7, z2, FPR_d5);
                     FPR_d4 = d4;				
                     
                     FPR_d3 = d3;
                     temp1 = __FMADD( temp1, z2, FPR_d4 );	temp2 = __FMADD( temp2, z2, FPR_d3 );

                     FPR_d2 = d2;				
                     FPR_t = pT->F.d;				__NOOP;
                     
                     temp1 = __FMADD( temp1, z, temp2 );	temp3 = z + FPR_t;
                     
                     temp2 = __FMADD( temp1, z, FPR_d2 );	FPR_u = FPR_t - temp3;
                     
                     FPR_s = FPR_u + z;				FPR_r = __FNMSUB( z, zTail, zTail );
                     
                     low = FPR_s + FPR_r;

                     FPR_r = __FMADD( temp2, z2, low );
                     FPR_s = log10e;				FPR_t = log10eTail;
                                          
                     result = FPR_r + temp3;
                     resultLow = temp3 - result + FPR_r;
                     FPR_u = __FMUL( result, FPR_t );
                     resultLow = __FMADD( resultLow, FPR_s, FPR_u );
                     result = __FMADD( result, FPR_s, resultLow );

                     FESETENVD( FPR_env );
                     __PROG_INEXACT( FPR_ln2 );
                     
                     return result;
              }
              else if ( pT->F.i.hi != 0 ) 
              {                                     // n != 0, and y not close to 1
                     register double FPR_c2, FPR_c3, FPR_c4, FPR_c5, FPR_c6;
                     
                     FPR_c6 = c6;				FPR_c4 = c4;
                     
                     FPR_c5 = c5;				FPR_c3 = c3;
                     
                     temp3 = __FMADD( FPR_c6, z2, FPR_c4 );	temp2 = __FMADD( FPR_c5, z2, FPR_c3 );
                     FPR_kConvDouble = kConvDouble.d;	
                     
                     FPR_s = ln2Tail;
                     n = nInHex.d; 				// float load double				
                     n -= FPR_kConvDouble; 			// subtract magic value			
                     
                     __NOOP;				FPR_t = pT->F.d;
                     low = __FMADD( n, FPR_s, zTail );		high = z + FPR_t;
                     
                     temp3 = __FMUL( temp3, z2 );		FPR_u = FPR_t - high;
                     		
                     temp2 = __FMADD( temp2, z, temp3 );	zTail = FPR_u + z;

                     FPR_c2 = c2;
                     temp2 = temp2 + FPR_c2;			temp1 = __FMADD( n, FPR_ln2, low );
                     
                     FPR_t = __FMSUB( n, FPR_ln2, temp1 );	temp3 = high + temp1;
                     
                     FPR_s = temp1 - temp3;			low = FPR_t + low;				
                     
                     temp1 = FPR_s + high;			FPR_r = __FMADD( temp2, z2, low );
                     FPR_s = log10e;				FPR_t = log10eTail;

                     result = ( ( FPR_r + temp1 ) + zTail ) + temp3;
                     resultLow = temp3 - result + zTail + temp1 + FPR_r;
                     FPR_u = __FMUL( result, FPR_t );
                     resultLow = __FMADD( resultLow, FPR_s, FPR_u );
                     result = __FMADD( result, FPR_s, resultLow );
                     
                     FESETENVD( FPR_env );
                     __PROG_INEXACT( FPR_ln2 );
                     
                     return result;
              }
              else
              {                                     // n != 0 and y close to 1                     
                     register double FPR_d2, FPR_d3, FPR_d4, FPR_d5, FPR_d6, FPR_d7, FPR_d8;
                    
                     FPR_d8 = d8;				FPR_d6 = d6;
                     
                     FPR_d7 = d7;				FPR_d5 = d5;
                     
                     temp1 = __FMADD( FPR_d8, z2, FPR_d6 );	temp2 = __FMADD( FPR_d7, z2, FPR_d5);
                     FPR_kConvDouble = kConvDouble.d;	
                                          
                     FPR_t = ln2Tail;
                     n = nInHex.d; 				
                     n -= FPR_kConvDouble;			
                     
                     low = __FMADD( n, FPR_t, zTail );		__NOOP;
                     FPR_d2 = d2;
                     
                     FPR_d4 = d4;				FPR_d3 = d3;
                     
                     temp1 = __FMADD( temp1, z2, FPR_d4 );	temp2 = __FMADD( temp2, z2, FPR_d3 );
                     temp3 = __FMADD( n, FPR_ln2, low );	__NOOP;
                     				
                     temp1 = __FMADD( temp1, z, temp2 );	FPR_s = __FMSUB( n, FPR_ln2, temp3 );
                     
                     temp2 = __FMADD( temp1, z, FPR_d2 );	low = FPR_s + low;
                     
                     FPR_r = __FMADD( temp2, z2, low );
                     FPR_s = log10e;				FPR_t = log10eTail;
                                          
                     result = ( FPR_r + z ) + temp3;
                     resultLow = temp3 - result + z + FPR_r;
                     FPR_u = __FMUL( result, FPR_t );
                     resultLow = __FMADD( resultLow, FPR_s, FPR_u );
                     result = __FMADD( result, FPR_s, resultLow );
                     
                     FESETENVD( FPR_env );
                     __PROG_INEXACT( FPR_ln2 );
                     
                     return result;
              }
       }
       
       OldEnvironment.d = FPR_env;
	   __NOOP;
	   __NOOP;
	   __NOOP;
	   
       if ( x == FPR_z )
       {
              OldEnvironment.i.lo |= FE_DIVBYZERO;
              x = -infinity.d;
       }
       else if ( x < FPR_z )
       {                                           // x < 0.0
              OldEnvironment.i.lo |= SET_INVALID;
              x = nan ( LOGORITHMIC_NAN );
       }
              
       FESETENVD_GRP( OldEnvironment.d );
       return x;
       
}

/*******************************************************************************
*                                                                              *
*    The base e log(1+x) function.  Caller’s rounding direction is honored.    *
*                                                                              *
*******************************************************************************/

double log1p ( double x ) 
{
       hexdouble yInHex, xInHex, OldEnvironment;
       register double yLow, n, zTail, high, low, z, z2, temp1, temp2, temp3, result;
       register uint32_t i;
       struct logTableEntry *tablePointer = ( struct logTableEntry * ) logTable;
       
       register double FPR_env, FPR_z, FPR_half, FPR_one, FPR_negOne, FPR_ln2, FPR_kConvDouble;
       register double FPR_r, FPR_s, FPR_t, FPR_u, FPR_y;
       register struct logTableEntry *pT;
       register int32_t nLong; // converted to double in two widely separated steps, avoiding LSU hazards
       hexdouble nInHex;
       
       nInHex.i.hi = 0x43300000; // store upper half

       FPR_z = 0.0;						FPR_half = 0.5;
       FPR_one = 1.0;						FPR_u = 1.0;
       FPR_ln2 = ln2;						FPR_negOne = -1.0;
       
       FEGETENVD( FPR_env );
       __ENSURE( FPR_z, FPR_negOne, FPR_ln2 );			__ENSURE( FPR_u, FPR_half, FPR_one );		
       FESETENVD( FPR_z );
       
//     N.B. x == NAN fails all comparisons and falls through to the return.

       if (likely( ( x > FPR_negOne ) && ( x < infinity.d ) )) 
       {
              FPR_y = FPR_one + x;                          // yInHex.d cannot be a denormal number
              yInHex.d = FPR_y;
              yLow = ( x < FPR_one ) ? ( FPR_one - FPR_y ) + x : ( x - FPR_y ) + FPR_one; 
              
              i = yInHex.i.hi & 0x000fffff;
              nLong = (int32_t) ( yInHex.i.hi >> 20 ) - 1023;
              xInHex.i.lo = 0x0;
              xInHex.i.hi = 0x7fe00000 - ( yInHex.i.hi & 0x7ff00000 );
              yInHex.i.hi = i | 0x3ff00000;                // now 1.0 <= y < 2.0
              if ( yInHex.i.hi & 0x00080000 ) 
              {                                     // 1.5 <= y < 2.0
                     nLong += 1;
                     FPR_u = FPR_half;
                     i = ( i >> 13 ) & 0x3f;               // table lookup index
              }
              else 
              {                                     // 1.0 <= y < 1.5
                     i = ( i >> 12 ) + 64;                 // table lookupndex
                     // FPR_u = FPR_one; via initialization above
              }
              
              nLong ^= 0x80000000; // flip sign bit
              nInHex.i.lo = nLong; // store lower half
              
              pT = &(tablePointer[(int32_t)i]);
              FPR_r = pT->X;					FPR_t = pT->G;
            
              FPR_y = yInHex.d;					
              FPR_s = __FMSUB( FPR_u, FPR_y, FPR_r );		yLow = __FMUL( yLow, xInHex.d );
              
              z = __FMUL( FPR_s, FPR_t );			yLow = __FMUL( yLow, FPR_u );
              FPR_u = __FNMSUB( z, FPR_r, FPR_s );		z2 = __FMUL( z, z );
              FPR_u = FPR_u + yLow;
              zTail = __FMUL( FPR_u, FPR_t);

              if ( (uint32_t)nLong == 0x80000000u /* iff n == 0.0 */ ) 
              {
                     register double FPR_d2, FPR_d3, FPR_d4, FPR_d5, FPR_d6, FPR_d7, FPR_d8;

                     if ( FPR_y == FPR_one )
                     {
                            FESETENVD( FPR_env );
                            if ( x != FPR_z ) 
                            {
                                   if ( __FABS( x ) <= largestDenorm )
                                        __PROG_UF_INEXACT( largestDenorm );
                                   else
                                        __PROG_INEXACT( FPR_ln2 );
                            }
                            return x;
                     }
                     
                     FPR_d8 = d8;				FPR_d6 = d6;
                     
                     FPR_d7 = d7;				FPR_d5 = d5;
                     
                     temp1 = __FMADD( FPR_d8, z2, FPR_d6 );	temp2 = __FMADD( FPR_d7, z2, FPR_d5);
                     FPR_d4 = d4;				
                     
                     FPR_d3 = d3;
                     temp1 = __FMADD( temp1, z2, FPR_d4 );	temp2 = __FMADD( temp2, z2, FPR_d3 );

                     FPR_d2 = d2;				
                     FPR_t = pT->F.d;				__NOOP;
                     
                     temp1 = __FMADD( temp1, z, temp2 );	temp3 = z + FPR_t;
                     
                     temp2 = __FMADD( temp1, z, FPR_d2 );	FPR_u = FPR_t - temp3;
                     
                     FPR_s = FPR_u + z;				FPR_r = __FNMSUB( z, zTail, zTail );
                     
                     low = FPR_s + FPR_r;
                     
                     result = __FMADD( temp2, z2, low );
              }
              else if ( pT->F.i.hi != 0 ) 
              {                                     // n != 0, and y not close to 1
                     register double FPR_c2, FPR_c3, FPR_c4, FPR_c5, FPR_c6;
                     
                     FPR_c6 = c6;				FPR_c4 = c4;
                     
                     FPR_c5 = c5;				FPR_c3 = c3;
                     
                     temp3 = __FMADD( FPR_c6, z2, FPR_c4 );	temp2 = __FMADD( FPR_c5, z2, FPR_c3 );
                     FPR_kConvDouble = kConvDouble.d;
                     
                     FPR_s = ln2Tail;
                     n = nInHex.d; 				// float load double				
                     n -= FPR_kConvDouble; 			// subtract magic value			
                     
                     __NOOP;				FPR_t = pT->F.d;
                     low = __FMADD( n, FPR_s, zTail );		high = z + FPR_t;
                     
                     temp3 = __FMUL( temp3, z2 );		FPR_u = FPR_t - high;
                     		
                     temp2 = __FMADD( temp2, z, temp3 );	zTail = FPR_u + z;

                     FPR_c2 = c2;
                     temp2 = temp2 + FPR_c2;			temp1 = __FMADD( n, FPR_ln2, low );
                     
                     FPR_t = __FMSUB( n, FPR_ln2, temp1 );	temp3 = high + temp1;
                     
                     FPR_s = temp1 - temp3;			low = FPR_t + low;				
                     
                     temp1 = FPR_s + high;			FPR_r = __FMADD( temp2, z2, low );
                     
                     result = ( FPR_r + temp1 ) + zTail;
              }
              else 
              {
                     register double FPR_d2, FPR_d3, FPR_d4, FPR_d5, FPR_d6, FPR_d7, FPR_d8;
                    
                     FPR_d8 = d8;				FPR_d6 = d6;
                     
                     FPR_d7 = d7;				FPR_d5 = d5;
                     
                     temp1 = __FMADD( FPR_d8, z2, FPR_d6 );	temp2 = __FMADD( FPR_d7, z2, FPR_d5);
                     FPR_kConvDouble = kConvDouble.d;	
                                          
                     FPR_t = ln2Tail;
                     n = nInHex.d; 				
                     n -= FPR_kConvDouble;			

                     low = __FMADD( n, FPR_t, zTail );		__NOOP;
                     FPR_d2 = d2;
                     
                     FPR_d4 = d4;				FPR_d3 = d3;

                     temp1 = __FMADD( temp1, z2, FPR_d4 );	temp2 = __FMADD( temp2, z2, FPR_d3 );
                     temp3 = __FMADD( n, FPR_ln2, low );	__NOOP;
                                          				
                     temp1 = __FMADD( temp1, z, temp2 );	FPR_s = __FMSUB( n, FPR_ln2, temp3 );
                     
                     temp2 = __FMADD( temp1, z, FPR_d2 );	low = FPR_s + low;

                     low = low + yLow;
                     result = __FMADD( temp2, z2, low ) + z;
              }

              result += temp3;
            
              FESETENVD( FPR_env );
              __PROG_INEXACT( FPR_ln2 );
            
              return result;
       }
       
       OldEnvironment.d = FPR_env;
       if ( x == FPR_negOne ) 
       {
              OldEnvironment.i.lo |= FE_DIVBYZERO;
              x = -infinity.d;
       }
       else if ( x < FPR_negOne )
       {
              OldEnvironment.i.lo |= SET_INVALID;
              x = nan ( LOGORITHMIC_NAN );
       }
       
       FESETENVD_GRP( OldEnvironment.d );
       return x;
}

#else /* BUILDING_FOR_CARBONCORE_LEGACY */

/*******************************************************************************
*      Floating-point constants.                                               *
*******************************************************************************/

static const double log2e          = 1.4426950408889634e+0;     // head of log2  of e { 0x3ff71547, 0x652b82fe }
static const double log2eTail      = 2.0355273740931033e-17;    // tail of log2  of e { 0x3c7777d0, 0xffda0d24 }

/*******************************************************************************
*                                                                              *
*    The base 2 logorithm function.  Caller’s rounding direction is honored.   *
*                                                                              *
*******************************************************************************/
double log2 ( double x ) 
{
       hexdouble yInHex, xInHex, OldEnvironment;
       register double n, zTail, low, z, z2, temp1, temp2, temp3, result;
       register uint32_t i;
       struct logTableEntry *tablePointer = ( struct logTableEntry * ) logTable;
       
       register double FPR_env, FPR_z, FPR_half, FPR_one, FPR_twoTo128, FPR_ln2, FPR_kConvDouble;
       register double FPR_r, FPR_s, FPR_t, FPR_u, FPR_y;
       register struct logTableEntry *pT;
       register int32_t nLong; // converted to double in two widely separated steps, avoiding LSU hazards
       hexdouble nInHex;
       
       xInHex.d = x;						nInHex.i.hi = 0x43300000; // store upper half

       FPR_z = 0.0;						FPR_half = 0.5;
       FPR_one = 1.0;						FPR_u = 1.0;
       FPR_twoTo128 = twoTo128;					FPR_ln2 = ln2;
       
       FEGETENVD( FPR_env );
       __ENSURE( FPR_z, FPR_twoTo128, FPR_ln2 );		__ENSURE( FPR_u, FPR_half, FPR_one );		
       FESETENVD( FPR_z );
              
       if ( FPR_z <= x && x < infinity.d ) 
       {                                             // x is finite and non-negative
              if ( x > largestDenorm )  
              {                                      // x is normal
                     i = xInHex.i.hi & 0x000fffff;
                     yInHex.i.lo = xInHex.i.lo;
                     yInHex.i.hi = i | 0x3ff00000;          // now 1.0 <= y < 2.0
                     if ( yInHex.i.hi & 0x00080000 ) 
                     {                               // 1.5 <= y < 2.0
                            nLong = ((int32_t) ( xInHex.i.hi >> 20 ) - 1022);
                            nLong ^= 0x80000000; // flip sign bit
                            nInHex.i.lo = nLong; // store lower half
                            i = ( i >> 13 ) & 0x3f;         // table lookup index
                            FPR_u = FPR_half;
                     }
                     else 
                     {                               // 1.0 <= y < 1.5
                            nLong = ((int32_t) ( xInHex.i.hi >> 20 ) - 1023);
                            nLong ^= 0x80000000; // flip sign bit
                            nInHex.i.lo = nLong; // store lower half
                            i = ( i >> 12 ) + 64;           // table lookup index
                            // FPR_u = FPR_one; via initialization above
                     }
              }
              else if ( x != FPR_z ) 
              {                                      // x is nonzero, denormal
                     xInHex.d = __FMUL( x, FPR_twoTo128 );
                    __NOOP;
                    __NOOP;
                    __NOOP;
                     i = xInHex.i.hi & 0x000fffff;
                     yInHex.i.lo = xInHex.i.lo;
                     yInHex.i.hi = i | 0x3ff00000;          // now 1.0 <= y < 2.0
                     if ( yInHex.i.hi & 0x00080000 ) 
                     {                               // 1.5 <= y < 2.0
                            nLong = ((int32_t) ( xInHex.i.hi >> 20 ) - 1150);
                            nLong ^= 0x80000000; // flip sign bit
                            nInHex.i.lo = nLong; // store lower half
                            i = ( i >> 13 ) & 0x3f;         // table lookup index
                            FPR_u = FPR_half;
                     }
                     else 
                     {                               // 1.0 <= y < 1.5
                            nLong = ((int32_t) ( xInHex.i.hi >> 20 ) - 1151);
                            nLong ^= 0x80000000; // flip sign bit
                            nInHex.i.lo = nLong; // store lower half
                            i = ( i >> 12 ) + 64;           // table lookup index
                            // FPR_u = FPR_one; via initialization above
                     }
              }
              else                                          // x is 0.0
              {
                     OldEnvironment.d = FPR_env;
					 __NOOP;
					 __NOOP;
					 __NOOP;
                     OldEnvironment.i.lo |= FE_DIVBYZERO;
                     result = -infinity.d;
                     FESETENVD_GRP( OldEnvironment.d );
                     return result;
              }
              
              {
                register double FPR_d2, FPR_d3, FPR_d4, FPR_d5, FPR_d6, FPR_d7, FPR_d8;

                FPR_d8 = d8;					FPR_d6 = d6;
                
                FPR_d7 = d7;					FPR_d5 = d5;

                pT = &(tablePointer[(int32_t)i]);
                FPR_r = pT->X;					FPR_t = pT->G;
              
                FPR_y = yInHex.d;					
                FPR_s = __FMSUB( FPR_u, FPR_y, FPR_r );		FPR_kConvDouble = kConvDouble.d;
              
                z = __FMUL( FPR_s, FPR_t );			
                FPR_u = __FNMSUB( z, FPR_r, FPR_s );		z2 = __FMUL( z, z );
                zTail = __FMUL( FPR_u, FPR_t);
              
                
                temp1 = __FMADD( FPR_d8, z2, FPR_d6 );		temp2 = __FMADD( FPR_d7, z2, FPR_d5);
                FPR_d4 = d4;				
                
                FPR_d3 = d3;
                temp1 = __FMADD( temp1, z2, FPR_d4 );		temp2 = __FMADD( temp2, z2, FPR_d3 );
    
                FPR_d2 = d2;				
                FPR_t = pT->F.d;				n = nInHex.d;
                
                temp1 = __FMADD( temp1, z, temp2 );		temp3 = z + FPR_t;
                
                temp2 = __FMADD( temp1, z, FPR_d2 );		FPR_u = FPR_t - temp3;
                
                FPR_s = FPR_u + z;				FPR_r = __FNMSUB( z, zTail, zTail );
                
                low = FPR_s + FPR_r;
    
                FPR_r = __FMADD( temp2, z2, low );
                FPR_s = log2e;					FPR_t = log2eTail;
                
                temp1 = FPR_r + temp3;				n -= FPR_kConvDouble;
                temp2 = temp3 - temp1 + FPR_r;
                FPR_u = __FMUL( temp1, FPR_t );		
                temp3 = __FMADD( temp2, FPR_s, FPR_u );
              }
              
              if ( (uint32_t)nLong == 0x80000000u /* iff n == 0.0 */ ) 
              {
                     result =  __FMADD( temp1, FPR_s, temp3 );
                     
                     FESETENVD( FPR_env );
                     if ( FPR_y != FPR_one )
                        __PROG_INEXACT( FPR_ln2 );
                        
                     return result;
              }
              else 
              {                                      // n != 0                     			
                     result = __FMADD( temp1, FPR_s, n);
                     FPR_t =  n - result;
                     temp3 = __FMADD( temp1, FPR_s, FPR_t ) + temp3;
                     result += temp3;
                     
                     FESETENVD( FPR_env );
                     if ( FPR_y != FPR_one )
                        __PROG_INEXACT( FPR_ln2 );
                        
                     return result;
              }
       }
       
       OldEnvironment.d = FPR_env;
       if ( x == FPR_z )
       {
              OldEnvironment.i.lo |= FE_DIVBYZERO;
              x = -infinity.d;
       }
       else if ( x < FPR_z )
       {                                           
              OldEnvironment.i.lo |= SET_INVALID;
              x = nan ( LOGORITHMIC_NAN );
       }
              
       FESETENVD_GRP( OldEnvironment.d );
       return x;
       
}

#endif /* BUILDING_FOR_CARBONCORE_LEGACY */
