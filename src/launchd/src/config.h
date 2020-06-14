#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <TargetConditionals.h>

#if __has_include(<quarantine.h>)
#define HAVE_QUARANTINE 1
#else
#define HAVE_QUARANTINE 0
#endif

#if __has_include(<responsibility.h>)
#define HAVE_RESPONSIBILITY 1
#else
#define HAVE_RESPONSIBILITY 0
#endif

#if __has_include(<sandbox.h>)
#define HAVE_SANDBOX 1
#else
#define HAVE_SANDBOX 0
#endif

#define HAVE_LIBAUDITD !TARGET_OS_EMBEDDED

#if !TARGET_OS_EMBEDDED && __has_include(<systemstats/systemstats.h>)
#define HAVE_SYSTEMSTATS 1
#else
#define HAVE_SYSTEMSTATS 0
#endif

#endif /* __CONFIG_H__ */
