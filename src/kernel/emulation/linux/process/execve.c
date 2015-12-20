#include "execve.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include "../fcntl/open.h"
#include "../unistd/read.h"
#include "../unistd/close.h"
#include "../unistd/readlink.h"
#include <stdint.h>
#include <libdyld/VirtualPrefix.h>

#define MH_MAGIC    0xfeedface
#define MH_CIGAM    0xcefaedfe
#define MH_MAGIC_64 0xfeedfacf
#define MH_CIGAM_64 0xcffaedfe
#define FAT_MAGIC   0xcafebabe
#define FAT_CIGAM   0xbebafeca

extern int strcmp(const char *s1, const char *s2);

long sys_execve(char* fname, char** argvp, char** envp)
{
	int ret, fd;
	uint32_t magic;
	char dyld_path[256];

	// Ideally, if everybody used binfmt_misc to allow direct
	// execution of Mach-O binaries under Darling, this wouldn't
	// be necessary. But we cannot rely on that.
	
	fd = sys_open(fname, BSD_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	ret = sys_read(fd, &magic, sizeof(magic));
	if (ret != 4)
		goto no_macho;

	if (magic == MH_MAGIC || magic == MH_CIGAM
			|| magic == MH_MAGIC_64 || magic == MH_CIGAM_64
			|| magic == FAT_MAGIC || magic == FAT_CIGAM)
	{
		// It is a Mach-O file
		int len, i;
		char** modargvp;
		
		len = __prefix_get_dyld_path(dyld_path, sizeof(dyld_path)-1);
		if (len < 0)
			goto no_macho;

		// Remove 64 or 32 suffix if present
		if (strcmp(&dyld_path[len - 2], "32") == 0
				|| strcmp(&dyld_path[len - 2], "64") == 0)
		{
			dyld_path[len-2] = '\0';
		}

		// Prepend dyld path
		len = 0;

		// Count arguments
		while (argvp[len++]);

		// Allocate a new argvp, execute dyld_path
		modargvp = (char**) __builtin_alloca(sizeof(void*) * (len+1));
		modargvp[0] = dyld_path;
		modargvp[1] = __prefix_translate_path(fname);

		for (i = 2; i < len+1; i++)
			modargvp[i] = argvp[i-1];

		argvp = modargvp;
		fname = dyld_path;
	}

no_macho:
	sys_close(fd);

	ret = LINUX_SYSCALL(__NR_execve, fname, argvp, envp);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

