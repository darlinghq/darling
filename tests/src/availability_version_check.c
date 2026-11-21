/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * _availability_version_check() is what compiler-rt's
 * __isPlatformVersionAtLeast() (behind @available/__builtin_available)
 * calls on macOS 10.15 and later. Each entry is a {platform, version} pair
 * with the version packed as major << 16 | minor << 8 | patch. The first
 * entry for the running platform decides the answer; a list without one
 * counts as available.
 * The expected results were observed on macOS 27 (arm64 and x86_64) and macOS 26.7.3 (x86_64).
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/sysctl.h>

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

/* The running OS version as the kernel reports it (kern.osproductversion, "major.minor[.patch]"). */
static int running_version(unsigned *major, unsigned *minor, unsigned *patch)
{
	char text[64];
	size_t len = sizeof text;
	*major = *minor = *patch = 0;
	if (sysctlbyname("kern.osproductversion", text, &len, NULL, 0) != 0)
		return 0;
	return sscanf(text, "%u.%u.%u", major, minor, patch) >= 2;
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

	/* the comparison is "running version >= requested version", down to the patch level */
	unsigned major, minor, patch;
	check("the running version can be read", running_version(&major, &minor, &patch));
	if (major != 0) {
		build_version_t same[] = { { PLATFORM_MACOS, V(major, minor, patch) } };
		build_version_t next_patch[] = { { PLATFORM_MACOS, V(major, minor, patch + 1) } };
		build_version_t next_minor[] = { { PLATFORM_MACOS, V(major, minor + 1, 0) } };
		build_version_t next_major[] = { { PLATFORM_MACOS, V(major + 1, 0, 0) } };
		build_version_t minor_start[] = { { PLATFORM_MACOS, V(major, minor, 0) } };
		build_version_t prev_major[] = { { PLATFORM_MACOS, V(major - 1, 255, 255) } };

		check("the running version itself is available", _availability_version_check(1, same));
		check("one patch level above the running version is not available", !_availability_version_check(1, next_patch));
		check("one minor version above the running version is not available", !_availability_version_check(1, next_minor));
		check("one major version above the running version is not available", !_availability_version_check(1, next_major));
		check("the running major.minor.0 is available", _availability_version_check(1, minor_start));
		check("the end of the previous major version is available", _availability_version_check(1, prev_major));
	}

	return failures != 0;
}
