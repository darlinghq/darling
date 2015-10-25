/*
 *  expf.s
 *
 *      by Ian Ollmann
 *
 *  Copyright (c) 2007, Apple Inc.  All Rights Reserved.
 *
 *  This file implements the C99 expf function for the MacOS X __i386__ and __x86_64__ abis. 
 */

#include <machine/asm.h>
#include "abi.h"

//
//  Overall approach
//
//      We calculate e**(y) as follows:
//
//          e**y = 2**x         x = y / ln(2)
//
//      ...and proceed largely as we do in exp2f():
//
//          i = trunc(x)
//          f = x - i
//          
//          2**(x) = 2**(f+i) = 2**f * 2**i
//
//      We choose trunc here, so that 2**f always has the property of pushing 2**i that much closer to the 
//      extrema of finite, non-zero floating point computation. That is:
//
//              x < 1  --> 2**f <= 1
//              x == 1 --> 2**f = 1
//              x > 1  --> 2**f >= 1
//
//      This means that our 2**i calculation will not underflow/overflow prematurely before 2**f has had its say.
//      Round to nearest even would produce somewhat more accurate results, but accomplishing something like that 
//      rounding without knowledge of the current rounding mode is expensive.
//      
//      In principle, the 2**f part can further be broken up as:
//
//          2**f = 2**(c+r) = 2**c * 2**r
//
//      Where c is chosen so as to make r as close to 0 as possible and c can be determined fairly exactly by 
//      consulting a lookup table. Our choice of trunc above means we cant sure of the sign of f,
//      so our table ends up being twice as large as it would have if we had chosen floor or ceil. On the bright 
//      side, we never have to worry about f = x - trunc(f) being inexact in single precision.
//
//      Unfortunately, in practice this reduction seemed to be bad for throughput, so this implementation went with
//      a large polynomial instead that covers the range -1,1 or -0.5,0.5 depending on whether we know the rounding
//      mode is nearest or not.
//
//      Most of the arithmetic will be done in double precision. I investigated calculating the fractional term as 2**r-1, so as 
//      to preserve precision -- we expect a lot of zeros between the 1 and the first non-zero bit. Normally, we dont want
//      throw that away until the final operation so that non-default rounding modes arrive at something close to the 
//      right answer. Doing float in double precision means that this is unnecessary however. 
//
//      This code is probably worth revisiting once more to see if we can come up with a cheap reduction. These huge
//      polynomials just arent good for latency.
//
//

.align (1<<4)
// 8th order minimax fit of exp2 on [-1.0,1.0].  |error| < 0.402865722354948566583852e-9:
expf_c:         .quad 0x40bc03f30399c376,   0x3ff000000001ea2a          // c4/c8 = 0.961813690023115610862381719985771e-2 / 0.134107709538786543922336536865157e-5, c0 = 1.0 + 0.278626872016317130037181614004e-10
                .quad 0x408f10e7f73e6d8f,   0x3fe62e42fd0933ee	        // c5/c8 = 0.133318252930790403741964203236548e-2 / 0.134107709538786543922336536865157e-5, c1 = .693147176943623740308984004029708   
                .quad 0x405cb616a9384e69,   0x3fcebfbdfd0f0afa	        // c6/c8 = 0.154016177542147239746127455226575e-3 / 0.134107709538786543922336536865157e-5, c2 = .240226505817268621584559118975830   
                .quad 0x4027173ebd288ba1,   0x3fac6b0a74f15403	        // c7/c8 = 0.154832722143258821052933667742417e-4 / 0.134107709538786543922336536865157e-5, c3 = 0.555041568519883074165425891257052e-1 
                .quad 0x3eb67fe1dc3105ba                                // c8 = 0.134107709538786543922336536865157e-5


.align (1<<3)
expf_nofenv_c:  .quad   0x3ff0000000058fca                          // 1.0 + -8.09329727503262700660348520172e-11
                .double 0.693147206709644041218074094717934
                .double 0.240226515050550309232521176082490
                .double 0.0555032721577134480296332498187050
                .double 0.00961799451418197772157647729048837
                .double 0.00134004316655903280893023589328151
                .double 0.000154780222739739895295319810010147

.literal8
reciprocalLn2:  .quad   0x3ff71547652b82fe                          // 1.0 / ln(2)

.text

#if defined( __x86_64__ )
	#define RELATIVE_ADDR( _a)								(_a)( %rip )
	#define RELATIVE_ADDR_B( _a)							(_a)( %rip )
#elif defined( __i386__ )
	#define RELATIVE_ADDR( _a)								(_a)-expf_body( CX_P )
	#define RELATIVE_ADDR_B( _a)							(_a)-expf_no_fenv_body( CX_P )

//a short routine to get the local address
.align (1<<4)
expf_pic:       movl    (%esp),     %ecx                    //copy address of local_addr to %ecx
                ret
#else
	#error arch not supported
#endif

//Accurate to within 0.5068 ulp in round to nearest. 
ENTRY( expf )
#if defined( __i386__ )
    movl        FRAME_SIZE(STACKP),     %eax
    movss       FRAME_SIZE(STACKP),     %xmm0
#else
    movd        %xmm0,                  %eax
#endif
    
    cvtss2sd    %xmm0,                  %xmm2       // (double) x
    movl        %eax,                   %ecx        // x
    andl        $0x7fffffff,            %eax        // |x|vl
    movl        $1023,                  %edx        // double precision bias
    subl        $0x33b8aa3b,            %eax        // subtract 0x1.0p-24f / ln(2)
    cmpl        $0x0f7d1d57,            %eax        // if( |x| > 126/ln(2) || isnan(x) || |x| < 0x1.0p-26f /ln(2) )
    ja          3f                                  //      goto 3

    //The test above was a little too aggressive. 
    //values 126 < x < 128, and -150 < x < -126 will come back here:
    //For x < -126, we change %edx to hold the exponent for 2**-126, 
    //      and subtract -126 from x

1:
//PIC
#if defined( __i386__ )
    calll       expf_pic                                    // set %ecx to point to local_addr
expf_body:   
#endif

    mulsd       RELATIVE_ADDR( reciprocalLn2 ), %xmm2
    lea         RELATIVE_ADDR( expf_c), CX_P


    //extract fractional part and exit early if it is zero
    cvttsd2si   %xmm2,                  %eax        // trunc(x)
    addl        %eax,                   %edx        // calculate biased exponent of result
    cvtsi2sd    %eax,                   %xmm1       // trunc(x)
    movd        %edx,                   %xmm7       // exponent part of result
    psllq       $52,                    %xmm7       // shift result exponent into place
    subsd       %xmm1,                  %xmm2       // get the fractional part


    
    // c0 + c1x1 + c2x2 + c3x3 + c4x4 + c5x5 + c6x6 + c7x7 + c8x8 
#if defined( __SSE3__ )
    movddup     %xmm2,                  %xmm1       // { x, x }
#else
    movapd      %xmm2,                  %xmm1       // x
    unpcklpd    %xmm1,                  %xmm1       // { x, x }
#endif    
    mulsd       %xmm2,                  %xmm2       // x*x
    movapd      %xmm1,                  %xmm3
    mulpd       48(CX_P),               %xmm1       // { c3x, (c7/c8)x }
    mulpd       16(CX_P),               %xmm3       // { c1x, (c5/c8)x }
#if defined( __SSE3__ )
    movddup     %xmm2,                  %xmm4       // { xx, xx }
#else
    movapd      %xmm2,                  %xmm4       // xx
    unpcklpd    %xmm4,                  %xmm4       // { xx, xx }
#endif
    mulsd       %xmm2,                  %xmm2       // xx*xx
    addpd       32(CX_P),               %xmm1       // { c2 + c3x, (c6/c8) + (c7/c8)x }
    addpd       (CX_P),                 %xmm3       // { c0 + c1x, (c4/c8) + (c5/c8)x }
    mulpd       %xmm4,                  %xmm1       // { c2xx + c3xxx, (c6/c8)xx + (c7/c8)xxx }
    addsd       %xmm2,                  %xmm3       // { c0 + c1x, (c4/c8) + (c5/c8)x + xxxx }
    mulsd       64(CX_P),               %xmm2       // c8 * xxxx
    addpd       %xmm1,                  %xmm3       // { c0 + c1x + c2xx + c3xxx, (c4/c8) + (c5/c8)x + (c6/c8)xx + (c7/c8)xxx + xxxx }
    movhlps     %xmm3,                  %xmm6       // { ?, c0 + c1x + c2xx + c3xxx }
    mulsd       %xmm2,                  %xmm3       // { ..., c8xxxx* ((c4/c8) + (c5/c8)x + (c6/c8)xx + (c7/c8)xxx + xxxx) }
    addsd       %xmm6,                  %xmm3       // c0 + c1x + c2xx + c3xxx + c4xxxx + c5xxxxx + c6xxxxxx + c7xxxxxxx + c8xxxxxxxxx
    mulsd       %xmm7,                  %xmm3       // 2**i * {c0 + c1x + c2xx + c3xxx + c4xxxx + c5xxxxx + c6xxxxxx + c7xxxxxxx + c8xxxxxxxxx}
    
//  convert to single precision and return
    cvtsd2ss    %xmm3,                  %xmm0
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE(STACKP)
    flds        FRAME_SIZE(STACKP)
#endif
    ret
        

3:  // |x| > 126 || isnan(x) || |x| < 0x1.0p-26f
    jge          4f                                  // if( |x| > 126 || isnan(x) )   goto 4

    //fall through for common case of |x| < 0x1.0p-26f
    movl        $0x3f800000,            %edx        // 1.0f
    movd        %edx,                   %xmm1       // 1.0f
    addss       %xmm1,                  %xmm0       // round away from 1.0 as appropriate for current rounding mode
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE(STACKP)
    flds        FRAME_SIZE(STACKP)
#endif
    ret

    // |x| > 126 || isnan(x)
4:  addl        $0x33b8aa3b,            %eax        // restore |x|
    cmpl        $0x7f800000,            %eax        // 
    ja          5f                                  // if isnan(x) goto 5
    je          6f                                  // if isinf(x) goto 6
    
    cmpl        $0xc358677d,            %ecx        // if( x <= -150 /ln(2) )
    jae         7f                                  //      goto 7

    cmpl        $0x4338aa3b,            %ecx        // if( x >= 128 /ln(2) )
    jge         8f                                  //      goto 8

    jmp         1b
    

5:  //x is nan
    addss       %xmm0,                  %xmm0       //Quiet the NaN
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE(STACKP)
    flds        FRAME_SIZE(STACKP)
#endif
    ret
    
6: // |x| is inf
    movss       %xmm0,                  %xmm1       // x
    psrad       $31,                    %xmm0       // x == -Inf ? -1U : 0
    andnps       %xmm1,                 %xmm0       // x == -Inf ? 0 : Inf
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE(STACKP)
    flds        FRAME_SIZE(STACKP)
#endif
    ret
    
7:  // x <= -150
    movl        $0x00800001,            %eax
    movd        %eax,                   %xmm0
    mulss       %xmm0,                  %xmm0       //Create correctly rounded result, set inexact/underflow
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE(STACKP)
    flds        FRAME_SIZE(STACKP)
#endif
    ret
    
8:  // x >= 128.0
    movl        $0x7f7fffff,            %eax        // FLT_MAX
    movd        %eax,                   %xmm0       // FLT_MAX
    addss       %xmm0,                  %xmm0       // Inf, set overflow
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE(STACKP)
    flds        FRAME_SIZE(STACKP)
#endif
    ret

//Uses round to nearest to deliver a reduced value between [ -0.5, 0.5 ]
//This allows us to use a smaller polynomial
//In addition, we dont bother to check for x is integer. This saves a few cycles in the general case.
//Accurate to within 0.534 ulp
ENTRY( expf$fenv_access_off )
#if defined( __i386__ )
    movl        FRAME_SIZE(STACKP),     %eax
    movss       FRAME_SIZE(STACKP),     %xmm0
#else
    movd        %xmm0,                  %eax
#endif
    
    cvtss2sd    %xmm0,                  %xmm2       // (double) x
    movl        %eax,                   %ecx        // x
    andl        $0x7fffffff,            %eax        // |x|vl
    movl        $1023,                  %edx        // double precision bias
    subl        $0x33b8aa3b,            %eax        // subtract 0x1.0p-24f / ln(2)
    cmpl        $0x0f7d1d57,            %eax        // if( |x| > 126/ln(2) || isnan(x) || |x| < 0x1.0p-26f /ln(2) )
    ja          3f                                  //      goto 3

    //The test above was a little too aggressive. 
    //values 126 < x < 128, and -150 < x < -126 will come back here:
    //For x < -126, we change %edx to hold the exponent for 2**-126, 
    //      and subtract -126 from x

1:
//PIC
#if defined( __i386__ )
    calll       expf_pic                                    // set %ecx to point to local_addr
expf_no_fenv_body:   
#endif

    mulsd       RELATIVE_ADDR( reciprocalLn2 ), %xmm2
    lea         RELATIVE_ADDR_B( expf_nofenv_c), CX_P

    //extract fractional part and exit early if it is zero
    andl        $0x80000000,            %ecx        // signof(x)
    orl         $0x43300000,            %ecx        // copysign( 0x1.0p52, x ) >> 32
    movd        %ecx,                   %xmm6       // copysign( 0x1.0p52, x ) >> 32
    movaps      %xmm2,                  %xmm1       // x
    psllq       $32,                    %xmm6       // copysign( 0x1.0p52, x )
    addsd       %xmm6,                  %xmm2       // x + copysign( 0x1.0p23f, x )
    subsd       %xmm6,                  %xmm2       // rint(x)
    cvttsd2si   %xmm2,                  %eax        // rint(x)
    addl        %eax,                   %edx        // rint(x) + bias
    movd        %edx,                   %xmm7       // result exponent >> 32
    subsd       %xmm2,                  %xmm1       // fractional part
    psllq       $52,                    %xmm7       // shift result exponent into place

    movsd       (3*8)(CX_P),            %xmm3       // c3
    movsd       (5*8)(CX_P),            %xmm5       // c5
    movsd       (6*8)(CX_P),            %xmm6       // c6
    movapd      %xmm1,                  %xmm2       // x
    mulsd       %xmm1,                  %xmm1       // x*x
    mulsd       %xmm2,                  %xmm3       // c3*x
    mulsd       %xmm2,                  %xmm5       // c5*x
    mulsd       (1*8)(CX_P),            %xmm2       // c1*x
    mulsd       %xmm1,                  %xmm6       // c6*xx
    addsd       (2*8)(CX_P),            %xmm3       // c2+c3x
    movapd      %xmm1,                  %xmm4       // xx
    mulsd       %xmm1,                  %xmm1       // xx*xx
    addsd       (4*8)(CX_P),            %xmm5       // c4+c5x
    addsd       (CX_P),                 %xmm2       // c0+c1x
    mulsd       %xmm4,                  %xmm3       // c2xx+c3xxx
    addsd       %xmm6,                  %xmm5       // c4+c5x+c6xx
    mulsd       %xmm1,                  %xmm5       // c4xxxx+c5xxxxx+c6xxxxxx
    addsd       %xmm2,                  %xmm3       // c0+c1x+c2xx+c3xxx
    addsd       %xmm5,                  %xmm3       // c0+c1x+c2xx+c3xxx+c4xxxx+c5xxxxx+c6xxxxxx

    mulsd       %xmm7,                  %xmm3       // scale by 2**i
    
//  convert to single precision and return
    cvtsd2ss    %xmm3,                  %xmm0
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE(STACKP)
    flds        FRAME_SIZE(STACKP)
#endif
    ret
        

3:  // |x| > 126 || isnan(x) || |x| < 0x1.0p-26f
    jge          4f                                  // if( |x| > 126 || isnan(x) )   goto 4

    //fall through for common case of |x| < 0x1.0p-26f
    movl        $0x3f800000,            %edx        // 1.0f
    movd        %edx,                   %xmm1       // 1.0f
    addss       %xmm1,                  %xmm0       // round away from 1.0 as appropriate for current rounding mode
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE(STACKP)
    flds        FRAME_SIZE(STACKP)
#endif
    ret

    // |x| > 126 || isnan(x)
4:  addl        $0x33b8aa3b,            %eax        // restore |x|
    cmpl        $0x7f800000,            %eax        // 
    ja          5f                                  // if isnan(x) goto 5
    je          6f                                  // if isinf(x) goto 6
    
    cmpl        $0xc358677d,            %ecx        // if( x <= -150 /ln(2) )
    jae         7f                                  //      goto 7

    cmpl        $0x4338aa3b,            %ecx        // if( x >= 128 /ln(2) )
    jge         8f                                  //      goto 8

    jmp         1b
    

5:  //x is nan
    addss       %xmm0,                  %xmm0       //Quiet the NaN
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE(STACKP)
    flds        FRAME_SIZE(STACKP)
#endif
    ret
    
6: // |x| is inf
    movss       %xmm0,                  %xmm1       // x
    psrad       $31,                    %xmm0       // x == -Inf ? -1U : 0
    andnps       %xmm1,                 %xmm0       // x == -Inf ? 0 : Inf
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE(STACKP)
    flds        FRAME_SIZE(STACKP)
#endif
    ret
    
7:  // x <= -150
    movl        $0x00800001,            %eax
    movd        %eax,                   %xmm0
    mulss       %xmm0,                  %xmm0       //Create correctly rounded result, set inexact/underflow
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE(STACKP)
    flds        FRAME_SIZE(STACKP)
#endif
    ret
    
8:  // x >= 128.0
    movl        $0x7f800000,            %eax        // Inf
#if defined( __i386__ )
    movl        %eax,                   FRAME_SIZE(STACKP)
    flds        FRAME_SIZE(STACKP)
#else
    movd        %eax,                   %xmm0
#endif
    ret

