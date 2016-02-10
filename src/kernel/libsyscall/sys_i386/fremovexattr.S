#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_fremovexattr
#error "SYS_fremovexattr not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(fremovexattr, fremovexattr, 3, cerror_nocancel)
#endif

