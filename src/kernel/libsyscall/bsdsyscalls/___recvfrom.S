#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_recvfrom
#error "SYS_recvfrom not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___recvfrom)
ZERO_EXTEND(2)
SYSCALL_NONAME(recvfrom, 6, cerror)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___recvfrom, recvfrom, 6, cerror)
#endif

#endif

#if defined(__i386__)
	.globl	_recvfrom$UNIX2003
	.set	_recvfrom$UNIX2003, ___recvfrom
#endif

#if defined(__x86_64__)
	.globl	_recvfrom
	.set	_recvfrom, ___recvfrom
#endif

#if defined(__ppc__)
	.globl	_recvfrom$UNIX2003
	.set	_recvfrom$UNIX2003, ___recvfrom
#endif

#if defined(__arm64__)
	.globl	_recvfrom
	.set	_recvfrom, ___recvfrom
#endif

