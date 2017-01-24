#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_fsync
#error "SYS_fsync not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(_fsync, fsync, 1, cerror)
#else
__SYSCALL2(___fsync, fsync, 1, cerror)
#endif

#if defined(__i386__)
	.globl	_fsync$UNIX2003
	.set	_fsync$UNIX2003, ___fsync
#endif

