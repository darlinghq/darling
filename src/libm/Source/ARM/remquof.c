/*
 *  remquof.c
 *  cLibm
 *
 *     Written by Jon Okada, started on December 7th, 1992.                      
 *     Modified by Paul Finlayson (PAF) for MathLib v2.                          
 *     Modified by A. Sazegari (ali) for MathLib v3.                             
 *     Modified and ported by Robert A. Murley (ram) for Mac OS X.               
 *	   Modified for cLibm, fixed a few edge cases, rewrote local_ funcs by Ian Ollmann.
 *
 *  Copyright 2007 Apple Inc. All rights reserved.
 *
 */
 
#include <math.h>
#include <float.h>
#include <stdint.h>

#ifdef ARMLIBM_SET_FLAGS
#include "required_arithmetic.h"
#endif

static inline int local_ilogbf( float x ) __attribute__ ((always_inline));
static inline int local_ilogbf( float x )
{
	union{ float f; uint32_t u;}u = {x};
	
	u.u &= 0x7fffffff;
	int32_t exp = u.u >> 23;
	
	if( __builtin_expect( (uint32_t) exp - 1U >= 254, 0 ) )
	{ // +-denorm, other possibilities: +-0, +-inf, NaN are screened out by caller
	
		u.u |= 0x3f800000U;
		u.f -= 1.0f;
		exp = u.u >> 23;
		
		return exp - (127+126);
	}

	return exp - 127;
}

static inline float local_scalbnf( float x, int n ) __attribute__ ((always_inline));
static inline float local_scalbnf( float x, int n )
{
	//create 2**n in double
	union{ uint64_t u; double d;} u = {  ( (int64_t) n + 1023) << 52 };
	double d = x;
 
	//scale by appropriate power of 2
	d *= u.d;
 
	//return result
	return (float) d;
}
 

  
float remquof( float x, float y, int *quo )
{
 
	*quo = 0;					// Init quotient result

	// deal with NaN
	if( x != x || y != y )
		return x + y;

	//x = Inf or y = 0, return Invalid per IEEE-754
	float fabsx = __builtin_fabsf(x);	
	if( fabsx == __builtin_inff() || 0.0f == y )
	{
#ifdef ARMLIBM_SET_FLAGS
		return required_add_float( __builtin_inff(), -__builtin_inff() );	//set invalid
#else
		return __builtin_nanf("");
#endif
	}

	//handle trivial case
	float fabsy = __builtin_fabsf(y);
	if( fabsy == __builtin_inff() || 0.0f == x )
	{
#ifdef ARMLIBM_SET_FLAGS
		required_add_float( fabsx, 0.0f );			//signal underflow (that is, no flag set, but exception occurs if unmasked) if x is denorm
#endif
		return x;
	}
 
	//we have to work
	int32_t iquo = 0;
	int32_t iscx = local_ilogbf(fabsx);
	int32_t iscy = local_ilogbf(fabsy);
	int32_t idiff = iscx - iscy;
	float x1, y1, z;
	x1 = fabsx;
	y1 = fabsy;
	union{ float d[2]; uint32_t u[2];} u = { { x, y } };
	int32_t sign = u.u[0] ^ u.u[1];
	sign = sign >> 31;

	if( idiff >= 0 )
	{
		int32_t i;
		
		if( idiff )
		{
			y1 = local_scalbnf( y1, -iscy );
			x1 = local_scalbnf( x1, -iscx );
			for( i = idiff; i != 0; i-- )
			{
				if( x1 >= y1 )
				{
					x1 -= y1;
					iquo += 1;
				}
				iquo += iquo;
				x1 += x1;
			}
			x1 = local_scalbnf( x1, iscy );
		}
		if( x1 >= fabsy )
		{
			x1 -= fabsy;
			iquo += 1;
		}
	}
	
	if( x1 < 0x1.0p127f )
	{
		z = x1 + x1;
		if( (z > fabsy) || ((z == fabsy) && ((iquo & 1) != 0 )))
		{
			x1 -= fabsy;
			iquo += 1;
		}
	}
	else
	{	// x1 is only this large if y is even bigger, so we can safely divide y by 2
		float halfy = 0.5f * fabsy;
		if( (x1 > halfy) || ((x1 == halfy) & ((iquo & 1) != 0 )))
		{
			x1 -= fabsy;
			iquo += 1;
		}
	}
	
	if( x < -0.0f )
		x1 = -x1;
	iquo &= 0x7f;
	
	*quo = (iquo ^ sign) - sign; 
	
	return x1;
}

