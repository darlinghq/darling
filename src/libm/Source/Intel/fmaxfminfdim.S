/*
 *  fmax.s
 *
 *      by Ian Ollmann
 *
 *  Copyright © 2007 Apple Inc. All Rights Reserved.
 */
 
 
#include <machine/asm.h>
#include "abi.h"

#if defined( BUILDING_FOR_CARBONCORE_LEGACY )

    #if defined( __i386__ )

    ENTRY( fmax )
        fldl    8+FRAME_SIZE(STACKP)          //{ y }
        fldl    FRAME_SIZE(STACKP)            //{ x, y }
        fucomi  %st(0), %st(0)                //Check for x is NaN
        fcmovb  %st(1), %st(0)                //{ x or y, y}
        fxch                                  //{ y, x or y }
        fucomi  %st(1), %st(0)                //Check y < x  or y is NaN
        fcmovb  %st(1), %st(0)                //{ max, x or y }
        fstp    %st(1)                        //{ max }
        ret

    ENTRY( fmin )
        fldl    8+FRAME_SIZE(STACKP)          //{ y }
        fldl    FRAME_SIZE(STACKP)            //{ x, y }
        fucomi  %st(0), %st(0)                //Check for x is NaN
        fcmovb  %st(1), %st(0)                //{ x or y, y }
        fucomi  %st(1), %st(0)                //Check x < y  or y is NaN
        fxch                                  //{ y, x or y }
        fcmovb  %st(1), %st(0)                //{ max, x or y }
        fstp    %st(1)                        //{ max }
        ret

	ENTRY( fdim )
		movsd	FRAME_SIZE( STACKP),	%xmm0		// x
		movsd	8+FRAME_SIZE( STACKP),	%xmm1		// y
		movapd	%xmm1,					%xmm2		// x
		ucomisd	%xmm0,					%xmm1		// if( x != x || y != y )
		jp		1f									//		goto 1
		cmpltpd	%xmm0,					%xmm2		//  y < x
		andpd	%xmm2,					%xmm0
		andpd	%xmm2,					%xmm1
	1:	subsd	%xmm1,					%xmm0
		movsd	%xmm0,					FRAME_SIZE(STACKP)
		fldl	FRAME_SIZE(STACKP)
		ret

    #else

    ENTRY( fmax )
        SUBP    $16,    STACKP
        
        //create an array on the stack s[2] = { x, y }
        movsd   %xmm0,  (STACKP)
        movsd   %xmm1,  8(STACKP)

        //test a = x < y 
        xorq    %rax,   %rax
        xorq    %rdx,   %rdx
        ucomisd %xmm1,  %xmm0           
        setb    %al                     // a = x < y || isnan(y) || isnan(x)
        ucomisd %xmm1,  %xmm1           
        setp    %dl                     // isnan(y)
        xorq    %rdx,    %rax           // if (isnan(y)) a = 0

        // return s[a]
        movsd   (STACKP,%rax,8), %xmm0  
        ADDP    $16,    STACKP
        ret

    ENTRY( fmin )
        SUBP    $16,    STACKP
        
        //create an array on the stack s[2] = { x, y }
        movsd   %xmm0,  (STACKP)
        movsd   %xmm1,  8(STACKP)

        //test a = y < x 
        xorq    %rax,   %rax
        xorq    %rdx,   %rdx
        ucomisd %xmm0,  %xmm1           
        setb    %al                     // a = y < x || isnan(y) || isnan(x)
        ucomisd %xmm1,  %xmm1           
        setp    %dl                     // isnan(y)
        xorq    %rdx,    %rax           // if (isnan(y)) a = 0

        // return s[a]
        movsd   (STACKP,%rax,8), %xmm0  
        ADDP    $16,    STACKP
        ret

    ENTRY( fdim )
        //a == ( y < x || isnan( y) || isnan(x) )
        xor     %eax,   %eax
        ucomisd %xmm0,  %xmm1       
        setb    %al                 
        
        // map a = {0,1} to a = { 0, -1ULL }
        movd    %eax,   %xmm2
        pxor    %xmm3,  %xmm3
        psubq   %xmm2,  %xmm3
        
        //mask x and y to zero if ( y > x && ! isnan( y) && ! isnan(x) )
        andpd   %xmm3,  %xmm1
        andpd   %xmm3,  %xmm0
        
        //Do the subtract
        subsd   %xmm1,  %xmm0
        ret

    #endif

#else

    //Not CARBONCORE LEGACY

    #if defined( __i386__ )
    ENTRY( fmaxf )
        flds    4+FRAME_SIZE(STACKP)          //{ y }
        flds    FRAME_SIZE(STACKP)            //{ x, y }
        fucomi  %st(0), %st(0)                //Check for x is NaN
        fcmovb  %st(1), %st(0)                //{ x or y, y}
        fxch                                  //{ y, x or y }
        fucomi  %st(1), %st(0)                //Check y < x  or y is NaN
        fcmovb  %st(1), %st(0)                //{ max, x or y }
        fstp    %st(1)                        //{ max }
        ret

    ENTRY( fminf )
        flds    4+FRAME_SIZE(STACKP)          //{ y }
        flds    FRAME_SIZE(STACKP)            //{ x, y }
        fucomi  %st(0), %st(0)                //Check for x is NaN
        fcmovb  %st(1), %st(0)                //{ x or y, y }
        fucomi  %st(1), %st(0)                //Check x < y  or y is NaN
        fxch                                  //{ y, x or y }
        fcmovb  %st(1), %st(0)                //{ max, x or y }
        fstp    %st(1)                        //{ max }
        ret

    //We have to use xmm here so that the subtraction doesnt take a double rounding
    ENTRY( fdimf )
        movss   FRAME_SIZE(STACKP), %xmm0
        movss   4+FRAME_SIZE(STACKP), %xmm1

        SUBP    $4,     STACKP

        //a == ( y < x || isnan( y) || isnan(x) )
        xor     %eax,   %eax
        ucomiss %xmm0,  %xmm1       
        setb    %al                 
        
        // map a = {0,1} to a = { 0, -1ULL }
        negl    %eax
        movd    %eax,   %xmm2
        
        //mask x and y to zero if ( y > x && ! isnan( y) && ! isnan(x) )
        andps   %xmm2,  %xmm1
        andps   %xmm2,  %xmm0
        
        //Do the subtract
        subss   %xmm1,  %xmm0
        movss   %xmm0,  (STACKP)
        flds    (STACKP)
        
        ADDP    $4,     STACKP
        ret

        
    #else

    ENTRY( fmaxf )
        SUBP    $8,    STACKP
        
        //create an array on the stack s[2] = { x, y }
        movss   %xmm0,  (STACKP)
        movss   %xmm1,  4(STACKP)

        //test a = x < y 
        xorq    %rax,   %rax
        xorq    %rdx,   %rdx
        ucomiss %xmm1,  %xmm0           
        setb    %al                     // a = x < y || isnan(y) || isnan(x)
        ucomiss %xmm1,  %xmm1           
        setp    %dl                     // isnan(y)
        xorq    %rdx,    %rax           // if (isnan(y)) a = 0

        // return s[a]
        movss   (STACKP,%rax,4), %xmm0  
        ADDP    $8,    STACKP
        ret

    ENTRY( fminf )
        SUBP    $8,    STACKP
        
        //create an array on the stack s[2] = { x, y }
        movss   %xmm0,  (STACKP)
        movss   %xmm1,  4(STACKP)

        //test a = y < x 
        xorq    %rax,   %rax
        xorq    %rdx,   %rdx
        ucomiss %xmm0,  %xmm1           
        setb    %al                     // a = y < x || isnan(y) || isnan(x)
        ucomiss %xmm1,  %xmm1           
        setp    %dl                     // isnan(y)
        xorq    %rdx,    %rax           // if (isnan(y)) a = 0

        // return s[a]
        movss   (STACKP,%rax,4), %xmm0  
        ADDP    $8,    STACKP
        ret

    ENTRY( fdimf )
        //a == ( y < x || isnan( y) || isnan(x) )
        xor     %eax,   %eax
        ucomiss %xmm0,  %xmm1       
        setb    %al                 
        
        // map a = {0,1} to a = { 0, -1ULL }
        negl    %eax
        movd    %eax,   %xmm2
        
        //mask x and y to zero if ( y > x && ! isnan( y) && ! isnan(x) )
        andps   %xmm2,  %xmm1
        andps   %xmm2,  %xmm0
        
        //Do the subtract
        subss   %xmm1,  %xmm0
        ret
        
    #endif


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


#endif /*BUILDING_FOR_CARBONCORE_LEGACY*/
