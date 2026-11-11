// Copyright (C) 2026 Lubos Dolezel
//
// _os_feature_enabled_impl, which the os_feature_enabled() macro calls,
// lives in /usr/lib/system/libsystem_featureflags.dylib and is re-exported
// by libSystem. An unknown feature is disabled. macOS 27.0.1 gives the same
// results (arm64 and x86_64).
//
// The prototype is not in a public header; it is inferred from callers.
#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

extern bool _os_feature_enabled_impl(const char *domain, const char *feature);

static int failures;

static void check(const char *name, int ok)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
	failures += !ok;
}

int main(void)
{
	check("an unknown feature is disabled",
	      !_os_feature_enabled_impl("x", "y"));

	void *libsystem = dlopen("/usr/lib/libSystem.B.dylib", RTLD_NOW);
	check("libSystem re-exports _os_feature_enabled_impl",
	      libsystem && dlsym(libsystem, "_os_feature_enabled_impl"));

	void *featureflags = dlopen("/usr/lib/system/libsystem_featureflags.dylib", RTLD_NOW);
	check("libsystem_featureflags exports _os_feature_enabled_impl",
	      featureflags && dlsym(featureflags, "_os_feature_enabled_impl"));

	Dl_info info;
	check("the symbol comes from libsystem_featureflags",
	      dladdr((void *)_os_feature_enabled_impl, &info)
	      && info.dli_fname
	      && strstr(info.dli_fname, "libsystem_featureflags") != NULL);

	printf("failures=%d\n", failures);
	return failures != 0;
}
