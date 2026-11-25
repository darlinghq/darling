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

// The kernel task (kernel_task, pid 0) is a task like the others: task_for_pid(0) gives its
// port, task_info() and mach_port_space_basic_info() answer for it (with the figures of the
// server that stands for the kernel), processor_set_tasks() lists it and the kern.proc sysctls
// know it as "kernel_task".

#include <errno.h>
#include <mach/mach.h>
#include <mach/processor_set.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysctl.h>
#include <unistd.h>

static int failures;

#define CHECK(cond) do { \
	if (!(cond)) { \
		printf("FAIL line %d: %s\n", __LINE__, #cond); \
		failures++; \
	} \
} while (0)

int main(void)
{
	mach_port_t kernel = MACH_PORT_NULL;
	kern_return_t kr = task_for_pid(mach_task_self(), 0, &kernel);
	CHECK(kr == KERN_SUCCESS && kernel != MACH_PORT_NULL);
	if (kr != KERN_SUCCESS)
		return 1;

	int pid = -1;
	CHECK(pid_for_task(kernel, &pid) == KERN_SUCCESS && pid == 0);

	task_basic_info_64_data_t basic;
	mach_msg_type_number_t count = TASK_BASIC_INFO_64_COUNT;
	CHECK(task_info(kernel, TASK_BASIC_INFO_64, (task_info_t)&basic, &count) == KERN_SUCCESS);
	CHECK(basic.suspend_count == 0);
	CHECK(basic.resident_size > 0 && basic.virtual_size >= basic.resident_size);

	task_vm_info_data_t vm;
	count = TASK_VM_INFO_COUNT;
	CHECK(task_info(kernel, TASK_VM_INFO, (task_info_t)&vm, &count) == KERN_SUCCESS);
	CHECK(vm.phys_footprint > 0);

	task_events_info_data_t events;
	count = TASK_EVENTS_INFO_COUNT;
	CHECK(task_info(kernel, TASK_EVENTS_INFO, (task_info_t)&events, &count) == KERN_SUCCESS);

	task_power_info_data_t power;
	count = TASK_POWER_INFO_COUNT;
	CHECK(task_info(kernel, TASK_POWER_INFO, (task_info_t)&power, &count) == KERN_SUCCESS);
	CHECK(power.total_user + power.total_system > 0);

	task_thread_times_info_data_t times;
	count = TASK_THREAD_TIMES_INFO_COUNT;
	CHECK(task_info(kernel, TASK_THREAD_TIMES_INFO, (task_info_t)&times, &count) == KERN_SUCCESS);

	task_kernelmemory_info_data_t kmem;
	count = TASK_KERNELMEMORY_INFO_COUNT;
	CHECK(task_info(kernel, TASK_KERNELMEMORY_INFO, (task_info_t)&kmem, &count) == KERN_SUCCESS);

	thread_act_array_t threads = NULL;
	mach_msg_type_number_t thread_count = 0;
	CHECK(task_threads(kernel, &threads, &thread_count) == KERN_SUCCESS);
	CHECK(thread_count >= 1);
	for (mach_msg_type_number_t i = 0; i < thread_count; i++) {
		thread_basic_info_data_t thread_info_data;
		count = THREAD_BASIC_INFO_COUNT;
		CHECK(thread_info(threads[i], THREAD_BASIC_INFO, (thread_info_t)&thread_info_data, &count) == KERN_SUCCESS);
		CHECK(thread_info_data.run_state >= TH_STATE_RUNNING && thread_info_data.run_state <= TH_STATE_HALTED);
		mach_port_deallocate(mach_task_self(), threads[i]);
	}

	ipc_info_space_basic_t space;
	CHECK(mach_port_space_basic_info(kernel, &space) == KERN_SUCCESS);

	// processor_set_tasks() lists it
	processor_set_name_array_t psets = NULL;
	mach_msg_type_number_t pset_count = 0;
	CHECK(host_processor_sets(mach_host_self(), &psets, &pset_count) == KERN_SUCCESS && pset_count >= 1);
	processor_set_t pset;
	CHECK(host_processor_set_priv(mach_host_self(), psets[0], &pset) == KERN_SUCCESS);
	task_array_t tasks = NULL;
	mach_msg_type_number_t task_count = 0;
	CHECK(processor_set_tasks_with_flavor(pset, TASK_FLAVOR_READ, &tasks, &task_count) == KERN_SUCCESS);
	int found = 0;
	for (mach_msg_type_number_t i = 0; i < task_count; i++) {
		int p = -1;
		if (pid_for_task(tasks[i], &p) == KERN_SUCCESS && p == 0)
			found++;
		mach_port_deallocate(mach_task_self(), tasks[i]);
	}
	CHECK(found == 1);

	// kern.proc knows it, by PID and in the list of all processes
	int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, 0 };
	struct kinfo_proc info;
	size_t len = sizeof(info);
	memset(&info, 0, sizeof(info));
	CHECK(sysctl(mib, 4, &info, &len, NULL, 0) == 0 && len == sizeof(info));
	CHECK(strcmp(info.kp_proc.p_comm, "kernel_task") == 0 && info.kp_proc.p_pid == 0 && info.kp_eproc.e_ppid == 0);
	mib[2] = KERN_PROC_ALL;
	len = 0;
	CHECK(sysctl(mib, 3, NULL, &len, NULL, 0) == 0);
	size_t size = len + 16 * sizeof(struct kinfo_proc);
	struct kinfo_proc* all = malloc(size);
	len = size;
	CHECK(sysctl(mib, 3, all, &len, NULL, 0) == 0);
	int zero = 0;
	for (size_t i = 0; i < len / sizeof(*all); i++)
		if (all[i].kp_proc.p_pid == 0)
			zero++;
	CHECK(zero == 1);
	free(all);

	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
