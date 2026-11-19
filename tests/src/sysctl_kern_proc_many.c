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

// sysctl(CTL_KERN, KERN_PROC, ...) must report every live process, however
// many there are: KERN_PROC_ALL lists them all, its size query covers them,
// and KERN_PROC_PID finds each one (including the one with the highest pid).
// A process that exits while the table is being read must not make other
// processes disappear from the listing.

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define NCHILDREN 120

static int failures;

#define CHECK(cond, ...) do { \
	if (!(cond)) { \
		printf("FAIL: "); printf(__VA_ARGS__); printf("\n"); \
		failures++; \
	} \
} while (0)

static struct kinfo_proc* list_all(size_t* count)
{
	int mib[3] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL };
	struct kinfo_proc* buf = NULL;

	for (int attempt = 0; attempt < 10; attempt++)
	{
		size_t len = 0;
		if (sysctl(mib, 3, NULL, &len, NULL, 0) != 0)
			return NULL;

		len += len / 2 + 16 * sizeof(struct kinfo_proc);
		buf = realloc(buf, len);
		if (sysctl(mib, 3, buf, &len, NULL, 0) == 0)
		{
			*count = len / sizeof(struct kinfo_proc);
			return buf;
		}
		if (errno != ENOMEM)
			break;
	}
	free(buf);
	return NULL;
}

static int find_pid(const struct kinfo_proc* procs, size_t count, pid_t pid)
{
	for (size_t i = 0; i < count; i++)
	{
		if (procs[i].kp_proc.p_pid == pid)
			return 1;
	}
	return 0;
}

static int lookup_pid(pid_t pid)
{
	int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, pid };
	struct kinfo_proc kp;
	size_t len = sizeof(kp);

	memset(&kp, 0, sizeof(kp));
	if (sysctl(mib, 4, &kp, &len, NULL, 0) != 0)
		return 0;
	return len >= sizeof(kp) && kp.kp_proc.p_pid == pid;
}

int main(void)
{
	pid_t kids[NCHILDREN];
	pid_t churn;
	pid_t highest = 0;

	for (int i = 0; i < NCHILDREN; i++)
	{
		kids[i] = fork();
		if (kids[i] < 0)
		{
			printf("FAIL: fork: %s\n", strerror(errno));
			return 1;
		}
		if (kids[i] == 0)
		{
			for (;;)
				pause();
		}
		if (kids[i] > highest)
			highest = kids[i];
	}

	// A child that keeps creating and reaping short-lived processes, so
	// that entries appear and vanish while this process reads the table.
	churn = fork();
	if (churn == 0)
	{
		for (;;)
		{
			pid_t p = fork();
			if (p == 0)
				_exit(0);
			if (p > 0)
				waitpid(p, NULL, 0);
		}
	}
	if (churn > highest)
		highest = churn;

	size_t count = 0;
	struct kinfo_proc* procs = list_all(&count);
	CHECK(procs != NULL, "KERN_PROC_ALL failed: %s", strerror(errno));

	int mib[3] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL };
	size_t sizelen = 0;
	CHECK(sysctl(mib, 3, NULL, &sizelen, NULL, 0) == 0, "size query failed");
	CHECK(sizelen / sizeof(struct kinfo_proc) >= NCHILDREN + 1,
		"size query covers %zu records, expected at least %d",
		sizelen / sizeof(struct kinfo_proc), NCHILDREN + 1);

	if (procs)
	{
		CHECK(count >= NCHILDREN + 1, "listing has %zu records, expected at least %d",
			count, NCHILDREN + 1);
		CHECK(find_pid(procs, count, getpid()), "own pid %d missing from the listing", getpid());

		int missing = 0;
		for (int i = 0; i < NCHILDREN; i++)
		{
			if (!find_pid(procs, count, kids[i]))
				missing++;
		}
		CHECK(missing == 0, "%d of %d children missing from KERN_PROC_ALL", missing, NCHILDREN);
		CHECK(find_pid(procs, count, highest), "highest pid %d missing from the listing", highest);
		free(procs);
	}

	// Repeat while the churn child exits processes: the long-lived children
	// must be listed every time.
	for (int round = 0; round < 20; round++)
	{
		procs = list_all(&count);
		if (!procs)
		{
			CHECK(0, "KERN_PROC_ALL failed in round %d", round);
			break;
		}
		int missing = 0;
		for (int i = 0; i < NCHILDREN; i++)
		{
			if (!find_pid(procs, count, kids[i]))
				missing++;
		}
		CHECK(missing == 0, "round %d: %d children missing while processes exit", round, missing);
		free(procs);
	}

	int pid_missing = 0;
	for (int i = 0; i < NCHILDREN; i++)
	{
		if (!lookup_pid(kids[i]))
			pid_missing++;
	}
	CHECK(pid_missing == 0, "KERN_PROC_PID found no record for %d of %d children", pid_missing, NCHILDREN);
	CHECK(lookup_pid(highest), "KERN_PROC_PID found no record for the highest pid %d", highest);
	CHECK(lookup_pid(getpid()), "KERN_PROC_PID found no record for the own pid");

	kill(churn, SIGKILL);
	waitpid(churn, NULL, 0);
	for (int i = 0; i < NCHILDREN; i++)
	{
		kill(kids[i], SIGKILL);
		waitpid(kids[i], NULL, 0);
	}

	if (failures)
		return 1;
	printf("PASS\n");
	return 0;
}
