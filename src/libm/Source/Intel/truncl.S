/*
 *  truncl.s
 *  LibmV5
 *
 *  Created by Ian Ollmann on 9/1/05.
 *  Copyright 2005 Apple Computer. All rights reserved.
 *
 */

#include "machine/asm.h"

#define LOCAL_STACK_SIZE 12
#include "abi.h"

#if defined( __i386__ )

ENTRY(truncl)
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
    fldz                                            //  { 0, 0 }
    fchs                                            //  { -0, 0 }
    fcmovb  %st(1), %st(0)
    fstp    %st(1)
    ret

// |x| >= 0x1.0p63 || isnan(x)    
3:  fldt    FRAME_SIZE(STACKP)
    fldz
    faddp
    ret

#elif defined( __x86_64__ )

ENTRY(truncl)
    fldt    FRAME_SIZE( STACKP )				// { x }
    movzwl  8+FRAME_SIZE( STACKP ),     %eax
	movswl	%cx,						%edx
    andl    $0x7fff,                    %eax
	subl	$0x3fff,					%eax	// push |x| < 1.0L negative
    cmpl    $63,						%eax
    jae     1f
    
#if defined( __SSE3__ )    
    fisttpll FRAME_SIZE( STACKP )
    fildll   FRAME_SIZE( STACKP )
#else
    fnstcw  FRAME_SIZE( STACKP )
    movw    FRAME_SIZE( STACKP ),       %cx
    movw    %cx,                        %dx
    orw     $0xc00,                     %cx
    movw    %cx,                        FRAME_SIZE( STACKP )
    fldcw   FRAME_SIZE( STACKP )
    frndint
    movw    %dx,                        FRAME_SIZE( STACKP )
    fldcw   FRAME_SIZE( STACKP )    
#endif
	ret

1:	// |x| < 1.0L  || |x| > 0x1.0p63L || isnan( x )
	jge		2f

	// |x| < 1.0L, return 0 of same sign
	fistpl	FRAME_SIZE( STACKP )				// set inexact if necessary
	andl	$0x80000000,				%edx
	movl	%edx,		FRAME_SIZE( STACKP )
	flds	FRAME_SIZE( STACKP )

2:	ret

#else
    #error unknown arch
#endif
