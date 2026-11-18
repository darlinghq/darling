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

// Process-name reporting: proc_name(), PROC_PIDTBSDINFO, PROC_PIDT_SHORTBSDINFO and
// sysctl(KERN_PROC_PID) report the executable's basename (p_comm is truncated to
// MAXCOMLEN, pbi_name to 2 * MAXCOMLEN - 1), for the process itself, for a forked
// child (which inherits the name) and for an exec'd child.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <libproc.h>
#include <sys/sysctl.h>
#include <sys/proc_info.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <mach-o/dyld.h>

static int failures;
static int verbose;

#define CHECK(what, got, want) do { \
	if (verbose) printf("%-28s \"%s\" (len %zu)\n", what, got, strlen(got)); \
	if (strcmp(got, want) != 0) { printf("FAIL %s: got \"%s\", want \"%s\"\n", what, got, want); failures++; } \
} while (0)

static void trunc_to(char* out, const char* in, size_t n) { strncpy(out, in, n); out[n] = 0; }

static void check_pid(const char* tag, pid_t pid, const char* base) {
	char want[64], what[96], got[256];
	int n;

	trunc_to(want, base, 16); /* MAXCOMLEN */
	{
		struct kinfo_proc kp; size_t len = sizeof(kp);
		int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, pid };
		memset(&kp, 0, sizeof(kp));
		if (sysctl(mib, 4, &kp, &len, NULL, 0) != 0 || len < sizeof(kp)) {
			printf("FAIL %s: sysctl KERN_PROC_PID: %s\n", tag, strerror(errno)); failures++;
		} else {
			snprintf(what, sizeof(what), "%s p_comm", tag);
			CHECK(what, kp.kp_proc.p_comm, want);
		}
	}
	{
		struct proc_bsdshortinfo si;
		n = proc_pidinfo(pid, PROC_PIDT_SHORTBSDINFO, 0, &si, sizeof(si));
		if (n != (int)sizeof(si)) { printf("FAIL %s: SHORTBSDINFO returned %d\n", tag, n); failures++; }
		else {
			char c[17]; memcpy(c, si.pbsi_comm, 16); c[16] = 0;
			trunc_to(want, base, 15);
			if (verbose) printf("%s pbsi_comm \"%s\"\n", tag, c);
			snprintf(what, sizeof(what), "%s pbsi_comm", tag);
			if (strcmp(c, want) != 0 && strncmp(c, want, 15) != 0) { printf("FAIL %s: got \"%s\" want \"%s\"\n", what, c, want); failures++; }
		}
	}
	{
		struct proc_bsdinfo bi;
		n = proc_pidinfo(pid, PROC_PIDTBSDINFO, 0, &bi, sizeof(bi));
		if (n != (int)sizeof(bi)) { printf("FAIL %s: TBSDINFO returned %d\n", tag, n); failures++; }
		else {
			char c[17], nm[33]; memcpy(c, bi.pbi_comm, 16); c[16] = 0; memcpy(nm, bi.pbi_name, 32); nm[32] = 0;
			if (verbose) printf("%s pbi_comm \"%s\" pbi_name \"%s\"\n", tag, c, nm);
			trunc_to(want, base, 15);
			if (strncmp(c, want, 15) != 0) { printf("FAIL %s pbi_comm: got \"%s\" want \"%s\"\n", tag, c, want); failures++; }
			trunc_to(want, base, 31);
			snprintf(what, sizeof(what), "%s pbi_name", tag);
			if (strcmp(nm, want) != 0) { printf("FAIL %s: got \"%s\" want \"%s\"\n", what, nm, want); failures++; }
		}
	}
	n = proc_name(pid, got, sizeof(got));
	trunc_to(want, base, 31);
	snprintf(what, sizeof(what), "%s proc_name", tag);
	if (n <= 0) { printf("FAIL %s returned %d\n", what, n); failures++; } else { got[n] = 0; CHECK(what, got, want); }
}

static const char* base_of(const char* p) { const char* b = strrchr(p, '/'); return b ? b + 1 : p; }

static void run_child(const char* tag, const char* exe, int do_exec, const char* base) {
	int fds[2]; pid_t pid;
	if (pipe(fds)) { perror("pipe"); failures++; return; }
	pid = fork();
	if (pid == 0) {
		close(fds[1]);
		if (do_exec) { char fd[16]; snprintf(fd, sizeof(fd), "%d", fds[0]); execl(exe, exe, "--wait", fd, (char*)NULL); _exit(127); }
		char c; read(fds[0], &c, 1); _exit(0);
	}
	close(fds[0]);
	usleep(300000); /* let the child reach its blocking read */
	check_pid(tag, pid, base);
	close(fds[1]);
	waitpid(pid, NULL, 0);
}

int main(int argc, char** argv) {
	char path[1024], dir[256], copy[1100];
	uint32_t sz = sizeof(path);
	if (argc == 3 && !strcmp(argv[1], "--wait")) { char c; read(atoi(argv[2]), &c, 1); return 0; }
	verbose = getenv("VERBOSE") != NULL;
	if (_NSGetExecutablePath(path, &sz)) return 2;
	{ char* r = realpath(path, NULL); if (r) { strlcpy(path, r, sizeof(path)); free(r); } }

	check_pid("self", getpid(), base_of(path));
	run_child("forked", path, 0, base_of(path));

	/* an exec'd child takes the name of the file that was exec'd, not argv[0] */
	snprintf(dir, sizeof(dir), "%s/proc-name-test.XXXXXX", getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp");
	if (!mkdtemp(dir)) { perror("mkdtemp"); return 2; }
	const char* longname = "a-rather-long-executable-name-for-p_comm";
	snprintf(copy, sizeof(copy), "%s/%s", dir, longname);
	{
		FILE* in = fopen(path, "rb"); FILE* out = fopen(copy, "wb"); char buf[65536]; size_t n;
		if (!in || !out) { perror("copy"); return 2; }
		while ((n = fread(buf, 1, sizeof(buf), in)) > 0) fwrite(buf, 1, n, out);
		fclose(in); fclose(out); chmod(copy, 0755);
	}
	run_child("exec-long", copy, 1, longname);
	unlink(copy); rmdir(dir);

	printf(failures ? "FAIL\n" : "PASS\n");
	return failures != 0;
}
