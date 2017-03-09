#ifndef SCDYNAMICSTORECOPYSPECIFIC_H
#define SCDYNAMICSTORECOPYSPECIFIC_H

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SCDynamicStore.h>
#include <SystemConfiguration/SCSchemaDefinitions.h>

#ifdef __cplusplus
extern "C" {
#endif

CFStringRef SCDynamicStoreCopyComputerName (SCDynamicStoreRef store, CFStringEncoding *nameEncoding);
CFStringRef SCDynamicStoreCopyConsoleUser (SCDynamicStoreRef store, uid_t *uid, gid_t *gid);
CFStringRef SCDynamicStoreCopyLocalHostName (SCDynamicStoreRef store);
CFStringRef SCDynamicStoreCopyLocation (SCDynamicStoreRef store);
CFDictionaryRef SCDynamicStoreCopyProxies (SCDynamicStoreRef store);

#ifdef __cplusplus
}
#endif

#endif

