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
*    tg.c                                                                      *
*                                                                              *
*    Double precision Tangent                                                  *
*                                                                              *
*    Copyright: © 1993-1997 by Apple Computer, Inc., all rights reserved       *
*                                                                              *
*    Written and ported by A. Sazegari, started on June 1997.                  *
*    Modified by Robert Murley, 2001                                           *
*                                                                              *
*    A MathLib v4 file.                                                        *
*                                                                              *
*    Based on the trigonometric functions from IBM/Taligent.                   *
*                                                                              *
*    Modification history:                                                     *
*    November  06 2001: commented out warning about Intel architectures.       *
*    July      20 2001: replaced __setflm with FEGETENVD/FESETENVD.            *
*                       replaced DblInHex typedef with hexdouble.              *
*    September 07 2001: added #ifdef __ppc__.                                  *
*    September 19 2001: added macros to detect PowerPC and correct compiler.   *
*    September 19 2001: added <CoreServices/CoreServices.h> to get to <fp.h>   *
*                       and <fenv.h>, removed denormal comments.               *
*    September 25 2001: removed more denormal comments.                        *
*    October   08 2001: removed <CoreServices/CoreServices.h>.                 *
*                       changed compiler errors to warnings.                   *
*                                                                              *
*    W A R N I N G:                                                            *
*    This routine requires a 64-bit double precision IEEE-754 model.           *
*    They are written for PowerPC only and are expecting the compiler          *
*    to generate the correct sequence of multiply-add fused instructions.      *
*                                                                              *
*    This routine is not intended for 32-bit Intel architectures.              *
*                                                                              *
*     08 Oct 01   ram   changed compiler errors to warnings.                   *
*                                                                              *
*      GCC compiler options:                                                   *
*            optimization level 3 (-O3)                                        *
*            -fschedule-insns -finline-functions -funroll-all-loops            *
*                                                                              *
*******************************************************************************/

#include    "math.h"
#include    "fenv_private.h"
#include    "fp_private.h"

#define TRIG_NAN  "33"

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

static const double kPiBy2 = 1.570796326794896619231322;      // 0x1.921fb54442d18p0
static const double kPiBy2Tail = 6.1232339957367660e-17;      // 0x1.1a62633145c07p-54
static const double k2ByPi = 0.636619772367581382;            // 0x1.45f306dc9c883p-1
static const double kRint = 6.755399441055744e15;             // 0x1.8p52
static const double kRintBig = 2.7021597764222976e16;         // 0x1.8p54
static const double kPiScale42 = 1.38168706094305449e13;      // 0x1.921fb54442d17p43
static const double kPiScale53 = 2.829695100811376e16;        // 0x1.921fb54442d18p54
static const hexdouble infinity = HEXDOUBLE(0x7ff00000, 0x00000000);

/*******************************************************************************
********************************************************************************
*                                 T   A   N                                    *
********************************************************************************
*******************************************************************************/

static const double ts11 = 8.898406739539066157565e-3,
            ts9  = 2.186936821731655951177e-2,
            ts7  = 5.396825413618260185395e-2,
            ts5  = 0.1333333333332513155016,
            ts3  = 0.3333333333333333333333;
static const double tt7  = 0.05396875452473400572649,
            tt5  = 0.1333333333304691192925,
            tt3  = 0.3333333333333333357614;
static const double k2ToM1023 = 0x1.0p-1023; // 1.112536929253600692e-308
static const uint32_t indexTable[] =
    {  
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,
    27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,
    38,  39,  40,  41,  42,  43,  44,  45,  47,  48,  49,
    50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,
    61,  62,  63,  64,  65,  66,  68,  69,  70,  71,  72,
    73,  74,  75,  76,  77,  78,  79,  81,  82,  83,  84,
    85,  86,  87,  88,  89,  91,  92,  93,  94,  95,  96,
    97,  98,  100, 101, 102, 103, 104, 105, 107, 108, 109,
    110, 111, 113, 114, 115, 116, 117, 119, 120, 121, 122,
    123, 125, 126, 127, 128, 130, 131, 132, 133, 135, 136,
    137, 139, 140, 141, 142, 144, 145, 146, 148, 149, 150,
    152, 153, 154, 156, 157, 159, 160, 161, 163, 164, 166,
    167, 168, 170, 171, 173, 174, 176, 177, 179, 180, 182,
    183, 185, 186, 188, 189, 191, 192, 194, 196, 197, 199,
    200, 202, 204, 205, 207, 209, 210, 212, 214, 215, 217,
    219, 220, 222, 224, 226, 228, 229, 231, 233, 235, 237,
    238, 240, 242, 244, 246, 248, 250, 252, 254, 256, 256
    };
    
static const double kMinNormal = 0x1.0p-1022;                 // 2.2250738585072014e-308;
static const double kPiDiv4 = 0.7853980064392090732;

double tan (   double x  )
{
      register double absOfX, intquo ,arg, argtail, aSquared, aThird, aFourth,
                      temp1, temp2, s, u, v, w, y, result;
      int tableIndex;
      uint32_t iz;
      hexdouble zD, aD, OldEnvironment;
      struct tableEntry *tablePointer;
     
      register double FPR_env, FPR_z, FPR_one, FPR_PiDiv4, FPR_256, FPR_piScale;
      register double FPR_t, FPR_u, FPR_inf, FPR_pi53, FPR_2divPi, FPR_PiDiv2, FPR_PiDiv2Tail, FPR_kRintBig, FPR_kRint;
      register uint32_t GPR_f;
      register struct tableEntry *pT;

      FEGETENVD( FPR_env );                     	// save env, set default
      
      FPR_z = 0.0;			 		tablePointer = ( struct tableEntry * ) ( tanatantable - ( 16 * 14 ) );      
      absOfX = __FABS( x );
      FPR_PiDiv4 = kPiDiv4;		 		FPR_256 = 256.0;
      FPR_kRint = kRint;		 		FPR_one = 1.0;
      FPR_PiDiv2 = kPiBy2;

      __ENSURE( FPR_PiDiv2, FPR_PiDiv4, FPR_256); 
      __ENSURE( FPR_z, FPR_kRint, FPR_one); 
      
      FESETENVD( FPR_z );
      
/*******************************************************************************
*     |x| < 0.7853980064392090732                                              *
*******************************************************************************/

      if ( absOfX < FPR_PiDiv4 ) 
      {  

            FPR_t = __FMADD( FPR_256, absOfX, FPR_kRint ); 
            aD.d = FPR_t;
            
            if (unlikely( absOfX == FPR_z ))
            {
                FESETENVD( FPR_env );       		// restore caller's mode
                return x; 				// +0 -0 preserved
            }
                

/*******************************************************************************
*     |x| < 0.0625                                                             *
*******************************************************************************/
            
            if ( aD.i.lo < 16u ) // No Store/Load hazard thanks to the imediately preceding branch
            {
                  register double FPR_ts3, FPR_ts5, FPR_ts7, FPR_ts9, FPR_ts11, FPR_Min;
                  
                  aSquared = __FMUL( absOfX, absOfX );	FPR_Min = kMinNormal;
                  __NOOP;

                  FPR_ts11 = ts11;		 	FPR_ts9 = ts9;
                  
                  FPR_ts7 = ts7;		 	FPR_ts5 = ts5;
                  
                  FPR_ts3 = ts3;			aThird = __FMUL( absOfX, aSquared );
                  
                  temp1 = __FMADD( FPR_ts11, aSquared, FPR_ts9 );
                  
                  temp1 = __FMADD( temp1, aSquared, FPR_ts7 );
                  
                  temp1 = __FMADD( temp1, aSquared, FPR_ts5 );
                  
                  temp1 = __FMADD( temp1, aSquared, FPR_ts3 );
                                    
                  if ( x > FPR_z )
                        result = __FMADD( temp1, aThird, absOfX );
                  else 
                  {
                        aThird = -aThird;
                        result = __FMSUB( temp1, aThird, absOfX );
                  }
                        
                  FESETENVD( FPR_env );             	// restore caller's mode
                  
                  if ( __FABS ( result ) < FPR_Min ) 
                        __PROG_UF_INEXACT( FPR_Min );
                  else
                        __PROG_INEXACT( FPR_PiDiv2 );

                  return result;
            }
            
/*******************************************************************************
*     .0625 <= x < .7853980064392090732.                                       *
*******************************************************************************/
            else 
            {
                  register double FPR_tt3, FPR_tt5, FPR_tt7, FPR_sqr, FPR_f0;
                  
                  tableIndex = indexTable[aD.i.lo - 16];
                  pT = &(tablePointer[tableIndex]);
                  FPR_f0 = pT->f0;
                  
                  w = absOfX - FPR_f0;  		// w = deltax
                  y = pT->p;            		// y = Tan from table
                  FPR_tt3 = tt3;

                  FPR_tt7 = tt7;			FPR_tt5 = tt5;
                  
                  FPR_sqr = __FMUL( w, w );		// calculate delta Tan
                  
                  temp1 = __FMADD( FPR_tt7, FPR_sqr, FPR_tt5 );
                  
                  temp1 = __FMADD( temp1, FPR_sqr, FPR_tt3 );
                  
                  temp1 = __FMADD( temp1, __FMUL( FPR_sqr, w ), w );
                  
                  v = __FMUL(y, temp1 );
                  
                  aSquared = __FMUL( v, v );		aThird = FPR_one + v;
                  
                  aFourth = __FMUL( aSquared,aSquared );w = __FMADD( aThird, aSquared, aThird );
                  
                  aThird = __FMADD( w, aFourth, w );	temp1 = __FMADD( v, y, temp1 );
                  
                  if ( x > FPR_z )			// fix final sign
                        result = __FMADD( temp1, aThird, y );
                  else 
                  {
                        aThird = -aThird;
                        result = __FMSUB( temp1, aThird, y );
                  }
                  
                  FESETENVD( FPR_env );            	 // restore caller's mode
                  __PROG_INEXACT( FPR_PiDiv2 );
                  
                  return result;
            }
      }
      
      if (unlikely( x != x ))                           // x is a NaN
      {
            FESETENVD( FPR_env );                   	// restore caller's mode
            return ( x );
      }
      
/*******************************************************************************
*     |x| > ¹/4 and perhaps |x| > kPiScale42.                                  *
*******************************************************************************/

      FPR_piScale = kPiScale42;	 			FPR_PiDiv2Tail = kPiBy2Tail;
      FPR_2divPi = k2ByPi; 			 	FPR_pi53 = kPiScale53;
      FPR_inf = infinity.d; 		 		FPR_kRintBig = kRintBig;
      __ENSURE( FPR_2divPi, FPR_PiDiv2, FPR_piScale );

      if (unlikely( absOfX > FPR_piScale )) 
      {                                                	// |x| is huge or infinite
            if ( absOfX == FPR_inf )  
            {                               	  	// infinite case is invalid
                  OldEnvironment.d = FPR_env;
				  __NOOP;
				  __NOOP;
				  __NOOP;
      
                  OldEnvironment.i.lo |= SET_INVALID;
                  FESETENVD_GRP( OldEnvironment.d );  	// restore caller's mode
                  return ( nan ( TRIG_NAN ) );    	// return NaN
            }
            
            while ( absOfX > FPR_pi53 )  
            {                               	  	// loop to reduce x below
                  intquo = __FMUL( x, FPR_2divPi );	// ¹*2^53 in magnitude
                  FPR_t = __FNMSUB( intquo, FPR_PiDiv2, x );
                  x = __FNMSUB( intquo, FPR_PiDiv2Tail, FPR_t );
                  absOfX = __FABS ( x ) ;
            }

/*******************************************************************************
*     final reduction of x to magnitude between 0 and 2*¹.                     *
*******************************************************************************/
            FPR_t = __FMADD( x, FPR_2divPi, FPR_kRintBig );
            intquo = FPR_t - FPR_kRintBig;
            FPR_t = __FNMSUB( intquo, FPR_PiDiv2, x );
            x = __FNMSUB( intquo, FPR_PiDiv2Tail, FPR_t );
            absOfX = __FABS( x );
      }
      
/*******************************************************************************
*     ¹/4 < x < ¹*2^42                                                         *
*******************************************************************************/
      
/*******************************************************************************
*     Further argument reduction is probably necessary.  A double-double       *
*     reduced argument is determined ( arg|argtail ).                          *
*******************************************************************************/
      FPR_t = __FMADD( x, FPR_2divPi, FPR_kRint );
      intquo = FPR_t - FPR_kRint; 
      zD.d = FPR_t;
      
      FPR_t = __FNMSUB( intquo, FPR_PiDiv2, x );
      arg = __FNMSUB( intquo, FPR_PiDiv2Tail, FPR_t );
      
      absOfX = __FABS( arg );	 			FPR_t -= arg;
      GPR_f = zD.i.lo;
      
      FPR_u = __FMADD( FPR_256, absOfX, FPR_kRint ); 
      aD.d = FPR_u;
      __NOOP;
      __NOOP;
      
      argtail = __FNMSUB( intquo, FPR_PiDiv2Tail, FPR_t );
      
/*******************************************************************************
*     |arg| < .0625.                                                           *
*******************************************************************************/

      if ( aD.i.lo < 16u ) // No Store/Load hazard, NOOPs above push this issue to next cycle.
      {
            register double FPR_ts3, FPR_ts5, FPR_ts7, FPR_ts9, FPR_ts11;
            
            aSquared = __FMUL( absOfX, absOfX );
            u = absOfX;
            v = argtail; 
            
            FPR_ts11 = ts11;		 		FPR_ts9 = ts9;
            
            FPR_ts7 = ts7;		 		FPR_ts5 = ts5;
            
            FPR_ts3 = ts3;           			iz = GPR_f & 1u; // quo modulo 2

            temp1 = __FMADD( FPR_ts11, aSquared, FPR_ts9 );
            
            temp1 = __FMADD( temp1, aSquared, FPR_ts7 );
            
            temp1 = __FMADD( temp1, aSquared, FPR_ts5 );
            
            temp1 = __FMADD( temp1, aSquared, FPR_ts3 );
            
            aThird = __FMUL( aSquared, u );
            
/*******************************************************************************
*     tangent approximation starts here.                                       *
*******************************************************************************/

            if ( iz == 0 ) 
            {
                  if ( arg > FPR_z ) 
                  {                                    	// positive arg
                        w = __FMADD( temp1, aThird, v );
                        result =  ( w + u );
                  }
                  else 
                  {                                    	// negative arg
                        w = __FNMSUB( temp1, aThird, v );
                        result = ( w - u );
                  }
                        
                  FESETENVD( FPR_env );             	// restore caller's mode
                  __PROG_INEXACT( FPR_PiDiv2 );
                  
                  return result;   
            }
            
/*******************************************************************************
*     cotangent approximation starts here.                                     *
*******************************************************************************/
            else 
            {
                  register double FPR_k2ToM1023;
                  
                  FPR_k2ToM1023 = k2ToM1023;
                  
                  if ( arg > FPR_z ) 
                  {                                    	// positive argument
                        s = __FMADD( temp1, aThird, v );
                        temp2 = s + u;
                  }
                  else 
                  {                                    	// negative argument
                        s = __FMSUB( temp1, aThird, v );
                        temp2 = s + u;
                  }
                  
                  aFourth = ( u - temp2 ) + s;
                  
                  if ( __FABS ( temp2 ) < k2ToM1023 ) 
                  {                                    	// huge result
                        if ( arg > FPR_z ) 
                              result = ( FPR_one / temp2 );
                        else 
                              result = ( FPR_one / ( -temp2 ) );
                              
                        FESETENVD( FPR_env );		// restore caller's mode
                        __PROG_INEXACT( FPR_PiDiv2 );
                        
                        return result;   
                  }
                  
                  u = FPR_one / temp2;
                  v = __FNMSUB( u, temp2, FPR_one );
                  temp1 = __FMSUB( aFourth, u, v );
                  
                  if ( arg > FPR_z )
                        result = __FMSUB( temp1, u, u );
                  else 
                        result = __FNMSUB( temp1, u, u );
                  
                  FESETENVD( FPR_env );             // restore caller's mode
                  __PROG_INEXACT( FPR_PiDiv2 );
                  
                  return result;   
            }
      } 
      
/*******************************************************************************
*     The following code covers the case where the reduced argument has        *
*     magnitude greater than 0.0625 but less than ¹/4.                         *
*******************************************************************************/
      
      tableIndex = indexTable [ aD.i.lo - 16 ];
      
      if ( arg > FPR_z ) 
      {                                                        	     // positive argument
            w = absOfX - tablePointer [tableIndex].f0 + argtail;     // argument delta
            aSquared = __FMUL( w, w );
            v = absOfX - tablePointer [tableIndex].f0 - w + argtail; // tail of argument delta
      }
      else 
      {                                                        	     // negative argument
            w = absOfX - tablePointer [tableIndex].f0 - argtail;     // argument delta
            aSquared = __FMUL( w, w );
            v = absOfX - tablePointer [tableIndex].f0 - w - argtail; // tail of argument delta
      }
      {
      register double FPR_tt3, FPR_tt5, FPR_tt7;
      
      FPR_t = v + w;					FPR_u = __FMUL( aSquared, w );
      y = tablePointer[tableIndex].p;			FPR_tt3 = tt3;

      FPR_tt7 = tt7;					FPR_tt5 = tt5;
      
      temp1 = __FMADD( FPR_tt7, aSquared, FPR_tt5 );
    
      temp1 = __FMADD( temp1, aSquared, FPR_tt3 );
    
      temp1 = __FMADD( temp1, FPR_u, FPR_t );
      
      v = __FMUL( y, temp1 );                                         // Tan( delta )
      }

/*******************************************************************************
*      polynomial approx of 1/(1-v)                                            *
*******************************************************************************/

      aSquared = __FMUL( v, v );	      		iz = GPR_f & 1u; // quo modulo 2
      aThird = FPR_one + v;
      aFourth = __FMUL( aSquared, aSquared );
      w = __FMADD( aThird, aSquared, aThird );
      aThird = __FMADD( w, aFourth, w );		// aThird = 1/(1-v)
      s = __FMUL( __FMADD( y, y, FPR_one ), aThird );
      
      if ( iz == 0 ) 
      {                                   		// tangent approximation
            if ( arg > FPR_z )
                  result = __FMADD( temp1, s, y );
            else
            {
                  y = -y;
                  result = __FNMSUB( temp1, s, y );
            }
                  
            FESETENVD( FPR_env );             		// restore caller's mode
            __PROG_INEXACT( FPR_PiDiv2 );
            
            return result;   
      }
      else 
      {                                          	// cotangent approx
            w = __FMADD( temp1, s, y );
            aFourth = __FMADD( temp1, s, ( y - w ) );
            u = FPR_one / w;
            v = __FNMSUB( u, w, FPR_one);
            w = __FMSUB( u, aFourth, v );
            
            if ( arg > FPR_z )
                  result = __FMSUB( u, w, u );
            else 
                  result = __FNMSUB( u, w, u );
            
            FESETENVD( FPR_env );             		// restore caller's mode
            __PROG_INEXACT( FPR_PiDiv2 );
            
            return result;   
      }
}
