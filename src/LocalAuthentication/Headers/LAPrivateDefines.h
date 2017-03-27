#ifndef LocalAuthentication_LAPrivateDefines_h
#define LocalAuthentication_LAPrivateDefines_h

#include <LocalAuthentication/LAPublicDefines.h>
#include <CoreFoundation/CFBase.h>

CF_EXTERN_C_BEGIN

/* These aren't actual values, just made-up ones */

#define kLAErrorParameter -2134234
#define kLAErrorNotInteractive -598423
#define kTKErrorCodeBadParameter -659833

typedef void* (*LAFunctionCopyExternalizedContext)(void*);

static CFIndex kLAOptionNotInteractive = 1;

CF_EXTERN_C_END

#endif
