#include "lstat64_extended.h"
#include "lstat.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_lstat64_extended(const char* path, struct stat64* stat, void* xsec, unsigned long* xsec_size)
{
	if (xsec_size)
		*xsec_size = 0;
	return sys_lstat64(path, stat);
}


