#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_writev
#error "SYS_writev not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_writev)
SYSCALL_NONAME(writev, 3, cerror)
ret
#else
#if defined(__x86_64__) || defined(__arm64__)
__SYSCALL2(_writev, writev, 3, cerror)
#else
__SYSCALL2(___writev, writev, 3, cerror)
#endif

#endif

#if defined(__i386__)
	.globl	_writev$UNIX2003
	.set	_writev$UNIX2003, ___writev
#endif

#if defined(__ppc__)
	.globl	_writev$UNIX2003
	.set	_writev$UNIX2003, ___writev
#endif

