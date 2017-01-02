#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_setlogin
#error "SYS_setlogin not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(___setlogin, setlogin, 1, cerror_nocancel)
#endif

