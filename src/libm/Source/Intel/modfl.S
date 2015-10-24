/*
 *  modfl.s
 *
 *      by Ian Ollmann 
 *
 *  Copyright 2007 Apple Inc. All rights reserved.
 *
 */
 
/*
#include <machine/asm.h>
#define LOCAL_STACK_SIZE	12
#include "abi.h"

#if defined( __LP64__ )
	#define DEST_P			%rdi
	#define LOAD_DEST_P
#else
	#define DEST_P			%eax
	#define LOAD_DEST_P		mov			SECOND_ARG_OFFSET(STACKP), DEST_P
#endif

// I tried branch free code here. Alas there were so many special cases, that 2/3 of the code was patchup after the fidll instruction.
// I've moved some special cases { 0, +-inf, NaN} out, which simplifies things quite a bit on any path you care to follow.

ENTRY( modfl )
        SUBP		$LOCAL_STACK_SIZE, STACKP
        
        LOAD_DEST_P
        fldt		FIRST_ARG_OFFSET(STACKP)			// {x}
        fld         %st(0)                              //  { x, x }
        fabs                                            //  { |x|, x }
        fld         %st(0)                              //  { |x|, |x|, x }
        frndint                                         //  { |i|, |x|, x }
        fucomi      %st(1), %st(0)                      //  { |i|, |x|, x }
        jz          1f
        
        //at this point we know we are a denormal or normal non-integer
        //the next step is to truncate the value. We've already rounded it
        //to an int. We just need to make sure that the right rounding direction
        //applied
        fld1                                            //  { 1, |i|, |x|, x }
        fldz                                            //  { 0, 1, |i|, |x|, x }
        fcmovnbe    %st(1), %st(0)                      //  { 0 or 1, 1, |i|, |x|, x }
        fstp        %st(1)                              //  { 0 or 1, |i|, |x|, x }
        fsubrp      %st(1), %st(0)                      //  { |i|, |x|, x }

        //fix the sign
        fld         %st(0)                              //  { |i|, |i|, |x|, x }
        fchs                                            //  { -|i|, |i|, |x|, x }
        fxch        %st(2)                              //  { |x|, |i|, -|i|, x }
        fucomip     %st(3), %st(0)                      //  { |i|, -|i|, x }
        fcmovne     %st(1), %st(0)                      //  { i result, -|i|, x }
        fstp        %st(1)                              //  { i result, x }
        
        //get the fractional part and store the iresult
        fsubr       %st(0), %st(1)                      //  { i result, f result }
        fstpt       (DEST_P)                            //  { -|i|, f result }
        ADDP		$LOCAL_STACK_SIZE, STACKP
        ret
        
1:      //special case entry for NaN, inf and integers, including zero
        fstp        %st(0)                              //  { |x|, x }
        fstp        %st(0)                              //  { x }

        //handle integers, Inf and zero 
        fldz                                            //  { 0, x }
        fadd        %st(0), %st(1)                      //  { 0, x }  silence NaN
        fucomi      %st(1), %st(0)                      //  { 0, x }
        fchs                                            //  { -0, x }
        fldz                                            //  { 0, -0, x }
        fcmovnbe    %st(1), %st(0)                      //  { +-0, -0, x }
        fcmove      %st(2), %st(0)                      //  { f, -0, x }
        fxch        %st(2)                              //  { x, -0, f }
        fstpt       (DEST_P)                            //  { -0, f }
        fstp        %st(0)                              //  { f }
        ADDP		$LOCAL_STACK_SIZE, STACKP
        ret
 */       

#include <machine/asm.h>
#include "abi.h"

#if defined( __i386__ )
    #define RESULT_P        %edx
#else
    #define RESULT_P        %rdi
#endif

ENTRY( modfl )
    xorl    %eax,                   %eax
    movw    8+FRAME_SIZE(STACKP),   %ax         //load sign + exponent of input
    movl    %eax,                   %ecx        //set aside sign + exponent
    andl    $0x7fff,                %eax        // remove sign
    addl    $(16384-62),            %eax
    cmpw    $(16383+16384-62),      %ax
    jl      1f
    
    //common case of 1.0 <= x < 2**64
    movq    FRAME_SIZE( STACKP),    %xmm0
    subl    $(16383+16384-62),      %eax
    movl    $63,                    %edx
    subl    %eax,                   %edx
    movd    %edx,                   %xmm1
#if defined( __i386__ )
    movl    16+FRAME_SIZE( STACKP ),    RESULT_P
#endif
    pcmpeqb %xmm2,                  %xmm2
    psllq   %xmm1,                  %xmm2
    pand    %xmm2,                  %xmm0
    movq    %xmm0,                  (RESULT_P)
    movw    %cx,                    8(RESULT_P)
    fldt    FRAME_SIZE(STACKP)      // { x, 0 }
    fldt    (RESULT_P)              // { truncl(x), x }
    fucomi  %st(1), %st(0)          // { truncl(x), x }
    je      4f          //if x is an integer goto 4
    
    fsubr   %st(0), %st(1)          // { truncl(x), fract }
    fstpt   (RESULT_P)              // { fract }
    ret
        
1:
#if defined( __i386__ )
    movl    16+FRAME_SIZE( STACKP ),    RESULT_P
#endif
    jae     2f                          //Inf, NaN, big numbers go to 2

    // |x| < 1.0
    pxor    %xmm0,                  %xmm0
    movq    %xmm0,                  (RESULT_P)
    andl    $0x8000,                %ecx
    movw    %cx,                    8(RESULT_P)
    fldt    FRAME_SIZE( STACKP )
    ret
    
2:  
    // |x| >= 2**63 or NAN
    fldz                            // { 0 }
    fldt    FRAME_SIZE( STACKP )    // { x, 0 }
    fucomi  %st(1), %st(0)      
    jp      3f                      // do NaNs elsewhere
    fstpt   ( RESULT_P)             // { 0 }
    fchs                            // { -0 }
    fldz                            // { 0, -0 }
    fcmovb  %st(1), %st(0)          // { fract, -0 }
    fstp    %st(1)
    ret

3:  //NaN                           // { x, 0 }
    fld     %st(0)                  // { x, x, 0 }
    fstpt   (RESULT_P)              // { x, 0 }
    fstp    %st(1)                  // { x }s
    ret

4:  //integer                   // { truncl(x), x }
    fstpt   (RESULT_P)          // { x }
    fldz                        // { 0, x }
    fucomi  %st(1), %st(0)
    fchs                        // { -0, x }
    fldz                        // { 0, -0, x }
    fcmovnb %st(1), %st(0)      // { fract, -0, x }
    fstp    %st(2)              // { -0, fract }
    fstp    %st(0)              // { fract }
    ret
