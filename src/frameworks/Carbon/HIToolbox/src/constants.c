#include <CoreFoundation/CoreFoundation.h>
#include <HIToolbox/TextInputSources.h>

/* HIToolbox.framework */
const CFStringRef kTISNotifySelectedKeyboardInputSourceChanged = CFSTR("com.apple.Carbon.TISNotifySelectedKeyboardInputSourceChanged");
const CFStringRef kTISNotifyEnabledKeyboardInputSourcesChanged = CFSTR("com.apple.Carbon.TISNotifyEnabledKeyboardInputSourcesChanged");

const CFStringRef kTISPropertyInputSourceCategory = CFSTR("TISPropertyInputSourceCategory");
const CFStringRef kTISPropertyInputSourceType = CFSTR("TISPropertyInputSourceType");
const CFStringRef kTISPropertyInputSourceIsASCIICapable = CFSTR("TISPropertyInputSourceIsASCIICapable");
const CFStringRef kTISPropertyInputSourceIsEnableCapable = CFSTR("TISPropertyInputSourceIsEnableCapable");
const CFStringRef kTISPropertyInputSourceIsSelectCapable = CFSTR("TISPropertyInputSourceIsSelectCapable");
const CFStringRef kTISPropertyInputSourceIsSelectable = CFSTR("TISPropertyInputSourceIsSelectable");
const CFStringRef kTISPropertyInputSourceIsEnabled = CFSTR("TISPropertyInputSourceIsEnabled");
const CFStringRef kTISPropertyInputSourceIsSelected = CFSTR("TISPropertyInputSourceIsSelected");
const CFStringRef kTISPropertyInputSourceIsFromSystem = CFSTR("TISPropertyInputSourceIsFromSystem");
const CFStringRef kTISPropertyInputSourceID = CFSTR("TISPropertyInputSourceID");
const CFStringRef kTISPropertyBundleID = CFSTR("TISPropertyBundleID");
const CFStringRef kTISPropertyVersion = CFSTR("TISPropertyVersion");
const CFStringRef kTISPropertyLocalizedName = CFSTR("TISPropertyLocalizedName");
const CFStringRef kTISPropertyInputSourceLanguages = CFSTR("TISPropertyInputSourceLanguages");
const CFStringRef kTISPropertyUnicodeKeyLayoutData = CFSTR("TISPropertyUnicodeKeyLayoutData");
const CFStringRef kTISPropertyIconRef = CFSTR("TISPropertyIconRef");
const CFStringRef kTISPropertyIconImageURL = CFSTR("TISPropertyIconImageURL");

const CFStringRef kTISCategoryKeyboardInputSource = CFSTR("TISCategoryKeyboardInputSource");
const CFStringRef kTISCategoryPaletteInputSource = CFSTR("TISCategoryPaletteInputSource");
const CFStringRef kTISCategoryInkInputSource = CFSTR("TISCategoryInkInputSource");

const CFStringRef kTISTypeKeyboardLayout = CFSTR("TISTypeKeyboardLayout");
const CFStringRef kTISTypeKeyboardInputMode = CFSTR("TISTypeKeyboardInputMode");
const CFStringRef kTISTypeKeyboardInputMethodModeEnabled = CFSTR("TISTypeKeyboardInputMethodModeEnabled");
const CFStringRef kTISTypeKeyboardInputMethodWithoutModes = CFSTR("TISTypeKeyboardInputMethodWithoutModes");
const CFStringRef kTISTypeCharacterPalette = CFSTR("TISTypeCharacterPalette");
const CFStringRef kTISTypeKeyboardViewer = CFSTR("TISTypeKeyboardViewer");
const CFStringRef kTISTypeInk = CFSTR("TISTypeInk");

const float kHIToolboxVersionNumber = 1163.0;
