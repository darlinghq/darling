#define __SYSCALL_32BIT_ARG_BYTES 0
#include "SYS.h"

#ifndef SYS_issetugid
#error "SYS_issetugid not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(issetugid, issetugid, 0, cerror_nocancel)
#endif

