#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_sfi_pidctl
#error "SYS_sfi_pidctl not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___sfi_pidctl, sfi_pidctl, 4, cerror_nocancel)
#endif

