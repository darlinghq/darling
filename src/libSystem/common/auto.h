#ifndef COMMON_AUTO_H
#define COMMON_AUTO_H

inline bool retvalOK(void* p) { return p != 0; }
inline bool retvalOK(int p) { return p != -1; }

#define MAP_FUNCTION1(rv,name,a1type) extern "C" rv __darwin_##name (a1type a1) { rv v = name(a1); if (!retvalOK(v)) errnoOut(); return v; }
#define MAP_FUNCTION2(rv,name,a1type,a2type) extern "C" rv __darwin_##name (a1type a1,a2type a2) { rv v = name(a1,a2); if (!retvalOK(v)) errnoOut(); return v; }
#define MAP_FUNCTION3(rv,name,a1type,a2type,a3type) extern "C" rv __darwin_##name (a1type a1,a2type a2,a3type a3) { rv v = name(a1,a2,a3); if (!retvalOK(v)) errnoOut(); return v; }
#define MAP_FUNCTION4(rv,name,a1type,a2type,a3type,a4type) extern "C" rv __darwin_##name (a1type a1,a2type a2,a3type a3,a4type a4) { rv v = name(a1,a2,a3,a4); if (!retvalOK(v)) errnoOut(); return v; }

template<typename RetVal, typename Func, typename... Params> RetVal AutoPathErrno(Func f, const char* path, Params... params)
{
	RetVal rv = f(translatePathCI(pathname), params);
	if (rv == -1)
		errnoOut();
	return rv;
}

template<typename RetVal, typename Func, typename... Params> RetVal AutoErrno(Func f, Params... params)
{
	RetVal rv = f(params);
	if (!retvalOK(rv))
		errnoOut();
	return rv;
}

#endif
