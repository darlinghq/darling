#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_coalition_info
#error "SYS_coalition_info not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___coalition_info, coalition_info, 4, cerror_nocancel)
#endif

