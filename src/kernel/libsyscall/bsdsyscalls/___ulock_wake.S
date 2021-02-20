#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_ulock_wake
#error "SYS_ulock_wake not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___ulock_wake)
SYSCALL_NONAME(ulock_wake, 3, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___ulock_wake, ulock_wake, 3, cerror_nocancel)
#endif

#endif

