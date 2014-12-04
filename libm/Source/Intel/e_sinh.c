/*
 * by Ian Ollmann
 * Copyright © 2005 by Apple Computer. All rights reserved.
 *
 *	Algorithm from mathLib v3
 */


#include "math.h"
#include "math_private.h"

long double sinhl( long double x )
{
    static const long double overflow = 11356;      //~ln(2)*16384
    
    long double fabsx = __builtin_fabsl( x );

	if( x != x )						return x + x;
	if( fabsx == __builtin_infl() )		return x;

	if( fabsx > 0x1.0p-32 )		//sqrt( negative epsilon )
	{
		if( fabsx < overflow )
		{
			fabsx = expm1l( fabsx );	
			fabsx = 0.5L * ( fabsx + fabsx / (1.0L + fabsx ) );
		}
		else
		{
			fabsx = expl( 0.5L * fabsx );
			fabsx = ( 0.5L * fabsx ) * fabsx;
		}
	}
	else
	{
		if( x == 0.0L )
			return x;
	
		//set inexact and underflow, if necessary
		fabsx *= 0x1.0p67;
		fabsx += 0x1.0p-16382L;
		fabsx *= 0x1.0p-67;		
	}

	if( x < 0.0 )
		fabsx = -fabsx;
	
	return fabsx;
}
