#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <SystemConfiguration/SystemConfiguration.h>

#define STUB() printf("STUB %s\n", __func__)

const CFStringRef kSCDynamicStoreDomainState = CFSTR("State:");
const CFStringRef kSCEntNetIPv4 = CFSTR("IPv4");
const CFStringRef kSCEntNetIPv6 = CFSTR("IPv6");
const CFStringRef kSCPropNetIPv4Router = CFSTR("Router");
const CFStringRef kSCPropNetIPv6Router = CFSTR("Router"); /* Yes, they are the same */

CFStringRef SCDynamicStoreKeyCreateNetworkGlobalEntity(CFAllocatorRef allocator, CFStringRef domain, CFStringRef entity)
{
	STUB();
	return NULL;
}

CFPropertyListRef SCDynamicStoreCopyValue(SCDynamicStoreRef store, CFStringRef key)
{
	STUB();
	return NULL;
}

SCDynamicStoreRef SCDynamicStoreCreate(CFAllocatorRef allocator, CFStringRef name, SCDynamicStoreCallBack callout, SCDynamicStoreContext *context)
{
	STUB();
	return NULL;
}

CFStringRef SCDynamicStoreKeyCreate(CFAllocatorRef allocator, CFStringRef fmt, ...)
{
	STUB();
	return NULL;
}

Boolean SCDynamicStoreSetDispatchQueue(SCDynamicStoreRef store, dispatch_queue_t queue)
{
	STUB();
	return FALSE;
}

Boolean SCDynamicStoreSetNotificationKeys(SCDynamicStoreRef store, CFArrayRef __nullable keys, CFArrayRef __nullable patterns) {
	STUB();
	return FALSE;
}
