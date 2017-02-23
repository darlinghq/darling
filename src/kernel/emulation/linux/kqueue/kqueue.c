#include "kqueue.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include "../../../../../platform-include/sys/errno.h"
#include "../simple.h"

int __attribute__((weak)) __attribute__((visibility("default"))) kqueue_impl(void)
{
	__simple_printf("No kqueue implementation?!\n");
	return -ENOSYS;
}

long sys_kqueue(void)
{
	return kqueue_impl();
}

