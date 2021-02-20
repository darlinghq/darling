#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_getfsstat64
#error "SYS_getfsstat64 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_getfsstat64)
SYSCALL_NONAME(getfsstat64, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_getfsstat64, getfsstat64, 3, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_getfsstat$INODE64
	.set	_getfsstat$INODE64, _getfsstat64
#endif

#if defined(__x86_64__)
	.globl	_getfsstat$INODE64
	.set	_getfsstat$INODE64, _getfsstat64
#endif

#if defined(__ppc__)
	.globl	_getfsstat$INODE64
	.set	_getfsstat$INODE64, _getfsstat64
#endif

#if defined(__arm__)
	.globl	_getfsstat
	.set	_getfsstat, _getfsstat64
#endif

#if defined(__arm64__)
	.globl	_getfsstat
	.set	_getfsstat, _getfsstat64
#endif

