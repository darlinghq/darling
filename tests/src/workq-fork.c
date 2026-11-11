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
 * Workqueue state across fork(): workers started through the workqueue SPI
 * in the parent are parked when the process forks; the child (which has no
 * such threads) must still get its thread requests serviced, and the parent
 * must keep working afterwards. Uses the SPI directly, not libdispatch,
 * which by design refuses use in the child of a multithreaded parent.
 */
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread/qos.h>

typedef unsigned long pthread_priority_t_;
extern int _pthread_workqueue_init(void (*func)(pthread_priority_t_), int offset, int flags);
extern int _pthread_workqueue_addthreads(int numthreads, pthread_priority_t_ priority);
extern pthread_priority_t_ _pthread_qos_class_encode(qos_class_t qos, int rel, unsigned long flags);

static atomic_int ran;
static void worker(pthread_priority_t_ pp) { atomic_fetch_add(&ran, 1); }

static int wait_for(int n)
{
	for (int i = 0; i < 5000 && atomic_load(&ran) < n; i++) usleep(1000);
	return atomic_load(&ran) >= n;
}

int main(void)
{
	alarm(60);
	setvbuf(stdout, NULL, _IONBF, 0);
	if (_pthread_workqueue_init(worker, 0, 0) != 0) { printf("SKIP: workqueue set up\n"); return 0; }
	pthread_priority_t_ pp = _pthread_qos_class_encode(QOS_CLASS_DEFAULT, 0, 0);
	_pthread_workqueue_addthreads(8, pp);
	int a = wait_for(8);
	usleep(100000); // workers are parked now
	printf("parent: %d ran\n", atomic_load(&ran));
	pid_t p = fork();
	if (p == 0) {
		atomic_store(&ran, 0);
		_pthread_workqueue_addthreads(8, pp);
		int ok = wait_for(8);
		printf("child: %d ran\n", atomic_load(&ran));
		_exit(ok ? 0 : 1);
	}
	int st = 0;
	waitpid(p, &st, 0);
	atomic_store(&ran, 0);
	_pthread_workqueue_addthreads(8, pp);
	int b = wait_for(8);
	int ok = a && b && WIFEXITED(st) && WEXITSTATUS(st) == 0;
	printf("%s (parent after fork %d)\n", ok ? "PASS" : "FAIL", atomic_load(&ran));
	return !ok;
}
