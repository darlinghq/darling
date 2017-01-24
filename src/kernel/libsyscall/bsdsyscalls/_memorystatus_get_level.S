#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_memorystatus_get_level
#error "SYS_memorystatus_get_level not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(_memorystatus_get_level, memorystatus_get_level, 1, cerror_nocancel)
#endif

