#ifndef _TEXTINPUTSOURCES_H
#define _TEXTINPUTSOURCES_H
#include <CoreFoundation/CFString.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __TISInputSource* TISInputSourceRef;

extern const CFStringRef kTISPropertyUnicodeKeyLayoutData;
extern const CFStringRef kTISPropertyInputSourceLanguages;
extern const CFStringRef kTISPropertyLocalizedName;

extern TISInputSourceRef TISCopyCurrentKeyboardLayoutInputSource(void);
extern TISInputSourceRef TISCopyCurrentKeyboardInputSource(void);

extern void* TISGetInputSourceProperty(TISInputSourceRef inputSourceRef, CFStringRef key);

#ifdef __cplusplus
}
#endif

#endif
