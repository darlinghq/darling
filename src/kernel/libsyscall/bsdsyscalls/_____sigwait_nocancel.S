#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS___sigwait_nocancel
#error "SYS___sigwait_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_____sigwait_nocancel)
SYSCALL_NONAME(__sigwait_nocancel, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_____sigwait_nocancel, __sigwait_nocancel, 2, cerror_nocancel)
#endif

#endif

