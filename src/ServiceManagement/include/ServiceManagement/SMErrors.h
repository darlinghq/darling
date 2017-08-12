#ifndef __SERVICE_MANAGEMENT_ERRORS__
#define __SERVICE_MANAGEMENT_ERRORS__

#include <TargetConditionals.h>
#include <sys/cdefs.h>

#include <xpc/xpc.h>
#include <CoreFoundation/CoreFoundation.h>

__BEGIN_DECLS

XPC_EXPORT
const CFStringRef kSMErrorDomainIPC;

XPC_EXPORT
const CFStringRef kSMErrorDomainFramework;

XPC_EXPORT
const CFStringRef kSMErrorDomainLaunchd;

enum {
	kSMErrorInternalFailure = 2,
	kSMErrorInvalidSignature,
	kSMErrorAuthorizationFailure,
	kSMErrorToolNotValid,
	kSMErrorJobNotFound,
	kSMErrorServiceUnavailable,
	kSMErrorJobPlistNotFound,
	kSMErrorJobMustBeEnabled,
	kSMErrorInvalidPlist,
};

__END_DECLS

#endif
