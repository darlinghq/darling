#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_fcntl_nocancel
#error "SYS_fcntl_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___fcntl_nocancel)
SIGN_EXTEND(2)
SYSCALL_NONAME(fcntl_nocancel, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___fcntl_nocancel, fcntl_nocancel, 3, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_fcntl
	.set	_fcntl, ___fcntl_nocancel
	.globl	_fcntl$NOCANCEL$UNIX2003
	.set	_fcntl$NOCANCEL$UNIX2003, ___fcntl_nocancel
#endif

#if defined(__ppc__)
	.globl	_fcntl
	.set	_fcntl, ___fcntl_nocancel
	.globl	_fcntl$NOCANCEL$UNIX2003
	.set	_fcntl$NOCANCEL$UNIX2003, ___fcntl_nocancel
#endif

