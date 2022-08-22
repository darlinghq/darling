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
#include "../simple.h"

#include <darlingserver/rpc.h>
#include "../elfcalls_wrapper.h"
#include "../unistd/write.h"
#include <mach-o/loader.h>
#include <mach-o/fat.h>

#undef memcpy
#include "../resources/dserver-rpc-defs.h"
extern bool isspace(char c);

extern void _xtrace_execve_inject(const char*** envp_ptr);

static inline bool istext(char c)
{
	return c >= 0x20 && c < 0x7F;
}

long sys_execve(const char* fname, const char** argvp, const char** envp)
{
	int ret;
	struct vchroot_expand_args vc;
	char mldr_path[4096];
	uint64_t mldr_path_length;
	const char* path_to_exec = vc.path;

	ret = dserver_rpc_mldr_path(mldr_path, sizeof(mldr_path), &mldr_path_length);
	if (ret < 0) {
		return errno_linux_to_bsd(ret);
	}

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

	close_internal(fd);

	bool is_script = false;
	bool is_macho = false;

	if (ret < 4) {
		return -ENOEXEC;
	}

	//if (ret >= 4)
	{
		is_script = shebang[0] == '#' && shebang[1] == '!';
		if (!is_script)
		{
			if (istext(shebang[0]) && istext(shebang[1]) && istext(shebang[2]) && istext(shebang[3]))
			{
				strcpy(shebang, "#!/bin/sh\n");
				is_script = true;
			}
		}
	}

	uint32_t magic = *(uint32_t*)shebang;
	is_macho = magic == MH_MAGIC || magic == MH_CIGAM || magic == MH_MAGIC_64 || magic == MH_CIGAM_64 || magic == FAT_MAGIC || magic == FAT_CIGAM;

	if (is_script)
	{
		char *nl, *interp, *arg;
		const char** modargvp;
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
				arg++;
				while (isspace(*arg) && *arg)
					arg++;
				if (*arg == '\0')
					arg = NULL; // no argument, just whitespace
			}

			// Count original arguments
			while (argvp[len++]);

			// Allocate a new argvp
			modargvp = (const char**) __builtin_alloca(sizeof(void*) * (len+3));

			i = 0;
			modargvp[i++] = mldr_path;
			modargvp[i++] = vc.path; // expanded later
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

			path_to_exec = mldr_path;
	} else if (is_macho) {
		const char** modargvp;
		char *buf;
		int len = 0;

		// count original arguments
		while (argvp[len++]);

		// allocate a new argvp and argv0
		modargvp = (const char**) __builtin_alloca(sizeof(void*) * (len+1));
		buf = __builtin_alloca(strlen(mldr_path) + 2 + strlen(vc.path));

		// set up the new argv0 (mldr path + "!" + executable path)
		strcpy(buf, mldr_path);
		strcat(buf, "!");
		strcat(buf, vc.path);
		modargvp[0] = buf;

		// append original arguments
		for (int i = 1; i < len+1; i++)
			modargvp[i] = argvp[i-1];

		argvp = modargvp;
		path_to_exec = mldr_path;
	}

	// set up the __mldr_sockpath env var if we're executing mldr
	if (is_script || is_macho) {
		const char** modenvp;
		char* buf;
		int len = 0;
		struct linux_sockaddr_un* server_socket_address = dserver_rpc_hooks_get_server_address();
		const char* server_socket_path = server_socket_address->sun_path;

		char mldr_lifetime_pipe_env[32] = { '\0' };
		__simple_snprintf(mldr_lifetime_pipe_env, sizeof(mldr_lifetime_pipe_env) - 1, "__mldr_lifetime_pipe=%d", __dserver_get_process_lifetime_pipe());

		// count original env vars
		while (envp[len++]);

		const int new_env_count = 2;

		// allocate a new envp and env0, env1
		modenvp = (const char**)__builtin_alloca(sizeof(void*) * (len + new_env_count));
		buf = __builtin_alloca(strlen(server_socket_path) + sizeof("__mldr_sockpath="));

		// set up the new env0
		strcpy(buf, "__mldr_sockpath=");
		strcat(buf, server_socket_path);
		modenvp[0] = buf;
		modenvp[1] = mldr_lifetime_pipe_env;

		// append original env vars
		for (int i = new_env_count; i < len + new_env_count; i++)
			modenvp[i] = envp[i-new_env_count];

		envp = modenvp;
	}

	// otherwise it's a Linux executable (ELF or something else binfmt handles);
	// this is the default

	linux_sigset_t set;
	set = (1ull << (SIGNAL_SIGEXC_SUSPEND-1));
	set |= (1ull << (SIGNAL_S2C-1));

	// darlingserver needs to know whether the execve completes successfully or not.
	// since pidfds don't notify on execve, we have to use a pipe with close-on-exec
	// that darlingserver will monitor. if it reads EOF, it knows the execve succeeded.
	// if it reads a single byte (that we send it), it knows the execve failed.

	int dserver_execve_pipe[2];

	// open a pipe with FD_CLOEXEC set
	ret = LINUX_SYSCALL(__NR_pipe2, dserver_execve_pipe, LINUX_O_CLOEXEC);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	// send a copy of the read end to the server (along with whether or not we're executing another Darling-managed binary)
	ret = dserver_rpc_checkout(dserver_execve_pipe[0], is_script || is_macho);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	// close the read end for ourselves
	close_internal(dserver_execve_pipe[0]);

	LINUX_SYSCALL(__NR_rt_sigprocmask, 0 /* LINUX_SIG_BLOCK */,
			&set, NULL, sizeof(linux_sigset_t));

	// let xtrace inject itself into the execve, if necessary
	_xtrace_execve_inject(&envp);

	ret = LINUX_SYSCALL(__NR_execve, path_to_exec, argvp, envp);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	// the execve failed; write to the write end of the pipe.
	// ignore errors.
	sys_write_nocancel(dserver_execve_pipe[1], "\x01", 1);
	close_internal(dserver_execve_pipe[1]);

	LINUX_SYSCALL(__NR_rt_sigprocmask, 1 /* LINUX_SIG_UNBLOCK */,
			&set, NULL, sizeof(linux_sigset_t));

	return ret;
}
