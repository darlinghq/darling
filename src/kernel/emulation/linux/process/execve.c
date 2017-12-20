#include "execve.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../fcntl/open.h"
#include "../unistd/read.h"
#include "../unistd/close.h"
#include "../unistd/readlink.h"
#include "../signal/sigexc.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <errno.h>

#define MH_MAGIC    0xfeedface
#define MH_CIGAM    0xcefaedfe
#define MH_MAGIC_64 0xfeedfacf
#define MH_CIGAM_64 0xcffaedfe
#define FAT_MAGIC   0xcafebabe
#define FAT_CIGAM   0xbebafeca

extern int strcmp(const char *s1, const char *s2);
extern char *strcat(char *dest, const char *src);
extern char *strcpy(char *dest, const char *src);
extern char* strchr(const char* s, int c);
extern void *memchr(const void *s, int c, __SIZE_TYPE__ n);
extern int strncmp(const char *s1, const char *s2, __SIZE_TYPE__ n);
extern __SIZE_TYPE__ strlen(const char *s);

static inline bool isspace(char c)
{
	return c <= ' ' && c > '\0';
}

long sys_execve(char* fname, char** argvp, char** envp)
{
	int ret;

	linux_sigset_t set;
	set = (1ull << (SIGNAL_SIGEXC_TOGGLE-1));
	set |= (1ull << (SIGNAL_SIGEXC_THUPDATE-1));

	LINUX_SYSCALL(__NR_rt_sigprocmask, 0 /* LINUX_SIG_BLOCK */,
			&set, NULL, sizeof(linux_sigset_t));

	ret = LINUX_SYSCALL(__NR_execve, fname, argvp, envp);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	LINUX_SYSCALL(__NR_rt_sigprocmask, 1 /* LINUX_SIG_UNBLOCK */,
			&set, NULL, sizeof(linux_sigset_t));

	return ret;
}
