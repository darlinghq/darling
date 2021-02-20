
#ifdef BUILDING_LIBICONV
#define __LIBICONV_DLL_EXPORTED __declspec(dllexport)
#else
#define __LIBICONV_DLL_EXPORTED __declspec(dllimport)
#endif
