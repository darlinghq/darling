#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_ffsctl
#error "SYS_ffsctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_ffsctl)
ZERO_EXTEND(1)
SYSCALL_NONAME(ffsctl, 4, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_ffsctl, ffsctl, 4, cerror_nocancel)
#endif

#endif

