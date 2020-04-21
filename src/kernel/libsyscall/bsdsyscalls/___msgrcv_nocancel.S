#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_msgrcv_nocancel
#error "SYS_msgrcv_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___msgrcv_nocancel, msgrcv_nocancel, 5, cerror_nocancel)
#endif

#if defined(__i386__)
	.globl	_msgrcv
	.set	_msgrcv, ___msgrcv_nocancel
	.globl	_msgrcv$NOCANCEL$UNIX2003
	.set	_msgrcv$NOCANCEL$UNIX2003, ___msgrcv_nocancel
#endif

#if defined(__x86_64__)
	.globl	_msgrcv$NOCANCEL
	.set	_msgrcv$NOCANCEL, ___msgrcv_nocancel
#endif

#if defined(__ppc__)
	.globl	_msgrcv
	.set	_msgrcv, ___msgrcv_nocancel
	.globl	_msgrcv$NOCANCEL$UNIX2003
	.set	_msgrcv$NOCANCEL$UNIX2003, ___msgrcv_nocancel
#endif

#if defined(__arm__)
	.globl	_msgrcv$NOCANCEL
	.set	_msgrcv$NOCANCEL, ___msgrcv_nocancel
#endif

#if defined(__arm64__)
	.globl	_msgrcv$NOCANCEL
	.set	_msgrcv$NOCANCEL, ___msgrcv_nocancel
#endif

