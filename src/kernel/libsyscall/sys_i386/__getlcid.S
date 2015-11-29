#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_getlcid
#error "SYS_getlcid not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(__getlcid, getlcid, 1, cerror_nocancel)
#endif

