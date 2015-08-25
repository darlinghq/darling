#include "base.h"
#include <asm/unistd.h>
#include "debug.h"
#include "duct_errno.h"
#include "write.h"

void* __bsd_syscall_table[512] = {
	[4] = sys_write,
	[397] = sys_write_nocancel,
};

#ifdef __x86_64__
__attribute__((naked))
long linux_syscall(long a1, long a2, long a3, long a4, long a5, long a6, int nr)
{
	__asm__ ("movq 8(%rsp), %rax\n"
			"syscall\n"
			"ret");
}
#else
#	error Unsupported platform!
#endif

int __unknown_syscall(int nr, ...)
{
	__simple_printf("Unimplemented syscall (%d)\n", nr);
	return -ENOSYS;
}

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

