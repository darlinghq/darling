#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_connect
#error "SYS_connect not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__connect, connect, 3, cerror)
#endif

#if defined(__i386__)
	.globl	connect$UNIX2003
		connect$UNIX2003 = __connect
#endif

