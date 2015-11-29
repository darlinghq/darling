#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_waitid_nocancel
#error "SYS_waitid_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__waitid_nocancel, waitid_nocancel, 4, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	waitid
		waitid = __waitid_nocancel
	.globl	waitid$NOCANCEL$UNIX2003
		waitid$NOCANCEL$UNIX2003 = __waitid_nocancel
#endif

