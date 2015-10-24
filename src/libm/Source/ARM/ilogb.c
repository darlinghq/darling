/*
 *  ilogb.h
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
#include "required_arithmetic.h"

int ilogb( double x )
{
	union{ double d; uint64_t u;}u = {x};
	
	u.u &= 0x7fffffffffffffffULL;
	int32_t exp = (int32_t) (u.u >> 52);
	
	if( __builtin_expect( (uint32_t) exp - 1U >= 2046, 0 ) )
	{ // +-0, +-denorm, +-inf, NaN
		if( 0.0 == x )
		{
			required_multiply_double( x, __builtin_inf() );	//set invalid
			return FP_ILOGB0;
		}
	
		if( x != x )
		{
			required_multiply_double( 0.0, __builtin_inf() );	//set invalid
			return FP_ILOGBNAN;
		}
	
		if( u.u == 0x7ff0000000000000ULL )
		{
			required_multiply_double( 0.0, x );	//set invalid
			return INT_MAX;
		}
	
		u.u |= 0x3ff0000000000000ULL;
		u.d -= 1.0;
		exp = (int32_t) (u.u >> 52);
		
		return exp - (1023+1022);
	}

	return exp - 1023;
}

#else

int ilogb( double x )
{
	union{ double d; uint64_t u;}u = {x};
	
	u.u &= 0x7fffffffffffffffULL;
	int32_t exp = (int32_t) (u.u >> 52);
	
	if( __builtin_expect( (uint32_t) exp - 1U >= 2046, 0 ) )
	{ // +-0, +-denorm, +-inf, NaN
		if( 0.0 == x )
		{
			return FP_ILOGB0;
		}
		
		if( x != x )
		{
			return FP_ILOGBNAN;
		}
		
		if( u.u == 0x7ff0000000000000ULL )
		{
			return INT_MAX;
		}
		
		u.u |= 0x3ff0000000000000ULL;
		u.d -= 1.0;
		exp = (int32_t) (u.u >> 52);
		
		return exp - (1023+1022);
	}
	
	return exp - 1023;
}

#endif // ARMLIBM_SET_FLAGS

