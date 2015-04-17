#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_aio_suspend
#error "SYS_aio_suspend not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(aio_suspend, aio_suspend, 3, cerror)
#endif

