#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_sendto_nocancel
#error "SYS_sendto_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__sendto_nocancel, sendto_nocancel, 6, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	sendto$NOCANCEL$UNIX2003
		sendto$NOCANCEL$UNIX2003 = __sendto_nocancel
#endif

