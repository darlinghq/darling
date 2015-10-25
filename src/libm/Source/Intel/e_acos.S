/*
 * Written by Ian Ollmann
 *
 * Copyright © 2005, Apple Computer Inc. All Rights Reserved.
 */

#include <machine/asm.h>

#define LOCAL_STACK_SIZE	4
#include "abi.h"

ENTRY(acosl)
	SUBP		$LOCAL_STACK_SIZE, STACKP
	movl		$0x00800000, (STACKP)		//  0x1.0p-126f
	fldt		FIRST_ARG_OFFSET(STACKP)	//	{x}
	fabs									//	{|x| }

	//clip tiny values to 2**-126 to prevent underflow
	flds		(STACKP)					//	{2**-126, |x| }
	fucomi		%st(1), %st(0)				//
	fcmovb		%st(1), %st(0)				//	{ 2**-126 or |x|, |x| }
	fstp		%st(1)						//	{ 2**-126 or |x| }

	//handle overflow / NaN input
	fld1									//	{1, 2**-126 or |x| }
	fucomi		%st(1), %st(0)
	jb			acosl_nan

	fstp		%st(1)						//	{ 1 }
	fldt		FIRST_ARG_OFFSET(STACKP)	//	{ x, 1 }
	je			acosl_one

	// asin(x) = atan( x / sqrt( 1 - x*x ) )
	fld			%st(0)				//	{ x, x, 1 }
	fsubr		%st(2), %st(0)		//	{ 1-x, x, 1 }
	fsqrt							//	{ sqrt( 1 - x ), x, 1 }
	fxch		%st(2)				//	{ 1, x, sqrt( 1 - x ) }
	faddp							//	{ 1 + x, sqrt( 1 - x ) }
	fsqrt							//	{ sqrt(1 + x), sqrt( 1 - x ) }
	fpatan							//	{ result / 2 }
	fadd		%st(0), %st(0)		//	{ result }
	ADDP		$LOCAL_STACK_SIZE, STACKP
	ret

acosl_one:							//	{ x, 1 }
	fucomip		%st(1), %st(0)		//	{ 1 }
	fldpi							//	{ pi, 1 }
    fld         %st(1)              //  { 1, pi, 1 }
    fsubr       %st(0), %st(2)      //  { 1, pi, 0 }
	fcmove		%st(2), %st(0)		//	{ pi or 0, 0 }
    fadd        %st(1), %st(0)      //  { pi+1, pi, 0 }           //set inexact
	fstp		%st(0)
	fcmove		%st(1), %st(0)		//	{ pi or 0, 0 }
	fstp		%st(1)				//	{ pi or 0 }
	ADDP		$LOCAL_STACK_SIZE, STACKP
	ret

acosl_nan:							//{ 1, |x| }
	fstp		%st(0)				//{ |x| }
	fchs							//{-|x| }
	fsqrt							//{nan}	set invalid flag
	ADDP		$LOCAL_STACK_SIZE, STACKP
	ret

