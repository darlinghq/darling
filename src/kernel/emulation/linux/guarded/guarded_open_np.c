#include "guarded_open_np.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../fcntl/open.h"

long sys_guarded_open_np(const char* filename, uint64_t* guardid, unsigned int guardflags, int flags, unsigned int mode)
{
	*guardid = 0;
	return sys_open(filename, flags, mode);
}

