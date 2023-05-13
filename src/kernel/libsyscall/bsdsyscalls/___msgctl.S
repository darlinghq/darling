#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_msgctl
#error "SYS_msgctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___msgctl)
SYSCALL_NONAME(msgctl, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___msgctl, msgctl, 3, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_msgctl$UNIX2003
	.set	_msgctl$UNIX2003, ___msgctl
#endif

#if defined(__x86_64__)
	.globl	_msgctl
	.set	_msgctl, ___msgctl
#endif

#if defined(__ppc__)
	.globl	_msgctl$UNIX2003
	.set	_msgctl$UNIX2003, ___msgctl
#endif

#if defined(__arm64__)
	.globl	_msgctl
	.set	_msgctl, ___msgctl
#endif

