#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS___disable_threadsignal
#error "SYS___disable_threadsignal not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___disable_threadsignal)
SYSCALL_NONAME(__disable_threadsignal, 1, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___disable_threadsignal, __disable_threadsignal, 1, cerror_nocancel)
#endif

#endif

