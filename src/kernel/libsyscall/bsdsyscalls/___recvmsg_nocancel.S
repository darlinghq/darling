#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_recvmsg_nocancel
#error "SYS_recvmsg_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___recvmsg_nocancel, recvmsg_nocancel, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_recvmsg$NOCANCEL$UNIX2003
	.set	_recvmsg$NOCANCEL$UNIX2003, ___recvmsg_nocancel
#endif

#if defined(__x86_64__)
	.globl	_recvmsg$NOCANCEL
	.set	_recvmsg$NOCANCEL, ___recvmsg_nocancel
#endif

#if defined(__ppc__)
	.globl	_recvmsg$NOCANCEL$UNIX2003
	.set	_recvmsg$NOCANCEL$UNIX2003, ___recvmsg_nocancel
#endif

#if defined(__arm__)
	.globl	_recvmsg$NOCANCEL
	.set	_recvmsg$NOCANCEL, ___recvmsg_nocancel
#endif

#if defined(__arm64__)
	.globl	_recvmsg$NOCANCEL
	.set	_recvmsg$NOCANCEL, ___recvmsg_nocancel
#endif

