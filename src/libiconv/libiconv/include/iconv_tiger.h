#include <iconv.h>

#if defined(__APPLE__)
#include <TargetConditionals.h>

#if TARGET_OS_OSX && (defined(__ppc__) || defined(__i386__))

#define __LIBICONV_DLL_EXPORTED_OBSOLETE __LIBICONV_DLL_EXPORTED __attribute__((availability(macosx, obsoleted=10.5)))

/* backward compatibility */

extern __LIBICONV_DLL_EXPORTED_OBSOLETE iconv_t libiconv_open (const char* __tocode, const char* __fromcode);

extern __LIBICONV_DLL_EXPORTED_OBSOLETE size_t libiconv (iconv_t _cd, char* * __restrict __inbuf, size_t * __restrict __inbytesleft, char* * __restrict __outbuf, size_t * __restrict __outbytesleft);

extern __LIBICONV_DLL_EXPORTED_OBSOLETE int libiconv_close (iconv_t __cd);

extern __LIBICONV_DLL_EXPORTED_OBSOLETE int libiconvctl (iconv_t __cd, int request, void* argument);

extern __LIBICONV_DLL_EXPORTED_OBSOLETE void libiconvlist (int (*do_one) (unsigned int namescount,
                                                                        const char * const * names,
                                                                        void* data),
                                                         void* data);
#endif
#endif
