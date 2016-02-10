#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_watchevent
#error "SYS_watchevent not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(watchevent, watchevent, 2, cerror_nocancel)
#endif

