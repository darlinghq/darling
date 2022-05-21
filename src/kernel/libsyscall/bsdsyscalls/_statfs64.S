#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_statfs64
#error "SYS_statfs64 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_statfs64)
SYSCALL_NONAME(statfs64, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_statfs64, statfs64, 2, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_statfs$INODE64
	.set	_statfs$INODE64, _statfs64
#endif

#if defined(__x86_64__)
	.globl	_statfs$INODE64
	.set	_statfs$INODE64, _statfs64
#endif

#if defined(__ppc__)
	.globl	_statfs$INODE64
	.set	_statfs$INODE64, _statfs64
#endif

#if defined(__arm64__)
	.globl	_statfs
	.set	_statfs, _statfs64
#endif

