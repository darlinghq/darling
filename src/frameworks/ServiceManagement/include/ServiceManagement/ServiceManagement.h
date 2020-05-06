#ifndef __SERVICE_MANAGEMENT__
#define __SERVICE_MANAGEMENT__

#include <TargetConditionals.h>
#include <sys/cdefs.h>

#include <xpc/xpc.h>
#include <CoreFoundation/CoreFoundation.h>
#include <ServiceManagement/SMErrors.h>

#if !(TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR)
#include <Security/Authorization.h>
#include <ServiceManagement/SMLoginItem.h>
#else
#define AuthorizationRef void *
#endif

#define kSMRightBlessPrivilegedHelper "com.apple.ServiceManagement.blesshelper"

#define kSMRightModifySystemDaemons "com.apple.ServiceManagement.daemons.modify"

__BEGIN_DECLS

XPC_EXPORT
const CFStringRef kSMDomainSystemLaunchd;

XPC_EXPORT
const CFStringRef kSMDomainUserLaunchd;


XPC_EXPORT XPC_NONNULL2
CFDictionaryRef
SMJobCopyDictionary(CFStringRef domain, CFStringRef jobLabel);

XPC_EXPORT
CFArrayRef
SMCopyAllJobDictionaries(CFStringRef domain);

XPC_EXPORT XPC_NONNULL2
Boolean
SMJobSubmit(CFStringRef domain, CFDictionaryRef job, AuthorizationRef auth,
	CFErrorRef *outError);

XPC_EXPORT XPC_NONNULL2
Boolean
SMJobRemove(CFStringRef domain, CFStringRef jobLabel,
	AuthorizationRef auth, Boolean wait, CFErrorRef *outError);

#if !(TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR)

XPC_EXPORT XPC_NONNULL2
Boolean
SMJobBless(CFStringRef domain, CFStringRef executableLabel,
	AuthorizationRef auth, CFErrorRef *outError);
#endif

#if (TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR)
#undef AuthorizationRef
#endif

__END_DECLS

#endif
