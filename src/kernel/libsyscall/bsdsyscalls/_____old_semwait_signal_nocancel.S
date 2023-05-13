#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS___old_semwait_signal_nocancel
#error "SYS___old_semwait_signal_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_____old_semwait_signal_nocancel)
SYSCALL_NONAME(__old_semwait_signal_nocancel, 5, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_____old_semwait_signal_nocancel, __old_semwait_signal_nocancel, 5, cerror_nocancel)
#endif

#endif

