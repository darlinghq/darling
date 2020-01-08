#ifndef _APPLE_SYSTEM_INFO_H_
#define _APPLE_SYSTEM_INFO_H_

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

CF_EXPORT
CFStringRef ASI_CopyComputerModelName(int arg0);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif
