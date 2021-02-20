#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_fstatfs64
#error "SYS_fstatfs64 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_fstatfs64)
SYSCALL_NONAME(fstatfs64, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_fstatfs64, fstatfs64, 2, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_fstatfs$INODE64
	.set	_fstatfs$INODE64, _fstatfs64
#endif

#if defined(__x86_64__)
	.globl	_fstatfs$INODE64
	.set	_fstatfs$INODE64, _fstatfs64
#endif

#if defined(__ppc__)
	.globl	_fstatfs$INODE64
	.set	_fstatfs$INODE64, _fstatfs64
#endif

#if defined(__arm__)
	.globl	_fstatfs
	.set	_fstatfs, _fstatfs64
#endif

#if defined(__arm64__)
	.globl	_fstatfs
	.set	_fstatfs, _fstatfs64
#endif

