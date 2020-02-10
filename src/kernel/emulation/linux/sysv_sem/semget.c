#include "semget.h"
#include "../base.h"
#include <linux-syscalls/linux.h>
#include "../errno.h"

#define IPCOP_semop	1
#define IPCOP_semget	2
#define IPCOP_semctl	3

long sys_semget(int32_t key, int n, int fl)
{
#ifdef __NR_semget
	int ret = LINUX_SYSCALL(__NR_semget, key, n, fl);
#else
	int ret = LINUX_SYSCALL(__NR_ipc, IPCOP_semget, key, n, fl);
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);
	return 0;
}
