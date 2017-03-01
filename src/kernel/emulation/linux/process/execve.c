#include "execve.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../fcntl/open.h"
#include "../unistd/read.h"
#include "../unistd/close.h"
#include "../unistd/readlink.h"
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
	int ret, fd, len;
	char mldr_path[256];
	union
	{
		uint32_t magic;
		char magic_array[256];
	} m;

	// Ideally, if everybody used binfmt_misc to allow direct
	// execution of Mach-O binaries under Darling, this wouldn't
	// be necessary. But we cannot rely on that.

	fd = sys_open(fname, BSD_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	ret = sys_read(fd, m.magic_array, sizeof(m.magic_array));
	sys_close(fd);

	if (ret < 4)
		return -ENOEXEC;

	len = LINUX_SYSCALL(__NR_readlink, "/proc/self/exe", mldr_path, sizeof(mldr_path)-1);
	if (len < 0)
		return -ENOEXEC;
	mldr_path[len] = '\0';

	if (m.magic == MH_MAGIC || m.magic == MH_CIGAM
			|| m.magic == MH_MAGIC_64 || m.magic == MH_CIGAM_64
			|| m.magic == FAT_MAGIC || m.magic == FAT_CIGAM)
	{
		// It is a Mach-O file
		int i;
		char** modargvp;
		char *buf;

		// Prepend mldr path
		len = 0;

		// Count arguments
		while (argvp[len++]);

		// Allocate a new argvp, execute mldr_path
		modargvp = (char**) __builtin_alloca(sizeof(void*) * (len+1));
		buf = __builtin_alloca(strlen(mldr_path) + 2 + strlen(fname));

		strcpy(buf, mldr_path);
		strcat(buf, "!");
		strcat(buf, fname);
		modargvp[0] = buf;

		for (i = 1; i < len+1; i++)
			modargvp[i] = argvp[i-1];

		argvp = modargvp;
		fname = mldr_path;
	}
	// Shebang support
	else if (m.magic_array[0] == '#' && m.magic_array[1] == '!')
	{
		char *nl, *interp, *arg;
		char** modargvp;
		int i, j;
		
		nl = memchr(m.magic_array, '\n', sizeof(m.magic_array));
		if (nl == NULL)
			return -ENOEXEC;
		
		*nl = '\0';
		
		for (i = 2; isspace(m.magic_array[i]); i++);
		
		interp = &m.magic_array[i];
		
		for (i = 0; !isspace(interp[i]) && interp[i]; i++);
		
		if (interp[i] == '\0')
			arg = NULL;
		else
			arg = &interp[i];
		
		if (arg != NULL)
		{
			*arg = '\0'; // terminate interp
			while (isspace(*arg))
				arg++;
			if (*arg == '\0')
				arg = NULL; // no argument, just whitespace
		}
		
		// Count original arguments
		while (argvp[len++]);
		
		// Allocate a new argvp
		modargvp = (char**) __builtin_alloca(sizeof(void*) * (len+3));
		
		i = 0;

		modargvp[i++] = mldr_path;
		modargvp[i++] = interp;
		if (arg != NULL)
			modargvp[i++] = arg;
		modargvp[i] = fname;
		
		// Append original arguments
		for (j = 1; j < len+1; j++)
			modargvp[i+j] = argvp[j];
		
		argvp = modargvp;
		fname = mldr_path;
	}
	else
	{
		return -ENOEXEC;
	}

	ret = LINUX_SYSCALL(__NR_execve, fname, argvp, envp);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
