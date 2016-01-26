#include "pathconf.h"
#include "fpathconf.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>

long sys_pathconf(const char* file, int name)
{
	return sys_fpathconf(-1, name);
}

