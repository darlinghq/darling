#define BUILDING_BASE_C
#include "base.h"
#include <asm/unistd.h>
#include "debug.h"
#include "duct_errno.h"
#include "syscalls.h"

#ifdef __x86_64__
__attribute__((naked))
long linux_syscall(long a1, long a2, long a3, long a4, long a5, long a6, int nr)
{
	__asm__ ("movl 8(%rsp), %eax\n"
			"movq %rcx, %r10\n"
			"syscall\n"
			"ret");
}
#else
#	error Unsupported platform!
#endif

long __unknown_syscall(int nr, ...)
{
	__simple_printf("Unimplemented syscall (%d)\n", nr);
	return -ENOSYS;
}

int __linux_syscall(int nr, long a1, long a2, long a3, long a4, long a5, long a6)
{
	return linux_syscall(a1, a2, a3, a4, a5, a6, nr);
}

#ifdef __TESTING
void _start()
{
#ifdef __x86_64__
	__asm__ ("andq    $-16, %rsp");
#elif defined(__i386__
	__asm__ ("andl    $-16, %esp");
#endif
	__simple_printf("Hello world\n");
	LINUX_SYSCALL1(__NR_exit, 0);
}
#endif

