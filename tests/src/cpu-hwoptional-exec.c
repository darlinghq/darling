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

// pthread_cpu_number_np() reports real CPU numbers, the arm64 hw.optional
// sysctls exist as integers, and a program that marks every descriptor
// close-on-exec can still exec another program.

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysctl.h>
#include <sys/wait.h>
#include <unistd.h>

static int failures;

#define CHECK(cond) do { \
	if (!(cond)) { \
		printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); \
		failures++; \
	} \
} while (0)

static int ncpu;
static volatile int nonzero_seen, out_of_range;

static void* sample_cpu(void* arg)
{
	(void) arg;
	for (int i = 0; i < 200000; i++)
	{
		size_t cpu = (size_t) -1;
		if (pthread_cpu_number_np(&cpu) != 0 || cpu >= (size_t) ncpu)
			out_of_range = 1;
		else if (cpu != 0)
			nonzero_seen = 1;
	}
	return NULL;
}

static void check_cpu_number(void)
{
	size_t len = sizeof(ncpu);
	CHECK(sysctlbyname("hw.ncpu", &ncpu, &len, NULL, 0) == 0);
	if (ncpu < 2)
	{
		puts("SKIP: one CPU, CPU numbers not checked");
		return;
	}

	// With every CPU busy, some thread runs on a CPU other than 0.
	pthread_t threads[64];
	int count = ncpu < 64 ? ncpu : 64;
	for (int i = 0; i < count; i++)
		pthread_create(&threads[i], NULL, sample_cpu, NULL);
	for (int i = 0; i < count; i++)
		pthread_join(threads[i], NULL);
	CHECK(!out_of_range);
	CHECK(nonzero_seen);
}

static void check_hw_optional(void)
{
#if defined(__arm64__)
	static const char* names[] = {
		"hw.optional.floatingpoint", "hw.optional.neon", "hw.optional.armv8_1_atomics",
		"hw.optional.armv8_crc32", "hw.optional.arm64", "hw.optional.arm.FEAT_LSE",
		"hw.optional.arm.FEAT_CRC32", "hw.optional.arm.FEAT_AES", "hw.optional.arm.FEAT_SHA256",
		"hw.optional.arm.FEAT_SSBS", "hw.optional.arm.FEAT_BTI",
	};
	for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++)
	{
		int value = -1;
		size_t len = sizeof(value);
		errno = 0;
		if (sysctlbyname(names[i], &value, &len, NULL, 0) != 0 || len != sizeof(int) || (value != 0 && value != 1))
		{
			printf("FAIL: %s (errno %d, len %zu, value %d)\n", names[i], errno, len, value);
			failures++;
		}
	}

	int value = 0;
	size_t len = sizeof(value);
	CHECK(sysctlbyname("hw.optional.arm64", &value, &len, NULL, 0) == 0 && value == 1);
	CHECK(sysctlbyname("hw.optional.floatingpoint", &value, &len, NULL, 0) == 0 && value == 1);
	CHECK(sysctlbyname("hw.optional.neon", &value, &len, NULL, 0) == 0 && value == 1);
#endif
}

static void check_exec_after_cloexec(const char* self)
{
	pid_t pid = fork();
	CHECK(pid >= 0);
	if (pid == 0)
	{
		// Mark every descriptor close-on-exec, as some programs do before exec.
		for (int fd = 3; fd < 1024; fd++)
		{
			int flags = fcntl(fd, F_GETFD);
			if (flags >= 0)
				fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
		}
		execl(self, self, "exec-child", (char*) NULL);
		_exit(127);
	}
	int status;
	CHECK(waitpid(pid, &status, 0) == pid);
	CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 42);
}

int main(int argc, char** argv)
{
	if (argc > 1 && strcmp(argv[1], "exec-child") == 0)
		return 42;

	check_cpu_number();
	check_hw_optional();
	check_exec_after_cloexec(argv[0]);

	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
