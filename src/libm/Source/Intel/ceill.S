/*
 *  ceill.s
 *
 *      by Ian Ollmann
 *
 *  Copyright (c) Apple Inc. 2007. All Rights Reserved.
 *
 *  C99 implementation of ceill for __i386__ and __x86_64__.
 */
 
 
#include <machine/asm.h>
#include "abi.h"


ENTRY( ceill )
    movswl  8+FRAME_SIZE( STACKP ),     %eax        // load signed exponent
    movl    %eax,                       %edx        // sign + exponent
    andl    $0x7fff,                    %eax        // exponent
    subl    $0x3fff,                    %eax        // remove bias
    cmpl    $63,                        %eax        // if( |x| >= 0x1.0p63 || |x| < 1.0 || isnan(x) )
    jae     2f                                      //      goto 2
    
    movl    $63,                        %ecx
    movq    FRAME_SIZE( STACKP ),       %xmm0       // significand
    subl    %eax,                       %ecx        // 63 - exponent
    pcmpeqb %xmm1,                      %xmm1       // -1LL
    movd    %ecx,                       %xmm7       // 63-exponent
    pxor    %xmm2,                      %xmm2       // 0
    psubq   %xmm1,                      %xmm2       // 1
    movdqa  %xmm2,                      %xmm1       // 1
    psllq   %xmm7,                      %xmm2       // one's bit
    psubq   %xmm1,                      %xmm2       // fract mask
    pandn   %xmm0,                      %xmm2
    movq    %xmm2,                      FRAME_SIZE( STACKP )
    pcmpeqd %xmm2,                      %xmm0
    pmovmskb   %xmm0,                   %eax
    cmp     $0xffff,                    %eax
    fldt    FRAME_SIZE( STACKP )                    // result
    je      1f
    
    //need to set the inexact flag and correct rounding
    sarl    $31,                        %edx
    notl    %edx
    andl    $0x3f800000,                %edx
    movl    %edx,                       FRAME_SIZE(STACKP)
    fadds   FRAME_SIZE( STACKP )                                // correct rounding

    // set inexact
    fldpi
    fmul    %st(0), %st(0)
    fstp    %st(0)    
    
1:  ret

// |x| >= 0x1.0p63 || |x| < 1.0 || isnan(x)    
2:  jge     3f
    
//  |x| < 1.0
    fldt    FRAME_SIZE( STACKP )                    // { x }
    fldz                                            // { 0, x }
    fucomip %st(1), %st(0)                          // { x }
    je      1b

    fistpl  FRAME_SIZE( STACKP )
    fldz                                            //  { 0 }
    fchs                                            //  { 1, -0 }
    fld1                                            //  { 1, -0 }
    fcmova  %st(1), %st(0)
    fstp    %st(1)
    ret

// |x| >= 0x1.0p63 || isnan(x)    
3:  fldt    FRAME_SIZE(STACKP)
    fldz
    faddp
    ret
