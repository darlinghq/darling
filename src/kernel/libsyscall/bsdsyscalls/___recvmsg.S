#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_recvmsg
#error "SYS_recvmsg not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___recvmsg, recvmsg, 3, cerror)
#endif

#if defined(__i386__)
	.globl	_recvmsg$UNIX2003
	.set	_recvmsg$UNIX2003, ___recvmsg
#endif

#if defined(__x86_64__)
	.globl	_recvmsg
	.set	_recvmsg, ___recvmsg
#endif

#if defined(__ppc__)
	.globl	_recvmsg$UNIX2003
	.set	_recvmsg$UNIX2003, ___recvmsg
#endif

#if defined(__arm__)
	.globl	_recvmsg
	.set	_recvmsg, ___recvmsg
#endif

#if defined(__arm64__)
	.globl	_recvmsg
	.set	_recvmsg, ___recvmsg
#endif

