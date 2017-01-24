#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_bind
#error "SYS_bind not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___bind, bind, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_bind$UNIX2003
	.set	_bind$UNIX2003, ___bind
#endif

#if defined(__x86_64__)
	.globl	_bind
	.set	_bind, ___bind
#endif

