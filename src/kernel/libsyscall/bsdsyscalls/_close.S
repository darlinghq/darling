#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_close
#error "SYS_close not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_close)
SYSCALL_NONAME(close, 1, cerror)
ret
#else
#if defined(__x86_64__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_close, close, 1, cerror)
#else
__SYSCALL2(___close, close, 1, cerror)
#endif

#endif

#if defined(__i386__)
	.globl	_close$UNIX2003
	.set	_close$UNIX2003, ___close
#endif

#if defined(__ppc__)
	.globl	_close$UNIX2003
	.set	_close$UNIX2003, ___close
#endif

