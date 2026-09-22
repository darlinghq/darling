#include <CoreVideo/CVDisplayLink.h>
#include <memory>
#include <cstring>
#include <mach/mach_time.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSScreen.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSValue.h>
#include <CoreGraphics/CGWindow.h>
#include <CoreGraphics/CGDirectDisplay.h>

static const NSString* kDirectDisplayArray = @"CGDirectDisplay";

CFTypeID CVDisplayLinkGetTypeID(void)
{
	return CFDictionaryGetTypeID();
}

CVReturn CVDisplayLinkCreateWithActiveCGDisplays(CVDisplayLinkRef* displayLinkOut)
{
	if (!displayLinkOut)
		return kCVReturnInvalidArgument;

	uint32_t displayCount;
	std::unique_ptr<CGDirectDisplayID[]> displays;

	CGError err = CGGetActiveDisplayList(0, nullptr, &displayCount);
	if (err != kCGErrorSuccess)
		return err;

	displays.reset(new CGDirectDisplayID[displayCount]);

	err = CGGetActiveDisplayList(displayCount, displays.get(), &displayCount);
	if (err != kCGErrorSuccess)
		return err;

	NSMutableDictionary* self = [[NSMutableDictionary alloc] init];
	NSMutableArray* array = [NSMutableArray arrayWithCapacity: displayCount];

	for (uint32_t i = 0; i < displayCount; i++)
		[array addObject: [NSNumber numberWithInt: displays[i]]];

	[self setObject: array
			forKey: (NSString *)kDirectDisplayArray];

	*displayLinkOut = (CVDisplayLinkRef) self;
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkCreateWithCGDisplays(
    CGDirectDisplayID * CV_NONNULL displayArray,
    CFIndex count,
    CV_RETURNS_RETAINED_PARAMETER CVDisplayLinkRef CV_NULLABLE * CV_NONNULL displayLinkOut )
{
	if (!displayLinkOut || !displayArray || count <= 0)
		return kCVReturnInvalidArgument;

	NSMutableDictionary* self = [[NSMutableDictionary alloc] init];
	NSMutableArray* array = [NSMutableArray arrayWithCapacity: count];

	for (CFIndex i = 0; i < count; i++)
		[array addObject: [NSNumber numberWithInt: displayArray[i]]];

	[self setObject: array
			forKey: (NSString *)kDirectDisplayArray];

	*displayLinkOut = (CVDisplayLinkRef) self;
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkCreateWithOpenGLDisplayMask(
    CGOpenGLDisplayMask mask,
    CV_RETURNS_RETAINED_PARAMETER CVDisplayLinkRef CV_NULLABLE * CV_NONNULL displayLinkOut )
{
	if (!displayLinkOut)
		return kCVReturnInvalidArgument;

	return CVDisplayLinkCreateWithActiveCGDisplays(displayLinkOut);
}

CVReturn CVDisplayLinkCreateWithCGDisplay(
    CGDirectDisplayID displayID,
    CV_RETURNS_RETAINED_PARAMETER CVDisplayLinkRef CV_NULLABLE * CV_NONNULL displayLinkOut )
{
	if (!displayLinkOut || displayID == kCGNullDirectDisplay)
		return kCVReturnInvalidArgument;

	NSMutableDictionary* self = [[NSMutableDictionary alloc] init];
	[self setObject: @[ [NSNumber numberWithInt: displayID] ]
			forKey: (NSString *)kDirectDisplayArray];

	*displayLinkOut = (CVDisplayLinkRef) self;
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkSetCurrentCGDisplay( CVDisplayLinkRef CV_NONNULL displayLink, CGDirectDisplayID displayID )
{
	if (!displayLink)
		return kCVReturnInvalidArgument;

	NSMutableDictionary* self = (NSMutableDictionary*) displayLink;
	[self setObject: @[ [NSNumber numberWithInt: displayID] ]
			forKey: (NSString *)kDirectDisplayArray];
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(CVDisplayLinkRef displayLink, CGLContextObj cglContext, CGLPixelFormatObj cglPixelFormat)
{
	if (!displayLink)
		return kCVReturnInvalidArgument;

	NSArray *windowArray = [[NSClassFromString(@"NSApplication") sharedApplication] windows];
	if (!windowArray)
		return kCVReturnError;

	for (NSWindow* window in windowArray)
	{
		CGWindow* cgw = [window platformWindow];
		CGLContextObj ctxt = (CGLContextObj)[cgw cglContext];
		if (ctxt == cglContext)
		{
			CGDirectDisplayID displayID = [window.screen cgDirectDisplayID];
			NSMutableDictionary* self = (NSMutableDictionary*) displayLink;

			[self setObject: @[[NSNumber numberWithInt: displayID]]
					forKey: (NSString *)kDirectDisplayArray];
			return kCVReturnSuccess;
		}
	}

	return kCVReturnError;
}

CGDirectDisplayID CVDisplayLinkGetCurrentCGDisplay( CVDisplayLinkRef CV_NONNULL displayLink )
{
	if (!displayLink)
		return kCVReturnInvalidArgument;

	NSMutableDictionary* self = (NSMutableDictionary*) displayLink;
	NSArray* ids = self[(NSString *)kDirectDisplayArray];

	if ([ids count] > 0)
		return (CGDirectDisplayID) [[ids firstObject] intValue];
	return kCGNullDirectDisplay;
}

CVReturn CVDisplayLinkStart(CVDisplayLinkRef displayLink)
{
	if (!displayLink)
		return kCVReturnInvalidArgument;
	// TODO
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkStop(CVDisplayLinkRef displayLink)
{
	if (!displayLink)
		return kCVReturnInvalidArgument;
	// TODO
	return kCVReturnSuccess;
}

Boolean CVDisplayLinkIsRunning(CVDisplayLinkRef displayLink)
{
	if (!displayLink)
		return false;
	// TODO
	return true;
}

CVDisplayLinkRef CVDisplayLinkRetain( CVDisplayLinkRef displayLink )
{
	if (!displayLink)
		return NULL;
	NSMutableDictionary* self = (NSMutableDictionary*) displayLink;
	return (CVDisplayLinkRef)[self retain];
}

void CVDisplayLinkRelease(CVDisplayLinkRef displayLink)
{
	if (!displayLink)
		return;
	NSMutableDictionary* self = (NSMutableDictionary*) displayLink;
	[self release];
}

CVReturn CVDisplayLinkSetOutputCallback(CVDisplayLinkRef displayLink, CVDisplayLinkOutputCallback callback, void *userInfo)
{
	if (!displayLink)
		return kCVReturnInvalidArgument;
	// TODO
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkSetOutputHandler( CVDisplayLinkRef displayLink, CVDisplayLinkOutputHandler handler )
{
	if (!displayLink || !handler)
		return kCVReturnInvalidArgument;
	// TODO
	return kCVReturnSuccess;
}

CVTime CVDisplayLinkGetNominalOutputVideoRefreshPeriod( CVDisplayLinkRef CV_NONNULL displayLink )
{
	CVTime time = { 0, 0, kCVTimeIsIndefinite };
	CGDirectDisplayID displayId = CVDisplayLinkGetCurrentCGDisplay(displayLink);
	if (displayId == kCGNullDirectDisplay)
		return time;

	CGDisplayModeRef mode = CGDisplayCopyDisplayMode(displayId);
	if (!mode)
		return time;

	double rate = CGDisplayModeGetRefreshRate(mode);
	if (rate < 1.0)
		rate = 60.0;

	time.flags = 0;
	time.timeValue = 1.0;
	time.timeScale = rate;

	CGDisplayModeRelease(mode);
	return time;
}

CVTime CVDisplayLinkGetOutputVideoLatency( CVDisplayLinkRef displayLink )
{
	CVTime time = { 0, 1, 0 };
	return time;
}

double CVDisplayLinkGetActualOutputVideoRefreshPeriod( CVDisplayLinkRef displayLink )
{
	if (!displayLink)
		return 0.0;
	CVTime nominal = CVDisplayLinkGetNominalOutputVideoRefreshPeriod(displayLink);
	if (nominal.timeScale > 0 && nominal.timeValue > 0)
		return (double)nominal.timeValue / (double)nominal.timeScale;
	return 1.0 / 60.0;
}

CVReturn CVDisplayLinkGetCurrentTime( CVDisplayLinkRef displayLink, CVTimeStamp * CV_NONNULL outTime )
{
	if (!displayLink || !outTime)
		return kCVReturnInvalidArgument;

	static mach_timebase_info_data_t s_timebase = { 0, 0 };
	if (s_timebase.denom == 0)
		mach_timebase_info(&s_timebase);

	uint64_t machTime = mach_absolute_time();
	uint64_t nanos = (s_timebase.denom > 0) ? (machTime * s_timebase.numer / s_timebase.denom) : machTime;

	memset(outTime, 0, sizeof(CVTimeStamp));
	outTime->version = 0;
	outTime->videoTime = (int64_t)nanos;
	outTime->videoTimeScale = 1000000000; // nanoseconds
	outTime->videoRefreshPeriod = (int64_t)(1000000000.0 * CVDisplayLinkGetActualOutputVideoRefreshPeriod(displayLink));
	outTime->flags = kCVTimeStampVideoTimeValid | kCVTimeStampHostTimeValid;
	outTime->hostTime = machTime;
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkTranslateTime( CVDisplayLinkRef displayLink, const CVTimeStamp * CV_NONNULL inTime, CVTimeStamp * CV_NONNULL outTime )
{
	if (!displayLink || !inTime || !outTime)
		return kCVReturnInvalidArgument;
	*outTime = *inTime;
	return kCVReturnSuccess;
}
