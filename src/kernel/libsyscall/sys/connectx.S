#define __SYSCALL_32BIT_ARG_BYTES 32
#include "SYS.h"

#ifndef SYS_connectx
#error "SYS_connectx not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(connectx, connectx, 8, cerror)
#endif

