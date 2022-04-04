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
#include "../unistd/getppid.h"

#include <darlingserver/rpc.h>

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

			// Use darlingserver mechanisms to set a tracing task (->disallow anyone else to attach)
			ret = dserver_rpc_set_tracer(0, sys_getppid());
			if (ret < 0) {
				__simple_printf("dserver_rpc_set_tracer failed internally: %d", ret);
				__simple_abort();
			}
			ret = errno_linux_to_bsd(-ret);
			
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
			// Use darlingserver mechanisms to set a tracing task (->disallow anyone else to attach)
			ret = dserver_rpc_set_tracer(pid, getpid());
			if (ret < 0) {
				__simple_printf("dserver_rpc_set_tracer failed internally: %d", ret);
				__simple_abort();
			}
			ret = errno_linux_to_bsd(-ret);

			sys_kill(pid, SIGSTOP, 1);

			ret = dserver_rpc_ptrace_sigexc(pid, true);
			if (ret < 0) {
				__simple_printf("dserver_rpc_ptrace_sigexc failed internally: %d", ret);
				__simple_abort();
			}
			ret = errno_linux_to_bsd(-ret);

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
			//linux_sigqueue(pid, SIGNAL_SIGEXC_TOGGLE, SIGRT_MAGIC_DISABLE_SIGEXC);

			//ret = 0; //LINUX_SYSCALL(__NR_ptrace, LINUX_PTRACE_DETACH, pid, addr, data);

			ret = dserver_rpc_ptrace_sigexc(pid, false);
			if (ret < 0) {
				__simple_printf("dserver_rpc_ptrace_sigexc failed internally: %d", ret);
				__simple_abort();
			}
			ret = errno_linux_to_bsd(-ret);
			
			// if (ret < 0)
			// 	ret = errno_linux_to_bsd(ret);
			
			//return ret;
			cmd = "PT_DETACH"; break;
		}
		case PT_SIGEXC:
		{
			__simple_kprintf("sigexc: self via ptrace\n");

			ret = dserver_rpc_ptrace_sigexc(getpid(), true);
			if (ret < 0) {
				__simple_printf("dserver_rpc_ptrace_sigexc failed internally: %d", ret);
				__simple_abort();
			}
			ret = errno_linux_to_bsd(-ret);

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
			int tid = -1;
			ret = dserver_rpc_tid_for_thread(addr, &tid);
			if (ret < 0) {
				__simple_printf("dserver_rpc_tid_for_thread failed internally: %d", ret);
				__simple_abort();
			} else if (ret > 0) {
				ret = errno_linux_to_bsd(-ret);
				return ret;
			}

			ret = dserver_rpc_ptrace_thupdate(tid, data);
			if (ret < 0) {
				__simple_printf("dserver_rpc_ptrace_thupdate failed internally: %d", ret);
				__simple_abort();
			}
			ret = errno_linux_to_bsd(-ret);

			// return ret;
			cmd = "PT_THUPDATE"; break;
		}
	}

	if (cmd != NULL)
		__simple_kprintf("ptrace() req=%s, ret=%d\n", cmd, ret);
	else
		__simple_kprintf("ptrace() req=%d\n", request);

	return ret;
}

