#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_guarded_writev_np
#error "SYS_guarded_writev_np not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_guarded_writev_np)
SYSCALL_NONAME(guarded_writev_np, 4, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_guarded_writev_np, guarded_writev_np, 4, cerror_nocancel)
#endif

#endif

