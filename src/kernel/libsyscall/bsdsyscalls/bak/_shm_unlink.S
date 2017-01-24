#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_shm_unlink
#error "SYS_shm_unlink not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(_shm_unlink, shm_unlink, 1, cerror_nocancel)
#endif

