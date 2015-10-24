/*
 *  nearbyintf.c
 *  cLibm
 *
 *  Created by Ian Ollmann on 6/15/07.
 *  Copyright 2007 Apple Inc. All rights reserved.
 *
 *	C99 implementation of nearbyintf()
 */

#include <math.h>

#ifdef ARMLIBM_SET_FLAGS
#include <fenv.h>
#pragma STDC FENV_ACCESS ON

double nearbyint( double x )
{
	fenv_t	oldEnv;
	
	feholdexcept( &oldEnv );
	double result = rint( x );
	fesetenv( &oldEnv );

	return result;
}

#endif // ARMLIBM_SET_FLAGS
