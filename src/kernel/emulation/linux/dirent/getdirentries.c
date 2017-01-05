#include "getdirentries.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <sys/dirent.h>

extern __SIZE_TYPE__ strlen(const char* s);
extern char* strcpy(char* dest, const char* src);
extern int strcmp(const char* str1, const char* str2);

#ifndef min
#	define min(a,b) ((a) < (b)) ? (a) : (b)
#endif

static inline void round_to_4(unsigned short* reclen)
{
	if (*reclen & 3)
	{
		*reclen += 3;
		*reclen /= 4;
		*reclen *= 4;
	}
}

long sys_getdirentries(int fd, char* ibuf, unsigned int len, long* basep)
{
	int ret, bpos = 0, opos = 0;
	char buf[1024];

	if (basep)
		*basep = 0;

	ret = LINUX_SYSCALL(__NR_getdents64, fd, buf, min(len, sizeof(buf)));
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	while (bpos < ret && opos < len)
	{
		struct linux_dirent64* l64;
		struct bsd_dirent* bsd;
		int slen;

		l64 = (struct linux_dirent64*) (buf + bpos);
		bsd = (struct bsd_dirent*) (ibuf + opos);
		slen = strlen(l64->d_name);

		if (len-opos < sizeof(struct bsd_dirent) + slen + 1)
			break;

		bsd->d_ino = l64->d_ino;
		bsd->d_type = l64->d_type;
		strcpy(bsd->d_name, l64->d_name);

		bsd->d_reclen = sizeof(struct bsd_dirent) + slen + 1;
		bsd->d_namlen = slen;

		round_to_4(&bsd->d_reclen);

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

	ret = LINUX_SYSCALL(__NR_getdents64, fd, buf, min(len, sizeof(buf)));
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	while (bpos < ret && opos < len)
	{
		struct linux_dirent64* l64;
		struct bsd_dirent64* bsd;
		int slen;

		l64 = (struct linux_dirent64*) (buf + bpos);
		bsd = (struct bsd_dirent64*) (ibuf + opos);
		slen = strlen(l64->d_name);

		if (len-opos < sizeof(struct bsd_dirent64) + slen + 1)
			break;

		bsd->d_ino = l64->d_ino;
		bsd->d_type = l64->d_type;
		strcpy(bsd->d_name, l64->d_name);

		bsd->d_reclen = sizeof(struct bsd_dirent64) + slen + 1;
		bsd->d_namlen = slen;
		bsd->d_seekoff = 0;

		round_to_4(&bsd->d_reclen);

		opos += bsd->d_reclen;
		bpos += l64->d_reclen;
	}

	return opos;
}
