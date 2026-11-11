// Copyright (C) 2026 Lubos Dolezel
//
// mach_generate_activity_id() hands out activity IDs that are unique system-wide:
// ranges reserved by two processes at the same time never overlap, a count of
// 1 to MACH_ACTIVITY_ID_COUNT_MAX (16) reserves that many consecutive IDs, and
// other counts fail with KERN_INVALID_ARGUMENT. Needs darlingserver to serve the
// trap (the server's mach_generate_activity_id call) and libsystem_kernel to use it.
#include <mach/mach.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

// a Mach trap exported by libsystem_kernel; not declared in the SDK headers
extern kern_return_t mach_generate_activity_id(mach_port_name_t target, int count, uint64_t* activity_id);

#define ACTIVITY_ID_COUNT_MAX 16
#define ROUNDS 2000

static int failures;
#define CHECK(cond, what) do { if (!(cond)) { printf("FAIL %s: %s\n", what, #cond); failures++; } else printf("PASS %s\n", what); } while (0)

struct range {
	uint64_t first;
	uint32_t count;
};

static int reserve(struct range* ranges, int rounds, int salt)
{
	for (int i = 0; i < rounds; i++) {
		ranges[i].count = (i + salt) % ACTIVITY_ID_COUNT_MAX + 1;
		if (mach_generate_activity_id(mach_task_self(), ranges[i].count, &ranges[i].first) != KERN_SUCCESS)
			return 0;
	}
	return 1;
}

static int compare(const void* a, const void* b)
{
	uint64_t x = ((const struct range*)a)->first, y = ((const struct range*)b)->first;
	return (x > y) - (x < y);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	uint64_t id = 0x5a5a5a5a5a5a5a5aULL;

	CHECK(mach_generate_activity_id(mach_task_self(), 0, &id) == KERN_INVALID_ARGUMENT, "a count of 0 is refused");
	CHECK(mach_generate_activity_id(mach_task_self(), -1, &id) == KERN_INVALID_ARGUMENT, "a negative count is refused");
	CHECK(mach_generate_activity_id(mach_task_self(), ACTIVITY_ID_COUNT_MAX + 1, &id) == KERN_INVALID_ARGUMENT, "a count above 16 is refused");
	CHECK(id == 0x5a5a5a5a5a5a5a5aULL, "a refused call leaves the output alone");

	uint64_t a, b;
	CHECK(mach_generate_activity_id(mach_task_self(), 4, &a) == KERN_SUCCESS && a != 0, "4 IDs are reserved");
	CHECK(mach_generate_activity_id(mach_task_self(), 1, &b) == KERN_SUCCESS && b >= a + 4, "the next reservation starts after them");

	// two processes reserve at the same time; no ID may be handed out twice
	static struct range ranges[2 * ROUNDS];
	int fds[2];
	if (pipe(fds) != 0) {
		perror("pipe");
		return 1;
	}
	pid_t child = fork();
	if (child == 0) {
		close(fds[0]);
		struct range mine[ROUNDS];
		int ok = reserve(mine, ROUNDS, 7);
		if (!ok || write(fds[1], mine, sizeof(mine)) != sizeof(mine))
			_exit(1);
		_exit(0);
	}
	close(fds[1]);
	int ok = reserve(ranges, ROUNDS, 0);
	size_t got = 0;
	while (got < sizeof(struct range) * ROUNDS) {
		ssize_t n = read(fds[0], (char*)&ranges[ROUNDS] + got, sizeof(struct range) * ROUNDS - got);
		if (n <= 0)
			break;
		got += n;
	}
	int status = 0;
	waitpid(child, &status, 0);
	CHECK(ok && got == sizeof(struct range) * ROUNDS && WIFEXITED(status) && WEXITSTATUS(status) == 0, "both processes reserve IDs");

	qsort(ranges, 2 * ROUNDS, sizeof(ranges[0]), compare);
	int overlaps = 0;
	for (int i = 1; i < 2 * ROUNDS; i++)
		if (ranges[i].first < ranges[i - 1].first + ranges[i - 1].count)
			overlaps++;
	CHECK(overlaps == 0, "ranges reserved by two processes do not overlap");

	if (failures) {
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("all checks passed\n");
	return 0;
}
