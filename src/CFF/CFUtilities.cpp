
#include "CFUtilities.h"
#include <CoreFoundation/CFString.h>
#include <stddef.h>

Boolean _CFExecutableLinkedOnOrAfter(CFSystemVersion version)
{
	// TODO: For now just say yep, we are newer
	return true;
}

static CFDictionaryRef _CFCopyVersionDictionary(CFStringRef path)
{
  static const CFStringRef keys[] =
  {
    CFSTR("ProductBuildVersion"),
    CFSTR("ProductCopyright"),
    CFSTR("ProductName"),
    CFSTR("ProductUserVisibleVersion"),
    CFSTR("ProductVersion")
  };

  static const CFStringRef values[] =
  {
    CFSTR("12345678"),
    CFSTR("See Source Files"),
    CFSTR("Darling"),
    CFSTR("10.8"),
    CFSTR("10.8")
  };

  return CFDictionaryCreate(NULL, (const void**)&keys[0], (const void**)&values[0], 5, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
}

CFDictionaryRef _CFCopySystemVersionDictionary(void)
{
  // TODO: Make this a real file installed by Darling and read by _CFCopyVersionDictionary
  return _CFCopyVersionDictionary(CFSTR("/System/Library/CoreServices/SystemVersion.plist"));
}

CFDictionaryRef _CFCopyServerVersionDictionary(void)
{
  // TODO: Make this a real file installed by Darling and read by _CFCopyVersionDictionary
  return _CFCopyVersionDictionary(CFSTR("/System/Library/CoreServices/ServerVersion.plist"));
}

