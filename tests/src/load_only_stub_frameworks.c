// Copyright (C) 2026 Lubos Dolezel
//
// Frameworks that exist only to satisfy a load command can be loaded, and
// loading them writes nothing to stderr even when STUB_VERBOSE is set.
//
// Darling-specific: these frameworks do not exist as empty images on macOS,
// where the real frameworks are loaded; the test only checks that the load
// succeeds, which holds on both.
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static const char *const kPaths[] = {
	"/System/Library/PrivateFrameworks/CoreCDP.framework/CoreCDP",
	"/System/Library/PrivateFrameworks/AppSSOCore.framework/AppSSOCore",
	"/System/Library/PrivateFrameworks/FontServices.framework/FontServices",
	"/System/Library/PrivateFrameworks/PhoneNumbers.framework/PhoneNumbers",
	"/System/Library/Frameworks/NotificationCenter.framework/NotificationCenter",
	"/System/Library/Frameworks/UserNotificationsUI.framework/UserNotificationsUI",
	"/System/Library/Frameworks/VisionKit.framework/VisionKit",
};

int main(void)
{
	setenv("STUB_VERBOSE", "1", 1);

	int fds[2];
	CHECK(pipe(fds) == 0);
	fcntl(fds[0], F_SETFL, O_NONBLOCK);
	int saved = dup(2);
	dup2(fds[1], 2);

	for (size_t i = 0; i < sizeof kPaths / sizeof kPaths[0]; i++) {
		void *h = dlopen(kPaths[i], RTLD_NOW);
		if (!h) {
			dup2(saved, 2);
			printf("FAIL: dlopen %s: %s\n", kPaths[i], dlerror());
			dup2(fds[1], 2);
			g_failures++;
		}
	}

	dup2(saved, 2);
	char buf[256];
	CHECK(read(fds[0], buf, sizeof buf) <= 0);

	if (g_failures) {
		printf("FAILED: %d\n", g_failures);
		return 1;
	}
	printf("OK\n");
	return 0;
}
