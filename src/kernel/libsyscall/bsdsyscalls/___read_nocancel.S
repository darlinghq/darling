#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_read_nocancel
#error "SYS_read_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___read_nocancel, read_nocancel, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_read
	.set	_read, ___read_nocancel
	.globl	_read$NOCANCEL$UNIX2003
	.set	_read$NOCANCEL$UNIX2003, ___read_nocancel
#endif

#if defined(__x86_64__)
	.globl	_read$NOCANCEL
	.set	_read$NOCANCEL, ___read_nocancel
#endif

#if defined(__ppc__)
	.globl	_read
	.set	_read, ___read_nocancel
	.globl	_read$NOCANCEL$UNIX2003
	.set	_read$NOCANCEL$UNIX2003, ___read_nocancel
#endif

#if defined(__arm__)
	.globl	_read$NOCANCEL
	.set	_read$NOCANCEL, ___read_nocancel
#endif

#if defined(__arm64__)
	.globl	_read$NOCANCEL
	.set	_read$NOCANCEL, ___read_nocancel
#endif

