#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS___pthread_chdir
#error "SYS___pthread_chdir not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(___pthread_chdir)
SYSCALL_NONAME(__pthread_chdir, 1, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(___pthread_chdir, __pthread_chdir, 1, cerror_nocancel)
#endif

#endif

