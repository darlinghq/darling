
/*
 * scalblnf.c
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc.  All Rights Reserved. 
 *
 *	C99 impelemtnation of scalblnf().
 */
 
#include <math.h>
#include <stdint.h>

float scalblnf( float x, long i )
{
	if( i > 300 )
		i = 300;
	if( i < -300 )
		i = -300;

	return scalbnf( x, (int) i );
}
