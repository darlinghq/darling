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
#include "../signal/sigexc.h"
#include "../process/wait4.h"
#include "../mach/lkm.h"
#include "../../../../external/lkm/api.h"
#include "../unistd/getppid.h"

// faster than sys_getpid(), because it caches the PID
extern int getpid(void);

long sys_ptrace(int request, int pid, void* addr, int data)
{
	const char* cmd = NULL;
	int ret;

	switch (request)
	{
		case PT_PTRACE_ME:
		{
			// ret = LINUX_SYSCALL(__NR_ptrace, LINUX_PTRACE_TRACEME, pid, addr, data);
			
			// if (ret < 0)
			// 	ret = errno_linux_to_bsd(ret);

			// Use LKM mechanisms to set a tracing task (->disallow anyone else to attach)
			struct set_tracer_args args = {
				.target = 0,
				.tracer = sys_getppid()
			};
			ret = lkm_call(NR_set_tracer, &args);
			if (ret < 0)
				ret = errno_linux_to_bsd(ret);
			
			//return ret;
			cmd = "PT_PTRACE_ME"; break;
		}
		case PT_KILL:
		{
			ret = sys_kill(pid, SIGKILL, 1);
			// ret = LINUX_SYSCALL(__NR_ptrace, LINUX_PTRACE_KILL, pid, addr, data);
			// if (ret < 0)
			// 	ret = errno_linux_to_bsd(ret);

			// return ret;
			cmd = "PT_KILL"; break;
		}
		case PT_ATTACHEXC:
		{
			// Use LKM mechanisms to set a tracing task (->disallow anyone else to attach)
			struct set_tracer_args args = {
				.target = pid,
				.tracer = getpid()
			};
			ret = lkm_call(NR_set_tracer, &args);
			if (ret < 0)
				ret = errno_linux_to_bsd(ret);
			else
			{
				// This triggers darling_sigexc_self() in the remote process.
				ret = linux_sigqueue(pid, SIGNAL_SIGEXC_TOGGLE, SIGRT_MAGIC_ENABLE_SIGEXC);

				if (ret < 0)
					ret = errno_linux_to_bsd(ret);
				// This doesn't work if the process is stopped, e.g. when spawning a stopped
				// process via posix_spawn(). So now we have to resume the process so that
				// it gets the RT signal above and triggers a fake SIGSTOP on its own.
				// int pstate = lkm_call(NR_pid_get_state, (void*)(long) pid);
				// if (pstate & 4 /* __TASK_STOPPED */)
				// 	sys_kill(pid, SIGCONT, 1);
			}

			cmd = "PT_ATTACHEXC";
			break;
		}
		case PT_ATTACH:
		{
			/*
			ret = LINUX_SYSCALL(__NR_ptrace, LINUX_PTRACE_ATTACH, pid, addr, data);
			
			if (ret < 0)
			{
				ret = errno_linux_to_bsd(ret);
			}
			else
			{
				// Call wait4 to process the SIGSTOP that is sent by kernel on PTRACE_ATTACH
				// and make sure it is passed to the tracee (to get it sent back to us as a Mach message).
				sys_wait4(pid, NULL, DARLING_WAIT_NORESTART | BSD_WSTOPPED, NULL);
			}
			*/
			ret = -ENOSYS;
			
			//return ret;
			cmd = "PT_ATTACH"; break;
		}
		case PT_DETACH:
		{
			// Tell the tracee to restore original application signal handlers.
			linux_sigqueue(pid, SIGNAL_SIGEXC_TOGGLE, SIGRT_MAGIC_DISABLE_SIGEXC);

			ret = 0; //LINUX_SYSCALL(__NR_ptrace, LINUX_PTRACE_DETACH, pid, addr, data);
			
			// if (ret < 0)
			// 	ret = errno_linux_to_bsd(ret);
			
			//return ret;
			cmd = "PT_DETACH"; break;
		}
		case PT_SIGEXC:
		{
			lkm_call(0x1028, "sigexc: self via ptrace\n");
			darling_sigexc_self();
			ret = 0;

			// return ret;
			cmd = "PT_SIGEXC"; break;
		}
		case PT_CONTINUE:
		{
			ret = sys_kill(pid, SIGCONT, 1);

			/*
			int signal = 0;
			if (data != -1)
				signal = signum_bsd_to_linux(data);
			ret = LINUX_SYSCALL(__NR_ptrace, LINUX_PTRACE_CONT, pid, NULL, signal);
			
			if (ret < 0)
				ret = errno_linux_to_bsd(ret);
			*/
			
			// return ret;
			cmd = "PT_CONTINUE"; break;
		}
		case PT_THUPDATE:
		{
			// Convert thread_t to process ID
			int tid = lkm_call(NR_tid_for_thread, addr);
			if (tid < 0)
				return -ESRCH;

			int signal = 0;
			if (data != -1)
				signal = signum_bsd_to_linux(data);

			ret = linux_sigqueue_thread(pid, tid, SIGNAL_SIGEXC_THUPDATE, signal);
			if (ret < 0)
				ret = errno_linux_to_bsd(ret);

			// return ret;
			cmd = "PT_THUPDATE"; break;
		}
	}

	char buf[128];
	if (cmd != NULL)
		__simple_sprintf(buf, "ptrace() req=%s, ret=%d\n", cmd, ret);
	else
		__simple_sprintf(buf, "ptrace() req=%d\n", request);
	lkm_call(0x1028, buf);

	return ret;
}

