#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS___mac_set_proc
#error "SYS___mac_set_proc not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___mac_set_proc)
SYSCALL_NONAME(__mac_set_proc, 1, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___mac_set_proc, __mac_set_proc, 1, cerror_nocancel)
#endif

#endif

#if defined(__i386__)
	.globl	___sandbox_msp
	.set	___sandbox_msp, ___mac_set_proc
#endif

#if defined(__x86_64__)
	.globl	___sandbox_msp
	.set	___sandbox_msp, ___mac_set_proc
#endif

#if defined(__ppc__)
	.globl	___sandbox_msp
	.set	___sandbox_msp, ___mac_set_proc
#endif

#if defined(__arm64__)
	.globl	___sandbox_msp
	.set	___sandbox_msp, ___mac_set_proc
#endif

