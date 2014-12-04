/*
 *  xmm_sqrt.c
 *  xmmLibm
 *
 *  Created by Ian Ollmann, Ph.D. on 7/14/05.
 *  Copyright © 2005 Apple Computer, Inc. All rights reserved.
 *
 *  This set of functions may seem a little silly at first. The compiler can generate
 *  sqrtsd or sqrtsf inline, so why do we need a function? It appears that people can 
 *  make a function pointer to sqrt and call that. Therefore an implementation of sqrt()
 *  should probably exist. We define a trivial one here.
 *  
 */
 
#include "xmmLibm_prefix.h"
#include "math.h"

double sqrt( double x )
{
    xDouble f = DOUBLE_2_XDOUBLE( x );
    f = _MM_SQRT_SD(f);           
    return XDOUBLE_2_DOUBLE( f );
}

float sqrtf( float x )
{
    xFloat f = FLOAT_2_XFLOAT( x );
    f = _mm_sqrt_ss(f);
    return XFLOAT_2_FLOAT( f );
}


