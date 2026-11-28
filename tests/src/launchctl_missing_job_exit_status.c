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

/*
 * launchctl exit statuses for a job or file that does not exist, as
 * observed on macOS 26.7.3:
 *   list <label>                        exits 113
 *   start / stop / remove <label>       exit 3 (ESRCH)
 *   load / unload <missing file>        report "Load failed: 5: Input/output
 *                                       error" on stderr but exit 0
 *
 * Runs on macOS and in Darling; prints PASS and exits 0.
 */
#include <errno.h>
#include <fcntl.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

extern char **environ;

static int failures;

static int run(const char *a1, const char *a2, char *errbuf, size_t errlen)
{
	int errpipe[2];
	posix_spawn_file_actions_t fa;
	pid_t pid;
	int status = -1;
	char *argv[] = { "launchctl", (char *)a1, (char *)a2, NULL };

	if (pipe(errpipe) != 0)
		return -1;
	posix_spawn_file_actions_init(&fa);
	posix_spawn_file_actions_addopen(&fa, 1, "/dev/null", O_WRONLY, 0);
	posix_spawn_file_actions_adddup2(&fa, errpipe[1], 2);
	posix_spawn_file_actions_addclose(&fa, errpipe[0]);
	if (posix_spawn(&pid, "/bin/launchctl", &fa, NULL, argv, environ) != 0) {
		printf("FAIL cannot spawn /bin/launchctl: %s\n", strerror(errno));
		return -1;
	}
	close(errpipe[1]);
	size_t n = 0;
	ssize_t r;
	while (n + 1 < errlen && (r = read(errpipe[0], errbuf + n, errlen - 1 - n)) > 0)
		n += (size_t)r;
	errbuf[n] = 0;
	close(errpipe[0]);
	waitpid(pid, &status, 0);
	posix_spawn_file_actions_destroy(&fa);
	return WIFEXITED(status) ? WEXITSTATUS(status) : -2;
}

static void expect(const char *a1, const char *a2, int want, const char *want_msg)
{
	char err[512];
	int rc = run(a1, a2, err, sizeof(err));

	if (rc != want) {
		failures++;
		printf("FAIL launchctl %s %s: exit %d, expected %d (stderr: %s)\n", a1, a2, rc, want, err);
	} else if (want_msg != NULL && strstr(err, want_msg) == NULL) {
		failures++;
		printf("FAIL launchctl %s %s: stderr lacks \"%s\": %s\n", a1, a2, want_msg, err);
	}
}

int main(void)
{
	char label[128], file[160];

	snprintf(label, sizeof(label), "com.example.launchctl-exit-status.%d", (int)getpid());
	snprintf(file, sizeof(file), "/tmp/%s.plist", label);

	expect("list", label, 113, "Could not find service");
	expect("start", label, 3, NULL);
	expect("stop", label, 3, NULL);
	expect("remove", label, 3, NULL);
	expect("load", file, 0, "Load failed: 5: Input/output error");
	expect("unload", file, 0, "Unload failed: 5: Input/output error");

	if (failures == 0)
		printf("PASS\n");
	return failures != 0;
}
