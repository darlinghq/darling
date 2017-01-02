#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_getattrlist
#error "SYS_getattrlist not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___getattrlist, getattrlist, 5, cerror_nocancel)
#endif

#if defined(__x86_64__)
	.globl	_getattrlist
	.set	_getattrlist, ___getattrlist
#endif

