

/*
 *	nextafterf.s
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc.  All Rights Reserved.
 *
 *	Implementation of C99 nextafterf for __i386__ and __x86_64__.
 */

#include <machine/asm.h>
#include "abi.h"

#if defined( BUILDING_FOR_CARBONCORE_LEGACY )
ENTRY( nextafterd )
#else
ENTRY( nextafter )
#endif
#if defined( __i386__ )
	movsd	FRAME_SIZE( STACKP ),	%xmm0
	movsd	8+FRAME_SIZE( STACKP ),	%xmm1
#endif

	ucomisd	%xmm0,					%xmm1		// if( x == y || isnan(x) || isnan(y) )
	je		2f									//		goto 2
	
	xorpd	%xmm2,					%xmm2		// 0.0f
	ucomisd %xmm0,					%xmm2		// if( x == 0 )
	je		1f									//		goto 1

	// x != y. x != 0.0f. X and Y are numeric.
	cmpltsd	%xmm0,					%xmm1		// y < x ? -1 : 0
	xorpd	%xmm2,					%xmm2		// 0.0f
	cmpltsd	%xmm0,					%xmm2		// 0.0f < x ? -1 : 0
	xorpd	%xmm2,					%xmm1		// move away from zero ? -1 : 0
	paddq	%xmm1,					%xmm1		// move away from zero ? -2 : 0
	pcmpeqb %xmm2,					%xmm2		// -1
	psubq	%xmm2,					%xmm1		// move away from zero ? -1 : 1
	psubq	%xmm1,					%xmm0		// initial stab at result
	
	//check to see if we overflowed, underflowed or did other bad things
	movapd	%xmm0,					%xmm7
	psrlq	$32,					%xmm7
	movd	%xmm7,					%eax		// result
	andl	$0x7fffffff,			%eax		// |result|
	subl	$0x00100000,			%eax		// push denormals negative
	cmpl	$0x7fe00000,			%eax		// if( |result| < 0x1.0p-126 || |result| == inf || isnan( result ) )
	jae		4f									//		goto 4
	
#if defined( __i386__ )
	movsd	%xmm0,					FRAME_SIZE( STACKP )
	fldl	FRAME_SIZE( STACKP )
#endif
	ret
	
	
	
1:	// x == 0 and y != x, so result is either 0x80000001 or 0x00000001
	pcmpeqb	%xmm0,					%xmm0		//	-1
	movdqa	%xmm0,					%xmm2		//	-1
	psllq	$63,					%xmm0		//	0x8000000000000000
	andpd	%xmm1,					%xmm0		//  signof( y )
	psubq	%xmm2,					%xmm0		//  signof( y ) + 1U
	
	//set inexact and underflow
	movapd	%xmm0,					%xmm1		//  0x8000000000000001 or 0x0000000000000001
	psllq	$52,					%xmm1		//	0x0010000000000000
	orps	%xmm0,					%xmm1		//	0x8010000000000001 or 0x0010000000000001
	mulsd	%xmm1,					%xmm1		//	set inexact and underflow
	
#if defined( __i386__ )
	movsd	%xmm0,					FRAME_SIZE( STACKP )
	fldl	FRAME_SIZE( STACKP )
#endif
	ret
	
	// x == y || isnan(x) || isnan(y)
2:	jp		3f									// if( isnan(x) || isnan(y) ) goto 3

    // x == y, return y
#if defined( __i386__ )
	movsd	%xmm1,					FRAME_SIZE( STACKP )
	fldl	FRAME_SIZE( STACKP )
#else
    movsd   %xmm1,                  %xmm0
#endif
	ret
    
    // nan
3:  addsd	%xmm1,					%xmm0		// either x or y or both are nan, so add the two to silence and move to xmm0
#if defined( __i386__ )
	movsd	%xmm0,					FRAME_SIZE( STACKP )
	fldl	FRAME_SIZE( STACKP )
#endif
	ret

// |result| < 0x1.0p-126 || |result| == inf || isnan( result )	
4:	je		5f									//	if result is infinite, goto 5
	jg		6f									//	if nan, goto 6

	// denormal, set underflow and inexact
	psrlq	$63,					%xmm2		// 1ULL
	movdqa	%xmm2,					%xmm3		// 1ULL
	pslld	$52,					%xmm2		// 0x0010000000000000ULL
	por		%xmm3,					%xmm2		// 0x0010000000000001ULL
	mulss	%xmm2,					%xmm2		// set inexact and underflow
#if defined( __i386__ )
	movsd	%xmm0,					FRAME_SIZE( STACKP )
	fldl	FRAME_SIZE( STACKP )
#endif
	ret

//	|result| is infinite
5:	pcmpeqb	%xmm1,					%xmm1		// -1LL
	paddq	%xmm0,					%xmm1		// copysign( max finite, result )
	movdqa	%xmm1,					%xmm2		// copysign( max finite, result )
	psllq	$10,					%xmm1		// copysign( 1.0f,		result )
	addsd	%xmm2,					%xmm1		// set inexact
	addsd	%xmm2,					%xmm2		// set overflow
#if defined( __i386__ )
	movsd	%xmm0,					FRAME_SIZE( STACKP )
	fldl	FRAME_SIZE( STACKP )
#endif
	ret

// NaN results were infinities that became NaNs, push back to infinity	
6:	pcmpeqb	%xmm1,					%xmm1		// -1
	paddq	%xmm1,					%xmm0		
#if defined( __i386__ )
	movsd	%xmm0,					FRAME_SIZE( STACKP )
	fldl	FRAME_SIZE( STACKP )
#endif
	ret
