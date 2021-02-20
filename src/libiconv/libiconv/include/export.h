
#if @HAVE_VISIBILITY@ && BUILDING_LIBICONV
#define __LIBICONV_DLL_EXPORTED __attribute__((__visibility__("default")))
#else
#define __LIBICONV_DLL_EXPORTED
#endif
