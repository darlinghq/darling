#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SCDynamicStore.h>

extern "C" {

CF_EXPORT const CFStringRef kSCPropNetProxiesExceptionsList;
CF_EXPORT const CFStringRef kSCPropNetProxiesHTTPEnable;
CF_EXPORT const CFStringRef kSCPropNetProxiesHTTPPort;
CF_EXPORT const CFStringRef kSCPropNetProxiesHTTPSProxy;
CF_EXPORT const CFStringRef kSCPropNetProxiesHTTPSPort;
CF_EXPORT const CFStringRef kSCPropNetProxiesFTPEnable;
CF_EXPORT const CFStringRef kSCPropNetProxiesFTPProxy;
CF_EXPORT const CFStringRef kSCPropNetProxiesFTPPort;
CF_EXPORT const CFStringRef kSCPropNetProxiesFTPPassive;

CFStringRef SCDynamicStoreCopyComputerName (SCDynamicStoreRef store, CFStringEncoding *nameEncoding);
CFStringRef SCDynamicStoreCopyConsoleUser (SCDynamicStoreRef store, uid_t *uid, gid_t *gid);
CFStringRef SCDynamicStoreCopyLocalHostName (SCDynamicStoreRef store);
CFStringRef SCDynamicStoreCopyLocation (SCDynamicStoreRef store);
CFDictionaryRef SCDynamicStoreCopyProxies (SCDynamicStoreRef store);

}
