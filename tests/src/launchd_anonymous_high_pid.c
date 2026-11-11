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

// A process that launchd did not start becomes an anonymous job on its first
// bootstrap request. That must work whatever its PID is. Under Darling, PIDs are
// Linux PIDs, which go above macOS's 99999; run this test once the container's
// PID counter has passed 100000 (see the PR's testing notes).
#include <mach/mach.h>
#include <servers/bootstrap.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
	mach_port_t port = MACH_PORT_NULL;
	pid_t pid = getpid();
	kern_return_t kr = bootstrap_look_up(bootstrap_port, "com.apple.system.notification_center", &port);

	if (kr != KERN_SUCCESS || port == MACH_PORT_NULL)
	{
		printf("FAIL: pid %d: bootstrap_look_up returned %d\n", pid, kr);
		return 1;
	}

	printf("PASS: pid %d%s\n", pid, pid < 100000 ? " (below 100000: the limit was not exercised)" : "");
	return 0;
}
