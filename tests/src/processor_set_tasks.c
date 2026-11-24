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

// processor_set_tasks() and processor_set_tasks_with_flavor() list a port of
// the requested flavor for every task: the task this process is, its parent,
// and the others it may inspect.  They need the privileged processor set port
// (host_processor_set_priv); the name port of the set is not enough.

#include <errno.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/processor_set.h>
#include <mach/task_info.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/sysctl.h>
#include <unistd.h>

static int failures;

#define CHECK(cond) do { \
	if (!(cond)) { \
		printf("FAIL line %d: %s\n", __LINE__, #cond); \
		failures++; \
	} \
} while (0)

static void release(task_array_t tasks, mach_msg_type_number_t count)
{
	for (mach_msg_type_number_t i = 0; i < count; i++)
		mach_port_deallocate(mach_task_self(), tasks[i]);
	if (tasks)
		vm_deallocate(mach_task_self(), (vm_address_t)tasks, count * sizeof(*tasks));
}

static int find_pid(task_array_t tasks, mach_msg_type_number_t count, pid_t pid)
{
	for (mach_msg_type_number_t i = 0; i < count; i++) {
		int p = -1;
		if (pid_for_task(tasks[i], &p) == KERN_SUCCESS && p == pid)
			return (int)i;
	}
	return -1;
}

int main(void)
{
	mach_port_t host = mach_host_self();
	processor_set_name_array_t psets = NULL;
	mach_msg_type_number_t pset_count = 0;

	kern_return_t kr = host_processor_sets(host, &psets, &pset_count);
	CHECK(kr == KERN_SUCCESS && pset_count >= 1);
	if (kr != KERN_SUCCESS || pset_count < 1) {
		printf("host_processor_sets: %s\n", mach_error_string(kr));
		return 1;
	}

	// The name port does not list tasks.
	task_array_t tasks = NULL;
	mach_msg_type_number_t count = 0;
	kr = processor_set_tasks(psets[0], &tasks, &count);
	CHECK(kr != KERN_SUCCESS);

	processor_set_t pset = MACH_PORT_NULL;
	CHECK(host_processor_set_priv(host, psets[0], &pset) == KERN_SUCCESS);

	// Control ports: this process is among them, and it is the port the process has for itself.
	kr = processor_set_tasks(pset, &tasks, &count);
	CHECK(kr == KERN_SUCCESS);
	if (kr == KERN_SUCCESS) {
		CHECK(count >= 2);
		int self = find_pid(tasks, count, getpid());
		CHECK(self >= 0);
		if (self >= 0)
			CHECK(tasks[self] == mach_task_self());
		CHECK(find_pid(tasks, count, getppid()) >= 0);
		// no task is listed twice
		for (mach_msg_type_number_t i = 0; i < count; i++)
			for (mach_msg_type_number_t j = i + 1; j < count; j++)
				CHECK(tasks[i] != tasks[j]);
		release(tasks, count);
	}

	// Every flavor lists the same processes; the read, inspect and name ports answer what their flavor allows.
	mach_msg_type_number_t counts[4];
	mach_task_flavor_t flavors[4] = { TASK_FLAVOR_CONTROL, TASK_FLAVOR_READ, TASK_FLAVOR_INSPECT, TASK_FLAVOR_NAME };
	for (int f = 0; f < 4; f++) {
		tasks = NULL;
		count = 0;
		kr = processor_set_tasks_with_flavor(pset, flavors[f], &tasks, &count);
		CHECK(kr == KERN_SUCCESS);
		counts[f] = count;
		if (kr != KERN_SUCCESS)
			continue;
		CHECK(find_pid(tasks, count, getpid()) >= 0);
		int self = find_pid(tasks, count, getpid());
		if (self >= 0 && flavors[f] != TASK_FLAVOR_NAME) {
			task_basic_info_64_data_t basic;
			mach_msg_type_number_t basic_count = TASK_BASIC_INFO_64_COUNT;
			CHECK(task_info(tasks[self], TASK_BASIC_INFO_64, (task_info_t)&basic, &basic_count) == KERN_SUCCESS);
			CHECK(basic.resident_size > 0);
		}
		if (self >= 0 && flavors[f] == TASK_FLAVOR_NAME) {
			int p = -1;
			CHECK(pid_for_task(tasks[self], &p) == KERN_SUCCESS && p == getpid());
		}
		release(tasks, count);
	}
	for (int f = 1; f < 4; f++)
		CHECK(counts[f] + 5 >= counts[0] && counts[0] + 5 >= counts[f]);

	// The number of listed tasks is the number of processes the kernel lists (give or take those that came and went).
	int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
	size_t len = 0;
	CHECK(sysctl(mib, 4, NULL, &len, NULL, 0) == 0);
	long procs = (long)(len / sizeof(struct kinfo_proc));
	CHECK(counts[0] + 8 >= procs && procs + 8 >= counts[0]);

	// A task that is created while the list is taken, or that went away, does not break it: ask repeatedly.
	for (int i = 0; i < 20; i++) {
		tasks = NULL;
		count = 0;
		CHECK(processor_set_tasks_with_flavor(pset, TASK_FLAVOR_NAME, &tasks, &count) == KERN_SUCCESS);
		release(tasks, count);
	}

	// An unknown flavor is rejected.
	tasks = NULL;
	count = 0;
	CHECK(processor_set_tasks_with_flavor(pset, 12345, &tasks, &count) == KERN_INVALID_ARGUMENT);

	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
