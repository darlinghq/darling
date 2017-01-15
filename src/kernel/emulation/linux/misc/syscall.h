#ifndef LINUX_SYSCALL_H
#define LINUX_SYSCALL_H
#include <stdint.h>

long sys_syscall(int no, uintptr_t a1, uintptr_t a2, uintptr_t a3, uintptr_t a4, uintptr_t a5, uintptr_t a6);

#endif

