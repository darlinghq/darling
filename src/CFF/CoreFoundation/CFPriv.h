#ifndef CFUTILITIES_H
#define CFUTILITIES_H

#include <stddef.h>
#include <stdio.h>
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDictionary.h>

enum {
	CFSystemVersionCheetah = 0,         /* 10.0 */
	CFSystemVersionPuma = 1,            /* 10.1 */
	CFSystemVersionJaguar = 2,          /* 10.2 */
	CFSystemVersionPanther = 3,         /* 10.3 */
	CFSystemVersionTiger = 4,           /* 10.4 */
	CFSystemVersionLeopard = 5,         /* 10.5 */
	CFSystemVersionSnowLeopard = 6,     /* 10.6 */
	CFSystemVersionLion = 7,            /* 10.7 */
	CFSystemVersionMountainLion = 8,    /* 10.8 */
	CFSystemVersionMavericks = 9,       /* 10.9 */
	CFSystemVersionYosemite = 10,       /* 10.10 */
	CFSystemVersionElCapitan = 11,      /* 11.11 */
    CFSystemVersionMax,

};
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

