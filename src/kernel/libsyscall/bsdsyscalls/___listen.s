#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_listen
#error "SYS_listen not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___listen)
SYSCALL_NONAME(listen, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___listen, listen, 2, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_listen$UNIX2003
	.set	_listen$UNIX2003, ___listen
#endif

#if defined(__x86_64__)
	.globl	_listen
	.set	_listen, ___listen
#endif

#if defined(__ppc__)
	.globl	_listen$UNIX2003
	.set	_listen$UNIX2003, ___listen
#endif

#if defined(__arm__)
	.globl	_listen
	.set	_listen, ___listen
#endif

#if defined(__arm64__)
	.globl	_listen
	.set	_listen, ___listen
#endif

