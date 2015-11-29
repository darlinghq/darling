#define __SYSCALL_32BIT_ARG_BYTES 16
#include "SYS.h"

#ifndef SYS_recvmsg_x
#error "SYS_recvmsg_x not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__)
__SYSCALL2(recvmsg_x, recvmsg_x, 4, cerror_nocancel)
#endif

