#ifndef COMMON_AUTO_H
#define COMMON_AUTO_H
#include "path.h"
#include "../libc/errno.h"

inline bool retvalOK(void* p) { return p != 0; }
inline bool retvalOK(int p) { return p != -1; }

#define MAP_FUNCTION1(rv,name,a1type) extern "C" rv __darwin_##name (a1type a1) { rv v = name(a1); if (!retvalOK(v)) errnoOut(); return v; }
#define MAP_FUNCTION2(rv,name,a1type,a2type) extern "C" rv __darwin_##name (a1type a1,a2type a2) { rv v = name(a1,a2); if (!retvalOK(v)) errnoOut(); return v; }
#define MAP_FUNCTION3(rv,name,a1type,a2type,a3type) extern "C" rv __darwin_##name (a1type a1,a2type a2,a3type a3) { rv v = name(a1,a2,a3); if (!retvalOK(v)) errnoOut(); return v; }
#define MAP_FUNCTION4(rv,name,a1type,a2type,a3type,a4type) extern "C" rv __darwin_##name (a1type a1,a2type a2,a3type a3,a4type a4) { rv v = name(a1,a2,a3,a4); if (!retvalOK(v)) errnoOut(); return v; }

template<typename RetVal, typename Func, typename... Params> RetVal AutoPathErrno(Func f, const char* pathname, Params... params)
{
	RetVal rv = f(translatePathCI(pathname), params...);
	if (!retvalOK(rv))
		errnoOut();
	return rv;
}

template<typename RetVal, typename Func, typename... Params> RetVal AutoPathErrno(Func f, char* pathname, Params... params)
{
	translatePathCI(pathname);
    RetVal rv = f(pathname, params...);
    if (!retvalOK(rv))
        errnoOut();
    return rv;
}

template<typename RetVal, typename Func, typename... Params> RetVal AutoErrno(Func f, Params... params)
{
	RetVal rv = f(params...);
	if (!retvalOK(rv))
		errnoOut();
	return rv;
}

namespace Darling
{
	struct MappedFlag
	{
		int darwin;
		int native;
	};
	int flagsDarwinToNative(const MappedFlag* flags, size_t count, int darwin);
	int flagsNativeToDarwin(const MappedFlag* flags, size_t count, int native);
}

#endif
