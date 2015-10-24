/*
 * Adapted from original written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 *	by Ian Ollmann, Apple Computer 2006
 */

#include <machine/asm.h>
#include "abi.h"

PRIVATE_ENTRY(__sinl) //not public. Currently used by single and double precision entry points
ENTRY(sinl)
	fldt	FIRST_ARG_OFFSET(STACKP)
	fsin
	fnstsw	%ax
	andw	$0x400,%ax
	jnz		1f
	ret

1:	fldpi
	fadd	%st(0)
	fxch	%st(1)
2:	fprem1
	fnstsw	%ax
	andw	$0x400,%ax
	jnz		2b
	fstp	%st(1)
	fsin
	ret
