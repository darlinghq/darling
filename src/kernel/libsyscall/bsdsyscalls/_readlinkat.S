#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_readlinkat
#error "SYS_readlinkat not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_readlinkat)
ZERO_EXTEND(3)
SYSCALL_NONAME(readlinkat, 4, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_readlinkat, readlinkat, 4, cerror_nocancel)
#endif

#endif

