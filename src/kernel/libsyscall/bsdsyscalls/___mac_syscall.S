#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS___mac_syscall
#error "SYS___mac_syscall not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___mac_syscall)
SYSCALL_NONAME(__mac_syscall, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___mac_syscall, __mac_syscall, 3, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	___sandbox_ms
	.set	___sandbox_ms, ___mac_syscall
#endif

#if defined(__x86_64__)
	.globl	___sandbox_ms
	.set	___sandbox_ms, ___mac_syscall
#endif

#if defined(__ppc__)
	.globl	___sandbox_ms
	.set	___sandbox_ms, ___mac_syscall
#endif

#if defined(__arm64__)
	.globl	___sandbox_ms
	.set	___sandbox_ms, ___mac_syscall
#endif

