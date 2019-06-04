#include <CoreFoundation/CoreFoundation.h>

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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CGDirectDisplay.h>
#include <CoreGraphics/CGGeometry.h>
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

typedef float CGGammaValue;
typedef int32_t CGWindowLevel;

typedef uint32_t UniCharArrayOffset;
typedef char* ConstUniCharArrayPtr;

typedef void* ATSFontMetrics;
typedef void** GDHandle;

typedef uint32_t FMFont;


// CGDirect Stubs - should probably be in cocoatron

CGError CGDisplayShowCursor(CGDirectDisplayID display)
{
    if (verbose) puts("STUB: CGDisplayShowCursor called");
	return (CGError)0;
}

boolean_t CGCursorIsVisible(void)
{
    if (verbose) puts("STUB: CGCursorIsVisible called");
	return false;
}

CFArrayRef CGDisplayAvailableModes(CGDirectDisplayID a)
{
	// Usual keys: Width, Height, Mode, BitsPerPixel, SamplesPerPixel, RefreshRate, UsableForDesktopGUI, IOFlags, kCGDisplayBytesPerRow, IODisplayModeID

    if (verbose) puts("STUB: CGDisplayAvailableModes called");

    CFTypeRef arrayValues[ 1 ];

    CFTypeRef dummyDisplayKeys[ 3 ];
    CFTypeRef dummyDisplayValues[ 3 ];

    dummyDisplayKeys[0] = CFSTR("Width");
    dummyDisplayKeys[1] = CFSTR("Height");
    dummyDisplayKeys[2] = CFSTR("BitsPerPixel");

    // TODO: put some real values here
    int val = 640;
    dummyDisplayValues[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &val);
    val = 480;
    dummyDisplayValues[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &val);
    val = 32;
    dummyDisplayValues[2] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &val);

    CFDictionaryRef dictionary = CFDictionaryCreate(
        kCFAllocatorDefault,
        (const void **)&dummyDisplayKeys,
        (const void **)&dummyDisplayValues,
        3,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks
    );

    arrayValues[0] = dictionary;

    CFArrayRef ret = CFArrayCreate(kCFAllocatorDefault, 
    	                      &arrayValues, 
    	                      1, 
    	                      &kCFTypeArrayCallBacks);

    CFRelease(dummyDisplayValues[0]);
    CFRelease(dummyDisplayValues[1]);
    CFRelease(dummyDisplayValues[2]);

    // TODO: autorelease ret?
	return ret;
}

CGRect CGDisplayBounds(CGDirectDisplayID a)
{
    if (verbose) puts("STUB: CGDisplayBounds called");
	return CGRectMake(0,0,640,480);
}

CGError CGDisplayHideCursor(CGDirectDisplayID a)
{
    if (verbose) puts("STUB: CGDisplayHideCursor called");
	return (CGError)0;
}

CGOpenGLDisplayMask CGDisplayIDToOpenGLDisplayMask(CGDirectDisplayID a)
{
    if (verbose) puts("STUB: CGDisplayIDToOpenGLDisplayMask called");
	return 0;
}

CGError CGDisplayMoveCursorToPoint(CGDirectDisplayID a, CGPoint b)
{
    if (verbose) puts("STUB: CGDisplayMoveCursorToPoint called");
	return (CGError)0;
}

void CGDisplayRestoreColorSyncSettings(void)
{
    if (verbose) puts("STUB: CGDisplayRestoreColorSyncSettings called");
	
}

CGError CGGetActiveDisplayList(uint32_t a, CGDirectDisplayID *b, uint32_t *c)
{
    if (verbose) puts("STUB: CGGetActiveDisplayList called");
	return (CGError)0;
}

CGError CGGetDisplayTransferByFormula(CGDirectDisplayID a, CGGammaValue *b, CGGammaValue *c, CGGammaValue *d, CGGammaValue *e, CGGammaValue *f, CGGammaValue *g, CGGammaValue *h, CGGammaValue *i, CGGammaValue *j)
{
    if (verbose) puts("STUB: CGGetDisplayTransferByFormula called");
	return (CGError)0;
}

CGError CGGetDisplayTransferByTable(CGDirectDisplayID a, uint32_t b, CGGammaValue *c, CGGammaValue *d, CGGammaValue *e, uint32_t *f)
{
    if (verbose) puts("STUB: CGGetDisplayTransferByTable called");
	return (CGError)0;
}

void CGGetLastMouseDelta(int32_t *a, int32_t *b)
{
    if (verbose) puts("STUB: CGGetLastMouseDelta called");
	
}

CGDirectDisplayID CGMainDisplayID(void)
{
    if (verbose) puts("STUB: CGMainDisplayID called");
	return (CGDirectDisplayID)0;
}

CGError CGSetDisplayTransferByFormula(CGDirectDisplayID a, CGGammaValue b, CGGammaValue c, CGGammaValue d, CGGammaValue e, CGGammaValue f, CGGammaValue g, CGGammaValue h, CGGammaValue i, CGGammaValue j)
{
    if (verbose) puts("STUB: CGSetDisplayTransferByFormula called");
	return (CGError)0;
}

CGError CGSetDisplayTransferByTable(CGDirectDisplayID a, uint32_t b, const CGGammaValue *c, const CGGammaValue *d, const CGGammaValue *e)
{
    if (verbose) puts("STUB: CGSetDisplayTransferByTable called");
	return (CGError)0;
}

CGError CGSetLocalEventsSuppressionInterval(CFTimeInterval a)
{
    if (verbose) puts("STUB: CGSetLocalEventsSuppressionInterval called");
	return (CGError)0;
}

CGWindowLevel CGShieldingWindowLevel(void)
{
    if (verbose) puts("STUB: CGShieldingWindowLevel called");
	return 0;
}

// End CGDirect Stubs


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

