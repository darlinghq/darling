// Copyright (C) 2026 Lubos Dolezel
//
// The host dynamic loader's LD_LIBRARY_PATH and LD_PRELOAD are not passed from
// the host into the container: they name host ELF libraries, and dyld reads
// LD_LIBRARY_PATH as a library search path. Other variables are passed.
//
// Run from the host with the variables set. When a user starts the setuid
// launcher, the host C library already removes them from the launcher's own
// environment, so the launcher's filtering only shows when root starts it:
//   sudo env LD_LIBRARY_PATH=/nonexistent LD_PRELOAD= \
//     DARLING_TEST_MARKER=1 darling shell /path/to/host_loader_env_absent
#include <stdio.h>
#include <stdlib.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	// Shows that the host environment was passed at all.
	CHECK(getenv("DARLING_TEST_MARKER") != NULL);

	CHECK(getenv("LD_LIBRARY_PATH") == NULL);
	CHECK(getenv("LD_PRELOAD") == NULL);

	if (g_failures)
	{
		printf("%d check(s) failed\n", g_failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
