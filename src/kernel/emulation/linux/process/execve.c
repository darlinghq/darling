#include "execve.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include "../fcntl/open.h"
#include "../unistd/read.h"
#include "../unistd/close.h"
#include "../unistd/readlink.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <libdyld/VirtualPrefix.h>

#define MH_MAGIC    0xfeedface
#define MH_CIGAM    0xcefaedfe
#define MH_MAGIC_64 0xfeedfacf
#define MH_CIGAM_64 0xcffaedfe
#define FAT_MAGIC   0xcafebabe
#define FAT_CIGAM   0xbebafeca

extern int strcmp(const char *s1, const char *s2);
extern char *strcat(char *dest, const char *src);
extern char *strcpy(char *dest, const char *src);
extern void* memchr(const void* s, int c, __SIZE_TYPE__ n);
extern char* strchr(const char* s, int c);
extern int strncmp(const char *s1, const char *s2, __SIZE_TYPE__ n);
extern __SIZE_TYPE__ strlen(const char *s);

static inline bool isspace(char c)
{
	return c <= ' ' && c > '\0';
}

long sys_execve(char* fname, char** argvp, char** envp)
{
	int ret, fd;
	char dyld_path[256];
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
	if (ret < 4)
		goto no_macho;
	
	if (__prefix_get() != NULL)
	{
		// Inject DPREFIX if DPREFIX is missing
		int i;
		bool has_dprefix = false;
		char** new_envp;
		
		for (i = 0; envp[i] != NULL; i++)
		{
			if (strncmp(envp[i], "DPREFIX=", 8) == 0)
			{
				has_dprefix = true;
				break;
			}
		}
		
		if (!has_dprefix)
		{
			char* dprefix;
			
			new_envp = (char**) __builtin_alloca(sizeof(char*) * (i+1));
			dprefix = (char*) __builtin_alloca(strlen(__prefix_get()) + 9);
			
			for (i = 0; envp[i] != NULL; i++)
				new_envp[i] = envp[i];
			
			strcpy(dprefix, "DPREFIX=");
			strcat(dprefix, __prefix_get());
			new_envp[i++] = dprefix;
			new_envp[i] = NULL;
			
			envp = new_envp;
		}
	}

	if (m.magic == MH_MAGIC || m.magic == MH_CIGAM
			|| m.magic == MH_MAGIC_64 || m.magic == MH_CIGAM_64
			|| m.magic == FAT_MAGIC || m.magic == FAT_CIGAM)
	{
		// It is a Mach-O file
		int len, i;
		char** modargvp;
		char *translated, *buf;
		
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
		
		translated = (char*) __prefix_translate_path_link(fname);
		buf = __builtin_alloca(strlen(translated) + 2 + strlen(argvp[0]));
		
		strcpy(buf, translated);
		strcat(buf, "!");
		strcat(buf, argvp[0]);
		modargvp[1] = buf;

		for (i = 2; i < len+1; i++)
			modargvp[i] = argvp[i-1];

		argvp = modargvp;
		fname = dyld_path;
	}
	else if (__prefix_get() != NULL)
	{
		// shebang handling...
		if (m.magic_array[0] == '#' && m.magic_array[1] == '!')
		{
			char *nl, *interp, *arg;
			char** modargvp;
			int i, j, len = 0;
			
			nl = memchr(m.magic_array, '\n', sizeof(m.magic_array));
			if (nl == NULL)
				goto no_macho;
			
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
			modargvp = (char**) __builtin_alloca(sizeof(void*) * (len+2));
			
			i = 0;
			modargvp[i++] = interp;
			if (arg != NULL)
				modargvp[i++] = arg;
			modargvp[i] = fname;
			
			// Append original arguments
			for (j = 1; j < len+1; j++)
				modargvp[i+j] = argvp[j];
			
			argvp = modargvp;
			fname = (char*) __prefix_translate_path(modargvp[0]);
		}
		else
		{
			fname = (char*) __prefix_translate_path(fname);
		}
	}

no_macho:
	sys_close(fd);

	ret = LINUX_SYSCALL(__NR_execve, fname, argvp, envp);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

