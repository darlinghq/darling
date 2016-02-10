#define __SYSCALL_32BIT_ARG_BYTES 0
#include "SYS.h"

#ifndef SYS_workq_open
#error "SYS_workq_open not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__workq_open, workq_open, 0, cerror_nocancel)
#endif

