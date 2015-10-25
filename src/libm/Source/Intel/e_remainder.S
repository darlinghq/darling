/*
 * Adapted from original written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 *	by Ian Ollmann, Apple Computer 2006
 */

#include <machine/asm.h>
#include "abi.h"

ENTRY(remainderl)
	fldt	SECOND_ARG_OFFSET(STACKP)
	fldt	FIRST_ARG_OFFSET(STACKP)
1:	fprem1
	fstsw	%ax
	btw		$10,%ax
	jc		1b
	fstp	%st(1)
	ret
