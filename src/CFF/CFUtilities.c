
#include "CFUtilities.h"
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
	// TODO: For now just say yep, we are newer
	return true;
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
  CFStringRef str = CFSTR(SHARE_PATH "/SystemVersion.plist");
  return _CFCopyVersionDictionary(str);
}

CFDictionaryRef _CFCopyServerVersionDictionary(void)
{
  CFStringRef str = CFSTR(SHARE_PATH "/SystemVersion.plist");
  return _CFCopyVersionDictionary(str);
}

