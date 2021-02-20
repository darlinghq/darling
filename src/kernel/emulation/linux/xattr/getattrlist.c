#include "getattrlist.h"
#include "getattrlistat.h"
#include "../base.h"
#include "../errno.h"
#include "../bsdthread/per_thread_wd.h"
#include <sys/errno.h>

long sys_getattrlist(const char* path, struct attrlist* alist, void *attributeBuffer, __SIZE_TYPE__ bufferSize, unsigned long options)
{
	return sys_getattrlistat(get_perthread_wd(), path, alist, attributeBuffer, bufferSize, options);
}

