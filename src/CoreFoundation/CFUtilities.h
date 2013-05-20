#ifndef CFUTILITIES_H
#define CFUTILITIES_H

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDictionary.h>

typedef enum {
} CFSystemVersion;

CF_EXPORT Boolean _CFExecutableLinkedOnOrAfter(CFSystemVersion version);

CF_EXPORT CFDictionaryRef _CFCopySystemVersionDictionary(void);
CF_EXPORT CFDictionaryRef _CFCopyServerVersionDictionary(void);

#endif

