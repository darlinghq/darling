#include <errno.h>

void _dirhelper_fork_child(void)
{
}

int _dirhelper()
{
	errno = ENOSYS;
	return -1;
}

