#ifndef CFUTILITIES_H
#define CFUTILITIES_H

#include <stddef.h>
#include <stdio.h>
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDictionary.h>

typedef int CFSystemVersion;

extern const CFStringRef _kCFSystemVersionProductNameKey;
extern const CFStringRef _kCFSystemVersionProductCopyrightKey;
extern const CFStringRef _kCFSystemVersionProductVersionKey;
extern const CFStringRef _kCFSystemVersionProductVersionExtraKey;
extern const CFStringRef _kCFSystemVersionProductUserVisibleVersionKey;
extern const CFStringRef _kCFSystemVersionBuildVersionKey;
extern const CFStringRef _kCFSystemVersionProductVersionStringKey;
extern const CFStringRef _kCFSystemVersionBuildStringKey;

CF_EXPORT Boolean _CFExecutableLinkedOnOrAfter(CFSystemVersion version);

CF_EXPORT CFDictionaryRef _CFCopySystemVersionDictionary(void);
CF_EXPORT CFDictionaryRef _CFCopyServerVersionDictionary(void);

#endif

