#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_thread_selfcounts
#error "SYS_thread_selfcounts not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_thread_selfcounts)
ZERO_EXTEND(2)
SYSCALL_NONAME(thread_selfcounts, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_thread_selfcounts, thread_selfcounts, 3, cerror_nocancel)
#endif

#endif

