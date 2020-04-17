#define __SYSCALL_32BIT_ARG_BYTES 8
#include "SYS.h"

#ifndef SYS_pid_shutdown_sockets
#error "SYS_pid_shutdown_sockets not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(_pid_shutdown_sockets, pid_shutdown_sockets, 2, cerror_nocancel)
#endif

