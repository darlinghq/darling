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
#include <lkm/api.h>
#include <mach/lkm.h>
#include "../vchroot_expand.h"
#include "../bsdthread/per_thread_wd.h"

static inline bool istext(char c)
{
	return c >= 0x20 && c < 0x7F;
}

long sys_execve(char* fname, char** argvp, char** envp)
{
	int ret;
	struct vchroot_expand_args vc;

	vc.flags = VCHROOT_FOLLOW;
	vc.dfd = get_perthread_wd();
	
	strcpy(vc.path, fname);

	ret = vchroot_expand(&vc);
	__simple_kprintf("execve expand %s -> %s, ret %d", fname, vc.path, ret);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	char shebang[256];
	int fd = sys_open(fname, BSD_O_RDONLY, 0);
	if (fd < 0)
		return fd;
	
	ret = sys_read(fd, shebang, sizeof(shebang));
	if (ret < 0)
		return ret;

	sys_close(fd);

	if (istext(shebang[0]) && istext(shebang[1]) && istext(shebang[2]) && istext(shebang[3]))
		strcpy(shebang, "#!/bin/sh\n");

	if (shebang[0] == '#' && shebang[1] == '!')
	{
		char *nl, *interp, *arg;
		char** modargvp;
		int i, j, len = 0;

		nl = memchr(shebang, '\n', ret);
		if (!nl)
			return -ENOEXEC;
		
		*nl = '\0';
		for (i = 2; isspace(shebang[i]); i++);
			
			interp = &shebang[i];
			
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
			strcpy(vc.path, interp);

			ret = vchroot_expand(&vc);
			if (ret < 0)
				return ret;
	}

	linux_sigset_t set;
	set = (1ull << (SIGNAL_SIGEXC_TOGGLE-1));
	set |= (1ull << (SIGNAL_SIGEXC_THUPDATE-1));

	LINUX_SYSCALL(__NR_rt_sigprocmask, 0 /* LINUX_SIG_BLOCK */,
			&set, NULL, sizeof(linux_sigset_t));

	ret = LINUX_SYSCALL(__NR_execve, vc.path, argvp, envp);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	LINUX_SYSCALL(__NR_rt_sigprocmask, 1 /* LINUX_SIG_UNBLOCK */,
			&set, NULL, sizeof(linux_sigset_t));

	return ret;
}
