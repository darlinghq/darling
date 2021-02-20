
#ifdef BUILDING_LIBCHARSET
#define LIBCHARSET_DLL_EXPORTED __declspec(dllexport)
#else
#define LIBCHARSET_DLL_EXPORTED __declspec(dllimport)
#endif
