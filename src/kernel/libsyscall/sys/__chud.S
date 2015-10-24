#define __SYSCALL_32BIT_ARG_BYTES 48
#include "SYS.h"

#ifndef SYS_chud
#error "SYS_chud not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(__chud, chud, 6, cerror_nocancel)
#endif

