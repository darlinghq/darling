/*
 * Adapted from original written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 *	by Ian Ollmann, Apple Computer 2006
 */

#include <machine/asm.h>
#include "abi.h"

ENTRY(sqrtl)
	fldt	FIRST_ARG_OFFSET(STACKP)
	fsqrt
	ret
