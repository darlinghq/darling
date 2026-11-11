/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * _availability_version_check() is what compiler-rt's
 * __isPlatformVersionAtLeast() (behind @available/__builtin_available)
 * calls on macOS 10.15 and later. Each entry is a {platform, version} pair
 * with the version packed as major << 16 | minor << 8 | patch. The first
 * entry for the running platform decides the answer; a list without one
 * counts as available.
 * The expected results were observed on macOS 27 (arm64 and x86_64).
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

typedef struct {
	uint32_t platform;
	uint32_t version;
} build_version_t;

extern bool _availability_version_check(size_t count, build_version_t versions[]);

#define PLATFORM_MACOS 1
#define PLATFORM_IOS 2
#define V(maj, min, patch) (((maj) << 16) | ((min) << 8) | (patch))

static int failures;

static void check(const char *what, int cond)
{
	printf("%s: %s\n", cond ? "PASS" : "FAIL", what);
	if (!cond)
		failures++;
}

int main(void)
{
	build_version_t old_mac[] = { { PLATFORM_MACOS, V(10, 15, 0) } };
	build_version_t future_mac[] = { { PLATFORM_MACOS, V(99, 0, 0) } };
	build_version_t ios_only[] = { { PLATFORM_IOS, V(1, 0, 0) } };
	build_version_t mixed[] = { { PLATFORM_IOS, V(99, 0, 0) }, { PLATFORM_MACOS, V(10, 15, 0) } };
	build_version_t mac_ios[] = { { PLATFORM_MACOS, V(99, 0, 0) }, { PLATFORM_IOS, V(1, 0, 0) } };
	build_version_t two_macs[] = { { PLATFORM_MACOS, V(10, 15, 0) }, { PLATFORM_MACOS, V(99, 0, 0) } };

	check("macOS 10.15 is available", _availability_version_check(1, old_mac));
	check("macOS 99 is not available", !_availability_version_check(1, future_mac));
	check("the macOS entry is used when other platforms come first", _availability_version_check(2, mixed));
	check("other platforms do not override the macOS entry", !_availability_version_check(2, mac_ios));
	check("the first macOS entry decides", _availability_version_check(2, two_macs));
	/* quirk: no entry for the running platform counts as available */
	check("no entry for macOS means available", _availability_version_check(1, ios_only));
	check("an empty list means available", _availability_version_check(0, old_mac));

	return failures != 0;
}
