#include "posix_spawn.h"
#include "execve.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../fcntl/open.h"
#include "../unistd/read.h"
#include "../unistd/write.h"
#include "../unistd/exit.h"
#include "../unistd/close.h"
#include "../unistd/pipe.h"
#include "../unistd/dup2.h"
#include "../unistd/dup.h"
#include "../unistd/getuid.h"
#include "../unistd/getgid.h"
#include "../unistd/setuid.h"
#include "../unistd/setgid.h"
#include "../unistd/setpgid.h"
#include "../signal/sigprocmask.h"
#include "../mach/lkm.h"
#include "../simple.h"
#include "lkm/api.h"
#include "fork.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/spawn.h>

#ifndef _POSIX_SPAWN_DISABLE_ASLR
#define _POSIX_SPAWN_DISABLE_ASLR 0x0100
#endif

#define LINUX_ADDR_NO_RANDOMIZE 0x40000


long sys_posix_spawn(int* pid, const char* path, const struct _posix_spawn_args_desc* desc,
		char** argvp, char** envp)
{
	int ret, my_pid;
	int pipe[2] = { -1, -1 };

	if (desc && desc->attrp && desc->attrp->psa_flags & POSIX_SPAWN_SETEXEC)
		goto no_fork;

	ret = LINUX_SYSCALL(__NR_pipe2, pipe, LINUX_O_CLOEXEC);
	if (ret < 0)
		return ret;

	if ((my_pid = sys_fork()) == 0)
	{
		mach_driver_init(NULL);

		// child
		// close the reading side
		close_internal(pipe[0]);

no_fork:
		if (desc && desc->attrp)
		{
			if (desc->attrp->psa_flags & POSIX_SPAWN_RESETIDS)
			{
				sys_setgid(sys_getgid());
				sys_setuid(sys_getuid());
			}
			if (desc->attrp->psa_flags & POSIX_SPAWN_SETPGROUP)
			{
				ret = sys_setpgid(0, desc->attrp->psa_pgroup);
				if (ret != 0)
					goto fail;
			}
			if (desc->attrp->psa_flags & POSIX_SPAWN_SETSIGMASK)
			{
				ret = sys_sigprocmask(3 /* SIG_SETMASK */, &desc->attrp->psa_sigmask, NULL);
				if (ret != 0)
					goto fail;
			}
			if (desc->attrp->psa_flags & _POSIX_SPAWN_DISABLE_ASLR)
			{
				unsigned int pers = LINUX_SYSCALL(__NR_personality, 0xffffffff);
				if (!(pers & LINUX_ADDR_NO_RANDOMIZE))
				{
					pers |= LINUX_ADDR_NO_RANDOMIZE;
					LINUX_SYSCALL(__NR_personality, pers);
				}
			}
			if (desc->attrp->psa_flags & POSIX_SPAWN_START_SUSPENDED)
			{
				lkm_call(NR_stop_after_exec, NULL);
			}

			// TODO: other attributes
		}
		if (desc && desc->factp)
		{
			int i;

			for (i = 0; i < desc->factp->psfa_act_count; i++)
			{
				const struct _psfa_action* act;

				act = &desc->factp->psfa_act_acts[i];

				if (act->psfaa_filedes == pipe[1])
				{
					ret = sys_dup(pipe[1]);
					if (ret < 0)
						goto fail;

					close_internal(pipe[1]);
					pipe[1] = ret;
				}

				switch (act->psfaa_type)
				{
					case PSFA_CLOSE:
						ret = close_internal(act->psfaa_filedes);
						if (ret != 0)
							goto fail;
						break;
					case PSFA_DUP2:
						ret = sys_dup2(act->psfaa_filedes, act->psfaa_openargs.psfao_oflag);
						if (ret < 0)
							goto fail;
						break;
					case PSFA_OPEN:
					{
						ret = sys_open(act->psfaa_openargs.psfao_path,
								act->psfaa_openargs.psfao_oflag, act->psfaa_openargs.psfao_mode);
						if (ret < 0)
							goto fail;

						if (ret != act->psfaa_filedes)
						{
							int fd = ret;
							ret = sys_dup2(ret, act->psfaa_filedes);

							if (ret < 0)
							{
								close_internal(fd);
								goto fail;
							}
						}
						break;
					}
					default:
						;
				}
			}
		}

		char binprefs[64];
		if (desc && desc->attrp && desc->attrp->psa_binprefs[0])
		{
			char** new_envp;
			int i, env_len = 0;

			while (envp[env_len])
				env_len++;
			env_len++; // terminating NULL

			// +1 for our new entry
			new_envp = (char**) __builtin_alloca((env_len + 1) * sizeof(char*));

			__simple_sprintf(binprefs, "__mldr_bprefs=%x,%x,%x,%x",
				desc->attrp->psa_binprefs[0],
				desc->attrp->psa_binprefs[1],
				desc->attrp->psa_binprefs[2],
				desc->attrp->psa_binprefs[3]);

			new_envp[0] = binprefs;
			for (i = 0; i < env_len; i++)
				new_envp[i+1] = envp[i];
			envp = new_envp;
		}

		ret = sys_execve((char*) path, argvp, envp);
fail:
		if (desc && desc->attrp && desc->attrp->psa_flags & POSIX_SPAWN_SETEXEC)
		{
			// no_fork case
			ret = errno_linux_to_bsd(ret);
			return ret;
		}
		else
		{
			sys_write(pipe[1], &ret, sizeof(ret));
			sys_exit(127);
			__builtin_unreachable();
		}
	}
	else
	{
		// parent
		int err;

		// close the writing side
		close_internal(pipe[1]);

		if (my_pid < 0)
		{
			ret = my_pid;
			goto out;
		}

		if (pid != NULL)
			*pid = my_pid;

		if (sys_read(pipe[0], &ret, sizeof(ret)) != sizeof(ret))
			ret = 0;
		close_internal(pipe[0]);
	}

out:
	return ret;
}
