/*
 *  xmm_sqrt.c
 *  xmmLibm
 *
 *  Created by Ian Ollmann, Ph.D. on 7/14/05.
 *  Copyright © 2005 Apple Computer, Inc. All rights reserved.
 *
 *  Modified by Marin Baron, on 30/11/25
 *  As part of the Darling project.
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
    return __builtin_sqrt( x );
}

float sqrtf( float x )
{
    return __builtin_sqrtf( x );
}


