#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_bsdthread_create
#error "SYS_bsdthread_create not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(___bsdthread_create, bsdthread_create, 5, cerror_nocancel)
#endif

