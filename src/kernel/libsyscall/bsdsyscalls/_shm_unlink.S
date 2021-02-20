#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_shm_unlink
#error "SYS_shm_unlink not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_shm_unlink)
SYSCALL_NONAME(shm_unlink, 1, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm__) || defined(__arm64__)
__SYSCALL2(_shm_unlink, shm_unlink, 1, cerror_nocancel)
#endif

#endif

