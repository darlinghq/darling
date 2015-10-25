
/*
 *	fpclassifyf.c
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc. All Rights Reserved.
 *
 *	C99 implementation for __fpclassifyf function (called by FPCLASSIFY macro.)
 */
 
#include <math.h>
#include <stdint.h>

int __fpclassifyf( float x )
{
	union{ float f; uint32_t u;}u = {x};

	uint32_t	exp = (u.u & 0x7fffffff) >> 23;

	if( 0 == exp )
	{
		if( u.u & 0x007fffffU )
			return FP_SUBNORMAL;
	
		return FP_ZERO;
	}

	if( 0xff == exp )
	{	
		if( u.u & 0x007fffffU )
			return FP_NAN;
			
		return FP_INFINITE;
	}

	return FP_NORMAL;
}
