#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_aio_fsync
#error "SYS_aio_fsync not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(_aio_fsync, aio_fsync, 2, cerror_nocancel)
#endif

