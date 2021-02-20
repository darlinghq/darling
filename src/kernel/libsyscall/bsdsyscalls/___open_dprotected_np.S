#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_open_dprotected_np
#error "SYS_open_dprotected_np not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___open_dprotected_np)
SYSCALL_NONAME(open_dprotected_np, 5, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___open_dprotected_np, open_dprotected_np, 5, cerror_nocancel)
#endif

#endif

