#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_madvise
#error "SYS_madvise not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_madvise, madvise, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_posix_madvise
	.set	_posix_madvise, _madvise
#endif

#if defined(__x86_64__)
	.globl	_posix_madvise
	.set	_posix_madvise, _madvise
#endif

#if defined(__ppc__)
	.globl	_posix_madvise
	.set	_posix_madvise, _madvise
#endif

#if defined(__arm__)
	.globl	_posix_madvise
	.set	_posix_madvise, _madvise
#endif

#if defined(__arm64__)
	.globl	_posix_madvise
	.set	_posix_madvise, _madvise
#endif

