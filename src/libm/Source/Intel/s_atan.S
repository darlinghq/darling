/*
 * Adapted from original written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 *	by Ian Ollmann, Apple Computer 2006
 */

#include <machine/asm.h>

#include "abi.h"

RCSID("$NetBSD: s_atan.S,v 1.5 2001/06/19 00:26:30 fvdl Exp $")

ENTRY(atanl)
	fldt	FIRST_ARG_OFFSET(STACKP)
	fld1
	fpatan
	ret
