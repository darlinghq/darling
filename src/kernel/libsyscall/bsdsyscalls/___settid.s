#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_settid
#error "SYS_settid not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___settid)
SYSCALL_NONAME(settid, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___settid, settid, 2, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	_pthread_setugid_np
	.set	_pthread_setugid_np, ___settid
#endif

#if defined(__x86_64__)
	.globl	_pthread_setugid_np
	.set	_pthread_setugid_np, ___settid
#endif

#if defined(__ppc__)
	.globl	_pthread_setugid_np
	.set	_pthread_setugid_np, ___settid
#endif

#if defined(__arm__)
	.globl	_pthread_setugid_np
	.set	_pthread_setugid_np, ___settid
#endif

#if defined(__arm64__)
	.globl	_pthread_setugid_np
	.set	_pthread_setugid_np, ___settid
#endif

