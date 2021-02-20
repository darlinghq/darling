#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_sigsuspend
#error "SYS_sigsuspend not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___sigsuspend)
SYSCALL_NONAME(sigsuspend, 1, cerror)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___sigsuspend, sigsuspend, 1, cerror)
#endif

#endif

