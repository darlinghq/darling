#include "statfs.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include "../simple.h"
#include "../../../../external/libc/include/fcntl.h"
#include "../fcntl/open.h"
#include "../unistd/close.h"
#include "../vchroot_expand.h"
#include "../bsdthread/per_thread_wd.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>

#undef strlcpy

extern char *strtok_r(char *str, const char *delim, char **saveptr);
extern __SIZE_TYPE__ strlcpy(char* dst, const char* src, __SIZE_TYPE__ size);
extern __SIZE_TYPE__ strlen(const char* str);
extern int strncmp(const char* s1, const char* s2, __SIZE_TYPE__ len);

long sys_statfs(const char* path, struct bsd_statfs* buf)
{
#ifdef __x86_64__
	return sys_statfs64(path, (struct bsd_statfs64*) buf);
#else
#	warning Not implemented
	return 0;
#endif
}

long sys_statfs64(const char* path, struct bsd_statfs64* buf)
{
	int fd, ret;
	struct simple_readline_buf rbuf;
	struct linux_statfs64 lbuf;
	char line[512];
	int max_len = 0;

	struct vchroot_expand_args vc;
	vc.flags = 0;
	vc.dfd = get_perthread_wd();

	strcpy(vc.path, path);
	ret = vchroot_expand(&vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

#ifdef __NR_statfs64
	ret = LINUX_SYSCALL2(__NR_statfs64, vc.path, &lbuf);
#else
	ret = LINUX_SYSCALL2(__NR_statfs, vc.path, &lbuf);
#endif

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	statfs_linux_to_bsd64(&lbuf, buf);

	fd = sys_open("/proc/self/mounts", O_RDONLY, 0);
	if (fd < 0)
		return fd;
	
	__simple_readline_init(&rbuf);

	buf->f_mntonname[0] = 0;
	buf->f_fstypename[0] = 0;
	buf->f_mntfromname[0] = 0;

	while (__simple_readline(fd, &rbuf, line, sizeof(line)))
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
		if (strncmp(p, vc.path, len) != 0 || len < max_len)
			continue;

		max_len = len;
		strlcpy(buf->f_mntonname, p, sizeof(buf->f_mntonname));

		p = strtok_r(NULL, " ", &saveptr);
		if (p == NULL)
			continue;

		strlcpy(buf->f_fstypename, p, sizeof(buf->f_fstypename));
		strlcpy(buf->f_mntfromname, mntfrom, sizeof(buf->f_mntfromname));
	}

	close_internal(fd);
	return 0;
}

