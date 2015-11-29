#define __SYSCALL_32BIT_ARG_BYTES 0
#include "SYS.h"

#ifndef SYS_kqueue
#error "SYS_kqueue not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(kqueue, kqueue, 0, cerror_nocancel)
#endif

