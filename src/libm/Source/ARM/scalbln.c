
/*
 * scalbln.c
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc.  All Rights Reserved. 
 *
 *	C99 impelemtnation of scalbln().
 */
 
#include <math.h>
#include <stdint.h>

double scalbln( double x, long i )
{
	if( i > 3000L )
		i = 3000L;
	if( i < -3000L )
		i = -3000L;

	return scalbn( x, (int) i );
}
