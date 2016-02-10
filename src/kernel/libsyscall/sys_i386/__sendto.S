#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_sendto
#error "SYS_sendto not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__sendto, sendto, 6, cerror)
#endif

#if defined(__i386__)
	.globl	sendto$UNIX2003
		sendto$UNIX2003 = __sendto
#endif

