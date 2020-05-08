#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_bsdthread_terminate
#error "SYS_bsdthread_terminate not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___bsdthread_terminate)
ZERO_EXTEND(1)
SYSCALL_NONAME(bsdthread_terminate, 4, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___bsdthread_terminate, bsdthread_terminate, 4, cerror_nocancel)
#endif

#endif

