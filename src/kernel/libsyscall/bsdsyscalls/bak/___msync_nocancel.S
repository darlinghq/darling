#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_msync_nocancel
#error "SYS_msync_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___msync_nocancel, msync_nocancel, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_msync$NOCANCEL$UNIX2003
	.set	_msync$NOCANCEL$UNIX2003, ___msync_nocancel
#endif

#if defined(__x86_64__)
	.globl	_msync$NOCANCEL
	.set	_msync$NOCANCEL, ___msync_nocancel
#endif

