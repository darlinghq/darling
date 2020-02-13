#include "TextInputSources.h"
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFData.h>
#import <AppKit/NSDisplay.h>

TISInputSourceRef TISCopyCurrentKeyboardLayoutInputSource(void)
{
	NSDisplay* display = [NSClassFromString(@"NSDisplay") currentDisplay];
	if (!display)
		return NULL;

	uint32_t length;
	UCKeyboardLayout* layout = [display keyboardLayout: &length];
	if (!layout)
		return NULL;

	CFDataRef data = CFDataCreate(NULL, (UInt8*) layout, length);
	free(layout);

	const void* keys[1] = { kTISPropertyUnicodeKeyLayoutData };
	const void* values[1] = { data };
	CFDictionaryRef dict = CFDictionaryCreate(NULL, keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	CFRelease(data);
	return (TISInputSourceRef) dict;
}

void* TISGetInputSourceProperty(TISInputSourceRef inputSourceRef, CFStringRef key)
{
	return (void*) CFDictionaryGetValue((CFDictionaryRef)inputSourceRef, key);
}

