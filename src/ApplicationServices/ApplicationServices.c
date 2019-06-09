/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CGDirectDisplay.h>

const char* ApplicationServicesVersionString = "Darling ApplicationServices-48";
const unsigned long long ApplicationServicesVersionNumber = 0x4048000000000000;
const CFStringRef kAXUIElementCopyHierarchyArrayAttributesKey = CFSTR("AXCHAA");
const CFStringRef kAXUIElementCopyHierarchyMaxArrayCountKey = CFSTR("AXCHMAC");
const CFStringRef kAXUIElementCopyHierarchyResultValueKey = CFSTR("value");
const CFStringRef kAXUIElementCopyHierarchyReturnAttributeErrorsKey = CFSTR("AXCHRE");
const CFStringRef kAXUIElementCopyHierarchySkipInspectionForAttributesKey = CFSTR("AXCHSIA");
const CFStringRef kAXUIElementCopyHierarchyTruncateStringsKey = CFSTR("AXTRUNC");
/* Not const for some reason */
CFStringRef kAXTrustedCheckOptionPrompt = CFSTR("AXTrustedCheckOptionPrompt");

#include "MacTypes.h"

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

// Types we need for the following stubs...

typedef uint32_t ATSFontRef;
typedef OptionBits ATSOptionFlags;
typedef void* ATSUTextLayout;
typedef void* ATSUStyle;
typedef uint32_t ATSUAttributeTag;
typedef Fixed ATSUTextMeasurement;
typedef void* ATSUAttributeValuePtr;

typedef uint32_t UniCharArrayOffset;
typedef char* ConstUniCharArrayPtr;

typedef void* ATSFontMetrics;
typedef void** GDHandle;

typedef uint32_t FMFont;



// ATS Stubs

ATSFontRef ATSFontFindFromName(CFStringRef a, ATSOptionFlags b)
{
    if (verbose) puts("STUB: ATSFontFindFromName called");
	return 0;
}

OSStatus ATSFontGetHorizontalMetrics(ATSFontRef a, ATSOptionFlags b, ATSFontMetrics *c)
{
    if (verbose) puts("STUB: ATSFontGetHorizontalMetrics called");
	return 0;
}

OSStatus ATSUCreateStyle(ATSUStyle *a)
{
    if (verbose) puts("STUB: ATSUCreateStyle called");
	return 0;
}

OSStatus ATSUCreateTextLayout(ATSUTextLayout *a)
{
    if (verbose) puts("STUB: ATSUCreateTextLayout called");
	return 0;
}

OSStatus ATSUDisposeStyle(ATSUStyle *a)
{
    if (verbose) puts("STUB: ATSUDisposeStyle called");
	return 0;
}

OSStatus ATSUDisposeTextLayout(ATSUTextLayout *a)
{
    if (verbose) puts("STUB: ATSUDisposeTextLayout called");
	return 0;
}

OSStatus ATSUDrawText(ATSUTextLayout a, UniCharArrayOffset b, UniCharCount c, ATSUTextMeasurement d, ATSUTextMeasurement e)
{
    if (verbose) puts("STUB: ATSUDrawText called");
	return 0;
}

OSStatus ATSUGetUnjustifiedBounds(ATSUTextLayout a , UniCharArrayOffset b , UniCharCount c , ATSUTextMeasurement * d , ATSUTextMeasurement * e , ATSUTextMeasurement * f , ATSUTextMeasurement * g)
{
    if (verbose) puts("STUB: ATSUGetUnjustifiedBounds called");
	return 0;
}

OSStatus ATSUMeasureTextImage(ATSUTextLayout a, UniCharArrayOffset b, UniCharCount c, ATSUTextMeasurement d, ATSUTextMeasurement e, Rect *f)
{
    if (verbose) puts("STUB: ATSUMeasureTextImage called");
	return 0;
}

OSStatus ATSUSetAttributes(ATSUStyle a, ItemCount b, const ATSUAttributeTag *c, const ByteCount *d, const ATSUAttributeValuePtr *e)
{
    if (verbose) puts("STUB: ATSUSetAttributes called");
	return 0;
}

OSStatus ATSUSetLayoutControls(ATSUTextLayout a, ItemCount b, const ATSUAttributeTag *c, const ByteCount *d, const ATSUAttributeValuePtr *e)
{
    if (verbose) puts("STUB: ATSUSetLayoutControls called");
	return 0;
}

OSStatus ATSUSetRunStyle(ATSUTextLayout a, ATSUStyle b, UniCharArrayOffset c, UniCharCount d)
{
    if (verbose) puts("STUB: ATSUSetRunStyle called");
	return 0;
}

OSStatus ATSUSetTextPointerLocation(ATSUTextLayout a, ConstUniCharArrayPtr b, UniCharArrayOffset c, UniCharCount d, UniCharCount e)
{
    if (verbose) puts("STUB: ATSUSetTextPointerLocation called");
	return 0;
}

OSStatus ATSUSetTransientFontMatching(ATSUTextLayout a, Boolean b)
{
    if (verbose) puts("STUB: ATSUSetTransientFontMatching called");
	return 0;
}

// End ATS Stubs

// QuickDraw Stubs

void InitCursor()
{
    if (verbose) puts("STUB: InitCursor called");	
}

static uint32_t gFakeScreenDevice;

GDHandle DMGetFirstScreenDevice(Boolean a)
{
    if (verbose) puts("STUB: DMGetFirstScreenDevice called");
	return (GDHandle)&gFakeScreenDevice;
}

GDHandle DMGetNextScreenDevice (GDHandle a, Boolean b)
{
    if (verbose) puts("STUB: DMGetNextScreenDevice called");
	return (GDHandle)0;
}

GDHandle GetMainDevice()
{
    if (verbose) puts("STUB: GetMainDevice called");
	return (GDHandle)0;
}

CGDirectDisplayID QDGetCGDirectDisplayID(GDHandle a)
{
    if (verbose) puts("STUB: QDGetCGDirectDisplayID called");
	return (CGDirectDisplayID)a;
}

// End QuickDraw Stubs


// Old QuickDraw Font Stubs

FMFont FMGetFontFromATSFontRef(ATSFontRef a)
{
    if (verbose) puts("STUB: FMGetFontFromATSFontRef called");
	return (FMFont)0;
}

// End Old QuickDraw Font Stubs

