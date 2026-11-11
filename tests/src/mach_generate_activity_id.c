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

// mach_generate_activity_id() hands out activity IDs (the IDs behind
// os_activity and voucher activities) from one system-wide counter: a
// request for 1 to 16 IDs reserves that many consecutive values, other
// counts fail with KERN_INVALID_ARGUMENT, and no two processes get the
// same value. Checked on macOS 27.0.1 (arm64 and x86_64 under Rosetta).

#include <mach/mach.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

extern kern_return_t mach_generate_activity_id(mach_port_name_t target, int count, uint64_t* activity_id);

#define N_CHILD 64

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL: " __VA_ARGS__); printf("\n"); failures++; } } while (0)

int main(void)
{
	uint64_t a = 0, b = 0;
	kern_return_t kr;

	kr = mach_generate_activity_id(mach_task_self(), 1, &a);
	CHECK(kr == KERN_SUCCESS && a != 0, "count 1: kr %d id %llu", kr, (unsigned long long)a);

	kr = mach_generate_activity_id(mach_task_self(), 4, &b);
	CHECK(kr == KERN_SUCCESS && b > a, "count 4: kr %d id %llu after %llu", kr,
		(unsigned long long)b, (unsigned long long)a);

	uint64_t untouched = 12345;
	kr = mach_generate_activity_id(mach_task_self(), 0, &untouched);
	CHECK(kr == KERN_INVALID_ARGUMENT && untouched == 12345, "count 0: kr %d", kr);
	kr = mach_generate_activity_id(mach_task_self(), -1, &untouched);
	CHECK(kr == KERN_INVALID_ARGUMENT && untouched == 12345, "count -1: kr %d", kr);
	kr = mach_generate_activity_id(mach_task_self(), 17, &untouched);
	CHECK(kr == KERN_INVALID_ARGUMENT && untouched == 12345, "count 17: kr %d", kr);
	kr = mach_generate_activity_id(mach_task_self(), 16, &untouched);
	CHECK(kr == KERN_SUCCESS && untouched > b + 3, "count 16: kr %d", kr);

	// macOS 27 reports a bad output pointer with the copyout error, EFAULT (14).
	kr = mach_generate_activity_id(mach_task_self(), 1, NULL);
	CHECK(kr == 14, "NULL output: kr %d", kr);
	uint64_t last = untouched + 15; // the last ID of the 16 reserved

	// IDs from another process do not collide with ours.
	int pfd[2];
	if (pipe(pfd) != 0) { perror("pipe"); return 1; }
	pid_t pid = fork();
	if (pid == 0) {
		close(pfd[0]);
		for (int i = 0; i < N_CHILD; i++) {
			uint64_t id = 0;
			if (mach_generate_activity_id(mach_task_self(), 1, &id) != KERN_SUCCESS)
				_exit(2);
			if (write(pfd[1], &id, sizeof(id)) != sizeof(id))
				_exit(3);
		}
		_exit(0);
	}
	close(pfd[1]);
	uint64_t mine[N_CHILD];
	for (int i = 0; i < N_CHILD; i++) {
		mine[i] = 0;
		mach_generate_activity_id(mach_task_self(), 1, &mine[i]);
	}
	uint64_t theirs[N_CHILD];
	int got = 0;
	ssize_t n;
	while (got < N_CHILD && (n = read(pfd[0], &theirs[got], sizeof(uint64_t))) == sizeof(uint64_t))
		got++;
	int status = 0;
	waitpid(pid, &status, 0);
	CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0 && got == N_CHILD,
		"child status %d, %d IDs", status, got);

	for (int i = 0; i < N_CHILD; i++) {
		CHECK(mine[i] > last, "parent id %llu not after the earlier reservations", (unsigned long long)mine[i]);
		for (int j = 0; j < got; j++)
			CHECK(mine[i] != theirs[j], "parent and child both got %llu", (unsigned long long)mine[i]);
	}

	if (failures)
		return 1;
	printf("PASS\n");
	return 0;
}
