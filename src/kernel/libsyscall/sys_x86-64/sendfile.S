#define __SYSCALL_32BIT_ARG_BYTES 28
#include "SYS.h"

#ifndef SYS_sendfile
#error "SYS_sendfile not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(sendfile, sendfile, 6, cerror_nocancel)
#endif

