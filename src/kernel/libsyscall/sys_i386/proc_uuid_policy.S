#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_proc_uuid_policy
#error "SYS_proc_uuid_policy not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(proc_uuid_policy, proc_uuid_policy, 4, cerror_nocancel)
#endif

