#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_connect_nocancel
#error "SYS_connect_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__connect_nocancel, connect_nocancel, 3, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	connect$NOCANCEL
		connect$NOCANCEL = __connect_nocancel
#endif

