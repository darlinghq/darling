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

// proc_listpids() and sysctl(KERN_PROC_*): every process is listed, the
// per-type filters work, and the size rules match macOS 27 (observed on
// arm64 and x86_64). A child running from an executable named "x) y" checks
// that /proc/<pid>/stat is parsed past a comm field containing ") ".

#include <errno.h>
#include <fcntl.h>
#include <libproc.h>
#include <signal.h>
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

static int contains(const int* pids, int n, int pid)
{
	for (int i = 0; i < n; i++)
		if (pids[i] == pid)
			return 1;
	return 0;
}

static int copy_file(const char* from, const char* to)
{
	char buf[65536];
	ssize_t n;
	int in = open(from, O_RDONLY);
	int out = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0755);
	if (in < 0 || out < 0)
		return -1;
	while ((n = read(in, buf, sizeof(buf))) > 0)
		if (write(out, buf, n) != n)
			return -1;
	close(in);
	close(out);
	return n;
}

int main(int argc, char** argv)
{
	if (argc > 1 && strcmp(argv[1], "sleep") == 0)
	{
		pause();
		return 0;
	}

	int size = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
	CHECK(size >= (int) (21 * sizeof(int)));
	CHECK(size % sizeof(int) == 0);

	int* pids = calloc(1, size);
	int n = proc_listpids(PROC_ALL_PIDS, 0, pids, size) / (int) sizeof(int);
	CHECK(n >= 2);
	CHECK(n <= size / (int) sizeof(int));
	CHECK(contains(pids, n, getpid()));
	CHECK(contains(pids, n, getppid()));

	// A buffer smaller than one pid is ENOMEM; a short one is truncated.
	errno = 0;
	CHECK(proc_listpids(PROC_ALL_PIDS, 0, pids, 2) == 0 && errno == ENOMEM);
	CHECK(proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(int)) == sizeof(int));

	CHECK(proc_listpids(PROC_KDBG_ONLY, 0, pids, size) == 0);
	CHECK(proc_listpids(99, 0, pids, size) == 0);

	n = proc_listpids(PROC_UID_ONLY, getuid(), pids, size) / (int) sizeof(int);
	CHECK(contains(pids, n, getpid()));
	n = proc_listpids(PROC_PGRP_ONLY, getpgrp(), pids, size) / (int) sizeof(int);
	CHECK(contains(pids, n, getpid()));

	// A child shows up under PROC_PPID_ONLY for its parent.
	char path[] = "/tmp/x) y";
	CHECK(copy_file(argv[0], path) == 0);
	pid_t child = fork();
	if (child == 0)
	{
		execl(path, path, "sleep", NULL);
		_exit(127);
	}
	CHECK(child > 0);
	usleep(500000);

	n = proc_listpids(PROC_PPID_ONLY, getpid(), pids, size) / (int) sizeof(int);
	CHECK(n == 1 && pids[0] == child);
	// The list grew with the child; size the buffer again so it fits.
	size = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0) + 8 * sizeof(int);
	pids = realloc(pids, size);
	n = proc_listpids(PROC_ALL_PIDS, 0, pids, size) / (int) sizeof(int);
	CHECK(contains(pids, n, child));

	// sysctl(KERN_PROC_PID) for the child: fields after comm are in place.
	int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, child };
	struct kinfo_proc kp;
	size_t len = sizeof(kp);
	memset(&kp, 0, sizeof(kp));
	CHECK(sysctl(mib, 4, &kp, &len, NULL, 0) == 0);
	CHECK(len == sizeof(kp));
	CHECK(kp.kp_proc.p_pid == child);
	CHECK(kp.kp_eproc.e_ppid == getpid());
	if (kp.kp_proc.p_comm[0] == 'x')
		CHECK(strcmp(kp.kp_proc.p_comm, "x) y") == 0);

	kill(child, SIGKILL);
	waitpid(child, NULL, 0);
	unlink(path);

	// KERN_PROC_ALL: a NULL buffer gives the size, a short one ENOMEM.
	mib[2] = KERN_PROC_ALL;
	len = 0;
	CHECK(sysctl(mib, 3, NULL, &len, NULL, 0) == 0);
	CHECK(len >= 2 * sizeof(struct kinfo_proc) && len % sizeof(struct kinfo_proc) == 0);
	len = 1000;
	CHECK(sysctl(mib, 3, NULL, &len, NULL, 0) == 0);
	CHECK(len >= 2 * sizeof(struct kinfo_proc));
	len = sizeof(kp);
	errno = 0;
	CHECK(sysctl(mib, 3, &kp, &len, NULL, 0) == -1 && errno == ENOMEM);

	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
