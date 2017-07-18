#include "chown.h"
#include "../base.h"
#include "../errno.h"
#include "fchownat.h"
#include "../bsdthread/per_thread_wd.h"
#include <linux-syscalls/linux.h>

long sys_chown(const char* path, int uid, int gid)
{
	return sys_fchownat(get_perthread_wd(), path, uid, gid, 0);
}
