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

// diskarbitrationd: keeps the disks (DAModel.h) and serves the DiskArbitration library (DAServer.h).
#include <dispatch/dispatch.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/event.h>
#include <sys/mount.h>

#include "DAModel.h"
#include "DAServer.h"

static dispatch_queue_t g_queue;

// Rescans after every change to the mount table, which kqueue reports as an EVFILT_FS event.
static void* watchMounts(void* unused)
{
	int kq = kqueue();
	struct kevent event;

	if (kq < 0)
	{
		fprintf(stderr, "diskarbitrationd: kqueue: %s\n", strerror(errno));
		return NULL;
	}
	EV_SET(&event, 0, EVFILT_FS, EV_ADD | EV_CLEAR, 0, 0, NULL);
	if (kevent(kq, &event, 1, NULL, 0, NULL) != 0)
	{
		fprintf(stderr, "diskarbitrationd: cannot watch the mount table: %s\n", strerror(errno));
		return NULL;
	}

	for (;;)
	{
		int count = kevent(kq, NULL, 0, &event, 1, NULL);
		if (count > 0)
		{
			dispatch_async(g_queue, ^{
				DAModelRescan(DAServerEvents());
			});
		}
		else if (count < 0 && errno != EINTR)
		{
			fprintf(stderr, "diskarbitrationd: kevent: %s\n", strerror(errno));
			return NULL;
		}
	}
}

int main(void)
{
	pthread_t watcher;

	g_queue = dispatch_queue_create("com.apple.DiskArbitration.diskarbitrationd", NULL);

	dispatch_sync(g_queue, ^{
		DAModelRescan(DAServerEvents());
	});
	DAServerStart(g_queue);

	if (pthread_create(&watcher, NULL, watchMounts, NULL) == 0)
		pthread_detach(watcher);

	dispatch_main();
	return 0;
}
