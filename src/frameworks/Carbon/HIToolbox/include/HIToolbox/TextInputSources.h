#ifndef _Carbon_TextInputSources_H_
#define _Carbon_TextInputSources_H_
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFURL.h>
#include <MacTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const CFStringRef kTISNotifySelectedKeyboardInputSourceChanged;
extern const CFStringRef kTISNotifyEnabledKeyboardInputSourcesChanged;

extern const CFStringRef kTISPropertyInputSourceCategory;
extern const CFStringRef kTISPropertyInputSourceType;
extern const CFStringRef kTISPropertyInputSourceIsASCIICapable;
extern const CFStringRef kTISPropertyInputSourceIsEnableCapable;
extern const CFStringRef kTISPropertyInputSourceIsSelectCapable;
extern const CFStringRef kTISPropertyInputSourceIsSelectable;
extern const CFStringRef kTISPropertyInputSourceIsEnabled;
extern const CFStringRef kTISPropertyInputSourceIsSelected;
extern const CFStringRef kTISPropertyInputSourceID;
extern const CFStringRef kTISPropertyBundleID;
extern const CFStringRef kTISPropertyVersion;
extern const CFStringRef kTISPropertyLocalizedName;
extern const CFStringRef kTISPropertyInputSourceLanguages;
extern const CFStringRef kTISPropertyUnicodeKeyLayoutData;
extern const CFStringRef kTISPropertyIconRef;
extern const CFStringRef kTISPropertyIconImageURL;

extern const CFStringRef kTISCategoryKeyboardInputSource;
extern const CFStringRef kTISCategoryPaletteInputSource;
extern const CFStringRef kTISCategoryInkInputSource;

extern const CFStringRef kTISTypeKeyboardLayout;
extern const CFStringRef kTISTypeKeyboardInputMode;
extern const CFStringRef kTISTypeKeyboardInputMethodModeEnabled;
extern const CFStringRef kTISTypeKeyboardInputMethodWithoutModes;
extern const CFStringRef kTISTypeCharacterPalette;
extern const CFStringRef kTISTypeKeyboardViewer;
extern const CFStringRef kTISTypeInk;

typedef struct __TISInputSource* TISInputSourceRef;

extern CFTypeID TISInputSourceGetTypeID(void);

extern CFArrayRef TISCreateInputSourceList(CFDictionaryRef properties, Boolean includeAllInstalled);

extern TISInputSourceRef TISCopyCurrentKeyboardLayoutInputSource(void);
extern TISInputSourceRef TISCopyCurrentKeyboardInputSource(void);
extern TISInputSourceRef TISCopyCurrentASCIICapableKeyboardInputSource(void);
extern TISInputSourceRef TISCopyCurrentASCIICapableKeyboardLayoutInputSource(void);
extern TISInputSourceRef TISCopyInputSourceForLanguage(CFStringRef language);

extern OSStatus TISSelectInputSource(TISInputSourceRef inputSource);
extern OSStatus TISEnableInputSource(TISInputSourceRef inputSource);
extern OSStatus TISDisableInputSource(TISInputSourceRef inputSource);

extern void* TISGetInputSourceProperty(TISInputSourceRef inputSourceRef, CFStringRef key);

extern OSStatus TISRegisterInputSource(CFURLRef location);
extern OSStatus TISDeregisterInputSource(TISInputSourceRef inputSource);

#ifdef __cplusplus
}
#endif

#endif
