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
 * libdispatch stress: work spread over the global queues of every QoS, a
 * set of serial queues and dispatch_apply, nested several levels deep and
 * repeated, with short sleeps so workers park and get reused. Every block
 * must run exactly once.
 */
#include <dispatch/dispatch.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>

#define ROUNDS 20
#define FANOUT 200

static atomic_long sum;
static const qos_class_t classes[] = { QOS_CLASS_BACKGROUND, QOS_CLASS_UTILITY, QOS_CLASS_DEFAULT,
	QOS_CLASS_USER_INITIATED, QOS_CLASS_USER_INTERACTIVE };
static dispatch_queue_t serial[8];

static void watchdog(int sig)
{
	static const char msg[] = "FAIL: timed out\n";
	write(1, msg, sizeof(msg) - 1);
	_exit(1);
}

int main(void)
{
	dispatch_group_t g = dispatch_group_create();
	long expected = 0;

	signal(SIGALRM, watchdog);
	alarm(120);

	for (int i = 0; i < 8; i++)
		serial[i] = dispatch_queue_create("org.darlinghq.test.stress", DISPATCH_QUEUE_SERIAL);

	for (int r = 0; r < ROUNDS; r++)
	{
		for (int i = 0; i < FANOUT; i++)
		{
			dispatch_queue_t q = dispatch_get_global_queue(classes[i % 5], 0);
			dispatch_group_async(g, q, ^{
				dispatch_group_async(g, serial[i % 8], ^{
					atomic_fetch_add(&sum, 1);
				});
				dispatch_apply(4, dispatch_get_global_queue(classes[(i + 1) % 5], 0), ^(size_t k) {
					atomic_fetch_add(&sum, 1);
				});
				if (i % 16 == 0)
					usleep(2000);
				atomic_fetch_add(&sum, 1);
			});
			expected += 6;
		}
		if (r % 5 == 4)
			usleep(50000);
	}

	long w = dispatch_group_wait(g, dispatch_time(DISPATCH_TIME_NOW, 100 * NSEC_PER_SEC));
	int ok = (w == 0 && atomic_load(&sum) == expected);
	printf("%s: %ld of %ld blocks ran\n", ok ? "PASS" : "FAIL", atomic_load(&sum), expected);
	return !ok;
}
