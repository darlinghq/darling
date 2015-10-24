/*
 *  e_cbrtl.s
 *  LibmV5
 *
 *  Created by Ian Ollmann on 8/27/05.
 *  Copyright 2005 Apple Computer. All rights reserved.
 *
 */

#define	ENTRY(name)							      \
  .globl name;				      			      \
  .align (1<<4);							      	      \
  name##:								      

#if defined( __LP64__ )
	#define LOCAL_STACK_SIZE	20
#else
	#define LOCAL_STACK_SIZE	28
#endif

#include "abi.h"

//.const
.align (1<<4)
onethird:       .long           0xaaaaaaab, 0xaaaaaaaa, 0x00003ffd, 0x00000000                  //(long double) 1.0L/3.0L

.align (1<<3)
correction:     .double         0.62996052494743658238361, 0.79370052598409973737585, 1.0, 1.2599210498948731647672, 1.5874010519681994747517
coeffs:         .double         1.7830491344381518, -1.5730724799776633, 1.2536000054780357, -0.60460822457398278, 0.15834924310704463, -0.017322841453552703

.align (1<<2)
infinity:       .single         +Infinity

// Stack:
//      old ebp
//      old ebx


.text

#if defined( __x86_64__ )
	#define RELATIVE_ADDR( _a)								(_a)( %rip )
#elif defined( __i386__ )
	#define RELATIVE_ADDR( _a)								(_a)-rel_addr( BX_P )
#else
	#error arch not supported
#endif


#if defined( __i386__ )
	//a short routine to get the local address
	local_addr:
		MOVP    (STACKP), BX_P
		ret
#endif


ENTRY( cbrtl )
	#if defined( __LP64__ )
		SUBP	$LOCAL_STACK_SIZE, STACKP
        movl	$0x55555556, 16(STACKP)           //write out fixed point 1/3, align stack to 16 bytes
        movq    $0, 8(STACKP)
        movq    $0, 0(STACKP)
		leaq	(correction + 16)(%rip), %r8
	#else
        pushl   BASEP                           //push ebp
        movl    STACKP, BASEP                   //copy stack pointer to ebp
        pushl   %ebx                            //push ebx
        pushl   $0x55555556                     //write out fixed point 1/3, align stack to 16 bytes
        pushl   $0
        pushl   $0
        pushl   $0
        pushl   $0
        CALLP   local_addr                      //load the address of rel_addr into %ebx
rel_addr:
	#endif

        //load our argument
        fldt            FIRST_ARG_OFFSET(STACKP)        //{x}

        //if( fabs(x) == INF || fabs(x) is NaN )
        //      return x + x
        fld             %ST(0)                          //{x, x}
        fabs                                            //{|x|, x}
        flds            RELATIVE_ADDR(infinity)         //{inf, |x|,    x}
        fucomip         %ST(1), %ST                     //{|x|, x}
        jne             test_zero
        fstp            %ST(0)                          //{x}
        fadd            %ST(0)                          //{x + x}
        jmp             my_cbrtl_exit

test_zero:
        //if( x == 0.0 )
        //      return x;
        fldz                                            //{0, |x|, x}
        fucomip         %ST(1), %ST                     //{|x|, x}
        jne             main_part                       //{|x|, x}
        fstp            %ST(0)                          //{x}
        jmp             my_cbrtl_exit                   //{x}

main_part:                                              //{|x|, x}
        //extract significand and exponent parts
        fxtract                                         //{ |significand|, exponent, x }

        //write out the exponent as an integer
        fxch                                            //{ exponent, |significand|, x }
        fistpl          (STACKP)                        //{ |significand|, x }

        //apply polynomial to significand, store in s,                                                  figure out what the new exponent is
        fld             %ST(0)                          //{ s, |significand|, x }
        fmull           RELATIVE_ADDR(coeffs+5*8)       //{ s*c5, |significand|, x }
        movl            (STACKP), %eax                  //                                              load the exponent
        faddl           RELATIVE_ADDR(coeffs+4*8)       //{ s*c5+c4, |significand|, x }
        imull           16(STACKP)                      //                                              divide the exponent by 3 (multiply by 0x55555556 and take high 32 bits, place in %edx)
        movl            (STACKP), %eax                  //                                              get exponent >> 1
        sarl            $31, %eax
        fmul            %ST(1)                          //{ (c4+c5*s)s, |significand|, x }
        faddl           RELATIVE_ADDR(coeffs+3*8)       //{ c3+(c4+c5*s)s, |significand|, x }
        subl            %eax,   %edx                    //                                              subtract the sign of the exponent (makes our approximation work for neg numbers)
        movl            %edx,   %eax                    //                                              copy exponent/3 to eax       
        fmul            %ST(1)                          //{ (c3+(c4+c5*s)s)s, |significand|, x }
        faddl           RELATIVE_ADDR(coeffs+2*8)       //{ c2+(c3+(c4+c5*s)s)s, |significand|, x }
        imul            $3,     %edx                    //                                              exponent/3 *= 3
        fmul            %ST(1)                          //{ (c2+(c3+(c4+c5*s)s)s)s, |significand|, x }
        faddl           RELATIVE_ADDR(coeffs+1*8)       //{ c1+(c2+(c3+(c4+c5*s)s)s)s, |significand|, x }
        subl            (STACKP), %edx                  //                                              remainder = (exponent/3)*3 - original exponent   (edx)
        fmul            %ST(1)                          //{ (c1+(c2+(c3+(c4+c5*s)s)s)s)s, |significand|, x }
        faddl           RELATIVE_ADDR(coeffs+0*8)       //{ c0+(c1+(c2+(c3+(c4+c5*s)s)s)s)s, |significand|, x }
        neg             %eax                            //                                              exponent = -exponent
        shld            $16, %eax, %eax                 //                                              exponent <<= 16
        andl            $0xFFFF0000, %eax               //                                              mask off the other mantissa bits
        addl            $0x3FFF8000, %eax               //                                              bias the exponent, set the top mantissa bit
        movl            %eax,   6(STACKP)               //                                              write out exponent/3


        //correct for exponent remainder to get our estimate
#if defined( __LP64__ )		
		movslq			%edx,	%rdx									//sign extend edx
		fmull           ( %r8, %rdx, 8 )								//{ e, |significand|, x}
#else
		fmull           correction + 16 - rel_addr( %ebx, %edx, 8 )		//{ e, |significand|, x}
#endif

        //fix up the sign of the estimate
        fld             %ST(0)                          //{ e, e, |significand|, x}
        fchs                                            //{-e, e, |significand|, x}
        fldz                                            //{0, -e, e, |significand|, x}
        fucomip         %ST(4), %ST                     //{-e, e, |significand|, x}     if( 0 < x )
        fcmovb          %ST(1), %ST(0)                  //{+-e, e, |significand|, x} 
        fstp            %ST(1)                          //{+-e, |significand|, x}
        fstp            %ST(1)                          //{+-e, x}

        //apply the appropriate exponent
        fldt            (STACKP)                        //{ new exponent, +-e, x }
        fmulp                                           //{ +-e with correct exponent, x }

        //       e += oneThird * e * (1.0L - x * e * e * e);
        fldt            RELATIVE_ADDR( onethird)        //{0.3333, e, x}
        fld             %ST(1)                          //{ e, 0.3333, e, x }
        fmul            %ST(3)                          //{ x*e, 0.3333, e, x}
        fmul            %ST(2)                          //{ e*x*e, 0.3333, e, x}
        fmul            %ST(2)                          //{ e*e*x*e, 0.3333, e, x}
        fld1                                            //{1.0, e*e*x*e, 0.3333, e, x}
        fsubp                                           //{1.0 - e*e*x*e, 0.3333, e, x}
        fld             %ST(2)                          //{e, 1.0 - e*e*x*e, 0.3333, e, x}
        fmul            %ST(2)                          //{0.3333*e, 1.0 - e*e*x*e, 0.3333, e, x}
        fmulp                                           //{0.3333*e*(1.0 - e*e*x*e), 0.3333, e, x}
        faddp           %ST(0), %ST(2)                  //{0.3333, e+0.3333*e*(1.0 - e*e*x*e), x }

        //       e += oneThird * e * (1.0L - x * e * e * e);
        fld             %ST(1)                          //{ e, 0.3333, e, x }
        fmul            %ST(3)                          //{ x*e, 0.3333, e, x}
        fmul            %ST(2)                          //{ e*x*e, 0.3333, e, x}
        fmul            %ST(2)                          //{ e*e*x*e, 0.3333, e, x}
        fld1                                            //{1.0, e*e*x*e, 0.3333, e, x}
        fsubp                                           //{1.0 - e*e*x*e, 0.3333, e, x}
        fld             %ST(2)                          //{e, 1.0 - e*e*x*e, 0.3333, e, x}
        fmul            %ST(2)                          //{0.3333*e, 1.0 - e*e*x*e, 0.3333, e, x}
        fmulp                                           //{0.3333*e*(1.0 - e*e*x*e), 0.3333, e, x}
        faddp           %ST(0), %ST(2)                  //{0.3333, e+0.3333*e*(1.0 - e*e*x*e), x }


        //       e = (e*x)*e;
        fld             %ST(1)                          //{e, 0.3333, e, x}
        fmul            %ST(3)                          //{e*x, 0.3333, e, x}
        fmulp           %ST(0), %ST(2)                  //{0.3333, (e*x)*e, x}

        //       e -= ( e - (x/(e*e)) ) * oneThird;
        fld             %ST(1)                          //{ e, 0.3333, e, x }
        fmul            %ST(0)                          //{ e*e, 0.3333, e, x}
        fdivr           %ST(3), %ST(0)                  //{ x/(e*e), 0.3333, e, x }
        fsubr           %ST(2), %ST(0)                  //{ e - x/(e*e), 0.3333, e, x }
        fmulp                                           //{ 0.3333*(e - x/(e*e)), e, x }
        fsubrp          %ST(0), %ST(1)                  //{ e - 0.3333*(e - x/(e*e)), x }
        fstp            %ST(1)

my_cbrtl_exit:
	#if defined( __i386__ )
        movl    20(STACKP),       %ebx
        movl    24(STACKP),       BASEP
	#endif
        ADDP    $LOCAL_STACK_SIZE,    STACKP
        ret
