#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_disconnectx
#error "SYS_disconnectx not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(disconnectx, disconnectx, 3, cerror)
#endif

