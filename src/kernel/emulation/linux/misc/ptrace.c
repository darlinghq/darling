#include "ptrace.h"
#include "../base.h"
#include "../simple.h"
#include "../errno.h"
#include <sys/errno.h>
#include <stddef.h>
#include <linux-syscalls/linux.h>
#include <sys/signal.h>
#include "../signal/kill.h"
#include "../signal/duct_signals.h"

#define LINUX_PTRACE_TRACEME	0
#define LINUX_PTRACE_ATTACH	16
#define LINUX_PTRACE_DETACH	17
#define LINUX_PTRACE_CONT	7

long sys_ptrace(int request, int pid, void* addr, int data)
{
	const char* cmd = NULL;

	switch (request)
	{
		case PT_PTRACE_ME:
		{
			int ret = LINUX_SYSCALL(__NR_ptrace, LINUX_PTRACE_TRACEME, pid, addr, data);
			
			if (ret < 0)
				ret = errno_linux_to_bsd(ret);
			
			return ret;
			//cmd = "PT_PTRACE_ME"; break;
		}
		case PT_KILL:
			return sys_kill(pid, SIGKILL, 1);
			// cmd = "PT_KILL"; break;
		case PT_ATTACH: // PT_ATTACH
		case PT_ATTACHEXC: // PT_ATTACHEXC
		{
		//do_attach:
			int ret = LINUX_SYSCALL(__NR_ptrace, LINUX_PTRACE_ATTACH, pid, addr, data);
			
			if (ret < 0)
				ret = errno_linux_to_bsd(ret);
			
			return ret;
			// cmd = "PT_ATTACH"; break;
		}
		case PT_DETACH:
		{
			int ret = LINUX_SYSCALL(__NR_ptrace, LINUX_PTRACE_DETACH, pid, addr, data);
			
			if (ret < 0)
				ret = errno_linux_to_bsd(ret);
			
			return ret;
			// cmd = "PT_DETACH"; break;
		}
		case PT_SIGEXC:
			cmd = "PT_SIGEXC"; break;
		case PT_CONTINUE:
		// case PT_THUPDATE:
		{
			int signal = 0;
			if (data != -1)
				signal = signum_bsd_to_linux(data);
			int ret = LINUX_SYSCALL(__NR_ptrace, LINUX_PTRACE_CONT, pid, NULL, signal);
			
			if (ret < 0)
				ret = errno_linux_to_bsd(ret);
			
			return ret;
			// cmd = "PT_THUPDATE"; break;
		}
		//case 14:
		//{
		//	goto do_attach;
			// cmd = "PT_ATTACHEXC"; break;
		//}
	}

	if (cmd != NULL)
		__simple_printf("ptrace() req=%s\n", cmd);
	else
		__simple_printf("ptrace() req=%d\n", request);

	return -ENOSYS;
}

