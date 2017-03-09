#ifndef _SCNETWORKREACHABILITY_H
#define _SCNETWORKREACHABILITY_H
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFRunLoop.h>
#include <sys/types.h>
#include <sys/socket.h>

typedef struct __SCNetworkReachability* SCNetworkReachabilityRef;

CFTypeID SCNetworkReachabilityGetTypeID(void);

enum {
	kSCNetworkReachabilityFlagsReachable = 1 << 1,
};

typedef int SCNetworkReachabilityFlags;
typedef void (*SCNetworkReachabilityCallBack)(SCNetworkReachabilityRef, SCNetworkConnectionFlags, void*);

typedef struct 
{
	CFIndex version;
	void* info;
	const void* (*retain)(const void*);
	void (*release)(const void*);
	CFStringRef (*copyDescription)(const void*);
} SCNetworkReachabilityContext;

Boolean SCNetworkReachabilityScheduleWithRunLoop(SCNetworkReachabilityRef ref, CFRunLoopRef rl, CFStringRef rlMode);
Boolean SCNetworkReachabilityUnscheduleFromRunLoop(SCNetworkReachabilityRef ref, CFRunLoopRef rl, CFStringRef rlMode);

Boolean SCNetworkReachabilitySetCallback(SCNetworkReachabilityRef target, SCNetworkReachabilityCallBack cb, SCNetworkReachabilityContext* ctxt);

Boolean SCNetworkReachabilityGetFlags(SCNetworkReachabilityRef target, SCNetworkReachabilityFlags* flags);
SCNetworkReachabilityRef SCNetworkReachabilityCreateWithName(CFAllocatorRef allocator, const char* nodeName);
SCNetworkReachabilityRef SCNetworkReachabilityCreateWithAddressPair(CFAllocatorRef allocator, const struct sockaddr* localAddress, const struct sockaddr* remoteAddress);
SCNetworkReachabilityRef SCNetworkReachabilityCreateWithAddress(CFAllocatorRef allocator, const struct sockaddr* remoteAddress);

#endif

