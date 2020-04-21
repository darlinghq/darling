#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_msgsnd
#error "SYS_msgsnd not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_msgsnd, msgsnd, 4, cerror)
#else
__SYSCALL2(___msgsnd, msgsnd, 4, cerror)
#endif

#if defined(__i386__)
	.globl	_msgsnd$UNIX2003
	.set	_msgsnd$UNIX2003, ___msgsnd
#endif

#if defined(__ppc__)
	.globl	_msgsnd$UNIX2003
	.set	_msgsnd$UNIX2003, ___msgsnd
#endif

