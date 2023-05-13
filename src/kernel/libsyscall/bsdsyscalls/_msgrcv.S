#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_msgrcv
#error "SYS_msgrcv not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_msgrcv)
SIGN_EXTEND(3)
ZERO_EXTEND(2)
SYSCALL_NONAME(msgrcv, 5, cerror)
ret
#else
#if defined(__x86_64__) || defined(__arm64__)
__SYSCALL2(_msgrcv, msgrcv, 5, cerror)
#else
__SYSCALL2(___msgrcv, msgrcv, 5, cerror)
#endif

#endif

#if defined(__i386__)
	.globl	_msgrcv$UNIX2003
	.set	_msgrcv$UNIX2003, ___msgrcv
#endif

#if defined(__ppc__)
	.globl	_msgrcv$UNIX2003
	.set	_msgrcv$UNIX2003, ___msgrcv
#endif

