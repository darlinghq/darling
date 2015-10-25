/*	This file makes it possible to compile the nanl implementation for
	64-bit long double by providing a separate source file which Xcode can be
	told to use the -mlong-double-64 switch with.

	Including "nanl.c" defines nanl.  We also define the synonym nanl$LDBL64.
*/

#include "nanl.c"

asm (".text	\n	.globl _nanl$LDBL64	\n	_nanl$LDBL64:	b _nanl");
