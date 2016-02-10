/*
 *  fdim.c
 *  cLibm
 *
 *  Created by Ian Ollmann on 6/13/07.
 *  Copyright 2007 Apple Inc. All rights reserved.
 *
 */

#include <math.h>

double fdim( double x, double y )
{
	double result = 0.0;

	if( x != x || y != y  )
		return x + y;
	
	if( x > y )
		result = x - y;	

	return result;
}
