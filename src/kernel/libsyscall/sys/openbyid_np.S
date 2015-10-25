#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_openbyid_np
#error "SYS_openbyid_np not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(openbyid_np, openbyid_np, 3, cerror_nocancel)
#endif

