#define __SYSCALL_32BIT_ARG_BYTES 28
#include "SYS.h"

#ifndef SYS_sendfile
#error "SYS_sendfile not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_sendfile)
SYSCALL_NONAME(sendfile, 6, cerror_nocancel)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_sendfile, sendfile, 6, cerror_nocancel)
#endif

#endif

