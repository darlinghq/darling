#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_fsync
#error "SYS_fsync not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__fsync, fsync, 1, cerror)
#endif


#if defined(__i386__)
	.globl	fsync$UNIX2003
		fsync$UNIX2003 = __fsync
#endif

