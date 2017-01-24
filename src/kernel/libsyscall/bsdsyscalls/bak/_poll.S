#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_poll
#error "SYS_poll not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(_poll, poll, 3, cerror)
#else
__SYSCALL2(___poll, poll, 3, cerror)
#endif

#if defined(__i386__)
	.globl	_poll$UNIX2003
	.set	_poll$UNIX2003, ___poll
#endif

