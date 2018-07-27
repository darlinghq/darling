#include <CoreServices/MDQuery.h>

CFStringRef kMDItemPath = CFSTR("MDItemPath");
CFStringRef kMDQueryDidFinishNotification = CFSTR("MDQueryDidFinishNotification");
CFStringRef kMDQueryDidUpdateNotification = CFSTR("MDQueryDidUpdateNotification");
CFStringRef kMDQueryUpdateAddedItems = CFSTR("MDQueryUpdateAddedItems");
CFStringRef kMDQueryUpdateChangedItems = CFSTR("MDQueryUpdateChangedItems");
CFStringRef kMDQueryUpdateRemovedItems = CFSTR("MDQueryUpdateRemovedItems");

MDQueryRef MDQueryCreate(CFAllocatorRef allocator, CFStringRef queryString, CFArrayRef valueListAttrs, CFArrayRef sortingAttrs)
{
	return NULL;
}

void MDQueryDisableUpdates(MDQueryRef query)
{
}

void MDQueryEnableUpdates(MDQueryRef query)
{
}

Boolean MDQueryExecute(MDQueryRef query, CFOptionFlags optionFlags)
{
	return FALSE;
}

void * MDQueryGetAttributeValueOfResultAtIndex(MDQueryRef query, CFStringRef name, CFIndex idx)
{
	return NULL;
}

const void * MDQueryGetResultAtIndex(MDQueryRef query, CFIndex idx)
{
	return NULL;
}

CFIndex MDQueryGetResultCount(MDQueryRef query)
{
	return 0;
}

void MDQuerySetSearchScope(MDQueryRef query, CFArrayRef scopeDirectories, OptionBits scopeOptions)
{
}

void MDQueryStop(MDQueryRef query)
{
}


