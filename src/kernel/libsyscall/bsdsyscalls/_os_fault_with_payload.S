#define __SYSCALL_32BIT_ARG_BYTES 32
#include "SYS.h"

#ifndef SYS_os_fault_with_payload
#error "SYS_os_fault_with_payload not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(_os_fault_with_payload, os_fault_with_payload, 6, cerror_nocancel)
#endif

