
/*
 *	lroundf.s
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc.  All Rights Reserved.
 *
 *	C99 lroundf for __i386__ and __x86_64__
 *
 */
 
#include <machine/asm.h>
#include "abi.h"


#if defined( __i386__ )


	ENTRY( llroundf )
		movl	FRAME_SIZE(STACKP),		%eax
		flds	FRAME_SIZE(STACKP)		//  { x }
		fld		%st(0)					//	{ x, x }
		SUBP	$16-FRAME_SIZE,			STACKP

		fistpll	(STACKP)				// { x }, llrint(x), set invalid / inexact if necessary
		movl	%eax,					%edx	// x
		andl	$0x7fffffff,			%eax	// |x|
		xorl	%eax,					%edx	// signof( x )
		cmpl	$0x4b000000,			%eax	// |x| >= 0x1.0p23f or NaN
		jae		1f
		
		fildll	(STACKP)						//	{ llrint(x), x }
		fucomip	%st(1),					%st(0)	//	{ x } x == llrint(x)
		fstp	%st(0)							//	{ }
		je		2f								//		return llrint(x)

		cmpl	$0x3effffff,			%eax	// |x| == 0.5f - 1 ulp
		je		4f								 
		
		// at this point we know that |x| < 0x1.0p23f, so we dont need a full 64-bit conversion, which is good because we really need trunc now.
		orl		$0x3f000000,			%edx	// copysign( 0.5f, x )
		movss	16(STACKP),				%xmm0	// x
		movd	%edx,					%xmm1	// copysign( 0.5f, x )
		addss	%xmm1,					%xmm0	// x + copysign( 0.5f, x )
		cvttss2si %xmm0,				%edx	// result = (int32_t) ( x + copysign( 0.5f, x ))
		movl	%edx,					%eax	// result
		sarl	$31,					%edx	// sign extended result
		ADDP	$16-FRAME_SIZE,			STACKP
		ret

1:		// |x| >= 0x1.0p23f or NaN
		fstp	%st(0)							//	{ }
		cmpl	$0x5f000000,			%eax	// |x| >= 0x1.0p63f
		jae		3f

		// |x| is non-overflowing integer  (NaN ends up here eventually too)
2:		movl	(STACKP),				%eax	// low 32 bits
		movl	4(STACKP),				%edx	// high 32 bits
		ADDP	$16-FRAME_SIZE,			STACKP
		ret

3:		// |x| overflows or is NaN
		cmpl	$0x7f800000,			%eax	// |x| > 0x1.0p63f
		ja		2b

		// |x| overflows
		subl	$1,						%edx	// x < 0 ? 0x7fffffff : -1U
		sarl	$31,					%edx	// x < 0 ? 0 : -1U
		movl	(STACKP),				%eax
		xorl	%edx,					%eax	// x < 0 ? low result : low result ^ -1U
		xorl	4(STACKP),				%edx
		ADDP	$16-FRAME_SIZE,			STACKP
		ret

4:		// |x| == 0.5f - 1 ulp, return 0
		xorl	%eax,					%eax
		xorl	%edx,					%edx
		ADDP	$16-FRAME_SIZE,			STACKP
		ret
		
#define LONG_MIN_f		0x4f000000

	ENTRY( lroundf )
		movl	FRAME_SIZE(STACKP),		%edx
		movss	FRAME_SIZE(STACKP),		%xmm0
#elif defined( __x86_64__ )

#define LONG_MIN_f		0x5f000000


	ENTRY( lroundf )
	ENTRY( llroundf )
		xorq	%rdx,					%rdx
		movd	%xmm0,					%edx		// |x|
#endif

		cvttss2si	%xmm0,				AX_P		// (long) x, set invalid / inexact if necessary
		MOVP	DX_P,					CX_P		// x
		and		$0x7fffffff,			DX_P		// |x|
		XORP	DX_P,					CX_P		// signof( x )
		cmpl	$0x4b000000,			%edx		// |x| >= 0x1.0p23f or NaN
		jae		2f

		cvtsi2ss	AX_P,				%xmm1		// trunc(x)
		ucomiss		%xmm0,				%xmm1		// x == trunc(x)
		je		1f									//		return (long) x

		orl		$0x3f000000,			%ecx		// copysign( 0.5f, x )
		movd	%ecx,					%xmm1		// copysign( 0.5f, x )
		cmpl	$0x3effffff,			%edx		// |x| == 0.5f - 1 ulp
		je		1f									//		return (long) x

		addss	%xmm1,					%xmm0		// x += copysign( 0.5, x )
		cvttss2si %xmm0,				AX_P		// (int) (x + copysign( 0.5, x ) )
	
	1:	ret
			
	2:	// |x| >= 0x1.0p23f or NaN
		cmpl	$0x7f800000,			%edx		// |x| is NaN
		ja		3f
		cmpl	$LONG_MIN_f,			%edx		// |x| < LONG_MIN
		jb		1b
		SUBP	$1,						CX_P		// x < 0 ? 0x7fffffff : -1LL
		sar		$31,					CX_P		// x < 0 ? 0 : -1LL
		XORP	CX_P,					AX_P		// flip LONG_LONG_MIN to LONG_LONG_MAX if needed
	3:	ret
	

