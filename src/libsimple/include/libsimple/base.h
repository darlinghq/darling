#ifndef _LIBSIMPLE_BASE_H_
#define _LIBSIMPLE_BASE_H_

#if __cplusplus
#define LIBSIMPLE_DECLARATIONS_BEGIN extern "C" {
#define LIBSIMPLE_DECLARATIONS_END }
#else
#define LIBSIMPLE_DECLARATIONS_BEGIN
#define LIBSIMPLE_DECLARATIONS_END
#endif

#define LIBSIMPLE_INLINE __attribute__((always_inline))

#endif // _LIBSIMPLE_BASE_H_
