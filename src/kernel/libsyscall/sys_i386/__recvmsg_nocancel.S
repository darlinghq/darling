#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_recvmsg_nocancel
#error "SYS_recvmsg_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__recvmsg_nocancel, recvmsg_nocancel, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	recvmsg$NOCANCEL$UNIX2003
		recvmsg$NOCANCEL$UNIX2003 = __recvmsg_nocancel
#endif

