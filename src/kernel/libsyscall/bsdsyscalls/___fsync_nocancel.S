#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_fsync_nocancel
#error "SYS_fsync_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___fsync_nocancel, fsync_nocancel, 1, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_fsync
	.set	_fsync, ___fsync_nocancel
	.globl	_fsync$NOCANCEL$UNIX2003
	.set	_fsync$NOCANCEL$UNIX2003, ___fsync_nocancel
#endif

#if defined(__x86_64__)
	.globl	_fsync$NOCANCEL
	.set	_fsync$NOCANCEL, ___fsync_nocancel
#endif

#if defined(__ppc__)
	.globl	_fsync
	.set	_fsync, ___fsync_nocancel
	.globl	_fsync$NOCANCEL$UNIX2003
	.set	_fsync$NOCANCEL$UNIX2003, ___fsync_nocancel
#endif

#if defined(__arm__)
	.globl	_fsync$NOCANCEL
	.set	_fsync$NOCANCEL, ___fsync_nocancel
#endif

#if defined(__arm64__)
	.globl	_fsync$NOCANCEL
	.set	_fsync$NOCANCEL, ___fsync_nocancel
#endif

