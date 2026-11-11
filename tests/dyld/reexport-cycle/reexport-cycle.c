// Copyright (C) 2026 Lubos Dolezel
//
// Symbol lookup through a cycle of re-exported dylibs: librxcyclea re-exports
// librxcycleb and librxcycleleaf, and librxcycleb re-exports librxcyclea. Looking up a
// symbol must not follow the cycle forever: a symbol that the leaf exports
// is found, and a missing one is reported as missing. Observed on macOS 27
// (arm64) with the same graph.
//
// Without the fix the lookups recurse until the stack overflows (SIGSEGV).

#include <dlfcn.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void check_handle(void* handle)
{
	int (*leaf)(void) = (int (*)(void)) dlsym(handle, "reexport_leaf");
	CHECK(leaf != NULL);
	if (leaf != NULL)
		CHECK(leaf() == 71);

	dlerror();
	CHECK(dlsym(handle, "reexport_missing") == NULL);
	CHECK(dlerror() != NULL);
}

int main(void)
{
	alarm(10);

	void* first = dlopen("@executable_path/librxcyclea.dylib", RTLD_LAZY | RTLD_LOCAL | RTLD_FIRST);
	CHECK(first != NULL);
	if (first == NULL) {
		printf("dlopen: %s\n", dlerror());
		return 1;
	}
	check_handle(first);

	void* all = dlopen("@executable_path/librxcyclea.dylib", RTLD_LAZY);
	CHECK(all != NULL);
	if (all != NULL)
		check_handle(all);

	CHECK(dlsym(RTLD_DEFAULT, "reexport_missing") == NULL);

	if (all != NULL)
		CHECK(dlclose(all) == 0);
	CHECK(dlclose(first) == 0);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
