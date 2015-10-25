
/* 
 *	logb.c
 *	
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc. All Rights Reserved. 
 *
 *	Unix 2003 implementation of logb()
 */
 
#include <math.h>
#include <stdint.h>

#ifdef ARMLIBM_SET_FLAGS

#include "required_arithmetic.h"

float logbf( float x )
{
	union{ float f; uint32_t u;}u = {x};
	
	u.u &= 0x7fffffffU;
	int32_t exp = u.u >> 23;
	
	if( __builtin_expect( (uint32_t) exp - 1U >= 254, 0 ) )
	{ // +-0, +-denorm, +-inf, NaN
		if( 0.0f == x )
			return required_divide_float( -1.0f, 0.0f );	//set div/0, return -inf
	
		if( x != x )
			return x + x;
	
		if( u.u == 0x7f800000U )
			return __builtin_inff();
	
		u.u |= 0x3f800000U;
		u.f -= 1.0f;
		exp = u.u >> 23;
		
		return (float) (exp - (127+126));
	}

	return (float) (exp - 127);
}

#else

float logbf( float x )
{
	union{ float f; uint32_t u;}u = {x};
	
	u.u &= 0x7fffffffU;
	int32_t exp = u.u >> 23;
	
	if( __builtin_expect( (uint32_t) exp - 1U >= 254, 0 ) )
	{ // +-0, +-denorm, +-inf, NaN
		if( 0.0f == x )
			return -__builtin_inff();	//set div/0, return -inf
		
		if( x != x )
			return x + x;
		
		if( u.u == 0x7f800000U )
			return __builtin_inff();
		
		u.u |= 0x3f800000U;
		u.f -= 1.0f;
		exp = u.u >> 23;
		
		return (float) (exp - (127+126));
	}
	
	return (float) (exp - 127);
}

#endif
