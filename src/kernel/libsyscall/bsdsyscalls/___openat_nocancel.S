#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_openat_nocancel
#error "SYS_openat_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___openat_nocancel)
SYSCALL_NONAME(openat_nocancel, 4, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___openat_nocancel, openat_nocancel, 4, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_openat$NOCANCEL
	.set	_openat$NOCANCEL, ___openat_nocancel
#endif

#if defined(__x86_64__)
	.globl	_openat$NOCANCEL
	.set	_openat$NOCANCEL, ___openat_nocancel
#endif

#if defined(__ppc__)
	.globl	_openat$NOCANCEL
	.set	_openat$NOCANCEL, ___openat_nocancel
#endif

#if defined(__arm__)
	.globl	_openat$NOCANCEL
	.set	_openat$NOCANCEL, ___openat_nocancel
#endif

