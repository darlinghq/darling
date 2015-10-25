/*
 * Written by Ian Ollmann, Copyright (c) Apple Computer, 2006.
 */

#include <machine/asm.h>
#include "abi.h"

ENTRY(atan2l)
	fldt	FIRST_ARG_OFFSET(STACKP)
	fldt	SECOND_ARG_OFFSET(STACKP)
	fpatan
	ret
