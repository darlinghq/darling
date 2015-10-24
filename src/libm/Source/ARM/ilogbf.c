/*
 *  ilogbf.h
 *  cLibm
 *
 *  Created by Ian Ollmann on 6/13/07.
 *  Copyright 2007 Apple Inc. All rights reserved.
 *
 */

#include <math.h>
#include <stdint.h>
#include <limits.h>

#ifdef ARMLIBM_SET_FLAGS
#include <fenv.h>
#include "required_arithmetic.h"
#pragma STDC FENV_ACCESS ON

int ilogbf( float x )
{
	union{ float f; uint32_t u;}u = {x};
	
	u.u &= 0x7fffffff;
	int32_t exp = u.u >> 23;
	
	if( __builtin_expect( (uint32_t) exp - 1U >= 254, 0 ) )
	{ // +-0, +-denorm, +-inf, NaN
		if( 0.0f == x )
		{
			required_multiply_float( x, __builtin_inff() );	//set invalid
			return FP_ILOGB0;
		}
	
		if( x != x )
		{
			required_multiply_float( 0.0f, __builtin_inff() );	//set invalid
			return FP_ILOGBNAN;
		}
	
		if( u.u == 0x7f800000 )
		{
			required_multiply_float( 0.0f, x );	//set invalid
			return INT_MAX;
		}
	
		u.u |= 0x3f800000U;
		u.f -= 1.0f;
		exp = u.u >> 23;
		
		return exp - (127+126);
	}

	return exp - 127;
}

#else

int ilogbf( float x )
{
	union{ float f; uint32_t u;}u = {x};
	
	u.u &= 0x7fffffff;
	int32_t exp = u.u >> 23;
	
	if( __builtin_expect( (uint32_t) exp - 1U >= 254, 0 ) )
	{ // +-0, +-denorm, +-inf, NaN
		if( 0.0f == x )
		{
			return FP_ILOGB0;
		}
		
		if( x != x )
		{
			return FP_ILOGBNAN;
		}
		
		if( u.u == 0x7f800000 )
		{
			return INT_MAX;
		}
		
		u.u |= 0x3f800000U;
		u.f -= 1.0f;
		exp = u.u >> 23;
		
		return exp - (127+126);
	}
	
	return exp - 127;
}

#endif // ARMLIBM_SET_FLAGS
