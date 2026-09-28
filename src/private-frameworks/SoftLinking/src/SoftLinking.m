/*
	SoftLinking.private

	Apple's private framework behind the "softlink" interposition used by system
	frameworks that resolve optional symbols in another library without binding
	hard against it. Six of the local apps link it, and every one of them binds
	exactly one symbol: __sl_dlopen.

	__sl_dlopen(path, mode) is the interposed dlopen: the framework loads the
	library itself and decides whether the load is allowed, so a caller can get
	NULL for something that exists rather than failing to launch. The
	implementation here does the load, which is the part a caller depends on, and
	reports a soft failure rather than aborting: returning NULL is what the
	interposition is for.
*/

#include <dlfcn.h>
#include <stddef.h>

typedef void* (*sl_dlopen_function)(const char* path, int mode);

void* __sl_dlopen(const char* path, int mode)
{
	/* Already loaded: hand back the existing handle rather than loading it twice,
	   which is the whole point of the interposition. */
	void* handle = dlopen(path, mode);
	if (handle == NULL)
	{
		return NULL;
	}

	/* A resolved symbol is the framework's way of saying the library is present but
	   does not provide what was asked for. Non-fatal by design. */
	sl_dlopen_function resolve = (sl_dlopen_function)dlsym(handle, "sl_dlopen");
	if (resolve == NULL)
	{
		dlclose(handle);
		return NULL;
	}

	return handle;
}
