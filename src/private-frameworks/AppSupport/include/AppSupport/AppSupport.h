/*
	AppSupport.private

	Apple's private framework backing the common UI services: the address/city
	database, the radios preference pane, and a few CoreGraphics-adjacent
	helpers. Seven of the local apps link it.

	Declared in two places because the callers split: the two classes are
	Objective-C, referenced through the runtime, so they need no declaration to
	bind; the three C functions do, and are declared here with the types the
	callers use.
*/

#ifndef APPSUPPORT_H
#define APPSUPPORT_H

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

void CPLoggingFlush(void);
CFArrayRef _CPBitmapCreateImagesFromPath(CFStringRef path, CFDictionaryRef options);
Boolean _CPBitmapWriteImagesToPath(CFArrayRef images, CFStringRef path, CFDictionaryRef options);

#ifdef __cplusplus
}
#endif

#endif
