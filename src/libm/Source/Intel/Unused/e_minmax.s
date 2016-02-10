/*
 *  e_minmax.s
 *  LibmV5
 *
 *  Created by Ian Ollmann on 8/19/05.
 *  Copyright 2005 Apple Computer. All rights reserved.
 *
 */

#include <machine/asm.h>
#include "abi.h"

// long double fminl( long double a, long double b )
ENTRY( fminl )                      
    fldt        FIRST_ARG_OFFSET(STACKP)    //  {a}
    fldt        SECOND_ARG_OFFSET(STACKP)   //  {b,a}
    fucomi      %ST(0), %ST					//  test b is NaN
    fcmovu      %ST(1), %ST(0)				// if( b is NaN) b = a
    fucomi      %ST(1), %ST					//  b > a
    fcmovnb     %ST(1), %ST(0)
    fstp        %ST(1)
    ret
    
// long double fmaxl( long double a, long double b )
ENTRY( fmaxl )                      
    fldt        FIRST_ARG_OFFSET(STACKP)    //  {a}
    fldt        SECOND_ARG_OFFSET(STACKP)   //  {b,a}
    fucomi      %ST(0), %ST					//  test b is NaN
    fcmovu      %ST(1), %ST(0)				// if( b is NaN) b = a
    fld         %ST(1)						//  {a, b, a}
    fucomip     %ST(1), %ST					//  a >= b
    fcmovnb     %ST(1), %ST(0)
    fstp        %ST(1)
    ret

// long double fdiml( long double a, long double b )
ENTRY( fdiml )
	fldz									//	{0}
    fldt        FIRST_ARG_OFFSET(STACKP)    //  {a, 0}
    fldt        SECOND_ARG_OFFSET(STACKP)   //  {b, a, 0}
    fucomi      %ST(1), %ST					//  {b, a, 0}      b >= a  and not NaN
	fcmovnb		%ST(2), %ST(0)				//	{b or 0, a, 0 }
	fxch									//	{a, b or 0, 0 }
	fcmovnb		%ST(2), %ST(0)				//	{a or 0, b or 0, 0 }
	fsubp									//	{result, 0 }
	fstp		%ST(1)						//	{result}
	ret
