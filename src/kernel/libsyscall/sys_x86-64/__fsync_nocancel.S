#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_fsync_nocancel
#error "SYS_fsync_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__fsync_nocancel, fsync_nocancel, 1, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	fsync$NOCANCEL
		fsync$NOCANCEL = __fsync_nocancel
#endif

