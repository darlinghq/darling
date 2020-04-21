#ifndef CORESERVICESPRIV_H_
#define CORESERVICESPRIV_H_

#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFURLPriv.h>

#include <CoreServices/CoreServices.h>
#include <LaunchServices/LaunchServicesPriv.h>

CF_EXTERN_C_BEGIN

Boolean _CSCheckFix(CFStringRef str);

// Also assumed
#define QTN_FLAG_ASSESSMENT_OK 1

CF_EXTERN_C_END

#endif
