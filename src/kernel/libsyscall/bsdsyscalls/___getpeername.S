#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_getpeername
#error "SYS_getpeername not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___getpeername)
SYSCALL_NONAME(getpeername, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___getpeername, getpeername, 3, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_getpeername$UNIX2003
	.set	_getpeername$UNIX2003, ___getpeername
#endif

#if defined(__x86_64__)
	.globl	_getpeername
	.set	_getpeername, ___getpeername
#endif

#if defined(__ppc__)
	.globl	_getpeername$UNIX2003
	.set	_getpeername$UNIX2003, ___getpeername
#endif

#if defined(__arm__)
	.globl	_getpeername
	.set	_getpeername, ___getpeername
#endif

#if defined(__arm64__)
	.globl	_getpeername
	.set	_getpeername, ___getpeername
#endif

