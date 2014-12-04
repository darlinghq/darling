
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

double logb( double x )
{
	union{ double d; uint64_t u;}u = {x};
	
	u.u &= 0x7fffffffffffffffULL;
	int32_t exp = (int32_t) (u.u >> 52);
	
	if( __builtin_expect( (uint32_t) exp - 1U >= 2046, 0 ) )
	{ // +-0, +-denorm, +-inf, NaN
		if( 0.0 == x )
			return required_divide_double( -1.0, 0.0 );	//set div/0, return -Inf
	
		if( x != x )
			return x + x;
	
		if( u.u == 0x7ff0000000000000ULL )
			return __builtin_inf();
	
		u.u |= 0x3ff0000000000000ULL;
		u.d -= 1.0;
		exp = (int32_t) (u.u >> 52);
		
		return (double) (exp - (1023+1022));
	}

	return (double) (exp - 1023);
}

#else

double logb( double x )
{
	union{ double d; uint64_t u;}u = {x};
	
	u.u &= 0x7fffffffffffffffULL;
	int32_t exp = (int32_t) (u.u >> 52);
	
	if( __builtin_expect( (uint32_t) exp - 1U >= 2046, 0 ) )
	{ // +-0, +-denorm, +-inf, NaN
		if( 0.0 == x )
			return -__builtin_inf();
		
		if( x != x )
			return x + x;
		
		if( u.u == 0x7ff0000000000000ULL )
			return __builtin_inf();
		
		u.u |= 0x3ff0000000000000ULL;
		u.d -= 1.0;
		exp = (int32_t) (u.u >> 52);
		
		return (double) (exp - (1023+1022));
	}
	
	return (double) (exp - 1023);
}

#endif
