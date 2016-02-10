#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_fileport_makeport
#error "SYS_fileport_makeport not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(fileport_makeport, fileport_makeport, 2, cerror_nocancel)
#endif

