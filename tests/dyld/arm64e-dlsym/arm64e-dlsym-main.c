// Copyright (C) 2026 Lubos Dolezel
//
// An arm64e program (arm64e-dlsym-main) running against plain-arm64 system
// libraries:
// - dyld picks the arm64e slice of a universal dylib, both for a load-time
//   dependency (libdlsymslices) and for dlopen (libdlsymdl, the same source);
// - a function pointer from dlsym can be called from arm64e code, which
//   authenticates it with the IA key and a zero discriminator;
// - a plain arm64 library calling dlsym in the same process gets a pointer it
//   can call with a plain BLR.

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int dlsym_slice(void);

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(int argc, char** argv)
{
	(void) argc;

	// load-time dependency: the arm64e slice of the universal dylib
	CHECK(dlsym_slice() == 'e');

	// dlopen a universal dylib, then dlsym from arm64e code into it and into libSystem (plain arm64)
	void* handle = dlopen("@executable_path/libdlsymdl.dylib", RTLD_NOW);
	CHECK(handle != NULL);
	if (handle != NULL) {
		int (*slice)(void) = (int (*)(void)) dlsym(handle, "dlsym_slice");
		CHECK(slice != NULL);
		if (slice != NULL)
			CHECK(slice() == 'e');
	}

	size_t (*len)(const char*) = (size_t (*)(const char*)) dlsym(RTLD_DEFAULT, "strlen");
	CHECK(len != NULL);
	if (len != NULL)
		CHECK(len(argv[0]) == strlen(argv[0]));

	// dlsym from a plain arm64 library in this arm64e process
	void* plain = dlopen("@executable_path/libdlsymplain.dylib", RTLD_NOW);
	CHECK(plain != NULL);
	if (plain != NULL) {
		int (*call)(void) = (int (*)(void)) dlsym(plain, "dlsym_plain_call_dlsym");
		CHECK(call != NULL);
		if (call != NULL)
			CHECK(call() == 7);
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
