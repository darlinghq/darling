#ifndef _UTTYPE_H
#define _UTTYPE_H

#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFArray.h>
#include <CoreServices/MacTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const CFStringRef kUTExportedTypeDeclarationsKey;

extern const CFStringRef kUTImportedTypeDeclarationsKey;

extern const CFStringRef kUTTypeIdentifierKey;

extern const CFStringRef kUTTypeTagSpecificationKey;

extern const CFStringRef kUTTypeConformsToKey;

extern const CFStringRef kUTTypeDescriptionKey;

extern const CFStringRef kUTTypeIconFileKey;

extern const CFStringRef kUTTypeReferenceURLKey;

extern const CFStringRef kUTTypeVersionKey;
extern const CFStringRef kUTTagClassFilenameExtension;

extern const CFStringRef kUTTagClassMIMEType;

extern const CFStringRef kUTTagClassNSPboardType;

extern const CFStringRef kUTTagClassOSType;

extern _Nullable CFStringRef
UTTypeCreatePreferredIdentifierForTag(
  CFStringRef inTagClass,
  CFStringRef inTag,
  _Nullable CFStringRef inConformingToUTI);

extern _Nullable CFArrayRef
UTTypeCreateAllIdentifiersForTag(
  CFStringRef inTagClass,
  CFStringRef inTag,
  _Nullable CFStringRef inConformingToUTI);

extern _Nullable CFStringRef
UTTypeCopyPreferredTagWithClass(
  CFStringRef inUTI,
  CFStringRef inTagClass);

extern _Nullable CFArrayRef
UTTypeCopyAllTagsWithClass(
  CFStringRef inUTI,
  CFStringRef inTagClass);

extern Boolean
UTTypeEqual(
  CFStringRef inUTI1,
  CFStringRef inUTI2);

extern Boolean
UTTypeConformsTo(
  CFStringRef inUTI,
  CFStringRef inConformsToUTI);

extern _Nullable CFStringRef
UTTypeCopyDescription(CFStringRef inUTI);

extern Boolean
UTTypeIsDeclared(CFStringRef inUTI);

extern Boolean
UTTypeIsDynamic(CFStringRef inUTI);

extern _Nullable CFDictionaryRef
UTTypeCopyDeclaration(CFStringRef inUTI);

extern _Nullable CFURLRef
UTTypeCopyDeclaringBundleURL(CFStringRef inUTI);

extern CFStringRef
UTCreateStringForOSType(OSType inOSType);

extern OSType
UTGetOSTypeFromString(CFStringRef inString);

#ifdef __cplusplus
}
#endif

#endif
