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

#include "math.h"
#include "fenv.h"
#include "fp_private.h"
#include "fenv_private.h"

float acosf( float x )		{ return (float)acos((double)( x )); }
float asinf( float x )		{ return (float)asin((double)( x )); }
float atanf( float x )		{ return (float)atan( (double)x ); }
float atan2f( float y, float x) { return (float)atan2( (double)y, (double)x ); }
float cosf( float x )		{ return (float)cos((double)( x )); }
float sinf( float x)		{ return (float)sin( (double)x ); }
float tanf( float x )		{ return (float)tan( (double)x ); }
float acoshf( float x )		{ return (float)acosh( (double)x ); }
float asinhf( float x )		{ return (float)asinh( (double)x ); }
float atanhf( float x )		{ return (float)atanh( (double)x ); }
float coshf( float x)		{ return (float)cosh( (double)x ); }
float sinhf( float x)		{ return (float)sinh( (double)x ); }
float tanhf( float x)		{ return (float)tanh( (double)x ); }

float expf( float x)		{ return (float)exp( (double)x ); }
float exp2f( float x)		{ return (float)exp2( (double)x ); }
float expm1f( float x )		{ return (float)expm1( (double)x ); }
float logf ( float x )		{ return (float)log( (double)x ); }
float log10f ( float x )	{ return (float)log10( (double)x ); }
float log2f ( float x )		{ return (float)log2( x ); }
float log1pf ( float x )	{ return (float)log1p( (double)x ); }

float cbrtf( float x )		{ return (float)cbrt((double)( x )); }
float powf ( float x, float y )	{ return (float)pow ( (double)x, (double)y ); }

float erff( float x )		{ return (float)erf((double)( x )); }
float erfcf( float x )		{ return (float)erfc((double)( x )); }
float lgammaf( float x )	{ return (float)lgamma((double)( x )); }
float tgammaf( float x )	{ return (float)tgamma((double)( x )); }

 /* By calling through the double sqrt, we let the 970 hardware have a crack at this. */
float sqrtf ( float x )	{ return (float) sqrt((double) x); }

static const hexsingle HugeF = { 0x7F800000 };
static const hexsingle NegHugeF = { 0xFF800000 };

float hypotf ( float x, float y )
{
        register float temp;
	hexdouble OldEnvironment, CurrentEnvironment;
        
        register float FPR_z, FPR_one, FPR_inf, FPR_Minf, FPR_absx, FPR_absy, FPR_big, FPR_small;
        register double FPR_env;
        
        FPR_z = 0.0f;					FPR_one = 1.0f;
        FPR_inf = HugeF.fval;				FPR_Minf = NegHugeF.fval;
        FPR_absx = __FABSF( x );			FPR_absy = __FABSF( y );
      
/*******************************************************************************
*     If argument is SNaN then a QNaN has to be returned and the invalid       *
*     flag signaled.                                                           * 
*******************************************************************************/
	
	if (unlikely( ( x == FPR_inf ) || ( y == FPR_inf ) || ( x == FPR_Minf ) || ( y == FPR_Minf ) ))
            return FPR_inf;
                
        if (unlikely( ( x != x ) || ( y != y ) ))
        {
            x = __FABSF ( x + y );
            return x;
        }
            
        if ( FPR_absx > FPR_absy )
        {
            FPR_big = FPR_absx;
            FPR_small = FPR_absy;
        }
        else
        {
            FPR_big = FPR_absy;
            FPR_small = FPR_absx;
        }
        
        // Now +0.0 <= FPR_small <= FPR_big < INFINITY
        
        if (unlikely( FPR_small == FPR_z ))
            return FPR_big;
            
        FEGETENVD( FPR_env );				// save environment, set default
        FESETENVD( FPR_z );

        temp = FPR_small / FPR_big;			OldEnvironment.d = FPR_env;
        temp = sqrtf ( FPR_one + temp * temp );	   
        
        FEGETENVD_GRP( CurrentEnvironment.d );
        CurrentEnvironment.i.lo &= ~FE_UNDERFLOW;	// Clear any inconsequential underflow
        FESETENVD_GRP( CurrentEnvironment.d );
        
        temp = FPR_big * temp;				// Might raise UNDERFLOW or OVERFLOW
            
        FEGETENVD_GRP( CurrentEnvironment.d );
        OldEnvironment.i.lo |= CurrentEnvironment.i.lo; // Pick up any UF or OF
        FESETENVD_GRP( OldEnvironment.d );        		// restore caller's environment

        return temp;
}

