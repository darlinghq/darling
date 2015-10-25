/*
 * by Ian Ollmann
 * Copyright © 2005, Apple Computer Inc. All rights reserved.
 *
 *	based on MathLib v3
 */
 
#include <math.h>

long double tanhl( long double x )
{
    static const long double overflow = 1.13565234062941445534588410310297337926799095235775e+04L / 2.0L;      //log(0x1.0p16384)
    long double fabsx = __builtin_fabsl( x );
    
	if( x != x )	return x + x;
	
	if( fabsx > 0x1.0p-32 )		//sqrt( negative epsilon )
	{
		if( fabsx < overflow )
		{
			fabsx = expm1l( -2.0L * fabsx );
			fabsx = -fabsx / (2.0L + fabsx );
		}
		else
			fabsx = 1.0L;
	}
	else
	{
		if( x == 0.0 )
			return x;
	
		fabsx *= 0x1.0p65;
		fabsx -= 0x1.0p-16382L;
		fabsx *= 0x1.0p-65;
	}

	if( x < 0 )
		fabsx = -fabsx;
	
	return fabsx;
}
