
//
//	by Ian Ollmann
//
//		based on MathLib v3
//
//	Copyright © 2005, Apple Computer Inc.  All rights reserved.
//


#include <math.h>
#include <xmmLibm_prefix.h>

double acosh( double x )
{
	static const double ln2 = 0x1.62e42fefa39ef358p-1;			//ln(2)

	if( x != x )	return x + x;
	if( x < 1.0 )	
	{
		SET_INVALID_FLAG();
		return __builtin_nan( "" );
	}
	
	double xm1 = x - 1.0;

	if( x < 1.25 )
		return log1p( xm1 + sqrt( xm1 + xm1 + xm1 * xm1 ));
	
	if( x < 0x1.0p27 )		//1/sqrt(negative epsilon )
		return log( x + x - 1.0 / ( x + sqrt( x * x -1.0 )));

	return log( x ) + ln2;
}

long double acoshl( long double x )
{
	static const long double ln2 = 0x1.62e42fefa39ef358p-1L;			//ln(2)

	if( x != x )	return x + x;
	if( x < 1.0L )	
	{
		SET_INVALID_FLAG();
		return __builtin_nanl( "" );
	}
	
	long double xm1 = x - 1.0L;


	if( x < 1.25L )
		return log1pl( xm1 + sqrtl( xm1 + xm1 + xm1 * xm1 ));
	
	if( x < 0x1.0p32 )		//1/sqrt(negative epsilon )
		return logl( x + x - 1.0L / ( x + sqrtl( x * x -1.0L )));

	return logl( x ) + ln2;
}


