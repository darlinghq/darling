/*
 *	copysign.s
 *
 *		by Ian Ollmann.
 *
 *	Copyright (c) 2007 Apple Inc., All Rights Reserved.
 *
 *	Implementation of C99 copysign functions for __i386__ and __x86_64__.
 */

#include <machine/asm.h>

#include "abi.h"


ENTRY( copysignf )
#if defined( __i386__ )
    movss   FRAME_SIZE(STACKP),		%xmm0
    movss   4+FRAME_SIZE(STACKP),	%xmm1
#endif
    pcmpeqw     %xmm2,  %xmm2           //0xffffffff
    psrld       $1,		%xmm2           //0x7fffffff
    andps       %xmm2,  %xmm0
    andnps      %xmm1,  %xmm2
    orps        %xmm2,  %xmm0
#if defined( __i386__ )
	movss		%xmm0,	FRAME_SIZE( STACKP )
    flds        FRAME_SIZE(STACKP)
#endif
    ret

ENTRY( copysign )
#if defined( __i386__ )
	movsd		FRAME_SIZE( STACKP ),	%xmm0		//x
	movsd		8+FRAME_SIZE( STACKP ),	%xmm1		//y
#endif
    pcmpeqw     %xmm2,  %xmm2           //0xffffffffffffffff
    psrlq       $1,     %xmm2           //0x7fffffffffffffff
    andpd       %xmm2,  %xmm0
    andnpd      %xmm1,  %xmm2
    orpd        %xmm2,  %xmm0
#if defined( __i386__ )
	movsd		%xmm0,	FRAME_SIZE( STACKP )
    fldl        FRAME_SIZE(STACKP)
#endif
	ret
	
ENTRY( copysignl )
	movzwl		8+FRAME_SIZE( STACKP ),		%eax	// x exp
	movzwl		24+FRAME_SIZE( STACKP ),	%edx	// y exp
	andl		$0x7fff,					%eax	
	andl		$0x8000,					%edx
	orl			%edx,						%eax
	movw		%ax,						8+FRAME_SIZE(STACKP)
	fldt		FRAME_SIZE(STACKP )
	ret

ENTRY( fabsf )
#if defined( __i386__ )
    flds        FIRST_ARG_OFFSET(STACKP)
    fabs
    ret
#else
    pcmpeqw     %xmm1,  %xmm1           //0xffffffff
    psrld       $1,     %xmm1           //0x7fffffff
    andps       %xmm1,  %xmm0
    ret
#endif

ENTRY( fabs )
#if defined( __i386__ )
    fldl        FIRST_ARG_OFFSET(STACKP)
    fabs
    ret
#else
    pcmpeqw     %xmm1,  %xmm1           //0xffffffffffffffff
    psrlq       $1,     %xmm1           //0x7fffffffffffffff
    andpd       %xmm1,  %xmm0
    ret
#endif

ENTRY( fabsl )
    fldt    FIRST_ARG_OFFSET(STACKP)
    fabs
    ret
    
