#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_rmdir
#error "SYS_rmdir not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__rmdir, rmdir, 1, cerror_nocancel)
#endif

