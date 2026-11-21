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

// proc_pidinfo(PROC_PIDUNIQIDENTIFIERINFO) reports the LC_UUID of the main
// executable of the target process in p_uuid (the value syslogd stores as the
// SenderMachUUID of a log message).  The expected value is read from the
// Mach-O image itself: from memory for this process, from the file for
// another executable.

#include <crt_externs.h>
#include <errno.h>
#include <fcntl.h>
#include <libproc.h>
#include <mach-o/dyld.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/proc_info.h>
#include <sys/wait.h>
#include <unistd.h>
#include <libkern/OSByteOrder.h>

extern char** environ;

// PROC_PIDUNIQIDENTIFIERINFO is private in the public SDK; the flavor number
// and the layout of its result come from the open-source sys/proc_info.h.
#define UNIQID_FLAVOR 17
struct uniqid_info {
	unsigned char p_uuid[16];
	uint64_t p_uniqueid;
	uint64_t p_puniqueid;
	int32_t p_idversion;
	uint32_t p_reserve2;
	uint64_t p_reserve3;
	uint64_t p_reserve4;
};

static int failures = 0;

static void check(const char* what, int ok)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

static int uuid_is_zero(const unsigned char u[16])
{
	for (int i = 0; i < 16; i++)
		if (u[i])
			return 0;
	return 1;
}

static const char* hex(const unsigned char u[16])
{
	static char buf[4][33];
	static int n;
	char* b = buf[n++ & 3];
	for (int i = 0; i < 16; i++)
		sprintf(b + 2 * i, "%02x", u[i]);
	return b;
}

// LC_UUID of the image whose mach_header starts at 'mh' (thin, host byte order).
static int uuid_of_image(const void* mh, size_t avail, unsigned char out[16])
{
	const struct mach_header* h = mh;
	size_t hsize;
	if (avail < sizeof(*h))
		return 0;
	if (h->magic == MH_MAGIC)
		hsize = sizeof(struct mach_header);
	else if (h->magic == MH_MAGIC_64)
		hsize = sizeof(struct mach_header_64);
	else
		return 0;
	const unsigned char* p = (const unsigned char*) mh + hsize;
	const unsigned char* end = (const unsigned char*) mh + avail;
	for (uint32_t i = 0; i < h->ncmds; i++)
	{
		const struct load_command* lc = (const struct load_command*) p;
		if ((size_t)(end - p) < sizeof(*lc) || lc->cmdsize < sizeof(*lc) || (size_t)(end - p) < lc->cmdsize)
			return 0;
		if (lc->cmd == LC_UUID && lc->cmdsize >= sizeof(struct uuid_command))
		{
			memcpy(out, ((const struct uuid_command*) lc)->uuid, 16);
			return 1;
		}
		p += lc->cmdsize;
	}
	return 0;
}

// LC_UUID of the slice of 'path' that matches this process's architecture.
static int uuid_of_file(const char* path, unsigned char out[16])
{
	int fd = open(path, O_RDONLY);
	if (fd < 0)
		return 0;
	size_t cap = 1 << 20, got = 0;
	unsigned char* buf = malloc(cap);
	ssize_t n;
	while (buf && got < cap && (n = read(fd, buf + got, cap - got)) > 0)
		got += n;
	close(fd);
	if (!buf)
		return 0;

	cpu_type_t want = _NSGetMachExecuteHeader()->cputype;
	int ok = 0;
	uint32_t magic = got >= 4 ? *(uint32_t*) buf : 0;
	if (magic == FAT_CIGAM && got >= sizeof(struct fat_header))
	{
		const struct fat_header* fh = (const struct fat_header*) buf;
		uint32_t cnt = OSSwapBigToHostInt32(fh->nfat_arch);
		const struct fat_arch* fa = (const struct fat_arch*)(buf + sizeof(*fh));
		for (uint32_t i = 0; i < cnt && (i + 1) * sizeof(*fa) + sizeof(*fh) <= got; i++)
		{
			if ((cpu_type_t) OSSwapBigToHostInt32(fa[i].cputype) != want)
				continue;
			uint32_t off = OSSwapBigToHostInt32(fa[i].offset);
			if (off < got)
				ok = uuid_of_image(buf + off, got - off, out);
			break;
		}
	}
	else
	{
		ok = uuid_of_image(buf, got, out);
	}
	free(buf);
	return ok;
}

static int pid_uuid(pid_t pid, unsigned char out[16])
{
	struct uniqid_info info;
	memset(&info, 0xa5, sizeof(info));
	int r = proc_pidinfo(pid, UNIQID_FLAVOR, 1, &info, sizeof(info));
	if (r != (int) sizeof(info))
		return 0;
	memcpy(out, info.p_uuid, 16);
	return 1;
}

static void wait_for_exit(pid_t pid)
{
	kill(pid, SIGKILL);
	waitpid(pid, NULL, 0);
}

int main(int argc, char** argv)
{
	if (argc > 1 && !strcmp(argv[1], "--sleep"))
	{
		sleep(30);
		return 0;
	}

	unsigned char expected[16], got[16];

	// The main executable's UUID from its in-memory header.
	const void* mh = _NSGetMachExecuteHeader();
	check("main executable has an LC_UUID", uuid_of_image(mh, 1 << 16, expected));

	// 1. self
	memset(got, 0, 16);
	check("proc_pidinfo PIDUNIQIDENTIFIERINFO succeeds for self", pid_uuid(getpid(), got));
	check("p_uuid of self is the main executable's LC_UUID", !memcmp(got, expected, 16));
	if (memcmp(got, expected, 16))
		printf("  got %s expected %s\n", hex(got), hex(expected));
	check("p_uuid of self is not all zeros", !uuid_is_zero(got));

	// the same file read back from disk agrees with the in-memory header
	char path[4096];
	uint32_t plen = sizeof(path);
	unsigned char fromfile[16];
	check("_NSGetExecutablePath", _NSGetExecutablePath(path, &plen) == 0);
	check("LC_UUID read from the executable file matches p_uuid",
	      uuid_of_file(path, fromfile) && !memcmp(fromfile, got, 16));

	// 2. a forked child that did not exec shares the parent's image
	int p[2];
	check("pipe", pipe(p) == 0);
	pid_t child = fork();
	if (child == 0)
	{
		char c;
		close(p[1]);
		read(p[0], &c, 1);
		_exit(0);
	}
	memset(got, 0, 16);
	check("proc_pidinfo succeeds for a fork child", pid_uuid(child, got));
	check("p_uuid of a fork child equals its parent's", !memcmp(got, expected, 16));
	write(p[1], "x", 1);
	waitpid(child, NULL, 0);
	close(p[0]);
	close(p[1]);

	// 3. a child running another image: a copy of this program started with
	//    posix_spawn reports the same UUID, and a different system executable
	//    reports that executable's own UUID.
	pid_t kid;
	char* const sargv[] = { argv[0], "--sleep", NULL };
	check("posix_spawn self", posix_spawn(&kid, path, NULL, NULL, sargv, environ) == 0);
	usleep(300000);
	memset(got, 0, 16);
	check("proc_pidinfo succeeds for a spawned child", pid_uuid(kid, got));
	check("p_uuid of a spawned copy of this program equals ours", !memcmp(got, expected, 16));
	wait_for_exit(kid);

	const char* other = "/bin/sleep";
	unsigned char otheruuid[16];
	if (uuid_of_file(other, otheruuid))
	{
		char* const oargv[] = { "sleep", "30", NULL };
		check("posix_spawn other executable", posix_spawn(&kid, other, NULL, NULL, oargv, environ) == 0);
		usleep(300000);
		memset(got, 0, 16);
		check("proc_pidinfo succeeds for the other executable", pid_uuid(kid, got));
		check("p_uuid is the other executable's LC_UUID", !memcmp(got, otheruuid, 16));
		if (memcmp(got, otheruuid, 16))
			printf("  got %s expected %s\n", hex(got), hex(otheruuid));
		check("the two executables have different UUIDs", memcmp(otheruuid, expected, 16) != 0);
		wait_for_exit(kid);
	}
	else
	{
		printf("SKIP: %s has no readable LC_UUID for this architecture\n", other);
	}

	// 4. a process that does not exist
	struct uniqid_info info;
	errno = 0;
	int r = proc_pidinfo(kid, UNIQID_FLAVOR, 1, &info, sizeof(info));
	check("proc_pidinfo of a reaped pid does not succeed", r != (int) sizeof(info));

	printf(failures ? "FAILED: %d check(s)\n" : "ALL PASSED\n", failures);
	return failures ? 1 : 0;
}
