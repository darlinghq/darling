
/*
 *	scalb.c
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc. All rights reserved. 
 *
 *	Unix 2003 implementation of scalb.
 */
 
 
#include <math.h>
#include <stdint.h>

double scalb( double x, double y )
{
	// Handle y is NaN
	if( y != y )
		return x + y;

	//clamp y
	if( y > 3000.0 )
		y = 3000.0;

	if( y < -3000.0 )
		y = -3000.0;

	return scalbn( x, (int) y );
}
