#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_msgsnd_nocancel
#error "SYS_msgsnd_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___msgsnd_nocancel, msgsnd_nocancel, 4, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_msgsnd
	.set	_msgsnd, ___msgsnd_nocancel
	.globl	_msgsnd$NOCANCEL$UNIX2003
	.set	_msgsnd$NOCANCEL$UNIX2003, ___msgsnd_nocancel
#endif

#if defined(__x86_64__)
	.globl	_msgsnd$NOCANCEL
	.set	_msgsnd$NOCANCEL, ___msgsnd_nocancel
#endif

