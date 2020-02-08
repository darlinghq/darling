#include "semctl.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stdint.h>

#define LINUX_IPC_64 0x100
#define IPCOP_semop	1
#define IPCOP_semget	2
#define IPCOP_semctl	3

#define IPC_SET 1
#define IPC_STAT 2

extern void *memcpy(void *dest, const void *src, __SIZE_TYPE__ n);

long sys_semctl(int semid, int semnum, int cmd, semun_t arg)
{
#ifdef __NR_semctl
	int ret = LINUX_SYSCALL(__NR_semctl, semid, semnum, cmd | LINUX_IPC_64, arg.buf);
#else
	int ret = LINUX_SYSCALL(__NR_ipc, IPCOP_semctl, semid, semnum, cmd | LINUX_IPC_64, &arg.buf);
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	if (cmd == IPC_SET || cmd == IPC_STAT)
	{
		struct linux_semid_ds orig;

		memcpy(&orig, arg.buf, sizeof(orig));

		arg.buf->sem_perm.uid = orig.sem_perm.uid;
		arg.buf->sem_perm.gid = orig.sem_perm.gid;
		arg.buf->sem_perm.cuid = orig.sem_perm.cuid;
		arg.buf->sem_perm.cgid = orig.sem_perm.cgid;
		arg.buf->sem_perm.mode = orig.sem_perm.mode;

		arg.buf->sem_base = 0;
		arg.buf->sem_nsems = orig.sem_nsems;
		arg.buf->sem_otime = orig.sem_otime;
		arg.buf->sem_ctime = orig.sem_ctime;
	}

	return 0;
}
