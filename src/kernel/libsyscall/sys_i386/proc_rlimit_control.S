#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_proc_rlimit_control
#error "SYS_proc_rlimit_control not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(proc_rlimit_control, proc_rlimit_control, 3, cerror_nocancel)
#endif

