#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_sendmsg_nocancel
#error "SYS_sendmsg_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___sendmsg_nocancel)
SYSCALL_NONAME(sendmsg_nocancel, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___sendmsg_nocancel, sendmsg_nocancel, 3, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_sendmsg$NOCANCEL$UNIX2003
	.set	_sendmsg$NOCANCEL$UNIX2003, ___sendmsg_nocancel
#endif

#if defined(__x86_64__)
	.globl	_sendmsg$NOCANCEL
	.set	_sendmsg$NOCANCEL, ___sendmsg_nocancel
#endif

#if defined(__ppc__)
	.globl	_sendmsg$NOCANCEL$UNIX2003
	.set	_sendmsg$NOCANCEL$UNIX2003, ___sendmsg_nocancel
#endif

#if defined(__arm__)
	.globl	_sendmsg$NOCANCEL
	.set	_sendmsg$NOCANCEL, ___sendmsg_nocancel
#endif

#if defined(__arm64__)
	.globl	_sendmsg$NOCANCEL
	.set	_sendmsg$NOCANCEL, ___sendmsg_nocancel
#endif

