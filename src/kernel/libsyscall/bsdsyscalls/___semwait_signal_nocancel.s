#define __SYSCALL_32BIT_ARG_BYTES 28
#include "SYS.h"

#ifndef SYS___semwait_signal_nocancel
#error "SYS___semwait_signal_nocancel not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___semwait_signal_nocancel)
SYSCALL_NONAME(__semwait_signal_nocancel, 6, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___semwait_signal_nocancel, __semwait_signal_nocancel, 6, cerror_nocancel)
#endif

#endif

