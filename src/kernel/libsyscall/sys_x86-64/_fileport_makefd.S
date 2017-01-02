#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_fileport_makefd
#error "SYS_fileport_makefd not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(_fileport_makefd, fileport_makefd, 1, cerror_nocancel)
#endif

