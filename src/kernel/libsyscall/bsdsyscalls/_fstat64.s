#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_fstat64
#error "SYS_fstat64 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_fstat64)
SYSCALL_NONAME(fstat64, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_fstat64, fstat64, 2, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_fstat$INODE64
	.set	_fstat$INODE64, _fstat64
#endif

#if defined(__x86_64__)
	.globl	_fstat$INODE64
	.set	_fstat$INODE64, _fstat64
#endif

#if defined(__ppc__)
	.globl	_fstat$INODE64
	.set	_fstat$INODE64, _fstat64
#endif

#if defined(__arm__)
	.globl	_fstat
	.set	_fstat, _fstat64
#endif

#if defined(__arm64__)
	.globl	_fstat
	.set	_fstat, _fstat64
#endif

