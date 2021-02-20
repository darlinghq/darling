#define __SYSCALL_32BIT_ARG_BYTES 24
#include "SYS.h"

#ifndef SYS_sysctl
#error "SYS_sysctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___sysctl)
ZERO_EXTEND(5)
SYSCALL_NONAME(sysctl, 6, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___sysctl, sysctl, 6, cerror_nocancel)
#endif

#endif

