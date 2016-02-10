
/*
 *	fpclassify.c
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc. All Rights Reserved.
 *
 *	C99 implementation for __fpclassify function (called by FPCLASSIFY macro.)
 */
 
#include "math.h"
#include <stdint.h>

int __fpclassify( double x )
{
	union{ double d; uint64_t u;}u = {x};

	uint32_t	exp = (uint32_t) ( (u.u & 0x7fffffffffffffffULL) >> 52 );

	if( 0 == exp )
	{
		if( u.u & 0x000fffffffffffffULL )
			return FP_SUBNORMAL;
	
		return FP_ZERO;
	}

	if( 0x7ff == exp )
	{	
		if( u.u & 0x000fffffffffffffULL )
			return FP_NAN;

		return FP_INFINITE;
	}

	return FP_NORMAL;
}
