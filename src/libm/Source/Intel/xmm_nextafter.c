/*
 *  xmm_nextafter.c
 *  xmmLibm
 *
 *  Created by Ian Ollmann on 8/19/05.
 *  Copyright 2005 Apple Computer. All rights reserved.
 *
 */

#include "fenv.h"
#include "xmmLibm_prefix.h"
#include "math.h"


float nexttowardf( float x, long double y )
{

    //must be a x or y is NaN
    if( EXPECT_FALSE( x != x ) )
        return x + x;

    if( EXPECT_TRUE( (long double) x < y ) )
        return nextafterf( x, __builtin_inff() );
    
    if( EXPECT_TRUE( (long double) x > y ) )
        return nextafterf( x, -__builtin_inff() );

    if( EXPECT_TRUE( (long double) x == y ) )
        return y;
        
    return y + y;
}

double nexttoward( double x, long double y )
{

    //must be a x or y is NaN
    if( EXPECT_FALSE( x != x ) )
        return x + x;
    
    if( EXPECT_TRUE( (long double) x < y ) )
        return nextafter( x, __builtin_inff() );
    
    if( EXPECT_TRUE( (long double) x > y ) )
        return nextafter( x, -__builtin_inff() );

    if( EXPECT_TRUE( (long double) x == y ) )
        return y;
        
    return y + y;
}


