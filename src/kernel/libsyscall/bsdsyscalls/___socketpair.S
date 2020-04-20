#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_socketpair
#error "SYS_socketpair not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___socketpair, socketpair, 4, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_socketpair$UNIX2003
	.set	_socketpair$UNIX2003, ___socketpair
#endif

#if defined(__x86_64__)
	.globl	_socketpair
	.set	_socketpair, ___socketpair
#endif

#if defined(__ppc__)
	.globl	_socketpair$UNIX2003
	.set	_socketpair$UNIX2003, ___socketpair
#endif

#if defined(__arm__)
	.globl	_socketpair
	.set	_socketpair, ___socketpair
#endif

#if defined(__arm64__)
	.globl	_socketpair
	.set	_socketpair, ___socketpair
#endif

