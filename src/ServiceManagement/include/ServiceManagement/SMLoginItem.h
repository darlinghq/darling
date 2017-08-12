#ifndef __SERVICE_MANAGEMENT_LOGIN_ITEM__
#define __SERVICE_MANAGEMENT_LOGIN_ITEM__

#include <TargetConditionals.h>
#include <sys/cdefs.h>

#include <xpc/xpc.h>
#include <CoreFoundation/CoreFoundation.h>

__BEGIN_DECLS

XPC_EXPORT XPC_NONNULL1
Boolean
SMLoginItemSetEnabled(CFStringRef identifier, Boolean enabled);

__END_DECLS

#endif
