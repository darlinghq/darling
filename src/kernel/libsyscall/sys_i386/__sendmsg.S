#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_sendmsg
#error "SYS_sendmsg not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__sendmsg, sendmsg, 3, cerror)
#endif

#if defined(__i386__)
	.globl	sendmsg$UNIX2003
		sendmsg$UNIX2003 = __sendmsg
#endif

