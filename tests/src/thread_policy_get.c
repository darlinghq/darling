// Copyright (C) 2026 Lubos Dolezel
//
// thread_policy_get() on a thread with no policy set, for each flavor, with
// get_default FALSE and TRUE. The expected results are those of macOS 27
// (arm64, and x86_64 under Rosetta); several are undocumented and marked as
// QUIRK in darlingserver's thread_policy_get. Before darlingserver implemented
// the call, the first request aborted the server.
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach/thread_policy.h>
#include <stdio.h>
#include <string.h>

#ifndef THREAD_POLICY_STATE
#define THREAD_POLICY_STATE 6
#endif
#ifndef THREAD_QOS_POLICY
#define THREAD_QOS_POLICY 9
#endif
#ifndef THREAD_QOS_POLICY_OVERRIDE
#define THREAD_QOS_POLICY_OVERRIDE 10
#endif

static int failures;
#define CHECK(cond, what) do { if (!(cond)) { printf("FAIL %s: %s\n", what, #cond); failures++; } else printf("PASS %s\n", what); } while (0)

#define POISON 0x5a5a5a5a

struct result {
	kern_return_t kr;
	mach_msg_type_number_t count;
	boolean_t get_default;
	integer_t data[16];
};

static struct result get(thread_policy_flavor_t flavor, mach_msg_type_number_t count, boolean_t get_default)
{
	struct result r;
	for (int i = 0; i < 16; i++)
		r.data[i] = POISON;
	r.count = count;
	r.get_default = get_default;
	r.kr = thread_policy_get(mach_thread_self(), flavor, r.data, &r.count, &r.get_default);
	return r;
}

// absolute time units to nanoseconds, rounded
static uint64_t to_ns(uint32_t abs)
{
	mach_timebase_info_data_t tb;
	mach_timebase_info(&tb);
	return ((uint64_t)abs * tb.numer + tb.denom / 2) / tb.denom;
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	struct result r;

	for (boolean_t gd = FALSE; gd <= TRUE; gd++) {
		const char* n = gd ? " (get_default TRUE)" : " (get_default FALSE)";
		char what[128];

		r = get(THREAD_EXTENDED_POLICY, THREAD_EXTENDED_POLICY_COUNT, gd);
		snprintf(what, sizeof(what), "EXTENDED%s", n);
		CHECK(r.kr == KERN_SUCCESS && r.count == THREAD_EXTENDED_POLICY_COUNT && r.data[0] == TRUE && r.get_default == gd, what);

		r = get(THREAD_TIME_CONSTRAINT_POLICY, THREAD_TIME_CONSTRAINT_POLICY_COUNT, gd);
		snprintf(what, sizeof(what), "TIME_CONSTRAINT%s", n);
		{
			thread_time_constraint_policy_t tc = (thread_time_constraint_policy_t)r.data;
			CHECK(r.kr == KERN_SUCCESS && r.count == THREAD_TIME_CONSTRAINT_POLICY_COUNT && r.get_default == TRUE
				&& tc->period == 0 && to_ns(tc->computation) == 5000000 && to_ns(tc->constraint) == 10000000
				&& tc->preemptible == TRUE, what);
		}

		r = get(THREAD_PRECEDENCE_POLICY, THREAD_PRECEDENCE_POLICY_COUNT, gd);
		snprintf(what, sizeof(what), "PRECEDENCE%s", n);
		CHECK(r.kr == KERN_SUCCESS && r.count == THREAD_PRECEDENCE_POLICY_COUNT && r.data[0] == 0 && r.get_default == gd, what);

		r = get(THREAD_AFFINITY_POLICY, THREAD_AFFINITY_POLICY_COUNT, gd);
		snprintf(what, sizeof(what), "AFFINITY%s", n);
		CHECK(r.kr == KERN_NOT_SUPPORTED, what);

		r = get(THREAD_BACKGROUND_POLICY, THREAD_BACKGROUND_POLICY_COUNT, gd);
		snprintf(what, sizeof(what), "BACKGROUND%s", n);
		CHECK(r.kr == KERN_INVALID_ARGUMENT, what);

		r = get(THREAD_POLICY_STATE, 16, gd);
		snprintf(what, sizeof(what), "POLICY_STATE%s", n);
		CHECK(r.kr == KERN_PROTECTION_FAILURE, what);

		r = get(THREAD_LATENCY_QOS_POLICY, THREAD_LATENCY_QOS_POLICY_COUNT, gd);
		snprintf(what, sizeof(what), "LATENCY_QOS%s", n);
		CHECK(r.kr == KERN_SUCCESS && r.count == THREAD_LATENCY_QOS_POLICY_COUNT && r.get_default == gd, what);

		r = get(THREAD_THROUGHPUT_QOS_POLICY, THREAD_THROUGHPUT_QOS_POLICY_COUNT, gd);
		snprintf(what, sizeof(what), "THROUGHPUT_QOS%s", n);
		CHECK(r.kr == KERN_SUCCESS && r.count == THREAD_THROUGHPUT_QOS_POLICY_COUNT && r.get_default == gd, what);

		r = get(THREAD_QOS_POLICY, 2, gd);
		snprintf(what, sizeof(what), "QOS%s", n);
		CHECK(r.kr == KERN_SUCCESS && r.count == 2 && r.data[0] == (gd ? 0 /* UNSPECIFIED */ : 4 /* LEGACY */) && r.data[1] == 0, what);

		r = get(THREAD_QOS_POLICY_OVERRIDE, 2, gd);
		snprintf(what, sizeof(what), "QOS_OVERRIDE%s", n);
		CHECK(r.kr == KERN_INVALID_ARGUMENT, what);
	}

	r = get(THREAD_TIME_CONSTRAINT_POLICY, 1, FALSE);
	CHECK(r.kr == KERN_INVALID_ARGUMENT, "TIME_CONSTRAINT with count 1");

	r = get(THREAD_EXTENDED_POLICY, 0, FALSE);
	CHECK(r.kr == KERN_SUCCESS && r.count == 0 && r.data[0] == POISON, "EXTENDED with count 0");

	r = get(99, 8, FALSE);
	CHECK(r.kr == KERN_INVALID_ARGUMENT, "unknown flavor");

	if (failures) {
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("all checks passed\n");
	return 0;
}
