#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_fstatat64
#error "SYS_fstatat64 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_fstatat64, fstatat64, 4, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_fstatat$INODE64
	.set	_fstatat$INODE64, _fstatat64
#endif

#if defined(__x86_64__)
	.globl	_fstatat$INODE64
	.set	_fstatat$INODE64, _fstatat64
#endif

#if defined(__ppc__)
	.globl	_fstatat$INODE64
	.set	_fstatat$INODE64, _fstatat64
#endif

#if defined(__arm__)
	.globl	_fstatat
	.set	_fstatat, _fstatat64
#endif

#if defined(__arm64__)
	.globl	_fstatat
	.set	_fstatat, _fstatat64
#endif

