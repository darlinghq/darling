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
 * Thread creation: 1000 threads are created and joined one after another
 * (and 200 more in batches of 8 at a time), each one runs on a usable stack
 * and returns its value. The default stack size of a new thread stays
 * 512 KiB (the documented default of pthread_attr_init on macOS).
 */
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void watchdog(int sig)
{
	static const char msg[] = "FAIL: timed out\n";
	write(1, msg, sizeof(msg) - 1);
	_exit(1);
}

static void* body(void* arg)
{
	// touch 256 KiB of the stack to show it is really there
	volatile char buf[256 * 1024];

	for (size_t i = 0; i < sizeof(buf); i += 4096)
		buf[i] = (char) i;
	return (void*) ((uintptr_t) arg + 1 + (buf[0] & 0));
}

static void* stack_size_probe(void* arg)
{
	return (void*) pthread_get_stacksize_np(pthread_self());
}

int main(void)
{
	pthread_attr_t attr;
	size_t default_size = 0;
	pthread_t th, batch[8];
	void* ret;
	int ok = 1;

	signal(SIGALRM, watchdog);
	alarm(120);

	pthread_attr_init(&attr);
	pthread_attr_getstacksize(&attr, &default_size);
	if (default_size != 512 * 1024)
	{
		printf("FAIL: default stack size %zu\n", default_size);
		ok = 0;
	}

	if (pthread_create(&th, NULL, stack_size_probe, NULL) != 0 || pthread_join(th, &ret) != 0)
	{
		printf("FAIL: probe thread\n");
		return 1;
	}
	// libpthread may count the pthread object inside the allocation
	if ((size_t) ret < 508 * 1024 || (size_t) ret > 512 * 1024)
	{
		printf("FAIL: new thread stack size %zu\n", (size_t) ret);
		ok = 0;
	}

	for (uintptr_t i = 0; i < 1000; i++)
	{
		int e = pthread_create(&th, NULL, body, (void*) i);

		if (e != 0)
		{
			printf("FAIL: pthread_create #%lu: %d\n", (unsigned long) i, e);
			return 1;
		}
		pthread_join(th, &ret);
		if ((uintptr_t) ret != i + 1)
		{
			printf("FAIL: thread %lu returned %p\n", (unsigned long) i, ret);
			ok = 0;
		}
	}

	for (int round = 0; round < 25; round++)
	{
		for (int i = 0; i < 8; i++)
		{
			if (pthread_create(&batch[i], NULL, body, (void*) (uintptr_t) i) != 0)
			{
				printf("FAIL: batch create\n");
				return 1;
			}
		}
		for (int i = 0; i < 8; i++)
		{
			pthread_join(batch[i], &ret);
			if ((uintptr_t) ret != (uintptr_t) i + 1)
				ok = 0;
		}
	}

	printf("%s\n", ok ? "PASS" : "FAIL");
	return !ok;
}
