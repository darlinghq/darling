/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * libproc / posix_spawn calls built on process_policy(2) and proc_info:
 *  - proc_set_cpumon_params / proc_get_cpumon_params / proc_disable_cpumon / proc_set_cpumon_defaults /
 *    proc_resume_cpumon (process_policy syscall; macOS: a set is read back, nothing set reads as 0, 0, an
 *    unknown pid is ESRCH),
 *  - proc_terminate (proc_info PROC_INFO_CALL_TERMINATE; macOS 26.7.3: a child gets SIGTERM and the call reports
 *    15, the caller's own pid is EPERM, an unknown pid ESRCH),
 *  - posix_spawnattr_set_importancewatch_port_np takes 128 ports (macOS 26.7.3: 128 accepted, 129 is EINVAL).
 * Before the implementation process_policy and proc_terminate failed with ENOSYS / ENOTSUP and the port limit was 32.
 */
#include <errno.h>
#include <mach/mach.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern int proc_set_cpumon_params(pid_t pid, int percentage, int interval);
extern int proc_get_cpumon_params(pid_t pid, int* percentage, int* interval);
extern int proc_set_cpumon_defaults(pid_t pid);
extern int proc_resume_cpumon(pid_t pid);
extern int proc_disable_cpumon(pid_t pid);
extern int proc_terminate(pid_t pid, int* sig);
extern int posix_spawnattr_set_importancewatch_port_np(posix_spawnattr_t* attr, int count, mach_port_t portarray[]);

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); failures++; } } while (0)

int main(void)
{
	pid_t me = getpid();
	int pct = -1, interval = -1;

	// nothing set: all zero
	CHECK(proc_get_cpumon_params(me, &pct, &interval) == 0);
	CHECK(pct == 0 && interval == 0);
	CHECK(proc_set_cpumon_params(me, 50, 300) == 0);
	CHECK(proc_get_cpumon_params(me, &pct, &interval) == 0);
	CHECK(pct == 50 && interval == 300);
	// a monitor with default or disabled parameters is accepted too
	CHECK(proc_set_cpumon_defaults(me) == 0);
	CHECK(proc_resume_cpumon(me) == 0);
	CHECK(proc_disable_cpumon(me) == 0);
	// the calling process may also be addressed as pid 0 for a get
	CHECK(proc_set_cpumon_params(me, 20, 60) == 0);
	CHECK(proc_get_cpumon_params(0, &pct, &interval) == 0);
	CHECK(pct == 20 && interval == 60);
	CHECK(proc_disable_cpumon(me) == 0);
	// an unknown process
	CHECK(proc_set_cpumon_params(0x3fffffff, 50, 300) == -1 && errno == ESRCH);

	// proc_terminate: the child dies of the reported signal
	pid_t child = fork();
	if (child == 0)
	{
		for (;;)
			pause();
	}
	CHECK(child > 0);
	int sig = 0;
	CHECK(proc_terminate(child, &sig) == 0);
	CHECK(sig == SIGTERM);
	int status = 0;
	CHECK(waitpid(child, &status, 0) == child);
	CHECK(WIFSIGNALED(status) && WTERMSIG(status) == SIGTERM);
	CHECK(proc_terminate(me, &sig) == EPERM);
	CHECK(proc_terminate(0x3fffffff, &sig) == ESRCH);
	CHECK(proc_terminate(child, NULL) == EINVAL);

	// importance watch ports: up to 128
	posix_spawnattr_t attr;
	CHECK(posix_spawnattr_init(&attr) == 0);
	mach_port_t ports[130];
	memset(ports, 0, sizeof ports);
	CHECK(posix_spawnattr_set_importancewatch_port_np(&attr, 32, ports) == 0);
	CHECK(posix_spawnattr_set_importancewatch_port_np(&attr, 128, ports) == 0);
	CHECK(posix_spawnattr_set_importancewatch_port_np(&attr, 129, ports) == EINVAL);
	posix_spawnattr_destroy(&attr);

	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	return failures != 0;
}
