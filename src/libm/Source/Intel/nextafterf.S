

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


ENTRY( nextafterf )
#if defined( __i386__ )
	movss	FRAME_SIZE( STACKP ),	%xmm0
	movss	4+FRAME_SIZE( STACKP ),	%xmm1
#endif

	ucomiss	%xmm0,					%xmm1		// if( x == y || isnan(x) || isnan(y) )
	je		2f									//		goto 2
	
	xorps	%xmm2,					%xmm2		// 0.0f
	ucomiss %xmm0,					%xmm2		// if( x == 0 )
	je		1f									//		goto 1

	// x != y. x != 0.0f. X and Y are numeric.
	cmpltss	%xmm0,					%xmm1		// y < x ? -1 : 0
	xorps	%xmm2,					%xmm2		// 0.0f
	cmpltss	%xmm0,					%xmm2		// 0.0f < x ? -1 : 0
	xorps	%xmm2,					%xmm1		// move away from zero ? -1 : 0
	paddd	%xmm1,					%xmm1		// move away from zero ? -2 : 0
	pcmpeqb %xmm2,					%xmm2		// -1
	psubd	%xmm2,					%xmm1		// move away from zero ? -1 : 1
	psubd	%xmm1,					%xmm0		// initial stab at result
	
	//check to see if we overflowed, underflowed or did other bad things
	movd	%xmm0,					%eax		// result
	andl	$0x7fffffff,			%eax		// |result|
	subl	$0x00800000,			%eax		// push denormals negative
	cmpl	$0x7f000000,			%eax		// if( |result| < 0x1.0p-126 || |result| == inf || isnan( result ) )
	jae		4f									//		goto 4
	
#if defined( __i386__ )
	movss	%xmm0,					FRAME_SIZE( STACKP )
	flds	FRAME_SIZE( STACKP )
#endif
	ret
	
	
	
1:	// x == 0 and y != x, so result is either 0x80000001 or 0x00000001
	pcmpeqb	%xmm0,					%xmm0		//	-1
	movdqa	%xmm0,					%xmm2		//	-1
	pslld	$31,					%xmm0		//	0x80000000
	andps	%xmm1,					%xmm0		//  signof( y )
	psubd	%xmm2,					%xmm0		//  signof( y ) + 1U
	
	//set inexact and underflow
	movaps	%xmm0,					%xmm1		//   0x80000001 or 0x00000001
	pslld	$23,					%xmm1		//	0x00800000
	orps	%xmm0,					%xmm1		//	0x80800001 or 0x00800001
	mulss	%xmm1,					%xmm1		//	set inexact and underflow
	
#if defined( __i386__ )
	movss	%xmm0,					FRAME_SIZE( STACKP )
	flds	FRAME_SIZE( STACKP )
#endif
	ret
	
	// x == y || isnan(x) || isnan(y)
2:	jp		3f									// if( isnan(x) || isnan(y) ) goto 3

#if defined( __i386__ )
	movss	%xmm1,					FRAME_SIZE( STACKP )
	flds	FRAME_SIZE( STACKP )
#else
    movss   %xmm1,                  %xmm0
#endif
	ret

    // nan
3:	addss	%xmm1,					%xmm0		// either x or y or both are nan, so add the two to silence and move to xmm0
#if defined( __i386__ )
	movss	%xmm0,					FRAME_SIZE( STACKP )
	flds	FRAME_SIZE( STACKP )
#endif
	ret

// |result| < 0x1.0p-126 || |result| == inf || isnan( result )	
4:	je		5f									//	if result is infinite, goto 5
	jg		6f									//	if nan, goto 6

	// denormal, set underflow and inexact
	psrld	$31,					%xmm2		// 1U
	movdqa	%xmm2,					%xmm3		// 1U
	pslld	$23,					%xmm2		// 0x00800000U
	por		%xmm3,					%xmm2		// 0x00800001U
	mulss	%xmm2,					%xmm2		// set inexact and underflow
#if defined( __i386__ )
	movss	%xmm0,					FRAME_SIZE( STACKP )
	flds	FRAME_SIZE( STACKP )
#endif
	ret

//	|result| is infinite
5:	pcmpeqb	%xmm1,					%xmm1		// -1
	paddd	%xmm0,					%xmm1		// copysign( max finite, result )
	movdqa	%xmm1,					%xmm2		// copysign( max finite, result )
	pslld	$7,						%xmm1		// copysign( 1.0f,		result )
	addss	%xmm2,					%xmm1		// set inexact
	addss	%xmm2,					%xmm2		// set overflow
#if defined( __i386__ )
	movss	%xmm0,					FRAME_SIZE( STACKP )
	flds	FRAME_SIZE( STACKP )
#endif
	ret

// NaN results were infinities that became NaNs, push back to infinity	
6:	pcmpeqb	%xmm1,					%xmm1		// -1
	paddd	%xmm1,					%xmm0		
#if defined( __i386__ )
	movss	%xmm0,					FRAME_SIZE( STACKP )
	flds	FRAME_SIZE( STACKP )
#endif
	ret
