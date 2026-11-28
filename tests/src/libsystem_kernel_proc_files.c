/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * /proc/<pid>/<file> in the guest, and the proc_info calls that read it:
 *  - /proc/<pid>/stat, cmdline, status, statm, comm, environ and auxv of the calling process and of its parent open
 *    and read back through the guest's path lookup, however the prefix's /proc is set up (a procfs mounted on it, or
 *    in the NonRoot mode a link to the host's /proc);
 *  - the same files reached through /proc/self and through a relative path from /proc;
 *  - proc_pidinfo(PROC_PIDUNIQIDENTIFIERINFO), which derives a process's unique id from its stat file (the start time
 *    in the high bits, the pid in the low 16, as xnu's p_uniqueid is built), for the process, its parent and a child,
 *    and ESRCH for a pid that does not exist.
 */
#include <errno.h>
#include <fcntl.h>
#include <libproc.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/proc_info.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// struct proc_uniqidentifierinfo of <sys/proc_info.h> (flavor 17), which the SDK headers do not expose to a plain program
#ifndef PROC_PIDUNIQIDENTIFIERINFO
#define PROC_PIDUNIQIDENTIFIERINFO 17
#endif
struct uniqidentifierinfo
{
	uint8_t p_uuid[16];
	uint64_t p_uniqueid;
	uint64_t p_puniqueid;
	int32_t p_idversion;
	uint32_t p_reserve2;
	uint64_t p_reserve3;
	uint64_t p_reserve4;
};

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); failures++; } } while (0)

static int read_proc_file(const char* path, char* buf, size_t size)
{
	int fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;
	ssize_t n = read(fd, buf, size - 1);
	close(fd);
	if (n < 0)
		return -1;
	buf[n] = '\0';
	return (int) n;
}

// "<pid> (<comm>) <state> <ppid> ..." of a stat file
static int stat_pid_ppid(const char* text, int* pid, int* ppid)
{
	const char* close_paren = strrchr(text, ')');
	if (!close_paren)
		return -1;
	char state;
	if (sscanf(text, "%d", pid) != 1 || sscanf(close_paren + 1, " %c %d", &state, ppid) != 2)
		return -1;
	return 0;
}

static void check_files_of(pid_t pid, int expect_ppid)
{
	static const char* const files[] = { "stat", "cmdline", "status", "statm", "comm", "environ", "auxv" };
	char path[128], buf[8192];

	for (unsigned i = 0; i < sizeof(files) / sizeof(files[0]); i++)
	{
		snprintf(path, sizeof(path), "/proc/%d/%s", pid, files[i]);
		int n = read_proc_file(path, buf, sizeof(buf));
		if (n < 0)
		{
			printf("FAIL cannot read %s (errno %d)\n", path, errno);
			failures++;
			continue;
		}
		if (strcmp(files[i], "stat") == 0)
		{
			int got_pid = -1, got_ppid = -1;
			CHECK(stat_pid_ppid(buf, &got_pid, &got_ppid) == 0);
			CHECK(got_pid == pid);
			if (expect_ppid >= 0)
				CHECK(got_ppid == expect_ppid);
		}
		else if (strcmp(files[i], "status") == 0)
		{
			char want[32];
			snprintf(want, sizeof(want), "\nPid:\t%d\n", pid);
			CHECK(strstr(buf, want) != NULL);
		}
		else if (strcmp(files[i], "comm") == 0 || strcmp(files[i], "statm") == 0)
			CHECK(n > 0);
	}
}

int main(void)
{
	pid_t me = getpid(), parent = getppid();
	char buf[8192], path[128];

	// the process's own files, by pid and by /proc/self
	check_files_of(me, parent);
	CHECK(read_proc_file("/proc/self/stat", buf, sizeof(buf)) > 0);
	int pid = -1, ppid = -1;
	CHECK(stat_pid_ppid(buf, &pid, &ppid) == 0 && pid == me && ppid == parent);
	CHECK(read_proc_file("/proc/self/cmdline", buf, sizeof(buf)) > 0);

	// the parent's
	if (parent > 1)
		check_files_of(parent, -1);

	// a relative path from /proc
	{
		char rel[64];
		int dfd = open("/proc", O_RDONLY | O_DIRECTORY);
		CHECK(dfd >= 0);
		snprintf(rel, sizeof(rel), "%d/stat", me);
		int fd = openat(dfd, rel, O_RDONLY);
		CHECK(fd >= 0);
		if (fd >= 0)
			close(fd);
		close(dfd);
		CHECK(chdir("/proc") == 0);
		CHECK(read_proc_file(rel, buf, sizeof(buf)) > 0);
		CHECK(chdir("/") == 0);
	}

	// a pid that does not exist
	CHECK(read_proc_file("/proc/2147483646/stat", buf, sizeof(buf)) < 0 && errno == ENOENT);

	// proc_pidinfo(PROC_PIDUNIQIDENTIFIERINFO): the unique ids of the process and of its parent
	struct uniqidentifierinfo mine, par, other;
	memset(&mine, 0, sizeof(mine));
	int n = proc_pidinfo(me, PROC_PIDUNIQIDENTIFIERINFO, 0, &mine, sizeof(mine));
	CHECK(n == (int) sizeof(mine));
	CHECK((mine.p_uniqueid & 0xffff) == (unsigned long long) (me & 0xffff));
	CHECK((mine.p_uniqueid >> 16) != 0);
	if (parent > 1)
	{
		memset(&par, 0, sizeof(par));
		n = proc_pidinfo(parent, PROC_PIDUNIQIDENTIFIERINFO, 0, &par, sizeof(par));
		CHECK(n == (int) sizeof(par));
		CHECK((par.p_uniqueid & 0xffff) == (unsigned long long) (parent & 0xffff));
		CHECK(mine.p_puniqueid == par.p_uniqueid);
	}

	// a child's: its parent is us, and a second query gives the same id
	pid_t child = fork();
	if (child == 0)
	{
		for (;;)
			pause();
	}
	CHECK(child > 0);
	memset(&other, 0, sizeof(other));
	n = proc_pidinfo(child, PROC_PIDUNIQIDENTIFIERINFO, 0, &other, sizeof(other));
	CHECK(n == (int) sizeof(other));
	CHECK((other.p_uniqueid & 0xffff) == (unsigned long long) (child & 0xffff));
	CHECK(other.p_puniqueid == mine.p_uniqueid);
	CHECK(other.p_uniqueid != mine.p_uniqueid);
	struct uniqidentifierinfo again;
	memset(&again, 0, sizeof(again));
	CHECK(proc_pidinfo(child, PROC_PIDUNIQIDENTIFIERINFO, 0, &again, sizeof(again)) == (int) sizeof(again));
	CHECK(again.p_uniqueid == other.p_uniqueid);
	snprintf(path, sizeof(path), "/proc/%d/stat", child);
	CHECK(read_proc_file(path, buf, sizeof(buf)) > 0);
	kill(child, SIGKILL);
	waitpid(child, NULL, 0);

	// a pid that does not exist
	struct uniqidentifierinfo none;
	errno = 0;
	CHECK(proc_pidinfo(2147483646, PROC_PIDUNIQIDENTIFIERINFO, 0, &none, sizeof(none)) <= 0);
	CHECK(errno == ESRCH);

	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	return failures != 0;
}
