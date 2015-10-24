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
*     File sqrt.c,                                                             *
*     Function square root (IEEE-754).                                         *
*                                                                              *
*     Implementation of square root based on the approximation provided by     *
*     Taligent, Inc. who received it from IBM.                                 *
*                                                                              *
*     Copyright © 1996-2001 Apple Computer, Inc.  All rights reserved.         *
*                                                                              *
*     Modified and ported by A. Sazegari, started on June 1996.                *
*     Modified and ported by Robert A. Murley (ram) for Mac OS X.              *
*                                                                              *
*     A MathLib v4 file.                                                       *
*                                                                              *
*     This file was received from Taligent which modified the original IBM     *
*     sources.  It returns the square root of its double argument, using       *
*     Markstein algorithm and requires MAF code generation.                    *
*                                                                              *
*     The algorithm uses Newton’s method to calculate the IEEE-754 correctly   *
*     rounded double square root, starting with 8 bit estimate for:            *
*     g ≈ √x and y ≈ 1/2√x.  Using MAF instructions, each iteration refines    *
*     the original approximations g and y with rounding mode set to nearest.   *
*     The final step calculates g with the caller’s rounding restored.  This   *
*     in turn guarantees proper IEEE rounding and exceptions. INEXACT is the   *
*     only possible exception raised in this calculation.  Initial guesses     *
*     for g and y are determined from argument x via table lookup into the     *
*     array SqrtTable[256].                                                    *
*                                                                              *
*     Note: NaN is set computationally.  It will be changed to the proper      *
*     NaN code later.                                                          *
*                                                                              *
*     April     14 1997: added this file to mathlib v3 and included mathlib’s  *
*                        nan code.                                             *
*     July      23 2001: replaced __setflm with FEGETENVD/FESETENVD.           *
*     August    28 2001: added #ifdef __ppc__.                                 *
*                        replaced DblInHex typedef with hexdouble.             *
*                        used standard exception symbols from fenv.h.          *
*     September 09 2001: added more comments.                                  *
*     September 10 2001: added macros to detect PowerPC and correct compiler.  *
*     September 18 2001: added <CoreServices/CoreServices.h> to get <fp.h>.    *
*     October   08 2001: removed <CoreServices/CoreServices.h>.                *
*                        changed SqrtTable to const private_extern             *
*                        changed compiler errors to warnings.                  *
*     November  06 2001: commented out warning about Intel architectures.      *
*     November  19 2001: <scp> Added single precision implementation.          *
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

#include     "fenv_private.h"
#include     "fp_private.h"
#include     "math.h"
#include <System/ppc/cpu_capabilities.h>

inline static uint32_t __attribute__((always_inline))
_get_cpu_capabilities(void) 
{
        uint32_t caps;
        asm("lwz %0, %1(0)" : "=r" (caps) : "I" (_COMM_PAGE_CPU_CAPABILITIES));
        return caps;
}

#define      twoTo512           1.34078079299425971e154    // 2**512    
#define      twoToMinus256      8.636168555094444625e-78   // 2**-256
#define      upHalfOfAnULP      0.500000000000000111       // 0x1.0000000000001p-1

#define      SQRT_NAN           "1"

/*******************************************************************************
*     SqrtTable[256] contains initial estimates for the high significand bits  *
*     of sqrt and correction factor.  This table is  also used by the IBM      *
*     arccos and arcsin functions.                                             *
*******************************************************************************/

__private_extern__
const unsigned short SqrtTable[256] =
      {
      27497, 27752, 28262, 28517, 28772, 29282, 29537, 29792, 30302, 30557, 31067, 31322,
      31577, 32088, 32343, 32598, 33108, 33363, 33618, 34128, 34384, 34639, 35149, 35404,
      35659, 35914, 36425, 36680, 36935, 37446, 37701, 37956, 38211, 38722, 38977, 39232,
      39487, 39998, 40253, 40508, 40763, 41274, 41529, 41784, 42040, 42550, 42805, 43061,
      43316, 43571, 44082, 44337, 44592, 44848, 45103, 45358, 45869, 46124, 46379, 46635,
      46890, 47401, 47656, 47911, 48167, 48422, 48677, 48933, 49443, 49699, 49954, 50209,
      50465, 50720, 50976, 51231, 51742, 51997, 52252, 52508, 52763, 53019, 53274, 53529,
      53785, 54296, 54551, 54806, 55062, 55317, 55573, 55828, 56083, 56339, 56594, 56850,
      57105, 57616, 57871, 58127, 58382, 58638, 58893, 59149, 59404, 59660, 59915, 60170,
      60426, 60681, 60937, 61192, 61448, 61703, 61959, 62214, 62470, 62725, 62981, 63236,
      63492, 63747, 64003, 64258, 64514, 64769, 65025, 65280, 511,   510,   764,   1018,
      1272,  1526,  1780,  2034,  2288,  2542,  2796,  3050,  3305,  3559,  3813,  4067,
      4321,  4576,  4830,  5084,  5338,  5593,  5847,  6101,  6101,  6356,  6610,  6864,
      7119,  7373,  7627,  7882,  8136,  8391,  8391,  8645,  8899,  9154,  9408,  9663,
      9917,  10172, 10172, 10426, 10681, 10935, 11190, 11444, 11699, 11699, 11954, 12208,
      12463, 12717, 12972, 13226, 13226, 13481, 13736, 13990, 14245, 14245, 14500, 14754,
      15009, 15264, 15518, 15518, 15773, 16028, 16282, 16537, 16537, 16792, 17047, 17301,
      17556, 17556, 17811, 18066, 18320, 18575, 18575, 18830, 19085, 19339, 19339, 19594,
      19849, 20104, 20104, 20359, 20614, 20868, 21123, 21123, 21378, 21633, 21888, 21888,
      22143, 22398, 22653, 22653, 22907, 23162, 23417, 23417, 23672, 23927, 23927, 24182,
      24437, 24692, 24692, 24947, 25202, 25457, 25457, 25712, 25967, 25967, 26222, 26477,
      26732, 26732, 26987, 27242 };

//    There are flag and rounding errors being generated in this file

static const double Two911      = 0x1.0p-911; // HEXDOUBLE( 0x07000000, 0x00000000 );
static const hexdouble infinity = HEXDOUBLE( 0x7ff00000, 0x00000000 );
static const double twoTo128    = 0x1.0p+128; // 0.340282366920938463463e39;
static const double twoToM128   = 0x1.0p-128; // 0.293873587705571876992e-38;

double __sqrt ( double x );
double __sqrt ( double x )
{
      register int index;
      hexdouble xInHex, yInHex, gInHex;
      register double OldEnvironment, g, y, y2, d, e;
      register uint32_t xhead, ghead, yhead;
      
      register double FPR_z, FPR_Two911, FPR_TwoM128, FPR_Two128, FPR_inf, FPR_HalfULP;
      
      xInHex.d = x;					FPR_z = 0.0;
      FPR_inf = infinity.d;				FPR_Two911 = Two911;
      FPR_TwoM128 = twoToM128;				FPR_Two128 = twoTo128;
      gInHex.i.lo = 0u;				yInHex.i.lo = 0u;
      FPR_HalfULP = upHalfOfAnULP;
      
      FEGETENVD( OldEnvironment );               	// save environment, set default
      
      __NOOP;
      __ENSURE( FPR_TwoM128, FPR_Two128, FPR_z );
      
       FESETENVD( FPR_z );
      __ENSURE( FPR_HalfULP, FPR_inf, FPR_Two911 );
      
/*******************************************************************************
*     Special case for GPUL: storing and loading floats avoids L/S hazard  
*******************************************************************************/
      __NOOP;
      if (likely(  FPR_TwoM128 < x &&  x < FPR_Two128 )) // Can float hold initial guess?
      {
            hexsingle GInHex, YInHex;
            register uint32_t GPR_t, GPR_foo;

/*******************************************************************************
*     Calculate initial estimates for g and y from x and SqrtTable[].          *
*******************************************************************************/
            
            xhead = xInHex.i.hi;			// high 32 bits of x
            index = ( xhead >> 13 ) & 0xffu; 		// table index
            GPR_t = SqrtTable[index];
  
            // Form *single* precision exponent estimate from double argument:
            // (((((xhead - bias1024) >> 1) + bias128) << 3) & 0x7f800000) =
            // (((((xhead - bias1024 + bias128 + bias128) >> 1) << 3) & 0x7f800000))
            
            ghead = ( ( xhead + 0x07f00000 + 0x07f00000 - 0x3ff00000 ) << 2 ) & 0x7f800000;
            GInHex.lval = ghead + ( ( 0xff00u & GPR_t ) << 7 );

            // Force GInHex.lval to memory. Then load it into g in the FPU register file
            // on the *following* cycle. This avoids a Store/Load hazard.
            asm volatile ( "add %0, %1, %2" : "=r" (GPR_foo) : "b" (&GInHex), "b" (&YInHex) : "memory" ); /* NOOP */
            g = GInHex.fval;

            yhead = 0x7e000000u - ghead;
            YInHex.lval = yhead + ( ( 0xffu & GPR_t ) << 15 ); 
            
            // Force YInHex.lval to memory. Then load it into y in the FPU register file
            // on the *following* cycle. This avoids a Store/Load hazard.
            asm volatile ( "add %0, %1, %2" : "=r" (GPR_foo) : "b" (&GInHex), "b" (&YInHex) : "memory" ); /* NOOP */
            __NOOP;
            __NOOP;
            y = YInHex.fval;
            
/*******************************************************************************
*     Iterate to refine both g and y.                                          *
*******************************************************************************/

            d = __FNMSUB( g, g, x );			y2 = y + y;
            
            g = __FMADD(y, d, g );                 				//   16-bit g
            
            e = __FNMSUB( y, g, FPR_HalfULP );		d = __FNMSUB( g, g, x );
            
            y = __FMADD( e, y2, y );             				//   16-bit y
            
            y2 = y + y;					g = __FMADD(y, d, g);   //   32-bit g
            
            e = __FNMSUB( y, g, FPR_HalfULP );		d = __FNMSUB( g, g, x );
            
            y = __FMADD( e, y2, y );                  				//   32-bit y
            
            y2 = y + y;                  		g = __FMADD(y, d, g );  //   64-bit g before rounding
            
            e = __FNMSUB( y, g, FPR_HalfULP );		d = __FNMSUB( g, g, x );
                                
            y = __FMADD( e, y2, y );                  				//   64-bit y
            
             FESETENVD( OldEnvironment );   					//   restore caller's environment
            
            __NOOP;
            return (  __FMADD( y, d, g ) );            				//   final step
      }
        
      if (likely( FPR_z < x && x < FPR_inf )) 
      {

/*******************************************************************************
*     First and most common section: argument > 2.0^(-911), about 5.77662E-275.*
*******************************************************************************/
            
            if (likely( FPR_Two911 < x )) 
            {
                  register uint32_t GPR_t;

/*******************************************************************************
*     Calculate initial estimates for g and y from x and SqrtTable[].          *
*******************************************************************************/
                  xhead = xInHex.i.hi;                         	// high 32 bits of x
                  index = ( xhead >> 13 ) & 0xffu; 		// table index
                  GPR_t = SqrtTable[index];
                  
                  ghead = ( ( xhead + 0x3ff00000 ) >> 1 ) & 0x7ff00000;
                  yhead = 0x7fc00000u - ghead;
                
                  gInHex.i.hi = ghead + ( ( 0xff00u & GPR_t ) << 4 );
                  g = gInHex.d;
                
                  yInHex.i.hi = yhead + ( ( 0xffu & GPR_t ) << 12 ); 
                  y = yInHex.d;
/*******************************************************************************
*     Iterate to refine both g and y.                                          *
*******************************************************************************/

                  d = __FNMSUB( g, g, x );		y2 = y + y;
                  
                  g = __FMADD(y, d, g );                 			//   16-bit g
                  
                  e = __FNMSUB( y, g, FPR_HalfULP );	d = __FNMSUB( g, g, x );
                  
                  y = __FMADD( e, y2, y );             				//   16-bit y
                  
                  y2 = y + y;				g = __FMADD(y, d, g);   //   32-bit g
                  
                  e = __FNMSUB( y, g, FPR_HalfULP );	d = __FNMSUB( g, g, x );
                  
                  y = __FMADD( e, y2, y );                  			//   32-bit y
                  
                  y2 = y + y;                  		g = __FMADD(y, d, g );  //   64-bit g before rounding
                  
                  e = __FNMSUB( y, g, FPR_HalfULP );	d = __FNMSUB( g, g, x );
                  			
                  y = __FMADD( e, y2, y );                  			//   64-bit y
                  
                  FESETENVD( OldEnvironment );   				//   restore caller's environment
                  return ( __FMADD( y, d, g ) );            			//   final step
            }

/*******************************************************************************
*     Second section: 0.0 < argument < 2.0^(-911) which is about 5.77662E-275. *
*     Identical to the previous segment aside from 2^512 scale factor.         *
*******************************************************************************/
            else
            { 
                  xInHex.d = x * twoTo512;         //   scale up by 2^512
		  __NOOP;
		  __NOOP;
		  __NOOP;
                  xhead = xInHex.i.hi;

/*******************************************************************************
*     Calculate initial estimates for g and y from x and SqrtTable[].          *
*******************************************************************************/

                  ghead = ( ( xhead + 0x3ff00000 ) >> 1) & 0x7ff00000;
                  index = ( xhead >> 13) & 0xffu; // table index
                  yhead = 0x7fc00000u - ghead;
                  gInHex.i.hi = ghead + ( ( 0xff00u & SqrtTable[index] ) << 4 );
                  yInHex.i.hi = yhead + ( ( 0xffu & SqrtTable[index] ) << 12 );
                  x = xInHex.d;
                  g = gInHex.d;
                  y = yInHex.d;

/*******************************************************************************
*     Iterate to refine both g and y.                                          *
*******************************************************************************/
            
                  d = __FNMSUB( g, g, x );		y2 = y + y;
                  
                  g = __FMADD(y, d, g );                 			 //   16-bit g
                  
                  e = __FNMSUB( y, g, FPR_HalfULP );	d = __FNMSUB( g, g, x );
                  
                  y = __FMADD( e, y2, y );             				//   16-bit y
                  
                  y2 = y + y;				g = __FMADD(y, d, g);   //   32-bit g
                  
                  e = __FNMSUB( y, g, FPR_HalfULP );	d = __FNMSUB( g, g, x );
                  
                  y = __FMADD( e, y2, y );                  			//   32-bit y
                  
                  y2 = y + y;                  		g = __FMADD(y, d, g );  //   64-bit g before rounding
                  
                  e = __FNMSUB( y, g, FPR_HalfULP );	d = __FNMSUB( g, g, x );
                  			
                  d *= twoToMinus256;			g *= twoToMinus256;     //   undo scaling
                  
                  y = __FMADD( e, y2, y );                  			//   64-bit y
                  
                  FESETENVD( OldEnvironment );   				//   restore caller's environment
                  return ( __FMADD( y, d, g ) );            			//   final step
            }
      }

/*******************************************************************************
*     Fourth section: special cases: argument is +INF, NaN, +/-0.0, or <0.     *
*******************************************************************************/
   
      FESETENVD( OldEnvironment );       		//   restore caller's environment

      if ( x < FPR_z && x == x  )			// < 0.0 and not NAN 
      {
            hexdouble env;
            x = nan ( SQRT_NAN );
            env.d = OldEnvironment;
	    __NOOP;
	    __NOOP;
	    __NOOP;
      
            env.i.lo |= SET_INVALID;
            FESETENVD_GRP( env.d );         		//   restore caller's environment
            return ( x );               		// return NAN
      }
      else return ( x );				// NANs and +/-0.0
}

/*
By pushing the software sqrt implementation into its own routine, we avoid the PIC setup and simply have:
_sqrt:
00000340        lwz     r0,0x8020(0)
00000344        andis.  r2,r0,0x2000
00000348        bne     0x350
0000034c        b       ___sqrt
00000350        fsqrt   f1,f1
00000354        blr
Perhaps gcc can someday be coaxed to eliminate the extra branch?
Matt suggests "use -freorder-blocks", we'll pick that up for the next release.
*/
double sqrt(double x)
{
      if ((_get_cpu_capabilities() & kHasFsqrt))
	return __fsqrt( x );
      else return __sqrt( x );
}
	    

