#include "getdirentries.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <sys/dirent.h>

extern unsigned long strlen(const char* s);
extern char* strcpy(char* dest, const char* src);

long sys_getdirentries(int fd, char* ibuf, unsigned int len, long* basep)
{
	int ret, bpos = 0, opos = 0;
	char buf[1024];

	if (basep)
		*basep = 0;

	ret = LINUX_SYSCALL(__NR_getdents64, fd, buf, sizeof(buf));
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	while (bpos < ret && opos < len)
	{
		struct linux_dirent64* l64;
		struct dirent* bsd;
		int len;

		l64 = (struct linux_dirent64*) (buf + bpos);
		bsd = (struct dirent*) (ibuf + opos);
		len = strlen(l64->d_name);

		if (opos-len < sizeof(struct dirent) + len + 1)
			break;

		bsd->d_ino = l64->d_ino;
		bsd->d_type = l64->d_type;
		strcpy(bsd->d_name, l64->d_name);
		bsd->d_reclen = sizeof(struct dirent) + len + 1;
		bsd->d_namlen = len;

		opos += bsd->d_reclen;
		bpos += l64->d_reclen;
	}

	return opos;
}

struct dirent64 __DARWIN_STRUCT_DIRENTRY;

long sys_getdirentries64(int fd, char* ibuf, unsigned int len, long* basep)
{
	int ret, bpos = 0, opos = 0;
	char buf[1024];

	if (basep)
		*basep = 0;

	ret = LINUX_SYSCALL(__NR_getdents64, fd, buf, sizeof(buf));
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	while (bpos < ret && opos < len)
	{
		struct linux_dirent64* l64;
		struct dirent64* bsd;
		int len;

		l64 = (struct linux_dirent64*) (buf + bpos);
		bsd = (struct dirent64*) (ibuf + opos);
		len = strlen(l64->d_name);

		if (opos-len < sizeof(struct dirent) + len + 1)
			break;

		bsd->d_ino = l64->d_ino;
		bsd->d_type = l64->d_type;
		strcpy(bsd->d_name, l64->d_name);
		bsd->d_reclen = sizeof(struct dirent) + len + 1;
		bsd->d_namlen = len;
		bsd->d_seekoff = 0;

		opos += bsd->d_reclen;
		bpos += l64->d_reclen;
	}

	return opos;
}
