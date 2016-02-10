/*
 *  copysignf.c
 *  cLibm
 *
 *  Created by Ian Ollmann on 6/13/07.
 *  Copyright 2007 Apple Inc. All rights reserved.
 *
 */

#include <math.h>
#include <stdint.h>

float copysignf( float x, float y )
{
	union{ float f; uint32_t u; }ux, uy;
	
	ux.f = x;
	uy.f = y;
	
	ux.u &= 0x7fffffffU;
	ux.u |= uy.u & 0x80000000U;

	return ux.f;
}
