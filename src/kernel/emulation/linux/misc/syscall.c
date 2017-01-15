#include "syscall.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <stddef.h>
#include "../syscalls.h"

long sys_syscall(int no, uintptr_t a1, uintptr_t a2, uintptr_t a3, uintptr_t a4, uintptr_t a5, uintptr_t a6)
{
	long (*ptr)();

	if (no <= 0 || no >= sizeof(__bsd_syscall_table)/sizeof(__bsd_syscall_table[0]))
		return __unknown_syscall(no);

	ptr = (long(*)()) __bsd_syscall_table[no];
	if (!ptr)
		return __unknown_syscall(no);

	return ptr(a1, a2, a3, a4, a5, a6);
}

