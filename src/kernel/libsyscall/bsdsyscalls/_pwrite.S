#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_pwrite
#error "SYS_pwrite not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_pwrite)
ZERO_EXTEND(2)
SYSCALL_NONAME(pwrite, 4, cerror)
ret
#else
#if defined(__x86_64__) || defined(__arm64__)
__SYSCALL2(_pwrite, pwrite, 4, cerror)
#else
__SYSCALL2(___pwrite, pwrite, 4, cerror)
#endif

#endif

#if defined(__i386__)
	.globl	_pwrite$UNIX2003
	.set	_pwrite$UNIX2003, ___pwrite
#endif

#if defined(__ppc__)
	.globl	_pwrite$UNIX2003
	.set	_pwrite$UNIX2003, ___pwrite
#endif

