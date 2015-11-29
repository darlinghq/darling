#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_read_nocancel
#error "SYS_read_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__read_nocancel, read_nocancel, 3, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	read$NOCANCEL
		read$NOCANCEL = __read_nocancel
#endif

