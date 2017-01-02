#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_getfsstat64
#error "SYS_getfsstat64 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(_getfsstat64, getfsstat64, 3, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	_getfsstat$INODE64
	.set	_getfsstat$INODE64, _getfsstat64
#endif

