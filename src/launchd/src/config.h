#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <TargetConditionals.h>

#if __has_include(<quarantine.h>) && !defined(DARLING)
#define HAVE_QUARANTINE 1
#else
#define HAVE_QUARANTINE 0
#endif

#if __has_include(<responsibility.h>) && !defined(DARLING)
#define HAVE_RESPONSIBILITY 1
#else
#define HAVE_RESPONSIBILITY 0
#endif

#if __has_include(<sandbox.h>) && !defined(DARLING)
#define HAVE_SANDBOX 1
#else
#define HAVE_SANDBOX 0
#endif

#define HAVE_LIBAUDITD !TARGET_OS_EMBEDDED && !defined(DARLING)

#if !TARGET_OS_EMBEDDED && __has_include(<systemstats/systemstats.h>) && !defined(DARLING)
#define HAVE_SYSTEMSTATS 1
#else
#define HAVE_SYSTEMSTATS 0
#endif

#endif /* __CONFIG_H__ */
