#include "fstatfs.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include "../simple.h"
#include "../../../../libc/include/fcntl.h"
#include "../fcntl/open.h"
#include "../unistd/close.h"
#include "../unistd/readlink.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>

extern char *strtok_r(char *str, const char *delim, char **saveptr);
extern __SIZE_TYPE__ strlcpy(char* dst, const char* src, __SIZE_TYPE__ size);
extern __SIZE_TYPE__ strlen(const char* str);
extern int strncmp(const char* s1, const char* s2, __SIZE_TYPE__ len);
extern int sprintf(char *str, const char *format, ...);

long sys_fstatfs(int fd, struct bsd_statfs* buf)
{
#ifdef __x86_64__
	return sys_fstatfs64(fd, (struct bsd_statfs64*) buf);
#else
#	warning Not implemented
	return 0;
#endif
}

long sys_fstatfs64(int fd, struct bsd_statfs64* buf)
{
	int fd_m, ret;
	struct simple_readline_buf rbuf;
	struct linux_statfs64 lbuf;
	char line[512];
	char path[1024];
	int max_len = 0;

#ifdef __NR_fstatfs64
	ret = LINUX_SYSCALL3(__NR_fstatfs64, fd, sizeof(lbuf), &lbuf);
#else
	ret = LINUX_SYSCALL2(__NR_fstatfs, fd, &lbuf);
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	statfs_linux_to_bsd64(&lbuf, buf);

	sprintf(line, "/proc/self/fd/%d", fd);
	ret = sys_readlink(line, path, sizeof(path));

	if (ret < 0)
		return ret;

	fd_m = sys_open("/proc/self/mounts", O_RDONLY, 0);
	if (fd_m < 0)
		return fd_m;
	
	__simple_readline_init(&rbuf);

	buf->f_mntonname[0] = 0;
	buf->f_fstypename[0] = 0;
	buf->f_mntfromname[0] = 0;
	buf->f_iosize = 512;

	while (__simple_readline(fd_m, &rbuf, line, sizeof(line)))
	{
		char* p;
		char* saveptr;
		char* mntfrom;
		int len;

		p = strtok_r(line, " ", &saveptr);
		if (p == NULL)
			continue;

		mntfrom = p;
		p = strtok_r(NULL, " ", &saveptr);
		if (p == NULL)
			continue;
		
		len = strlen(p);
		if (strncmp(p, path, len) != 0 || len < max_len)
			continue;

		max_len = len;
		strlcpy(buf->f_mntonname, p, sizeof(buf->f_mntonname));

		p = strtok_r(NULL, " ", &saveptr);
		if (p == NULL)
			continue;

		strlcpy(buf->f_fstypename, p, sizeof(buf->f_fstypename));
		strlcpy(buf->f_mntfromname, mntfrom, sizeof(buf->f_mntfromname));
	}

	sys_close(fd_m);
	return 0;
}

