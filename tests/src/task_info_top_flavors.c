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

// The task_info() and thread_info() flavors that top(1) is built on: CPU times of
// live and terminated threads, event counters, power info, kernel memory info,
// the purgeable-aware VM info, and the basic IPC space info (number of ports).

#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach/mach_vm.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int failures;

#define CHECK(cond) do { \
	if (!(cond)) { \
		printf("FAIL line %d: %s\n", __LINE__, #cond); \
		failures++; \
	} \
} while (0)

static uint64_t usec(time_value_t t)
{
	return (uint64_t)t.seconds * 1000000 + t.microseconds;
}

static void burn(int milliseconds)
{
	struct timespec start, now;
	volatile unsigned long spin = 0;
	clock_gettime(CLOCK_MONOTONIC, &start);
	do {
		for (int i = 0; i < 100000; i++)
			spin++;
		clock_gettime(CLOCK_MONOTONIC, &now);
	} while ((now.tv_sec - start.tv_sec) * 1000 + (now.tv_nsec - start.tv_nsec) / 1000000 < milliseconds);
}

static void* burner(void* arg)
{
	burn(400);
	return arg;
}

// sum of the times of the live threads of this task, from thread_info()
static uint64_t live_thread_usec(void)
{
	thread_act_array_t threads;
	mach_msg_type_number_t count = 0;
	uint64_t total = 0;
	CHECK(task_threads(mach_task_self(), &threads, &count) == KERN_SUCCESS);
	for (mach_msg_type_number_t i = 0; i < count; i++) {
		thread_basic_info_data_t info;
		mach_msg_type_number_t info_count = THREAD_BASIC_INFO_COUNT;
		if (thread_info(threads[i], THREAD_BASIC_INFO, (thread_info_t)&info, &info_count) == KERN_SUCCESS)
			total += usec(info.user_time) + usec(info.system_time);
		mach_port_deallocate(mach_task_self(), threads[i]);
	}
	vm_deallocate(mach_task_self(), (vm_address_t)threads, count * sizeof(*threads));
	return total;
}

static uint64_t terminated_usec(void)
{
	task_basic_info_64_data_t basic;
	mach_msg_type_number_t count = TASK_BASIC_INFO_64_COUNT;
	CHECK(task_info(mach_task_self(), TASK_BASIC_INFO_64, (task_info_t)&basic, &count) == KERN_SUCCESS);
	return usec(basic.user_time) + usec(basic.system_time);
}

int main(void)
{
	task_t self = mach_task_self();

	// A thread of this task that is still there is charged to the thread; once it has exited,
	// its time is that of the task's terminated threads (TASK_BASIC_INFO).
	uint64_t live_before = live_thread_usec();
	uint64_t gone_before = terminated_usec();
	pthread_t thread;
	pthread_create(&thread, NULL, burner, NULL);
	burn(400);
	pthread_join(thread, NULL);
	uint64_t live_after = live_thread_usec();
	uint64_t gone_after = terminated_usec();
	// this thread burnt 0.4 s; clock ticks are 10 ms
	CHECK(live_after >= live_before + 300000);
	CHECK(gone_after >= gone_before + 300000);

	// TASK_THREAD_TIMES_INFO is the time of the live threads
	task_thread_times_info_data_t tt;
	mach_msg_type_number_t count = TASK_THREAD_TIMES_INFO_COUNT;
	CHECK(task_info(self, TASK_THREAD_TIMES_INFO, (task_info_t)&tt, &count) == KERN_SUCCESS);
	CHECK(usec(tt.user_time) + usec(tt.system_time) >= 300000);

	// TASK_POWER_INFO: total CPU time (live and terminated threads), in absolute time units
	task_power_info_data_t power;
	count = TASK_POWER_INFO_COUNT;
	CHECK(task_info(self, TASK_POWER_INFO, (task_info_t)&power, &count) == KERN_SUCCESS && count == TASK_POWER_INFO_COUNT);
	mach_timebase_info_data_t tb;
	mach_timebase_info(&tb);
	uint64_t total_ns = (power.total_user + power.total_system) * tb.numer / tb.denom;
	CHECK(total_ns >= 600000000ULL);
	CHECK(total_ns <= 60ULL * 1000000000);
	count = TASK_POWER_INFO_COUNT - 1;
	CHECK(task_info(self, TASK_POWER_INFO, (task_info_t)&power, &count) == KERN_INVALID_ARGUMENT);

	// TASK_EVENTS_INFO: page faults from touching fresh memory, context switches from sleeping
	task_events_info_data_t ev0, ev1;
	count = TASK_EVENTS_INFO_COUNT;
	CHECK(task_info(self, TASK_EVENTS_INFO, (task_info_t)&ev0, &count) == KERN_SUCCESS && count == TASK_EVENTS_INFO_COUNT);
	size_t size = 64 << 20;
	char* memory = malloc(size);
	for (size_t i = 0; i < size; i += 4096)
		memory[i] = 1;
	for (int i = 0; i < 100; i++)
		usleep(1000);
	count = TASK_EVENTS_INFO_COUNT;
	CHECK(task_info(self, TASK_EVENTS_INFO, (task_info_t)&ev1, &count) == KERN_SUCCESS);
	// (the host may back the memory with huge pages, so there are far fewer faults than pages)
	CHECK(ev1.faults - ev0.faults >= 8);
	CHECK(ev1.csw - ev0.csw >= 50);
	CHECK(ev1.messages_sent >= ev0.messages_sent && ev1.messages_received >= ev0.messages_received);
	count = TASK_EVENTS_INFO_COUNT - 1;
	CHECK(task_info(self, TASK_EVENTS_INFO, (task_info_t)&ev1, &count) == KERN_INVALID_ARGUMENT);
	free(memory);

	// TASK_KERNELMEMORY_INFO
	task_kernelmemory_info_data_t km;
	count = TASK_KERNELMEMORY_INFO_COUNT;
	CHECK(task_info(self, TASK_KERNELMEMORY_INFO, (task_info_t)&km, &count) == KERN_SUCCESS && count == TASK_KERNELMEMORY_INFO_COUNT);

	// TASK_VM_INFO_PURGEABLE is TASK_VM_INFO with the purgeable counters
	task_vm_info_data_t vm, vmp;
	count = TASK_VM_INFO_COUNT;
	CHECK(task_info(self, TASK_VM_INFO, (task_info_t)&vm, &count) == KERN_SUCCESS);
	count = TASK_VM_INFO_COUNT;
	CHECK(task_info(self, TASK_VM_INFO_PURGEABLE, (task_info_t)&vmp, &count) == KERN_SUCCESS && count == TASK_VM_INFO_COUNT);
	CHECK(vmp.phys_footprint > 0 && vmp.phys_footprint < (1ULL << 40));
	CHECK(vmp.virtual_size == vm.virtual_size);

	// the basic IPC space info: the number of ports in use grows with each port
	ipc_info_space_basic_t before, after;
	CHECK(mach_port_space_basic_info(self, &before) == KERN_SUCCESS);
	CHECK(before.iisb_table_inuse > 0);
	mach_port_name_t ports[8];
	for (int i = 0; i < 8; i++)
		CHECK(mach_port_allocate(self, MACH_PORT_RIGHT_RECEIVE, &ports[i]) == KERN_SUCCESS);
	CHECK(mach_port_space_basic_info(self, &after) == KERN_SUCCESS);
	CHECK(after.iisb_table_inuse == before.iisb_table_inuse + 8);
	mach_port_name_array_t names;
	mach_port_type_array_t types;
	mach_msg_type_number_t name_count, type_count;
	CHECK(mach_port_names(self, &names, &name_count, &types, &type_count) == KERN_SUCCESS);
	CHECK(after.iisb_table_inuse >= name_count);
	for (int i = 0; i < 8; i++)
		mach_port_mod_refs(self, ports[i], MACH_PORT_RIGHT_RECEIVE, -1);
	CHECK(mach_port_space_basic_info(self, &after) == KERN_SUCCESS);
	CHECK(after.iisb_table_inuse == before.iisb_table_inuse);

	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
