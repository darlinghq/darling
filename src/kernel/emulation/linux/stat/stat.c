#include "stat.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "fstatat.h"
#include "../bsdthread/per_thread_wd.h"

long sys_stat(const char* path, struct stat* stat)
{
	return sys_fstatat(get_perthread_wd(), path, stat, 0);
}

long sys_stat64(const char* path, struct stat64* stat)
{
	return sys_fstatat64(get_perthread_wd(), path, stat, 0);
}
