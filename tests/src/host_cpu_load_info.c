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

// host_statistics(HOST_CPU_LOAD_INFO) and the per-processor PROCESSOR_CPU_LOAD_INFO
// (host_processor_info) report the time spent in the user, system, idle and nice
// states, in ticks of 1/100 s that only ever grow.

#include <errno.h>
#include <mach/mach.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

static int failures;

#define CHECK(cond) do { \
	if (!(cond)) { \
		printf("FAIL line %d: %s\n", __LINE__, #cond); \
		failures++; \
	} \
} while (0)

static uint64_t total(const host_cpu_load_info_data_t* info)
{
	uint64_t sum = 0;
	for (int i = 0; i < CPU_STATE_MAX; i++)
		sum += info->cpu_ticks[i];
	return sum;
}

int main(void)
{
	mach_port_t host = mach_host_self();
	host_cpu_load_info_data_t before, after, again;
	mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;

	kern_return_t kr = host_statistics(host, HOST_CPU_LOAD_INFO, (host_info_t)&before, &count);
	CHECK(kr == KERN_SUCCESS && count == HOST_CPU_LOAD_INFO_COUNT);
	if (kr != KERN_SUCCESS) {
		printf("host_statistics(HOST_CPU_LOAD_INFO): %s\n", mach_error_string(kr));
		return 1;
	}
	CHECK(before.cpu_ticks[CPU_STATE_IDLE] > 0);
	CHECK(total(&before) > 0);

	// A buffer too small for the structure is KERN_FAILURE.
	host_cpu_load_info_data_t small;
	count = HOST_CPU_LOAD_INFO_COUNT - 1;
	CHECK(host_statistics(host, HOST_CPU_LOAD_INFO, (host_info_t)&small, &count) == KERN_FAILURE);

	// The 64-bit entry point accepts the flavor as well.
	count = HOST_CPU_LOAD_INFO_COUNT;
	CHECK(host_statistics64(host, HOST_CPU_LOAD_INFO, (host_info64_t)&again, &count) == KERN_SUCCESS);

	// Burn CPU for a while: the counters advance, none goes back.
	struct timespec start, now;
	clock_gettime(CLOCK_MONOTONIC, &start);
	volatile unsigned long spin = 0;
	do {
		for (int i = 0; i < 100000; i++)
			spin++;
		clock_gettime(CLOCK_MONOTONIC, &now);
	} while ((now.tv_sec - start.tv_sec) * 1000 + (now.tv_nsec - start.tv_nsec) / 1000000 < 500);

	count = HOST_CPU_LOAD_INFO_COUNT;
	CHECK(host_statistics(host, HOST_CPU_LOAD_INFO, (host_info_t)&after, &count) == KERN_SUCCESS);
	for (int i = 0; i < CPU_STATE_MAX; i++)
		CHECK(after.cpu_ticks[i] >= before.cpu_ticks[i]);
	// 0.5 s on one CPU is 50 ticks; allow for the sampling granularity
	CHECK(total(&after) - total(&before) >= 25);
	CHECK(after.cpu_ticks[CPU_STATE_USER] > before.cpu_ticks[CPU_STATE_USER]);

	// host_processor_info: one entry per processor.
	natural_t cpu_count = 0;
	processor_info_array_t info = NULL;
	mach_msg_type_number_t info_count = 0;
	kr = host_processor_info(host, PROCESSOR_CPU_LOAD_INFO, &cpu_count, &info, &info_count);
	CHECK(kr == KERN_SUCCESS);
	if (kr == KERN_SUCCESS) {
		CHECK(cpu_count >= 1);
		CHECK(info_count == cpu_count * CPU_STATE_MAX);
		long online = sysconf(_SC_NPROCESSORS_ONLN);
		CHECK((long)cpu_count == online);
		processor_cpu_load_info_t cpus = (processor_cpu_load_info_t)info;
		uint64_t sum = 0;
		for (natural_t i = 0; i < cpu_count; i++)
			for (int s = 0; s < CPU_STATE_MAX; s++)
				sum += cpus[i].cpu_ticks[s];
		// the per-processor counters add up to about the host's: they are read a moment later, and Linux
		// rounds each processor down to whole ticks separately
		uint64_t host_total = total(&after);
		uint64_t diff = sum > host_total ? sum - host_total : host_total - sum;
		CHECK(diff < 150 * cpu_count);
		vm_deallocate(mach_task_self(), (vm_address_t)info, info_count * sizeof(integer_t));
	}

	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
