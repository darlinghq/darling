#ifndef _SCNETWORKCONNECTION_H
#define _SCNETWORKCONNECTION_H
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFDictionary.h>

typedef struct __SCNetworkConnection* SCNetworkConnectionRef;
typedef int SCNetworkConnectionFlags;
typedef int SCNetworkConnectionStatus;

enum {
	kSCNetworkConnectionInvalid = -1,
	kSCNetworkConnectionDisconnected,
	kSCNetworkConnectionConnecting,
	kSCNetworkConnectionConnected,
	kSCNetworkConnectionDisconnecting
};

typedef void (*SCNetworkConnectionCallBack)(SCNetworkConnectionRef, SCNetworkConnectionStatus, void*);

typedef struct 
{
	CFIndex version;
	void* info;
	const void* (*retain)(const void*);
	void (*release)(const void*);
	CFStringRef (*copyDescription)(const void*);
} SCNetworkConnectionContext;

CFTypeID SCNetworkConnectionGetTypeID(void);

Boolean SCNetworkConnectionScheduleWithRunLoop(SCNetworkConnectionRef ref, CFRunLoopRef rl, CFStringRef rlMode);
Boolean SCNetworkConnectionUnscheduleFromRunLoop(SCNetworkConnectionRef ref, CFRunLoopRef rl, CFStringRef rlMode);

Boolean SCNetworkConnectionStop(SCNetworkConnectionRef ref, Boolean force);

Boolean SCNetworkConnectionStart(SCNetworkConnectionRef ref, CFDictionaryRef userOptions, Boolean linger);

Boolean SCNetworkConnectionCopyUserPreferences(CFDictionaryRef selectionOptions, CFStringRef serviceID, CFDictionaryRef* userOptions);

SCNetworkConnectionRef SCNetworkConnectionCreateWithServiceID(CFAllocatorRef allocator, CFStringRef serviceID, SCNetworkConnectionCallBack cb, SCNetworkConnectionContext* ctxt); 

#endif

