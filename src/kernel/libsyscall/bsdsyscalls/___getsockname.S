#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_getsockname
#error "SYS_getsockname not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___getsockname, getsockname, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_getsockname$UNIX2003
	.set	_getsockname$UNIX2003, ___getsockname
#endif

#if defined(__x86_64__)
	.globl	_getsockname
	.set	_getsockname, ___getsockname
#endif

#if defined(__ppc__)
	.globl	_getsockname$UNIX2003
	.set	_getsockname$UNIX2003, ___getsockname
#endif

#if defined(__arm__)
	.globl	_getsockname
	.set	_getsockname, ___getsockname
#endif

#if defined(__arm64__)
	.globl	_getsockname
	.set	_getsockname, ___getsockname
#endif

