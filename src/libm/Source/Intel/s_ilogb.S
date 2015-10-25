/*
 * Written by Ian Ollmann
 *
 * Copyright © 2005, Apple Computer. All rights reserved.
 */

#include <machine/asm.h>

#define LOCAL_STACK_SIZE		4
#include "abi.h"


ENTRY(ilogbl)
    SUBP        $LOCAL_STACK_SIZE,  STACKP
	movl		$0x4f000000, (STACKP)           //  0x1.0p31
    flds        (STACKP)                        //  { 0x1.0p31 }
	fldt		FIRST_ARG_OFFSET(STACKP)        //	{ x, 0x1.0p31 }
	fldz										//	{ 0, x, 0x1.0p31 }
	fucomip		%st(1), %st(0)					//	{ x, 0x1.0p31 }							if( x == 0 or NaN )
	fcmove		%st(1), %st(0)					//	{ x or 0x1.0p31, 0x1.0p31 }				then x = 0x1.0p31			//avoids div/0 for fxtract(0)
	fxtract										//	{ sig, exp, 0x1.0p31 }
	fstp		%st(0)							//	{ exp, 0x1.0p31 }
	fcmove		%st(1), %st(0)					//	{ exp or 0x1.0p31, 0x1.0p31 }		if( x == 0 or NaN )	then exp = 0x1.0p31		//set to overflow
    movl        $0x7fffffff,    %edx            //  load the inf result 
    fucomi      %st(1), %st(0)                  //  compare exp with 0x1.0p31. Only inf inputs will have exponents that test > 0x1.0p31.
    fistpl      (STACKP)                        //  convert exponent to int. Set invalid if input was zero, NaN or Inf
    fstp        %st(0)                          //  { }
    movl        (STACKP),   %eax                //  load result into eax
    cmova       %edx,       %eax                //  patch up Inf result to return 0x7fffffff instead
    ADDP        $LOCAL_STACK_SIZE,  STACKP
    ret

