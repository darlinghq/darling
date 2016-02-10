/*
 *  math_errhandling.c
 *  armLibm
 *
 *  Created by iano on 11/9/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 *  This is to support the C99 required math_errhandling symbol
 */

#include "math.h"

int __math_errhandling ( void )
{
    return MATH_ERREXCEPT;
}

