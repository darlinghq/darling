#include <CoreFoundation/CoreFoundation.h>
#include <HIToolbox/TextInputSources.h>

/* HIToolbox.framework */
const CFStringRef kTISPropertyInputSourceID = CFSTR("TISPropertyInputSourceID");
const CFStringRef kTISPropertyInputSourceIsFromSystem = CFSTR("TISPropertyInputSourceIsFromSystem");
const CFStringRef kTISPropertyInputSourceIsASCIICapable = CFSTR("TISPropertyInputSourceIsASCIICapable");

const CFStringRef kTISPropertyUnicodeKeyLayoutData = CFSTR("TISPropertyUnicodeKeyLayoutData");
const CFStringRef kTISCategoryKeyboardInputSource = CFSTR("TISCategoryKeyboardInputSource");
const CFStringRef kTISPropertyInputSourceCategory = CFSTR("TISPropertyInputSourceCategory");
const CFStringRef kTISPropertyInputSourceType = CFSTR("TISPropertyInputSourceType");
const CFStringRef kTISTypeKeyboardLayout = CFSTR("TISTypeKeyboardLayout");
