#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_sendto_nocancel
#error "SYS_sendto_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___sendto_nocancel, sendto_nocancel, 6, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_sendto$NOCANCEL$UNIX2003
	.set	_sendto$NOCANCEL$UNIX2003, ___sendto_nocancel
#endif

#if defined(__x86_64__)
	.globl	_sendto$NOCANCEL
	.set	_sendto$NOCANCEL, ___sendto_nocancel
#endif

