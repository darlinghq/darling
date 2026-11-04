// task-info-flavors.c
// A guest must not be able to abort or crash darlingserver through task_info().
// Expected output: "PASS", exit 0. Run under a host-side timeout: before the
// fix the server dies and this process blocks (see task-info-flavors.md).

#include <stdio.h>
#include <mach/mach.h>

#define UNKNOWN_FLAVOR 0x7ffffff0

static int failures;

static void expect(const char* what, kern_return_t got, kern_return_t want)
{
	if (got != want) {
		printf("FAIL %s: kr=%d (%s), want %d (%s)\n", what, got, mach_error_string(got), want, mach_error_string(want));
		failures++;
	}
}

int main(void)
{
	natural_t buf[64];
	mach_msg_type_number_t count = sizeof(buf) / sizeof(buf[0]);
	expect("unknown flavor", task_info(mach_task_self(), UNKNOWN_FLAVOR, (task_info_t)buf, &count), KERN_INVALID_ARGUMENT);

	// a thread port converts to TASK_NULL on the server side
	mach_task_basic_info_data_t mtbi;
	count = MACH_TASK_BASIC_INFO_COUNT;
	mach_port_t thread = mach_thread_self();
	expect("task_info on a thread port", task_info(thread, MACH_TASK_BASIC_INFO, (task_info_t)&mtbi, &count), KERN_INVALID_ARGUMENT);
	mach_port_deallocate(mach_task_self(), thread);

	// server still serving requests
	count = MACH_TASK_BASIC_INFO_COUNT;
	kern_return_t kr = task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&mtbi, &count);
	expect("MACH_TASK_BASIC_INFO", kr, KERN_SUCCESS);
	if (kr == KERN_SUCCESS && mtbi.resident_size == 0) {
		printf("FAIL MACH_TASK_BASIC_INFO: resident_size is 0\n");
		failures++;
	}

	// TASK_BASIC_INFO is flavor 5 on x86_64 and TASK_BASIC_INFO_64_2 (18) on arm64
	task_basic_info_data_t tbi;
	count = TASK_BASIC_INFO_COUNT;
	kr = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&tbi, &count);
	expect("TASK_BASIC_INFO", kr, KERN_SUCCESS);
	if (kr == KERN_SUCCESS && (count != TASK_BASIC_INFO_COUNT || tbi.resident_size == 0)) {
		printf("FAIL TASK_BASIC_INFO: count=%u resident_size=%llu\n", count, (unsigned long long)tbi.resident_size);
		failures++;
	}

	if (failures == 0)
		printf("PASS\n");
	return failures != 0;
}
