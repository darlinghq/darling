#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_stat64
#error "SYS_stat64 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_stat64)
SYSCALL_NONAME(stat64, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_stat64, stat64, 2, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_stat$INODE64
	.set	_stat$INODE64, _stat64
#endif

#if defined(__x86_64__)
	.globl	_stat$INODE64
	.set	_stat$INODE64, _stat64
#endif

#if defined(__ppc__)
	.globl	_stat$INODE64
	.set	_stat$INODE64, _stat64
#endif

#if defined(__arm__)
	.globl	_stat
	.set	_stat, _stat64
#endif

#if defined(__arm64__)
	.globl	_stat
	.set	_stat, _stat64
#endif

