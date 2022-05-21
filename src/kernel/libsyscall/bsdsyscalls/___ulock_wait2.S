#define __SYSCALL_32BIT_ARG_BYTES 32
#include "SYS.h"

#ifndef SYS_ulock_wait2
#error "SYS_ulock_wait2 not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___ulock_wait2)
SYSCALL_NONAME(ulock_wait2, 5, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(___ulock_wait2, ulock_wait2, 5, cerror_nocancel)
#endif

#endif

