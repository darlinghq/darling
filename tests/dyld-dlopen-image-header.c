// Copyright (C) 2026 Lubos Dolezel
//
// _dyld_get_dlopen_image_header() (declared in <mach-o/dyld_priv.h>) returns
// the mach header of the image that a dlopen() handle refers to: the same
// header that dladdr() reports as dli_fbase for a symbol of that image. The
// special RTLD_* handles do not refer to one image and give NULL.
//   clang -o /tmp/dyld-dlopen-image-header tests/dyld-dlopen-image-header.c   (run inside Darling; any arch)
// Exits non-zero on failure.
#include <dlfcn.h>
#include <mach-o/loader.h>
#include <stdbool.h>
#include <stdio.h>

extern const struct mach_header* _dyld_get_dlopen_image_header(void* handle);

static int fail;

static void check(bool ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	fail |= !ok;
}

int main(void)
{
	void* h = dlopen("/usr/lib/libc++.1.dylib", RTLD_NOW);
	check(h != NULL, "dlopen libc++");
	if (!h)
		return 1;

	const struct mach_header* mh = _dyld_get_dlopen_image_header(h);
	check(mh != NULL, "header of a dlopen handle");

	Dl_info info;
	void* sym = dlsym(h, "_ZNSt3__15mutex4lockEv");
	check(sym != NULL && dladdr(sym, &info) && info.dli_fbase == mh, "header is that of the image dladdr reports");
	check(mh != NULL && (mh->magic == MH_MAGIC || mh->magic == MH_MAGIC_64) && mh->filetype == MH_DYLIB,
		"header is a dylib's mach header");

	// RTLD_NOLOAD gives a handle to the same image
	void* again = dlopen("/usr/lib/libc++.1.dylib", RTLD_NOLOAD);
	check(again != NULL && _dyld_get_dlopen_image_header(again) == mh, "RTLD_NOLOAD handle has the same header");

	check(_dyld_get_dlopen_image_header(RTLD_DEFAULT) == NULL, "RTLD_DEFAULT gives NULL");
	check(_dyld_get_dlopen_image_header(RTLD_SELF) == NULL, "RTLD_SELF gives NULL");
	check(_dyld_get_dlopen_image_header(RTLD_NEXT) == NULL, "RTLD_NEXT gives NULL");
	check(_dyld_get_dlopen_image_header(RTLD_MAIN_ONLY) == NULL, "RTLD_MAIN_ONLY gives NULL");

	if (again)
		dlclose(again);
	dlclose(h);
	return fail;
}
