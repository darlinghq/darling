#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_mach_eventlink_signal
#error "SYS_mach_eventlink_signal not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___mach_eventlink_signal)
SYSCALL_NONAME(mach_eventlink_signal, 2, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___mach_eventlink_signal, mach_eventlink_signal, 2, cerror_nocancel)
#endif

#endif

