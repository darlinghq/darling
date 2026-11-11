/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// posix_spawn and a forked child that talks to the kernel without exec'ing.
// On a host with Yama's ptrace_scope set to 1 both failed (EPERM from the
// child's first darlingserver call) before the native fork child renewed
// its ptracer exception; with ptrace_scope 0 this passes either way.
// Run on such a host: echo 1 > /proc/sys/kernel/yama/ptrace_scope (as root).

#include <errno.h>
#include <mach/mach.h>
#include <spawn.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

static int failures;

#define CHECK(cond) do { \
	if (!(cond)) { \
		printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); \
		failures++; \
	} \
} while (0)

int main(void)
{
	pid_t pid;
	int status;
	char* argv[] = { "/usr/bin/true", NULL };

	for (int i = 0; i < 3; i++)
	{
		CHECK(posix_spawn(&pid, argv[0], NULL, NULL, argv, environ) == 0);
		CHECK(waitpid(pid, &status, 0) == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0);
	}

	pid = fork();
	CHECK(pid >= 0);
	if (pid == 0)
	{
		// Mach calls that have darlingserver copy data out to us.
		task_basic_info_data_t info;
		mach_msg_type_number_t count = TASK_BASIC_INFO_COUNT;
		if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t) &info, &count) != KERN_SUCCESS)
			_exit(1);
		mach_port_t port;
		if (mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port) != KERN_SUCCESS)
			_exit(2);
		_exit(0);
	}
	CHECK(waitpid(pid, &status, 0) == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0);

	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
