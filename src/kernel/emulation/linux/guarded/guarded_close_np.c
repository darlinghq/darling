#include "guarded_close_np.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../unistd/close.h"

long sys_guarded_close_np(int fd, uint64_t* guardid)
{
	*guardid = 0;
	return sys_close(fd);
}

