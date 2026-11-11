// Copyright (C) 2026 Lubos Dolezel
//
// Built for plain arm64 only (libdlsymplain.dylib), like Darling's own
// libraries. It calls a dlsym result with a plain BLR, so dyld must give it
// an unsigned pointer even when the main program is arm64e.

#include <dlfcn.h>
#include <stdlib.h>

int dlsym_plain_call_dlsym(void)
{
	int (*fn)(int) = (int (*)(int)) dlsym(RTLD_DEFAULT, "abs");
	if (fn == NULL)
		return -1;
	return fn(-7);
}
