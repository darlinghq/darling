#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_setgroups
#error "SYS_setgroups not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(setgroups, setgroups, 2, cerror_nocancel)
#endif

