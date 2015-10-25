/*
 *	by Ian Ollmann
 *
 *	based on fdlibm cosh by Sun Microsystems, with improvements for efficiency and proper flag setting on i386. 
 *
 *	Copyright (C) 2005 by Apple Computer, Inc. All rights reserved.
 */



#include "math.h"
#include "math_private.h"

long double coshl( long double x )
{
    static const long double overflow = 0x1.62e9bb80635d81d4p+13L;		 //ln(LDBL_MAX) + ln(2.0)
    static const long double ln2 = 0.693147180559945309417232121458176568L;                             //ln(2)
    long double fabsx = __builtin_fabsl( x );
    long double t, w;

	if( x != x )	return x + x;

    if( fabsx < __builtin_infl() )
    {
        if( fabsx < 0.5L * ln2 )
        {
			if( fabsx > 0x1.0p-1000L )	//avoid underflow, save time
				fabsx = expm1l( fabsx );
            w = 1.0L + fabsx;
            if( fabsx < 0x1.0p-67L )
                return w;
            return 1.0L + (fabsx*fabsx)/(w+w);
        }
        
        if( fabsx < 22.L )
        {
            t =  expl( fabsx );
            return 0.5L * t + 0.5L/t;
        }
        
        if( fabsx < overflow )
        {
            w = expl( 0.5L * fabsx );
            t = 0.5L * w;
            return t * w;
        }
		
		return fabsx * 0x1.0p16383L;
    }

    //Nan or Inf result
    return fabsx + fabsx;
}
