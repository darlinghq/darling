// CFLAGS:
// Copyright (C) 2026 Lubos Dolezel

#include <dlfcn.h>
#include <stdio.h>
#include <sys/stat.h>

static int failures = 0;
#define check(cond) do { int ok_ = (cond) ? 1 : 0; printf("%s: %s\n", ok_ ? "PASS" : "FAIL", #cond); if (!ok_) failures++; } while (0)

static int loads(const char *name)
{
	char path[512];
	snprintf(path, sizeof(path), "/System/Library/Frameworks/%s.framework/Versions/A/%s", name, name);
	void *h = dlopen(path, RTLD_NOW);
	if (!h)
		printf("dlopen(%s): %s\n", path, dlerror());
	return h != NULL;
}

int main(void)
{
	// Frameworks that applications link without binding symbols must load.
	check(loads("ICADevices"));
	check(loads("FinderSync"));
	check(loads("CoreDisplay"));
	check(loads("DeviceCheck"));

	// JavaRuntimeSupport is also reachable from the top-level Frameworks
	// directory, as well as from inside JavaVM.framework.
	struct stat st;
	check(stat("/System/Library/Frameworks/JavaVM.framework/Frameworks/JavaRuntimeSupport.framework/Versions/A/JavaRuntimeSupport", &st) == 0);
	check(stat("/System/Library/Frameworks/JavaRuntimeSupport.framework/Versions/A/JavaRuntimeSupport", &st) == 0);
	check(loads("JavaRuntimeSupport"));

	return failures ? 1 : 0;
}
