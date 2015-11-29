#define __SYSCALL_32BIT_ARG_BYTES 32
#include "SYS.h"

#ifndef SYS_getdirentriesattr
#error "SYS_getdirentriesattr not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(getdirentriesattr, getdirentriesattr, 8, cerror_nocancel)
#endif

