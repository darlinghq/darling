// Host-side test of mldr's socket bitmap: descriptors handed out must stay unique
// while others come and go, and freeing everything must leave a usable bitmap.
#include <stdio.h>
#include <stdlib.h>
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

	if (bitmap.bit_length != 0 || bitmap.bits != NULL)
	{
		printf("FAIL: %zu entries left after freeing everything\n", bitmap.bit_length);
		return 1;
	}
	if (get() != HIGHEST)
	{
		printf("FAIL: the bitmap does not start over once empty\n");
		return 1;
	}

	printf("socket_bitmap: OK\n");
	return 0;
}
