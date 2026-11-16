/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// GetFrontProcess() on a display server that cannot say which client has the input focus
// (Wayland, or no display at all): the answer is the process that most recently reported that
// it was activated, shared by all processes, through CoreGraphics' private
// _CGSSetFrontProcess()/_CGSGetFrontProcess(). The reporting process is a child of this one, as
// in the case of a backend reporting its own pid when a window gets the keyboard focus.
//
// With DISPLAY set the X11 backend answers from the window manager instead (the owner of
// _NET_ACTIVE_WINDOW); the test then only checks that GetFrontProcess still succeeds.

#include <ApplicationServices/ApplicationServices.h>
#include <dlfcn.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failures++; } } while (0)

typedef int32_t (*GetFrontFn)(pid_t *);
typedef int32_t (*SetFrontFn)(pid_t);

static pid_t frontPID(void) {
	ProcessSerialNumber psn = { 0, 0 };
	pid_t pid = -1;
	if (GetFrontProcess(&psn) != noErr || GetProcessPID(&psn, &pid) != noErr)
		return -1;
	return pid;
}

int main(int argc, char **argv) {
	// Asker mode: a freshly started process (not a fork: libnotify tokens of the parent are not
	// usable in a forked child) prints what GetFrontProcess() reports to the pipe fd in argv[2].
	if (argc == 3 && !strcmp(argv[1], "ask")) {
		pid_t seen = frontPID();
		(void) !write(atoi(argv[2]), &seen, sizeof(seen));
		return 0;
	}

	GetFrontFn getFront = (GetFrontFn) dlsym(RTLD_DEFAULT, "_CGSGetFrontProcess");
	SetFrontFn setFront = (SetFrontFn) dlsym(RTLD_DEFAULT, "_CGSSetFrontProcess");
	CHECK(getFront != NULL && setFront != NULL);
	if (!getFront || !setFront) {
		printf("FAIL\n");
		return 1;
	}

	CHECK(GetFrontProcess(NULL) == paramErr);
	CHECK(getFront(NULL) == 1001);  // kCGErrorIllegalArgument
	CHECK(setFront(0) == 1001);
	CHECK(setFront(-5) == 1001);
	CHECK(frontPID() > 0);

	if (getenv("DISPLAY") != NULL) {
		printf("DISPLAY is set: the X11 backend answers; round trip skipped\n");
		printf(failures ? "FAIL\n" : "PASS\n");
		return failures != 0;
	}

	pid_t child = fork();
	if (child == 0) {
		pause();
		_exit(0);
	}
	CHECK(child > 0);

	CHECK(setFront(child) == 0);
	pid_t got = 0;
	CHECK(getFront(&got) == 0 && got == child);
	CHECK(frontPID() == child);

	// The same answer for a process that did not set it: ask from a second, separately started process.
	int fds[2];
	CHECK(pipe(fds) == 0);
	pid_t asker = fork();
	if (asker == 0) {
		char fdArg[16];
		snprintf(fdArg, sizeof(fdArg), "%d", fds[1]);
		execl(argv[0], argv[0], "ask", fdArg, (char *) NULL);
		_exit(127);
	}
	pid_t seen = -1;
	CHECK(read(fds[0], &seen, sizeof(seen)) == (ssize_t) sizeof(seen) && seen == child);
	waitpid(asker, NULL, 0);

	// A process that is gone is not reported: the fallback is this process or the last
	// SetFrontProcess() target, never the stale pid.
	kill(child, SIGKILL);
	waitpid(child, NULL, 0);
	CHECK(frontPID() != child);
	CHECK(frontPID() > 0);

	// Reporting this process makes it the front process.
	CHECK(setFront(getpid()) == 0);
	CHECK(frontPID() == getpid());

	printf(failures ? "FAIL\n" : "PASS\n");
	return failures != 0;
}
