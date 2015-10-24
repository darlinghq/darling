/*
 * 32-bit version Adapted from original written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 *	by Ian Ollmann, Apple Computer 2006
 *
 * LP64 version by Ian Ollmann
 */

#include <machine/asm.h>
#include "abi.h"

//This is not a API required by any standard that we know of
//It is provided for backwards compatibility
#if defined( __LP64__ )

	ENTRY(significand)
		SUBP	$8, STACKP
		movsd	%xmm0, (STACKP)
		fldl	(STACKP)
		fxtract
		fstp	%st(1)
		fstl	(STACKP)
		movsd	(STACKP), %xmm0
		ADDP	$8, STACKP
		ret

#else

	ENTRY(significand)
		fldl	FIRST_ARG_OFFSET(STACKP)
		fxtract
		fstp	%st(1)
		ret

#endif
