#include "getfsstat.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include "../simple.h"
#include "../../../../libc/include/fcntl.h"
#include "../fcntl/open.h"
#include "../unistd/close.h"
#include <asm/unistd.h>
#include <stddef.h>

extern char *strtok_r(char *str, const char *delim, char **saveptr);
extern unsigned long strlcpy(char* dst, const char* src, unsigned long size);

long sys_getfsstat(struct bsd_statfs* buf, int bufsize, int flags)
{
#ifdef __x86_64__
	return sys_getfsstat64((struct bsd_statfs64*) buf, bufsize, flags);
#else
#	warning Not implemented
	return 0;
#endif
}

long sys_getfsstat64(struct bsd_statfs64* buf, int bufsize, int flags)
{
	int fd;
	struct simple_readline_buf rbuf;
	char line[512];
	int count = 0;

	fd = sys_open("/proc/self/mounts", O_RDONLY, 0);
	if (fd < 0)
		return fd;
	
	__simple_readline_init(&rbuf);

	while (__simple_readline(fd, &rbuf, line, sizeof(line)))
	{
		char* p;
		char* saveptr;
		struct linux_statfs64 lbuf;

		if (buf != NULL)
		{
			if (bufsize < sizeof(struct bsd_statfs64) * (count+1))
				break;

			p = strtok_r(line, " ", &saveptr);
			if (p == NULL)
				continue;

			strlcpy(buf[count].f_mntfromname, p, sizeof(buf[count].f_mntfromname));

			p = strtok_r(NULL, " ", &saveptr);
			if (p == NULL)
				continue;

			strlcpy(buf[count].f_mntonname, p, sizeof(buf[count].f_mntonname));

			p = strtok_r(NULL, " ", &saveptr);
			if (p == NULL)
				continue;

			strlcpy(buf[count].f_fstypename, p, sizeof(buf[count].f_fstypename));

#ifdef __NR_statfs64
			if (LINUX_SYSCALL2(__NR_statfs64, (char*buf[count].f_mntonname, &lbuf) < 0)
#else
			if (LINUX_SYSCALL2(__NR_statfs, (char*)buf[count].f_mntonname, &lbuf) < 0)
#endif
				continue;

			statfs_linux_to_bsd64(&lbuf, &buf[count]);
		}

		count++;
	}

	sys_close(fd);
	return count;
}

