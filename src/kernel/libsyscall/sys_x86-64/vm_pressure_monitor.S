#define __SYSCALL_32BIT_ARG_BYTES 12
#include "SYS.h"

#ifndef SYS_vm_pressure_monitor
#error "SYS_vm_pressure_monitor not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__x86_64__)
__SYSCALL2(vm_pressure_monitor, vm_pressure_monitor, 3, cerror_nocancel)
#endif

