#ifndef _MDQUERY_H
#define _MDQUERY_H
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFArray.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef CFTypeRef MDQueryRef;

extern CFStringRef kMDItemPath;
extern CFStringRef kMDQueryDidFinishNotification;
extern CFStringRef kMDQueryDidUpdateNotification;
extern CFStringRef kMDQueryUpdateAddedItems;
extern CFStringRef kMDQueryUpdateChangedItems;
extern CFStringRef kMDQueryUpdateRemovedItems;

MDQueryRef MDQueryCreate(CFAllocatorRef allocator, CFStringRef queryString, CFArrayRef valueListAttrs, CFArrayRef sortingAttrs);

void MDQueryDisableUpdates(MDQueryRef query);

void MDQueryEnableUpdates(MDQueryRef query);

Boolean MDQueryExecute(MDQueryRef query, CFOptionFlags optionFlags);

void * MDQueryGetAttributeValueOfResultAtIndex(MDQueryRef query, CFStringRef name, CFIndex idx);

const void * MDQueryGetResultAtIndex(MDQueryRef query, CFIndex idx);

CFIndex MDQueryGetResultCount(MDQueryRef query);

void MDQuerySetSearchScope(MDQueryRef query, CFArrayRef scopeDirectories, OptionBits scopeOptions);

void MDQueryStop(MDQueryRef query);



#ifdef __cplusplus
}
#endif

#endif

