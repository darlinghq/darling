#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_lstat64
#error "SYS_lstat64 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_lstat64)
SYSCALL_NONAME(lstat64, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_lstat64, lstat64, 2, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_lstat$INODE64
	.set	_lstat$INODE64, _lstat64
#endif

#if defined(__x86_64__)
	.globl	_lstat$INODE64
	.set	_lstat$INODE64, _lstat64
#endif

#if defined(__ppc__)
	.globl	_lstat$INODE64
	.set	_lstat$INODE64, _lstat64
#endif

#if defined(__arm__)
	.globl	_lstat
	.set	_lstat, _lstat64
#endif

#if defined(__arm64__)
	.globl	_lstat
	.set	_lstat, _lstat64
#endif

