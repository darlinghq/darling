#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_disconnectx
#error "SYS_disconnectx not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__arm64__)
MI_ENTRY_POINT(_disconnectx)
SYSCALL_NONAME(disconnectx, 3, cerror)
ret
#else
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || defined(__arm64__)
__SYSCALL2(_disconnectx, disconnectx, 3, cerror)
#endif

#endif

