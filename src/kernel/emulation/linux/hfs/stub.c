#include "stub.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <sys/errno.h>
#include "../fcntl/open.h"

long sys_mkcomplex()
{
	return -ENOTSUP;
}

long sys_statv()
{
	return -ENOTSUP;
}

long sys_lstatv()
{
	return -ENOTSUP;
}

long sys_fstatv()
{
	return -ENOTSUP;
}

long sys_getdirentriesattr()
{
	return -ENOTSUP;
}

long sys_exchangedata()
{
	return -ENOTSUP;
}

long sys_searchfs()
{
	return -ENOTSUP;
}

long sys_delete()
{
	return -ENOTSUP;
}

long sys_copyfile()
{
	return -ENOTSUP;
}

long sys_open_dprotected_np(const char* path, int flags, int class, int dpflags, int mode) {
	// just defer to regular sys_open
	return sys_open(path, flags, mode);
};
