#define __SYSCALL_32BIT_ARG_BYTES 28
#include "SYS.h"

#ifndef SYS___semwait_signal
#error "SYS___semwait_signal not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___semwait_signal)
SYSCALL_NONAME(__semwait_signal, 6, cerror)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___semwait_signal, __semwait_signal, 6, cerror)
#endif

#endif

