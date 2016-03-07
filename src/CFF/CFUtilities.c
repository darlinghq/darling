
#include "CoreFoundation/CFPriv.h"
#include "CoreFoundation/CFLogUtilities.h"
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFPropertyList.h>
#include <stddef.h>
#include "CFStringConst.h"
#include <stdbool.h>
#include "darling-config.h"

CONST_STRING_DECL(_kCFSystemVersionProductNameKey, "ProductName");
CONST_STRING_DECL(_kCFSystemVersionProductCopyrightKey, "ProductCopyright");
CONST_STRING_DECL(_kCFSystemVersionProductVersionKey, "ProductVersion");
CONST_STRING_DECL(_kCFSystemVersionProductVersionExtraKey, "ProductVersionExtra");
CONST_STRING_DECL(_kCFSystemVersionProductUserVisibleVersionKey, "UserVisibleVersion");
CONST_STRING_DECL(_kCFSystemVersionBuildVersionKey, "ProductBuildVersion");
CONST_STRING_DECL(_kCFSystemVersionProductVersionStringKey, "ProductVersionString");
CONST_STRING_DECL(_kCFSystemVersionBuildStringKey, "ProductBuildString");

__attribute__((constructor))
	static void initConstants()
{
	CFTypeID type = CFStringGetTypeID();
	GSRuntimeConstantInit(_kCFSystemVersionProductNameKey, type);
	GSRuntimeConstantInit(_kCFSystemVersionProductCopyrightKey, type);
	GSRuntimeConstantInit(_kCFSystemVersionProductVersionKey, type);
	GSRuntimeConstantInit(_kCFSystemVersionProductVersionExtraKey, type);
	GSRuntimeConstantInit(_kCFSystemVersionBuildVersionKey, type);
	GSRuntimeConstantInit(_kCFSystemVersionProductUserVisibleVersionKey, type);
	GSRuntimeConstantInit(_kCFSystemVersionProductVersionStringKey, type);
	GSRuntimeConstantInit(_kCFSystemVersionBuildStringKey, type);
}

Boolean _CFExecutableLinkedOnOrAfter(CFSystemVersion version)
{
	// Like original CF, we always use the last version
	const uint16_t libSystemVersions[] = {
		50,
		55,
		56,
		67,
		73,
		89,
		125, // 10.6
		159, // 10.7
		169, // 10.8
		1197, // 10.9
		1213, // 10.10
		1226, // 10.11 (up until now)
	};
	if (version >= CFSystemVersionMax)
		return false;
	return (NSVersionOfLinkTimeLibrary("System") >> 16) >= libSystemVersions[version];
}

static CFDictionaryRef _CFCopyVersionDictionary(CFStringRef path)
{
  CFURLRef url;
  CFDataRef resourceData;
  Boolean status;
  SInt32 errorCode;
  CFErrorRef err;
  CFPropertyListRef plist;

  url = CFURLCreateWithFileSystemPath(NULL, path, kCFURLPOSIXPathStyle, 0);
  status = CFURLCreateDataAndPropertiesFromResource(NULL, url, &resourceData,
		  NULL, NULL, &errorCode);

  if (!status)
  {
	  CFRelease(url);
	  return NULL;
  }

  plist = CFPropertyListCreateWithData(NULL, resourceData, kCFPropertyListImmutable,
		  NULL, &err);

  CFRelease(resourceData);
  CFRelease(url);

  return plist;
}

CFDictionaryRef _CFCopySystemVersionDictionary(void)
{
  CFStringRef str = CFSTR("/System/Library/CoreServices/SystemVersion.plist");
  return _CFCopyVersionDictionary(str);
}

CFDictionaryRef _CFCopyServerVersionDictionary(void)
{
  CFStringRef str = CFSTR("/System/Library/CoreServices/SystemVersion.plist");
  return _CFCopyVersionDictionary(str);
}

void CFLog(int32_t level, CFStringRef format, ...)
{
}

