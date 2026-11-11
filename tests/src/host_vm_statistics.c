// Copyright (C) 2026 Lubos Dolezel
//
// host_statistics64(HOST_VM_INFO64) and host_statistics(HOST_VM_INFO) report
// page counts in units of vm_page_size: the pages they account for (free,
// active, inactive, speculative, wired and, for the 64-bit flavor, held by the
// compressor) add up to no more than the physical memory (hw.memsize), and to
// a large part of it.
#include <mach/mach.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/sysctl.h>

static int failures;
#define CHECK(cond, what) do { if (!(cond)) { printf("FAIL %s: %s\n", what, #cond); failures++; } else printf("PASS %s\n", what); } while (0)

int main(void)
{
	uint64_t memsize = 0;
	size_t len = sizeof(memsize);
	if (sysctlbyname("hw.memsize", &memsize, &len, NULL, 0) != 0 || memsize == 0) {
		printf("FAIL hw.memsize unavailable\n");
		return 1;
	}
	printf("hw.memsize %llu, vm_page_size %lu\n", (unsigned long long)memsize, (unsigned long)vm_page_size);

	vm_statistics64_data_t s64;
	mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
	kern_return_t kr = host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&s64, &count);
	CHECK(kr == KERN_SUCCESS && count == HOST_VM_INFO64_COUNT, "host_statistics64(HOST_VM_INFO64) succeeds");
	if (kr == KERN_SUCCESS) {
		uint64_t pages = (uint64_t)s64.free_count + s64.active_count + s64.inactive_count + s64.speculative_count
			+ s64.wire_count + s64.compressor_page_count;
		uint64_t bytes = pages * vm_page_size;
		printf("64-bit: free %u active %u inactive %u speculative %u wired %u compressor %u -> %llu bytes\n",
			s64.free_count, s64.active_count, s64.inactive_count, s64.speculative_count, s64.wire_count,
			s64.compressor_page_count, (unsigned long long)bytes);
		CHECK(s64.free_count > 0, "some memory is free");
		CHECK(s64.active_count + s64.wire_count > 0, "some memory is in use");
		CHECK(bytes <= memsize, "the counted pages fit in physical memory");
		CHECK(bytes >= memsize / 2, "the counted pages cover most of physical memory");
	}

	vm_statistics_data_t s32;
	count = HOST_VM_INFO_COUNT;
	kr = host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&s32, &count);
	CHECK(kr == KERN_SUCCESS, "host_statistics(HOST_VM_INFO) succeeds");
	if (kr == KERN_SUCCESS) {
		uint64_t pages = (uint64_t)s32.free_count + s32.active_count + s32.inactive_count + s32.speculative_count
			+ s32.wire_count;
		uint64_t bytes = pages * vm_page_size;
		printf("32-bit: free %u active %u inactive %u speculative %u wired %u -> %llu bytes\n",
			s32.free_count, s32.active_count, s32.inactive_count, s32.speculative_count, s32.wire_count,
			(unsigned long long)bytes);
		CHECK(s32.free_count > 0, "HOST_VM_INFO: some memory is free");
		CHECK(bytes <= memsize, "HOST_VM_INFO: the counted pages fit in physical memory");
		CHECK(bytes >= memsize / 2, "HOST_VM_INFO: the counted pages cover most of physical memory");
	}

	if (failures) {
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("all checks passed\n");
	return 0;
}
