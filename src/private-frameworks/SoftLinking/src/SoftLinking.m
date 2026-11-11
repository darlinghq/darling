/*
	SoftLinking.private

	Apple's private framework behind the "softlink" interposition used by system
	frameworks that resolve optional symbols in another library without binding
	hard against it. Six of the local apps link it, and they bind two symbols:
	sl_dlopen and TSUSoftLinkingGetFrameworkFunction.

	sl_dlopen(path, mode) is the interposed dlopen: the framework loads the library
	itself and decides whether the load is allowed, so a caller can get NULL for
	something that exists rather than failing to launch. The implementation here does
	the load, which is the part a caller depends on, and reports a soft failure
	rather than aborting, because returning NULL is what the interposition is for.

	TSUSoftLinkingGetFrameworkFunction returns a symbol out of a framework that has
	been softlinked, or NULL when that framework is not present. Nothing is
	softlinked here, so it reports NULL for everything, which is the honest answer
	and the one a caller is required to handle.
*/

#include <dlfcn.h>
#include <stddef.h>

void* _sl_dlopen(const char* path, int mode)
{
	/* A soft link loads the library when it is present and reports NULL when it is
	   not, so the caller takes its own fallback instead of failing to launch. */
	return dlopen(path, mode);
}

void* TSUSoftLinkingGetFrameworkFunction(const char* framework, const char* function)
{
	void* handle;

	/* No framework is softlinked on this system, so there is no handle to resolve
	   out of. NULL is the documented answer for an absent framework. */
	(void)framework;
	(void)function;
	handle = NULL;

	return handle;
}
