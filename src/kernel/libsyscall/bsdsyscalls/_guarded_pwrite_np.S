#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_guarded_pwrite_np
#error "SYS_guarded_pwrite_np not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_guarded_pwrite_np)
ZERO_EXTEND(3)
SYSCALL_NONAME(guarded_pwrite_np, 5, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_guarded_pwrite_np, guarded_pwrite_np, 5, cerror_nocancel)
#endif

#endif

