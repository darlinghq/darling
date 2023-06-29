#ifndef LocalAuthentication_LAPrivateDefines_h
#define LocalAuthentication_LAPrivateDefines_h

#include <LocalAuthentication/LAPublicDefines.h>
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFError.h>

#include <sys/types.h>
#include <stdbool.h>

CF_EXTERN_C_BEGIN

/* These aren't actual values, just made-up ones */

#define kLAErrorParameter -2134234
#define kLAErrorNotInteractive -598423
#define kTKErrorCodeBadParameter -659833
#define kLAPolicyContinuityUnlock 255
#define kLAOptionUserId 255

typedef void* (*LAFunctionCopyExternalizedContext)(void*);

static CFIndex kLAOptionNotInteractive = 1;
static CFIndex kLAOptionCallerName = 2;

bool LAEvaluatePolicy(CFTypeRef context, int policy, CFMutableDictionaryRef options, CFErrorRef *error);
bool LAVerifySEP(uid_t pw_uid, CFErrorRef *error);

CF_EXTERN_C_END

#endif
