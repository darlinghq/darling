#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_msync
#error "SYS_msync not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___msync, msync, 3, cerror)
#endif

#if defined(__i386__)
	.globl	_msync$UNIX2003
	.set	_msync$UNIX2003, ___msync
#endif

#if defined(__x86_64__)
	.globl	_msync
	.set	_msync, ___msync
#endif

