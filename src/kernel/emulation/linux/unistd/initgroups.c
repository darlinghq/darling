#include "initgroups.h"
#include "setgroups.h"
#include "../base.h"

long sys_initgroups(unsigned int size, int* gidset, int gmuid)
{
	return sys_setgroups(size, gidset);
}

