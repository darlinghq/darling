/*
 *  fdimf.c
 *  cLibm
 *
 *  Created by Ian Ollmann on 6/13/07.
 *  Copyright 2007 Apple Inc. All rights reserved.
 *
 */

#include <math.h>

float fdimf( float x, float y )
{
	float result = 0.0f;

	if( x != x || y != y  )
		return x + y;
	
	if( x > y )
		result = x - y;	

	return result;
}
