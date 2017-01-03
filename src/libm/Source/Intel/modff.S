
/*
 *  modff.s
 *
 *      by Ian Ollmann
 *
 *  Copyright (c) 2007 Apple Inc. All Rights Reserved.
 */

#include <machine/asm.h>
#include "abi.h"

#if defined( __i386__ )
    #define RESULT_P        %eax
#else
    #define RESULT_P        %rdi
#endif


ENTRY( modff )
#if defined( __i386__ )
    movl    FRAME_SIZE( STACKP ),       %ecx            //load x
#else
	movd	%xmm0,						%ecx
#endif
    movl    %ecx,                       %edx            // x
    andl    $0x7fffffff,                %ecx            // |x|
    xorl    %ecx,                       %edx            // sign of x
    addl    $0x35000000,                %ecx            // add 128-22 to exponent. This forces Inf, NaN, numbers >= 2**23 to be negative
    cmpl    $0x74800000,                %ecx            // values with exponents less than 128-22 are fractional numbers only
    jl      1f                                          // branch for all exceptional conditions
    
    //we presume that the common case is 1.0 <= x < 2**23
#if defined( __i386__ )
    movss   FRAME_SIZE( STACKP ),       %xmm0			// load x
#endif
	movd	%edx,						%xmm2			// load signof( x ) to xmm
    shrl    $23,                        %ecx
    subl    $(127+128-22),              %ecx
    movl    $0xff800000,                %eax
    sarl    %cl,                        %eax
    movd    %eax,                       %xmm1
#if defined( __i386__ )
    movl    4+FRAME_SIZE( STACKP),      RESULT_P
#endif
	andps	%xmm0,						%xmm1			// find trunc(x)
    subss   %xmm1,                      %xmm0			// fract = x - trunc(x)
    orps    %xmm2,                      %xmm0			// fract = copysign( fract, x )
#if defined( __i386__ )
    movss   %xmm0,                      (RESULT_P)
    flds    (RESULT_P)
#endif
    movss   %xmm1,                      (RESULT_P)
    ret
    
1:  jae     2f                                          // Inf, NaN, big numbers go to 2
    // |x| < 1.0
#if defined( __i386__ )
    movl    4+FRAME_SIZE( STACKP),      RESULT_P
#endif
    movl    %edx,                       (RESULT_P)
#if defined( __i386__ )
    flds    FRAME_SIZE( STACKP )
#endif
    ret
    
2:  cmp     $0xb4800000,                %ecx            
    ja      3f                          //do NaNs elsewhere

    // |x| >= 2**23
#if defined( __i386__ )
    movl    4+FRAME_SIZE( STACKP),      RESULT_P
    movl    %edx,                       (RESULT_P)
    flds    (RESULT_P)
    movl    FRAME_SIZE( STACKP ),       %edx
    movl    %edx,                       (RESULT_P)
#else
	movss	%xmm0,						(RESULT_P)
    movd    %edx,                       %xmm0
#endif
    ret
      
3:  //Nan
#if defined( __i386__)
    movl    4+FRAME_SIZE( STACKP),      RESULT_P
    flds    FRAME_SIZE( STACKP )                        // { x }
    fld     %st(0)
    fstp    (RESULT_P)
#else
    movss   %xmm0,                      (RESULT_P)
#endif
    ret

