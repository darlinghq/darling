#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_pread_nocancel
#error "SYS_pread_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__pread_nocancel, pread_nocancel, 4, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	pread
		pread = __pread_nocancel
	.globl	pread$NOCANCEL$UNIX2003
		pread$NOCANCEL$UNIX2003 = __pread_nocancel
#endif

