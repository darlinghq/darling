#include <CoreServices/CoreServices.h>
#include <HIToolbox/CarbonEvents.h>
#include <HIToolbox/TextServices.h>

// There is no Text Services Manager under Darling, so a document carries no
// properties. Report that instead of claiming the property was stored.

OSStatus TSMSetDocumentProperty(TSMDocumentID docID, OSType propertyTag, ByteCount propertySize, void *propertyData)
{
	return unimpErr;
}

OSStatus TSMRemoveDocumentProperty(TSMDocumentID docID, OSType propertyTag)
{
	return unimpErr;
}
