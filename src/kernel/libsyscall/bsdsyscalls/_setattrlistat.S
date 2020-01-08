#define __SYSCALL_32BIT_ARG_BYTES 20
#include "SYS.h"

#ifndef SYS_setattrlistat
#error "SYS_setattrlistat not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(_setattrlistat, setattrlistat, 6, cerror_nocancel)
#endif

