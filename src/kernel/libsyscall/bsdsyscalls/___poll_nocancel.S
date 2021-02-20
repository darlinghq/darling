#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_poll_nocancel
#error "SYS_poll_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___poll_nocancel)
SYSCALL_NONAME(poll_nocancel, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___poll_nocancel, poll_nocancel, 3, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_poll
	.set	_poll, ___poll_nocancel
	.globl	_poll$NOCANCEL$UNIX2003
	.set	_poll$NOCANCEL$UNIX2003, ___poll_nocancel
#endif

#if defined(__x86_64__)
	.globl	_poll$NOCANCEL
	.set	_poll$NOCANCEL, ___poll_nocancel
#endif

#if defined(__ppc__)
	.globl	_poll
	.set	_poll, ___poll_nocancel
	.globl	_poll$NOCANCEL$UNIX2003
	.set	_poll$NOCANCEL$UNIX2003, ___poll_nocancel
#endif

#if defined(__arm__)
	.globl	_poll$NOCANCEL
	.set	_poll$NOCANCEL, ___poll_nocancel
#endif

#if defined(__arm64__)
	.globl	_poll$NOCANCEL
	.set	_poll$NOCANCEL, ___poll_nocancel
#endif

