#include "semctl.h"
#include "../base.h"
#include "../errno.h"
#include "../duct_errno.h"
#include <linux-syscalls/linux.h>
#include <stdint.h>

#if defined(__i386__) || defined(__arm__)
#define LINUX_IPC_64 0x100
#else
#define LINUX_IPC_64 0
#endif

#define IPCOP_semop	1
#define IPCOP_semget	2
#define IPCOP_semctl	3

#define IPC_SET 1
#define IPC_STAT 2

static const int cmd_map[] = { // bsd cmds to linux cmds
	0, 1, 2, 14, 11, 12, 13, 15, 16, 17
};

extern void *memcpy(void *dest, const void *src, __SIZE_TYPE__ n);

long sys_semctl(int semid, int semnum, int cmd, union semun arg)
{
	if (cmd < 0 || cmd >= sizeof(cmd_map) / sizeof(cmd_map[0]))
		return -EINVAL;

	int linux_cmd = cmd_map[cmd];

#ifdef __NR_semctl
	int ret = LINUX_SYSCALL(__NR_semctl, semid, semnum, linux_cmd | LINUX_IPC_64, arg.buf);
#else
	int ret = LINUX_SYSCALL(__NR_ipc, IPCOP_semctl, semid, semnum, linux_cmd | LINUX_IPC_64, &arg.buf);
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
