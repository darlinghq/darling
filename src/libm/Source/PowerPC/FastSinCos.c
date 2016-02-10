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
*    FastSinCos.c                                                              *
*                                                                              *
*    Double precision Sine and Cosine.                                         *
*                                                                              *
*    Copyright © 1997-2001 by Apple Computer, Inc. All rights reserved.        *
*                                                                              *
*    Written by A. Sazegari, started on June 1997.                             *
*    Modified and ported by Robert A. Murley (ram) for Mac OS X.               *
*                                                                              *
*    A MathLib v4 file.                                                        *
*                                                                              *
*    Based on the trigonometric functions from IBM/Taligent.                   *
*                                                                              *
*    November  06 2001: commented out warning about Intel architectures.       *
*    July      20 2001: replaced __setflm with FEGETENVD/FESETENVD.            *
*                       replaced DblInHex typedef with hexdouble.              *
*    September 07 2001: added #ifdef __ppc__.                                  *
*    September 09 2001: added more comments.                                   *
*    September 10 2001: added macros to detect PowerPC and correct compiler.   *
*    September 18 2001: added <CoreServices/CoreServices.h> to get to <fp.h>   *
*                       and <fenv.h>, removed denormal comments.               *
*    October   08 2001: removed <CoreServices/CoreServices.h>.                 *
*                       changed compiler errors to warnings.                   *
*                                                                              *
*    These routines have a long double (107-bits) argument reduction to        *
*    better match their long double counterpart.                               *
*                                                                              *
*    W A R N I N G:                                                            *
*    These routines require a 64-bit double precision IEEE-754 model.          *
*    They are written for PowerPC only and are expecting the compiler          *
*    to generate the correct sequence of multiply-add fused instructions.      *
*                                                                              *
*    These routines are not intended for 32-bit Intel architectures.           *
*                                                                              *
*    A version of gcc higher than 932 is required.                             *
*                                                                              *
*    GCC compiler options:                                                     *
*          optimization level 3 (-O3)                                          *
*          -fschedule-insns -finline-functions -funroll-all-loops              *
*                                                                              *
*******************************************************************************/

#include    "fenv_private.h"
#include    "fp_private.h"
#include    "math.h"

#define     TRIG_NAN          "33"

/*******************************************************************************
*      Floating-point constants.                                               *
*******************************************************************************/

static const double kPiScale42 = 1.38168706094305449e13;   // 0x1.921fb54442d17p43
static const double kPiScale53 = 2.829695100811376e16;     // 0x1.921fb54442d18p54
static const double piOver4 = 0.785398163397448390;        // 0x1.921fb54442d19p-1
static const double piOver2 = 1.570796326794896619231322;  // 0x1.921fb54442d18p0
static const double piOver2Tail = 6.1232339957367660e-17;  // 0x1.1a62633145c07p-54
static const double twoOverPi = 0.636619772367581382;      // 0x1.45f306dc9c883p-1
//static const double k2ToM26 = 1.490116119384765625e-8;     // 0x1.0p-26;
static const double kMinNormal = 0x1.0p-1022;			   // 2.2250738585072014e-308;
static const double kRintBig = 2.7021597764222976e16;      // 0x1.8p54
static const double kRint = 6.755399441055744e15;          // 0x1.8p52
static const hexdouble infinity = HEXDOUBLE(0x7ff00000, 0x00000000);

/*******************************************************************************
*      Approximation coefficients.                                             *
*******************************************************************************/

static const double s13 = 1.5868926979889205164e-10;       // 1.0/13!
static const double s11 = -2.5050225177523807003e-8;       // -1.0/11!
static const double s9 =  2.7557309793219876880e-6;        // 1.0/9!
static const double s7 = -1.9841269816180999116e-4;        // -1.0/7!
static const double s5 =  8.3333333332992771264e-3;        // 1.0/5!
static const double s3 = -0.16666666666666463126;          // 1.0/3!
static const double c14 = -1.138218794258068723867e-11;    // -1.0/14!
static const double c12 = 2.087614008917893178252e-9;      // 1.0/12!
static const double c10 = -2.755731724204127572108e-7;     // -1.0/10!
static const double c8 =  2.480158729870839541888e-5;      // 1.0/8!
static const double c6 = -1.388888888888735934799e-3;      // -1.0/6!
static const double c4 =  4.166666666666666534980e-2;      // 1.0/4!
static const double c2 = -.5;                              // -1.0/2!

double sin ( double x ) 
{
      register double absOfX, intquo, arg, argtail, xSquared, xThird, xFourth, temp1, temp2, result;
      register uint32_t ltable;
      hexdouble      z, OldEnvironment;
      
      register double FPR_env, FPR_z, FPR_Min, FPR_pi4, FPR_piScale;
      register double FPR_t, FPR_inf, FPR_pi53, FPR_2divPi, FPR_PiDiv2, FPR_PiDiv2Tail, FPR_kRintBig, FPR_kRint;
      register uint32_t GPR_f;

      FEGETENVD( FPR_env );                     	// save env, set default
      FPR_z = 0.0;				
      
      FPR_pi4 = piOver4;
      absOfX = __FABS ( x ); 		 		__ENSURE( FPR_z, FPR_z, FPR_pi4 ); // ensure z and pi4 are in registers
      
      FESETENVD( FPR_z );
           
      if ( absOfX < FPR_pi4 ) 				// |x| < ¹/4
      {
            if (likely( absOfX != FPR_z ))
            {
                register double FPR_s3, FPR_s5, FPR_s7, FPR_s9, FPR_s11, FPR_s13;
                    
/*******************************************************************************
*      at this point, x is normal with magnitude between 0 and ¹/4.            *
*******************************************************************************/
                // sin polynomial approximation
                
                xSquared = __FMUL( x, x );  		
                FPR_Min = kMinNormal;
                __ENSURE( FPR_z, FPR_z, FPR_Min ); 	// ensure FPR_Min is loaded into register
                
                FPR_s9 = s9; 			 	FPR_s13 = s13;			
                
                FPR_s7 = s7; 			 	FPR_s11 = s11;
                
                xFourth = __FMUL( xSquared, xSquared ); xThird = __FMUL( x, xSquared );
                FPR_s5 = s5; 	                

                FPR_s3 = s3;
                temp1 = __FMADD( s13, xFourth, s9 );  	temp2 = __FMADD( s11, xFourth, s7 );

                temp1 = __FMADD( temp1, xFourth, s5 );  temp2 = __FMADD( temp2, xFourth, s3 );
    
                temp1 = __FMADD( temp1, xSquared, temp2 ); result = __FMADD( temp1, xThird, x );
                
                FESETENVD( FPR_env );        		// restore caller's mode

                if (unlikely( __FABS( result ) < FPR_Min ))
                    __PROG_UF_INEXACT( FPR_Min );
                else
                    __PROG_INEXACT( FPR_pi4 );
    
                return ( result );
            } 
            else 
            {
                FESETENVD( FPR_env );       		// restore caller's mode
                return x; 				// +0 -0 preserved
            }
        }

      
      if (unlikely( x != x ))                           // x is a NaN
      {
            FESETENVD( FPR_env );  // restore caller's mode
            return ( x );
      }

/*******************************************************************************
*      x has magnitude > ¹/4.                                                  *
*******************************************************************************/
      FPR_piScale = kPiScale42;
      FPR_piScale = __FMADD( FPR_z, FPR_z, FPR_piScale );
      FPR_inf = infinity.d; 		 		FPR_pi53 = kPiScale53;
      FPR_2divPi = twoOverPi; 	 			FPR_PiDiv2 = piOver2;
      FPR_PiDiv2Tail = piOver2Tail;	 		FPR_kRintBig = kRintBig;
      FPR_kRint = kRint;
      
      if (unlikely( absOfX > FPR_piScale )) 
      {            
/*******************************************************************************
*      |x| is huge or infinite.                                                *
*******************************************************************************/
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
            {                               	 	// loop to reduce x below
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
*     |x| < pi*2^42: further reduction is probably necessary.  A double-double *
*     reduced argument is determined ( arg:argtail ) .  It is possible that x  *
*     has been reduced below pi/4 in magnitude, but x is definitely nonzero    *
*     and safely in the normal range.                                          *
*******************************************************************************/
      // find integer quotient of x/(¹/2)
      FPR_t = __FMADD( x, FPR_2divPi, FPR_kRint ); 	intquo = FPR_t - FPR_kRint; 
      z.d = FPR_t;
      
      FPR_t = __FNMSUB( intquo, FPR_PiDiv2, x );  	arg = __FNMSUB( intquo, FPR_PiDiv2Tail, FPR_t );
      xSquared = __FMUL( arg, arg );		  	FPR_t -= arg;
      
      GPR_f = z.i.lo;
      xFourth = __FMUL( xSquared, xSquared );  		argtail = __FNMSUB( intquo, FPR_PiDiv2Tail, FPR_t );
      
/*******************************************************************************
*     multiple of ¹/2 ( mod 4)  determines approx used and sign of result.     *
*******************************************************************************/
      
      ltable = GPR_f & FE_ALL_RND;
      
      if ( ltable & 0x1u )  
      {                                    		// argument closest to ±¹/2
/*******************************************************************************
*     use cosine approximation.                                                *
*******************************************************************************/
            register double FPR_c2, FPR_c4, FPR_c6, FPR_c8, FPR_c10, FPR_c12, FPR_c14;
            
            FPR_c10 = c10; 				FPR_c14 = c14;
            FPR_c8 = c8; 			 	FPR_c12 = c12;
            
            temp1 = __FMADD( FPR_c14, xFourth, FPR_c10 ); temp2 = __FMADD( FPR_c12, xFourth, FPR_c8);
            FPR_c6 = c6; 			
            
            FPR_c4 = c4;
            temp1 = __FMADD( temp1, xFourth, FPR_c6 ); 	temp2 = __FMADD( temp2, xFourth, FPR_c4);
            
            FPR_c2 = c2;
            temp1 = __FMADD( temp1, xFourth, FPR_c2 );
            
            temp1 = __FMADD( xSquared, temp2, temp1 );
            temp1 = __FMSUB( arg, temp1, argtail );	// second-order correction
            
            if ( ltable < 2 )                   	// adjust sign of result
                  result = __FMADD( arg, temp1, 1.0 ); 	// positive
            else 
            {
                  arg = - arg;
                  result =__FMSUB( arg, temp1, 1.0 ); 	// negative
            }
            FESETENVD( FPR_env );        		// restore caller's mode
            __PROG_INEXACT( FPR_PiDiv2 );
      } 
      else 
      {
/*******************************************************************************
*     use sine approximation.                                                  *
*******************************************************************************/
            register double FPR_s3, FPR_s5, FPR_s7, FPR_s9, FPR_s11, FPR_s13;
            
            FPR_s9 = s9; 			 	FPR_s13 = s13;
            FPR_s7 = s7; 			 	FPR_s11 = s11;
            
            temp1 = __FMADD( FPR_s13, xFourth, FPR_s9 );temp2 = __FMADD( FPR_s11, xFourth, FPR_s7 );
            FPR_s5 = s5; 			
            
            FPR_s3 = s3;
            temp1 = __FMADD( temp1, xFourth, FPR_s5 );	temp2 = __FMADD( temp2, xFourth, FPR_s3 );
            
            temp1 = __FMADD( temp1, xSquared, temp2 );  xThird = __FMUL( arg, xSquared );
            
            temp1 = __FMADD( temp1, xThird, argtail ); 	// second-order correction

            if ( ltable < 2 )                    // adjust sign of final result
                  result = arg + temp1 ;         // positive
            else 
			{
                  arg = - arg;
                  result = arg - temp1;          // negative
			}
			
            FESETENVD( FPR_env );        		// restore caller's mode
            __PROG_INEXACT( FPR_PiDiv2 );
      }
      
      return ( result ) ;
}

/*******************************************************************************
*     Cosine section.                                                          *
*******************************************************************************/
double cos ( double x ) 
{
      register double absOfX, intquo, arg, argtail, xSquared, xThird, xFourth,
                      temp1, temp2, result;
      register uint32_t iquad;
      hexdouble z, OldEnvironment;
      
      register double FPR_env, FPR_z, FPR_pi4, FPR_piScale;
      register double FPR_t, FPR_inf, FPR_pi53, FPR_2divPi, FPR_PiDiv2, FPR_PiDiv2Tail, FPR_kRintBig, FPR_kRint;
      register uint32_t GPR_f;

      FEGETENVD( FPR_env );                     	// save env, set default
      FPR_z = 0.0;				
      
      FPR_pi4 = piOver4;
      absOfX = __FABS ( x ); 		 		__ENSURE( FPR_z, FPR_z, FPR_pi4 ); // ensure z and pi4 are in registers
      
      FESETENVD( FPR_z );
           
      if ( absOfX < FPR_pi4 )				// |x| < ¹/4
      {
            if (likely( absOfX != FPR_z )) 
            {
                // cos polynomial approximation
                register double FPR_c2, FPR_c4, FPR_c6, FPR_c8, FPR_c10, FPR_c12, FPR_c14, FPR_One;
                
                xSquared = __FMUL( x, x );                     
                FPR_One = 1.0;
                __ENSURE( FPR_z, FPR_z, FPR_One );

                FPR_c10 = c10; 				FPR_c14 = c14;
                
                FPR_c8 = c8; 			 	FPR_c12 = c12;
                
                FPR_c6 = c6; 			 	FPR_c4 = c4;

                xFourth = __FMUL( xSquared, xSquared );
                __NOOP;
                FPR_c2 = c2;

                temp1 = __FMADD( FPR_c14, xFourth, FPR_c10 ); 	temp2 = __FMADD( FPR_c12, xFourth, FPR_c8);
                temp1 = __FMADD( temp1, xFourth, FPR_c6 );	temp2 = __FMADD( temp2, xFourth, FPR_c4);
                
                temp1 = __FMADD( temp1, xFourth, FPR_c2 ); 	temp1 = __FMADD( xSquared, temp2, temp1 );
                result = __FMADD( xSquared, temp1, FPR_One );
                
                FESETENVD( FPR_env );        		// restore caller's mode
                __PROG_INEXACT( FPR_pi4 );
                        
                return ( result );
            } 
            else 
            {
                FESETENVD( FPR_env );       		// restore caller's mode
                return 1.0; 				
            }
      }

      if (unlikely( x != x ))                           // x is a NaN
      {
            FESETENVD( FPR_env );        		// restore caller's mode
            return ( x );
      }

/*******************************************************************************
*      x has magnitude > ¹/4.                                                  *
*******************************************************************************/
       FPR_piScale = kPiScale42;
       FPR_piScale = __FMADD( FPR_z, FPR_z, FPR_piScale );
       FPR_inf = infinity.d; 		 		FPR_pi53 = kPiScale53;
       FPR_2divPi = twoOverPi; 	 			FPR_PiDiv2 = piOver2;
       FPR_PiDiv2Tail = piOver2Tail;	 		FPR_kRintBig = kRintBig;
       FPR_kRint = kRint;

      if (unlikely( absOfX > FPR_piScale ))  

/*******************************************************************************
*      |x| is huge or infinite.                                                *
*******************************************************************************/
      {
            if ( absOfX == infinity.d )  
            {                               		// infinite case is invalid
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
*     |x| < pi*2^42: further reduction is probably necessary.  A double-double *
*     reduced argument is determined ( arg:argtail ) .  It is possible that x  *
*     has been reduced below pi/4 in magnitude, but x is definitely nonzero    *
*     and safely in the normal range.                                          *
*******************************************************************************/

        // find integer quotient of x/(¹/2)
        FPR_t = __FMADD( x, FPR_2divPi, FPR_kRint );
            
        intquo = FPR_t - FPR_kRint; 
        z.d = FPR_t;
      
        FPR_t = __FNMSUB( intquo, FPR_PiDiv2, x );
      
        arg = __FNMSUB( intquo, FPR_PiDiv2Tail, FPR_t );
      
        xSquared = __FMUL( arg, arg );		  	FPR_t -= arg;
        GPR_f = z.i.lo;
      
        xFourth = __FMUL( xSquared, xSquared );  	argtail = __FNMSUB( intquo, FPR_PiDiv2Tail, FPR_t );
      
        iquad = (GPR_f + 1 ) & FE_ALL_RND;        	// iquad = int multiple mod 4
      
/*******************************************************************************
*     multiple of ¹/2 ( mod 4)  determines approx used and sign of result.     *
*******************************************************************************/

      if ( iquad & 0x1u)  
      {                                      		// arg closest to 0 or ¹
/*******************************************************************************
*     use cosine approximation.                                                *
*******************************************************************************/
            register double FPR_c2, FPR_c4, FPR_c6, FPR_c8, FPR_c10, FPR_c12, FPR_c14;
            
            FPR_c10 = c10; 			 	FPR_c14 = c14;
            FPR_c8 = c8; 			 	FPR_c12 = c12;
            
            temp1 = __FMADD( FPR_c14, xFourth, FPR_c10 ); temp2 = __FMADD( FPR_c12, xFourth, FPR_c8);
            FPR_c6 = c6; 			

            FPR_c4 = c4;
            temp1 = __FMADD( temp1, xFourth, FPR_c6 );  temp2 = __FMADD( temp2, xFourth, FPR_c4);
            
            FPR_c2 = c2;
            temp1 = __FMADD( temp1, xFourth, FPR_c2 );  temp1 = __FMADD( xSquared, temp2, temp1 );
            
            temp1 = __FMSUB( arg, temp1, argtail );    	// second-order correction
            if ( iquad < 2 )                       	// adjust sign of result
                  result = __FMADD( arg, temp1, 1.0 );  // positive
            else 
            {
                  arg = - arg;
                  result =__FMSUB( arg, temp1, 1.0 );	// negative
            }
      }
      else 
      {
/*******************************************************************************
*     use sine approximation.                                                  *
*******************************************************************************/
            register double FPR_s3, FPR_s5, FPR_s7, FPR_s9, FPR_s11, FPR_s13;
            
            FPR_s9 = s9; 			 	FPR_s13 = s13;
            FPR_s7 = s7; 			 	FPR_s11 = s11;
            
            temp1 = __FMADD( FPR_s13, xFourth, FPR_s9 ); temp2 = __FMADD( FPR_s11, xFourth, FPR_s7 );
            FPR_s5 = s5; 			
            
            FPR_s3 = s3;
            temp1 = __FMADD( temp1, xFourth, FPR_s5 ); 	temp2 = __FMADD( temp2, xFourth, FPR_s3 );
            
            temp1 = __FMADD( temp1, xSquared, temp2 );  xThird = __FMUL( arg, xSquared );
            
            temp1 = __FMADD( temp1, xThird, argtail ); 	// second-order correction

            if ( iquad < 2 )                      // adjust sign of result
                  result = temp1 + arg;
            else 
			{
                  arg = - arg;
                  result = arg - temp1;
			}
      }

      FESETENVD( FPR_env );        				// restore caller's mode
      __PROG_INEXACT( FPR_PiDiv2 );
          
      return ( result ) ;
}

