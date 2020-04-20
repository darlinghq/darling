#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_read
#error "SYS_read not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_read, read, 3, cerror)
#else
__SYSCALL2(___read, read, 3, cerror)
#endif

#if defined(__i386__)
	.globl	_read$UNIX2003
	.set	_read$UNIX2003, ___read
#endif

#if defined(__ppc__)
	.globl	_read$UNIX2003
	.set	_read$UNIX2003, ___read
#endif

