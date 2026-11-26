#include <HIToolbox/TextInputSources.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFData.h>
#include <os/lock.h>
#include <stdlib.h>
#import <Foundation/Foundation.h>

static os_unfair_lock g_keyboardLock = OS_UNFAIR_LOCK_INIT;
static TISInputSourceRef g_lastKeyboardLayout = NULL;

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

TISInputSourceRef TISCopyCurrentKeyboardInputSource(void)
{
	return TISCopyCurrentKeyboardLayoutInputSource();
}

// The keyboard layout belongs to the window-system backend in CoreGraphics (which sits below
// HIToolbox): a dictionary with the "data" ('uchr'), "name" and "fullName" of the current layout,
// or NULL when the backend has none.
extern "C" CFDictionaryRef _CGSCopyCurrentKeyboardLayoutInfo(void);
static CFDictionaryRef copyBackendKeyboardLayout(void)
{
	return _CGSCopyCurrentKeyboardLayoutInfo();
}

TISInputSourceRef TISCopyCurrentKeyboardLayoutInputSource(void)
{
	NSDictionary* info = [(NSDictionary*) copyBackendKeyboardLayout() autorelease];
	CFDataRef data = (CFDataRef) info[@"data"];
	if (!data)
		return NULL;

	os_unfair_lock_lock(&g_keyboardLock);
	if (g_lastKeyboardLayout != NULL)
	{
		CFDataRef lastData = (CFDataRef) CFDictionaryGetValue((CFDictionaryRef) g_lastKeyboardLayout, kTISPropertyUnicodeKeyLayoutData);
		if (lastData && CFEqual(lastData, data))
		{
			TISInputSourceRef rv = (TISInputSourceRef) CFRetain((CFDictionaryRef) g_lastKeyboardLayout);
			os_unfair_lock_unlock(&g_keyboardLock);
			return rv;
		}
	}
	os_unfair_lock_unlock(&g_keyboardLock);

	NSString *name = info[@"name"], *fullName = info[@"fullName"];
	NSString *sourceID = name ? [NSString stringWithFormat:@"com.apple.keylayout.%@", name] : @"com.apple.keylayout.US";
	NSArray *languages = @[ @"en" ];

	NSDictionary *dictObj = @{
		(id)kTISPropertyInputSourceID: sourceID,
		(id)kTISPropertyLocalizedName: fullName ?: (name ?: @"U.S."),
		(id)kTISPropertyInputSourceLanguages: languages,
		(id)kTISPropertyUnicodeKeyLayoutData: (id)data,
		(id)kTISPropertyInputSourceCategory: (id)kTISCategoryKeyboardInputSource,
		(id)kTISPropertyInputSourceType: (id)kTISTypeKeyboardLayout,
		(id)kTISPropertyInputSourceIsASCIICapable: (id)kCFBooleanTrue,
		(id)kTISPropertyInputSourceIsSelectable: (id)kCFBooleanTrue,
		(id)kTISPropertyInputSourceIsSelectCapable: (id)kCFBooleanTrue,
		(id)kTISPropertyInputSourceIsEnabled: (id)kCFBooleanTrue,
		(id)kTISPropertyInputSourceIsSelected: (id)kCFBooleanTrue,
		(id)kTISPropertyInputSourceIsEnableCapable: (id)kCFBooleanTrue,
	};
	CFDictionaryRef dict = (CFDictionaryRef)[dictObj retain];

	os_unfair_lock_lock(&g_keyboardLock);

	if (g_lastKeyboardLayout)
		CFRelease(g_lastKeyboardLayout);

	g_lastKeyboardLayout = (TISInputSourceRef) CFRetain(dict);

	os_unfair_lock_unlock(&g_keyboardLock);

	return (TISInputSourceRef) dict;
}

void* TISGetInputSourceProperty(TISInputSourceRef inputSourceRef, CFStringRef key)
{
	if (!inputSourceRef || !key)
		return NULL;
	return (void*) CFDictionaryGetValue((CFDictionaryRef)inputSourceRef, key);
}

CFTypeID TISInputSourceGetTypeID(void)
{
	return CFDictionaryGetTypeID();
}

CFArrayRef TISCreateInputSourceList(CFDictionaryRef properties, Boolean includeAllInstalled)
{
	TISInputSourceRef current = TISCopyCurrentKeyboardLayoutInputSource();
	if (!current)
		return (CFArrayRef)[[NSArray array] retain];

	if (properties)
	{
		NSDictionary *filter = (NSDictionary *)properties;
		NSDictionary *source = (NSDictionary *)current;
		for (id key in filter)
		{
			id filterVal = filter[key];
			id sourceVal = source[key];
			// Only sources that have every requested property, with an equal
			// value, match. A source without the property does not.
			if (!sourceVal || ![filterVal isEqual:sourceVal])
			{
				CFRelease(current);
				return (CFArrayRef)[[NSArray array] retain];
			}
		}
	}

	NSArray *list = [NSArray arrayWithObject:(id)current];
	CFRelease(current);
	return (CFArrayRef)[list retain];
}

CFArrayRef TISCreateASCIICapableInputSourceList(void)
{
	// The enabled keyboard input sources that can type ASCII.
	const void* keys[] = { kTISPropertyInputSourceCategory, kTISPropertyInputSourceIsASCIICapable };
	const void* values[] = { kTISCategoryKeyboardInputSource, kCFBooleanTrue };
	CFDictionaryRef filter = CFDictionaryCreate(NULL, keys, values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	CFArrayRef list = TISCreateInputSourceList(filter, false);
	CFRelease(filter);
	return list;
}

TISInputSourceRef TISCopyCurrentASCIICapableKeyboardInputSource(void)
{
	return TISCopyCurrentKeyboardLayoutInputSource();
}

TISInputSourceRef TISCopyCurrentASCIICapableKeyboardLayoutInputSource(void)
{
	return TISCopyCurrentKeyboardLayoutInputSource();
}

TISInputSourceRef TISCopyInputSourceForLanguage(CFStringRef language)
{
	return TISCopyCurrentKeyboardLayoutInputSource();
}

OSStatus TISSelectInputSource(TISInputSourceRef inputSource)
{
	return noErr;
}

OSStatus TISEnableInputSource(TISInputSourceRef inputSource)
{
	return noErr;
}

OSStatus TISDisableInputSource(TISInputSourceRef inputSource)
{
	return noErr;
}

OSStatus TISRegisterInputSource(CFURLRef location)
{
	return noErr;
}

OSStatus TISDeregisterInputSource(TISInputSourceRef inputSource)
{
	return noErr;
}

// CarbonCore's keyboard-layout provider (for the KCHR cache of GetScriptManagerVariable): the
// 'uchr' data of the current keyboard layout, retained, or NULL.
extern "C" void CarbonCoreSetKeyboardLayoutProvider(CFDataRef (*provider)(void));

static CFDataRef copyCurrentLayoutData(void)
{
	TISInputSourceRef source = TISCopyCurrentKeyboardLayoutInputSource();
	if (!source)
		return NULL;
	CFDataRef data = (CFDataRef) TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData);
	if (data)
		CFRetain(data);
	CFRelease(source);
	return data;
}

__attribute__((constructor))
static void registerKeyboardLayoutProvider(void)
{
	CarbonCoreSetKeyboardLayoutProvider(copyCurrentLayoutData);
}
