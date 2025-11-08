#ifndef _Carbon_TextInputSources_H_
#define _Carbon_TextInputSources_H_
#include <CoreFoundation/CFString.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const CFStringRef kTISNotifySelectedKeyboardInputSourceChanged;

extern const CFStringRef kTISPropertyInputSourceID;
extern const CFStringRef kTISPropertyUnicodeKeyLayoutData;
extern const CFStringRef kTISCategoryKeyboardInputSource;
extern const CFStringRef kTISPropertyInputSourceCategory;
extern const CFStringRef kTISPropertyInputSourceType;
extern const CFStringRef kTISTypeKeyboardLayout;
extern const CFStringRef kTISPropertyInputSourceLanguages;
extern const CFStringRef kTISPropertyLocalizedName;

typedef struct __TISInputSource* TISInputSourceRef;

extern TISInputSourceRef TISCopyCurrentKeyboardLayoutInputSource(void);
extern TISInputSourceRef TISCopyCurrentKeyboardInputSource(void);

extern void* TISGetInputSourceProperty(TISInputSourceRef inputSourceRef, CFStringRef key);

extern TISInputSourceRef TISCopyCurrentASCIICapableKeyboardLayoutInputSource(void);

#ifdef __cplusplus
}
#endif

#endif
