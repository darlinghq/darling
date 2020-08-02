#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <SystemConfiguration/SystemConfiguration.h>

#define STUB() printf("STUB %s\n", __func__)

const CFStringRef kSCDynamicStoreDomainState = CFSTR("State:");
const CFStringRef kSCEntNetIPv4 = CFSTR("IPv4");
const CFStringRef kSCEntNetIPv6 = CFSTR("IPv6");
const CFStringRef kSCPropNetIPv4Router = CFSTR("Router");
const CFStringRef kSCPropNetIPv6Router = CFSTR("Router"); /* Yes, they are the same */
const CFStringRef kSCCompSystem = CFSTR("System");

const CFStringRef kSCConsoleSessionUserName		= CFSTR("kCGSSessionUserNameKey");		/* value is CFString */
const CFStringRef kSCConsoleSessionUID			= CFSTR("kCGSSessionUserIDKey");		/* value is CFNumber (a uid_t) */
const CFStringRef kSCConsoleSessionConsoleSet		= CFSTR("kCGSSessionConsoleSetKey");		/* value is CFNumber */
const CFStringRef kSCConsoleSessionOnConsole		= CFSTR("kCGSSessionOnConsoleKey");		/* value is CFBoolean */
const CFStringRef kSCConsoleSessionLoginDone		= CFSTR("kCGSessionLoginDoneKey");		/* value is CFBoolean */
const CFStringRef kSCConsoleSessionID			= CFSTR("kCGSSessionIDKey");			/* value is CFNumber */
const CFStringRef kSCConsoleSessionSystemSafeBoot	= CFSTR("kCGSSessionSystemSafeBoot");		/* value is CFBoolean */
const CFStringRef kSCConsoleSessionLoginwindowSafeLogin	= CFSTR("kCGSSessionLoginwindowSafeLogin");	/* value is CFBoolean */

const CFStringRef kSCCompAnyRegex = CFSTR("[^/]+");
const CFStringRef kSCEntNetDNS = CFSTR("DNS");

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

CFStringRef SCDynamicStoreKeyCreateComputerName(CFAllocatorRef allocator) {
	STUB();
	return NULL;
};

Boolean SCPreferencesSetComputerName(SCPreferencesRef	prefs, CFStringRef __nullable	name, CFStringEncoding	nameEncoding) {
	STUB();
	return FALSE;
};

Boolean SCPreferencesSetLocalHostName(SCPreferencesRef	prefs, CFStringRef __nullable	name) {
	STUB();
	return FALSE;
};

CFStringRef SCDynamicStoreKeyCreateHostNames(CFAllocatorRef allocator) {
	STUB();
	return NULL;
};

CFStringRef SCDynamicStoreKeyCreateNetworkInterfaceEntity(CFAllocatorRef allocator, CFStringRef domain, CFStringRef ifname, CFStringRef entity) {
	STUB();
	return NULL;
};
