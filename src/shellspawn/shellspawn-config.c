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

#include "shellspawn-config.h"

#include <darling/emulation/linux_premigration/ext/for-sysconfig.h>
#include <errno.h>
#include <os/log.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

// darling-prefsd normally pushes during the launchd bootstrap, before
// shellspawn is even loaded, so the wait returns at once. This bound only
// matters when darling-prefsd is missing or broken; it is well below the 60
// seconds the darling launcher waits for shellspawn's socket.
#define SYSTEM_CONFIG_WAIT_SECONDS 10

static volatile sig_atomic_t g_waitTimedOut;

static void onWaitTimer(int sig)
{
	(void) sig;
	g_waitTimedOut = 1;
}

void waitForSystemConfig(void)
{
	struct sigaction action;
	struct itimerval timer, oldTimer;
	int error;

	// No SA_RESTART: the timer must interrupt the wait
	memset(&action, 0, sizeof(action));
	action.sa_handler = onWaitTimer;
	sigemptyset(&action.sa_mask);
	sigaction(SIGALRM, &action, NULL);

	// The timer repeats every second after it first fires, so a signal that
	// arrives just before the wait is entered again cannot leave it blocked
	memset(&timer, 0, sizeof(timer));
	timer.it_value.tv_sec = SYSTEM_CONFIG_WAIT_SECONDS;
	timer.it_interval.tv_sec = 1;
	setitimer(ITIMER_REAL, &timer, &oldTimer);

	do
		error = __darling_wait_system_config();
	while (error == EINTR && !g_waitTimedOut);

	// The handler stays installed: a timer signal still on its way must not
	// end shellspawn. exec() resets it for the programs shellspawn starts.
	setitimer(ITIMER_REAL, &oldTimer, NULL);

	if (error == 0)
		return;

	if (g_waitTimedOut)
	{
		os_log_error(OS_LOG_DEFAULT, "shellspawn: no system configuration from darling-prefsd after %d seconds; using the defaults", SYSTEM_CONFIG_WAIT_SECONDS);
		fprintf(stderr, "shellspawn: no system configuration from darling-prefsd after %d seconds; using the defaults\n", SYSTEM_CONFIG_WAIT_SECONDS);
	}
	else
	{
		os_log_error(OS_LOG_DEFAULT, "shellspawn: cannot wait for the system configuration: %{public}s; using the defaults", strerror(error));
		fprintf(stderr, "shellspawn: cannot wait for the system configuration: %s; using the defaults\n", strerror(error));
	}
}
