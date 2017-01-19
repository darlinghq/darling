#include "stub.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../../../../../platform-include/sys/errno.h"

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


// We could support the following four syscalls
// because darling-dmg exports at least some
// of this metadata.
long sys_getattrlist()
{
	return -ENOTSUP;
}

long sys_setattrlist()
{
	return -ENOTSUP;
}

long sys_fgetattrlist()
{
	return -ENOTSUP;
}

long sys_fsetattrlist()
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


