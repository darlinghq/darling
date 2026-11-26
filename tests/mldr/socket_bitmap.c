// Host-side test of mldr's socket bitmap: descriptors handed out must stay unique
// while others come and go, freeing everything must leave a usable bitmap, and
// placing a descriptor must never replace one that is already open.
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include "socket_bitmap.h"

#define HIGHEST 1023
#define LONG_LIVED 8
#define WAVES 200
#define MAX_WAVE_SIZE 24

static socket_bitmap_t bitmap = {
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.highest = HIGHEST,
};

static int in_use[HIGHEST + 1];

static int get(void)
{
	int fd = socket_bitmap_get(&bitmap);

	if (fd < 0 || fd > HIGHEST)
	{
		printf("FAIL: got descriptor %d\n", fd);
		exit(1);
	}
	if (in_use[fd])
	{
		printf("FAIL: descriptor %d handed out twice\n", fd);
		exit(1);
	}
	in_use[fd] = 1;
	return fd;
}

static void put(int fd)
{
	in_use[fd] = 0;
	socket_bitmap_put(&bitmap, fd);
}

// A descriptor some other code already holds at the top of the range, such as one a
// native library opened, must survive mldr placing its own sockets there.
static int test_place(void)
{
	socket_bitmap_t placing = {
		.mutex = PTHREAD_MUTEX_INITIALIZER,
		.highest = 200,
	};
	int foreign[2], ours[2];
	struct stat before, after, placed_stat, ours_stat;

	if (pipe(foreign) != 0 || pipe(ours) != 0 || dup2(foreign[0], 200) != 200)
	{
		printf("FAIL: cannot set up descriptors\n");
		return 1;
	}
	fstat(200, &before);

	int placed = socket_bitmap_place(&placing, ours[0]);
	if (placed < 0 || placed == 200)
	{
		printf("FAIL: placed at %d\n", placed);
		return 1;
	}

	fstat(200, &after);
	fstat(placed, &placed_stat);
	fstat(ours[0], &ours_stat);
	if (before.st_ino != after.st_ino || placed_stat.st_ino != ours_stat.st_ino)
	{
		printf("FAIL: placing replaced a descriptor that was already open\n");
		return 1;
	}

	if (socket_bitmap_place(&placing, ours[0]) == placed)
	{
		printf("FAIL: the same descriptor was placed twice\n");
		return 1;
	}
	return 0;
}

// A descriptor inherited across exec usually already sits on the slot the new process
// hands out first. Placing it must keep it there instead of moving it off its own slot.
static int test_place_on_own_slot(void)
{
	socket_bitmap_t placing = {
		.mutex = PTHREAD_MUTEX_INITIALIZER,
		.highest = 190,
	};
	int inherited[2];
	struct stat before, after;

	if (pipe(inherited) != 0 || dup2(inherited[1], 190) != 190)
	{
		printf("FAIL: cannot set up descriptors\n");
		return 1;
	}
	fstat(190, &before);

	int placed = socket_bitmap_place(&placing, 190);
	if (placed != 190)
	{
		printf("FAIL: a descriptor already on its slot was moved to %d\n", placed);
		return 1;
	}

	fstat(190, &after);
	if (before.st_ino != after.st_ino)
	{
		printf("FAIL: the descriptor on its own slot was replaced\n");
		return 1;
	}

	// the slot stays taken
	if (socket_bitmap_get(&placing) != 189)
	{
		printf("FAIL: the slot of a descriptor placed in place was handed out again\n");
		return 1;
	}
	return 0;
}

// The bitmap is allocated whole on first use and never reallocated or freed afterwards: the child of a fork
// releases the parent's descriptors through socket_bitmap_put() before the host's malloc locks are in a known
// state, and a realloc() or free() there can wait forever for a lock another thread of the parent held at the fork.
static int test_allocation_is_stable(void)
{
	socket_bitmap_t stable = {
		.mutex = PTHREAD_MUTEX_INITIALIZER,
		.highest = 255,
	};
	int fds[100];

	fds[0] = socket_bitmap_get(&stable);
	uint8_t* first = stable.bits;
	if (first == NULL)
	{
		printf("FAIL: no bitmap after the first descriptor\n");
		return 1;
	}
	for (int i = 1; i < 100; i++)
		fds[i] = socket_bitmap_get(&stable);
	if (stable.bits != first)
	{
		printf("FAIL: the bitmap moved while it grew\n");
		return 1;
	}
	for (int i = 0; i < 100; i++)
		socket_bitmap_put(&stable, fds[i]);
	if (stable.bits != first)
	{
		printf("FAIL: the bitmap was reallocated or freed while it emptied\n");
		return 1;
	}
	return 0;
}

// With no explicit top, the numbers count down from the soft descriptor limit, but never from above 16384:
// the kernel sizes a process's descriptor table to its highest descriptor, and a limit of hundreds of thousands
// made every process carry a table of megabytes.
static int test_default_top_is_capped(void)
{
	struct rlimit saved, raised;
	socket_bitmap_t fresh = {
		.mutex = PTHREAD_MUTEX_INITIALIZER,
		.highest = -1,
	};

	if (getrlimit(RLIMIT_NOFILE, &saved) != 0)
	{
		printf("FAIL: getrlimit\n");
		return 1;
	}
	raised = saved;
	raised.rlim_cur = saved.rlim_max == RLIM_INFINITY || saved.rlim_max > 100000 ? 100000 : saved.rlim_max;
	if (raised.rlim_cur < 16384 || setrlimit(RLIMIT_NOFILE, &raised) != 0)
	{
		printf("skipped: the descriptor limit cannot be raised above 16384 here\n");
		return 0;
	}
	int fd = socket_bitmap_get(&fresh);
	setrlimit(RLIMIT_NOFILE, &saved);
	if (fd != 16383)
	{
		printf("FAIL: the first descriptor with a limit of %llu is %d, not 16383\n", (unsigned long long) raised.rlim_cur, fd);
		return 1;
	}
	return 0;
}

int main(void)
{
	int keepers[LONG_LIVED];

	for (int i = 0; i < LONG_LIVED; i++)
		keepers[i] = get();

	// Waves of every size, freed in the order they were handed out, so the bitmap
	// grows and shrinks across byte boundaries while the first entries stay in use.
	for (int wave = 0; wave < WAVES; wave++)
	{
		int fds[MAX_WAVE_SIZE];
		const int count = 1 + wave % MAX_WAVE_SIZE;

		for (int i = 0; i < count; i++)
			fds[i] = get();
		for (int i = 0; i < count; i++)
			put(fds[i]);
	}

	// Descriptors the bitmap never handed out are ignored.
	socket_bitmap_put(&bitmap, HIGHEST + 1);
	socket_bitmap_put(&bitmap, 3);

	for (int i = 0; i < LONG_LIVED; i++)
		put(keepers[i]);

	// the allocation stays once made (no realloc / free after the first get: fork-safe)
	if (bitmap.bit_length != 0)
	{
		printf("FAIL: %zu entries left after freeing everything\n", bitmap.bit_length);
		return 1;
	}
	if (get() != HIGHEST)
	{
		printf("FAIL: the bitmap does not start over once empty\n");
		return 1;
	}

	if (test_place() != 0)
		return 1;
	if (test_place_on_own_slot() != 0)
		return 1;
	if (test_allocation_is_stable() != 0)
		return 1;
	if (test_default_top_is_capped() != 0)
		return 1;

	printf("socket_bitmap: OK\n");
	return 0;
}
