/*
 *  ilogb.s
 *
 *      by Ian Ollmann
 *
 *  Copyright © 2007 Apple Inc. All Rights Reserved.
 */
 
#include <machine/asm.h>
#include "abi.h"

ENTRY( ilogbf )
//move argument to eax
#if defined( __i386__ )
    movl    FRAME_SIZE(STACKP), %eax
#else
    movd    %xmm0,  %eax
#endif

    andl    $0x7f800000,        %eax        //expose exponent
    addl    $0x00800000,        %eax        //add 1 to exponent, promotes NaN/Inf to 2**31
    cmpl    $0x00800000,        %eax        //test for exceptional case
    jle     1f                              //jump to special code for zeros, NaNs, Infs and denorms
    sarl    $23,                %eax        //move exponent to unit precision
    subl    $128,               %eax        //remove (bias+1)
    ret
    
1:  je  2f                                  //handle zeros/denorms at 2

    //load initial value
#if defined( __i386__ )
    movss   FRAME_SIZE(STACKP), %xmm0
#endif

    cvtss2si    %xmm0,          %eax        //set invalid and produce a 0x80000000
    cmpeqss %xmm0,              %xmm0       // x == x
    movd    %xmm0,              %edx
    addl    %edx,               %eax        // c = |x| == inf ? 0x7fffffff : 0x80000000
    ret
    
2:  //reload initial value
#if defined( __i386__ )
    movl    FRAME_SIZE(STACKP), %edx
    movss   FRAME_SIZE(STACKP), %xmm0
#else
    movd    %xmm0,  %edx
#endif

    //set invalid for zero
    xorps       %xmm1,  %xmm1
    cmpeqss     %xmm1,  %xmm0
    cvtss2si    %xmm0,  %ecx

    andl    $0x7fffffff,        %edx        //take absolute value
    bsrl    %edx,               %eax        //get positition of leading set bit
    subl    $(126+23),          %eax        //subtract out bias for denorm
    cmp     $0,                 %edx
    cmove   %ecx,               %eax
    ret
    
ENTRY( ilogb )
//move high 32-bits of argument to eax
#if defined( __i386__ )
    //We take this roundabout route to avoid a store forwarding stall, which costs us 8 cycles on Yonah.
    movq    FRAME_SIZE(STACKP), %xmm0
    psrlq   $32,    %xmm0
    movd    %xmm0,  %eax
#else
    movd    %xmm0,  %rax
    shrq    $32,    %rax
#endif

    andl    $0x7ff00000,        %eax        //expose exponent
    addl    $0x00100000,        %eax        //add 1 to exponent, promotes NaN/Inf to 2**31
    cmpl    $0x00100000,        %eax        //test for exceptional cases
    jle     1f                              //jump to special code for zeros, NaNs, Infs and denorms
    sarl    $20,                %eax        //move exponent to unit precision
    subl    $1024,              %eax        //remove (bias+1)
    ret

1:  je      2f                              //handle zeros/denorms at 2

    //reload initial value
#if defined( __i386__ )
    movsd   FRAME_SIZE(STACKP), %xmm0
#endif
    cvtsd2si    %xmm0,          %eax        //set invalid and produce a 0x80000000
    cmpeqsd %xmm0,              %xmm0       // x == x
    movd    %xmm0,              %edx
    addl    %edx,               %eax        // c = |x| == inf ? 0x7fffffff : 0x80000000
    ret
    
2: // zero/denorm case 
    //reload initial value
#if defined( __i386__ )
    movsd   FRAME_SIZE(STACKP), %xmm0
    movl    4+FRAME_SIZE(STACKP), %edx
#else
    movd    %xmm0,  %rdx
#endif
    xorpd   %xmm1,  %xmm1
    cmpeqsd   %xmm1,  %xmm0
    cvtsd2si  %xmm0, %ecx       //set invalid for zero, and produce ILOGB_ZERO. Denorms produce 0 here.
    
#if defined( __i386__ )
    andl    $0x7fffffff,    %edx    //take absolute value
    movl    $32,            %eax
    cmp     $0,             %edx
    cmove   %edx,           %eax
    cmove   FRAME_SIZE(STACKP), %edx
    bsrl    %edx,           %edx
    subl    $(1022+52),     %eax
    addl    %edx,           %eax
    cmp     $0,             %ecx
    cmovne  %ecx,           %eax
    ret

#else
    //take absolute value
    salq    $1,     %rdx
    shrq    $1,     %rdx
    bsrq    %rdx,   %rax
    subl    $(1022+52), %eax
    cmp     $0,     %rdx
    cmove   %ecx,   %eax
    ret
#endif

ENTRY( ilogbl )
    movw    8+FRAME_SIZE( STACKP ),     %ax
    andw    $0x7fff,                    %ax
    addw    $1,                         %ax
    cmpw    $1,                         %ax
    jle     1f
    andl    $0xffff,                    %eax
    subl    $16384,                     %eax
    ret
    
1:  je      2f
    
    fldt    FRAME_SIZE(STACKP)          // {x}
    pcmpeqb %xmm0,      %xmm0           // -1
    xor     %eax,		%eax
    cvtss2si %xmm0,     %edx            // set invalid
    fucomip  %st(0), %st(0)             // test for NaN
    setp    %al                         // isnan(x)
    addl    $0x7fffffff,   %eax         // isnan(x) ? 0x80000000 : 0x7fffffff
    ret


2: //zeros / denorms
    fildll  FRAME_SIZE(STACKP)          // load in 0/denormal mantissa as integer
    SUBP    $(32-FRAME_SIZE), STACKP
    fstpt   (STACKP)                    // write it back out
    xorl    %eax,           %eax
    movw    8(STACKP),      %ax
    subl    $(16382+63+16383), %eax
    movl    $0x80000000,    %edx
    cmp     $(-16382+-63+-16383), %eax
    cmove   %edx,           %eax
    ADDP    $(32-FRAME_SIZE), STACKP

    //set invalid if necessary
    xorps   %xmm1,          %xmm1
    movd    %eax,           %xmm0
    cmpeqss %xmm1,          %xmm0
    cvtss2si    %xmm0,      %edx
    ret


ENTRY( logbf )
    //move argument to eax
#if defined( __i386__ )
    movl    FRAME_SIZE(STACKP), %eax
#else
    movd    %xmm0,  %eax
#endif

    andl    $0x7f800000,        %eax        //expose exponent
    addl    $0x00800000,        %eax        //add 1 to exponent, promotes NaN/Inf to 2**31
    cmpl    $0x00800000,        %eax        //test for exceptional case
    jle     1f                              //jump to special code for zeros, NaNs, Infs and denorms
    sarl    $23,                %eax        //move exponent to unit precision
    subl    $128,               %eax        //remove (bias+1)

#if defined( __i386__ )    
    SUBP    $4, STACKP
    movl    %eax,   (STACKP)
    fildl   (STACKP)
    ADDP    $4, STACKP
#else
    cvtsi2ss   %eax,           %xmm0
#endif
    ret
    
1:  je  2f                                  //handle zeros/denorms at 2

    //Infinity and NaN just return |x|
#if defined( __i386__ )
    flds    FRAME_SIZE(STACKP)
    fabs
#else
    movaps  %xmm0,  %xmm1
    cmpeqps %xmm0, %xmm0
    pslld   $31,    %xmm0
    andnps  %xmm1,  %xmm0
#endif
    ret
    
2:  
    //load |x| as if it was an int (equivalent to multiplying denorm by 2**(23+126))
#if defined( __i386__ )
    movl    FRAME_SIZE(STACKP),     %edx
#else
    movd    %xmm0,                  %edx
#endif
    andl    $0x7fffffff,            %edx
    cvtsi2ss  %edx,                 %xmm1       //read the denorm/0 in as an integer

    //move a = x==0 ? 1.0f: 0.0f to xmm
    movl    $0x3f800000,            %eax
    xorl    %ecx,                   %ecx
    cmp     $0,                     %edx
    cmovne  %ecx,                   %eax
    movd    %eax,                   %xmm0       //load x == 0 ? 1.0f : 0.0f     (0.0f is to avoid inexact flag on denorms)

    // do isdenorm(x) ? 0.0f / denorm*2**(23+126) : 1.0 / 0
    divss   %xmm1,                  %xmm0       //set div/0 flag if x is zero

    //extract the exponent of the scaled denorm
    movd    %xmm1,                  %eax        //move the bits back to gpr, formatted as float now
    andl    $0x7f800000,            %eax        //extract exponent
    sarl    $23,                    %eax        //reduce to unit precision
    subl    $(126+23+127),          %eax        //correct for new bias after int->float conversion

    //convert to float
    cvtsi2ss %eax,                  %xmm0       //logb requires its result in floating point format, so convert the unbiased exponent to float
    movd    %xmm0,                  %edx        //and bring it back

    //if( x == 0 ) return -inf
    movl    $0xff800000,            %ecx
    cmp     $(-126-23-127),         %eax
    cmove   %ecx,                   %edx

#if defined( __i386__ )
    SUBP    $4, STACKP
    movl    %edx,  (STACKP)
    flds    (STACKP)
    ADDP    $4, STACKP
#else
    movd    %edx,                   %xmm0
#endif
    ret

ENTRY( logb )
    //move argument to eax
#if defined( __i386__ )
    //We take this roundabout route to avoid a store forwarding stall, which costs us 8 cycles on Yonah.
    movq    FRAME_SIZE(STACKP), %xmm0
    psrlq   $32,    %xmm0
    movd    %xmm0,  %eax
#else
    movd    %xmm0,  %rax
    shrq    $32,    %rax
#endif

    andl    $0x7ff00000,        %eax        //expose exponent
    addl    $0x00100000,        %eax        //add 1 to exponent, promotes NaN/Inf to 2**31
    cmpl    $0x00100000,        %eax        //test for exceptional case
    jle     1f                              //jump to special code for zeros, NaNs, Infs and denorms
    sarl    $20,                %eax        //move exponent to unit precision
    subl    $1024,              %eax        //remove (bias+1)

#if defined( __i386__ )    
    SUBP    $4, STACKP
    movl    %eax,   (STACKP)
    fildl   (STACKP)
    ADDP    $4, STACKP
#else
    cvtsi2sd    %eax,          %xmm0
#endif
    ret
    
1:  je  2f                                  //handle zeros/denorms at 2

    //Infinity and NaN just return |x|
#if defined( __i386__ )
    fldl    FRAME_SIZE(STACKP)
    fabs
#else
    movapd  %xmm0,  %xmm1
    cmpeqpd %xmm0, %xmm0
    psllq   $63,    %xmm0
    andnpd  %xmm1,  %xmm0
#endif
    ret
    
2:  
    SUBP    $(16 - FRAME_SIZE),     STACKP

#if defined( __i386__ )
    //take absolute value of x
    movq    (16-FRAME_SIZE+FRAME_SIZE)(STACKP),             %xmm0
    pcmpeqb %xmm1,                  %xmm1
    psllq   $63,                    %xmm1
    pandn   %xmm0,                  %xmm1
    movq    %xmm1,                  (STACKP)
    
    //convert to double (interpreting bits as int -- same a multiplying denormal by 2**(1022+52))
    fildll  (STACKP)                // { x * 2**(1022+52) }
    fstpt   (STACKP)
    
    //load in the exponent
    xorl    %eax,                   %eax
    movw    8(STACKP),              %ax
    
    //subtract the bias
    subl    $(1022+52+16383),       %eax
    cmp     $(-1022-52-16383),      %eax
    movl    %eax,                   (STACKP)
    
    //deal with the div/0 flag we need to set for the 0 case
    fld1                            // { 1 }
    fldz                            // { 0, 1 }
    fcmovne     %st(1), %st(0)
    fdivrp      %st(1), %st(0)      // set div/0 if necessary, gives either 1 or Inf
    
    fildl       (STACKP)            //{ denormal result, 1 or Inf }
    fmulp                           //{ result }
    
    ADDP    $(16 - FRAME_SIZE),     STACKP
    ret    
    
#else
    //Fetch original value. 
    movd    %xmm0,                  %rax

	//Check to see if original value is zero
	xorpd	%xmm1,					%xmm1
	cmpneqsd	%xmm0,				%xmm1		// x == 0 ? 0 : -1ULL 

	//take absolute value
    salq    $1,                     %rax
    shrq    $1,                     %rax

    //convert to double (interpreting bits as int -- same a multiplying denormal by 2**(1022+52))
    cvtsi2sd    %rax,               %xmm0

	//change test result from -1 to 1.0
	psllq	$54,					%xmm1		// x == 0 ? 0 : 0xFFC0000000000000 
	psrlq	$2,						%xmm1		// x == 0 ? 0 : 0x3ff0000000000000

    //move exponent to units position
    movd    %xmm0,                  %rax
    shrq    $52,                    %rax

    //subtract out bias
    subq    $(1022+52+1023),		%rax
    
    //convert to double precision
    cvtsi2sd    %eax,               %xmm0

	//divide result by x == 0 ? 0 : 1.0 to get correct zero result and set div/0 if x == 0
	divsd	%xmm1,					%xmm0

    ADDP    $(16 - FRAME_SIZE),     STACKP
    ret
    
#endif


ENTRY( logbl )
    SUBP    $(32-FRAME_SIZE),       STACKP
    
    //load exponent
    xorl    %eax,                   %eax
    movw    40(STACKP),             %ax
    
    //take absolute value
    andl    $0x7fff,                %eax
    
    //add 1 to exponent, pushes NaN/Inf into 2**15 bit
    addl    $1,                     %eax
    
    //check for exceptional conditions
    cmpw    $1,                     %ax
    
    //deal with exceptional conditions elsewhere
    jle     1f
    
    //remove the bias+1
    subl    $(16383+1),             %eax
    
    //convert to long double
    movl    %eax,                   (STACKP)

    fildl   (STACKP)
    
    ADDP    $(32-FRAME_SIZE),       STACKP
    ret

1:  je      2f          //deal with zero / denorm elsewhere

    //load value
    fldt    32(STACKP)              // {x}
    fucomi  %st(0), %st(0)          // isnan(x)
    fld     %st(0)                  // {x, x}
    fabs                            // {|x|, x}
    fcmovne %st(1), %st(0)          // {result, x}
    fstp    %st(1)                  // {result}
    ADDP    $(32-FRAME_SIZE),       STACKP
    ret

2:  //denormal/zero
    fildll  32(STACKP)              // load in 0/denormal mantissa as integer. Fails for pseudodenormals, but they "don't happen".
    fstpt   (STACKP)                // write it back out to stack as floating point
    xorl    %eax,           %eax
    movw    8(STACKP),      %ax     // read the exponent
    subl    $(16382+63+16383), %eax // subtract out the bias
    cmp     $(-16382+-63+-16383), %eax //check for zero

    //convert the exponent to float
    movl    %eax,       (STACKP)
    fildl   (STACKP)                // { exp }

    //set div/0 for zero result
    fld1                            // { 1, exp }
    fldz                            // { 0, 1, exp }
    fcmovne     %st(1), %st(0)
    fdivrp      %st(0), %st(1)      // { 1 or Inf, exp } set div/0 if necessary, gives either 1 or Inf
    fmulp                           // { result }
    
    ADDP    $(32-FRAME_SIZE), STACKP
    ret
    

    

