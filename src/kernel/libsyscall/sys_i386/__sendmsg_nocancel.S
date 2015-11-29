#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_sendmsg_nocancel
#error "SYS_sendmsg_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__sendmsg_nocancel, sendmsg_nocancel, 3, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	sendmsg$NOCANCEL$UNIX2003
		sendmsg$NOCANCEL$UNIX2003 = __sendmsg_nocancel
#endif

