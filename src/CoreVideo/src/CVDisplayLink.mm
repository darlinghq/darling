#include <CoreVideo/CVDisplayLink.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSScreen.h>
#import <Foundation/NSDictionary.h>
#include <CoreGraphics/CGWindow.h>
#include <CoreGraphics/CGDirectDisplay.h>
#include <memory>

static const NSString* kDirectDisplayArray = @"CGDirectDisplay";

CVReturn CVDisplayLinkCreateWithActiveCGDisplays(CVDisplayLinkRef* displayLinkOut)
{
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

	for (int i = 0; i < displayCount; i++)
		[array addObject: [NSNumber numberWithInt: displays[i]]];

	[self setObject: array
			forKey: kDirectDisplayArray];

	*displayLinkOut = (CVDisplayLinkRef) self;
	return kCVReturnSuccess;
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

void CVDisplayLinkRelease(CVDisplayLinkRef displayLink)
{
	NSMutableDictionary* self = (NSMutableDictionary*) displayLink;
	[self release];
}

CVReturn CVDisplayLinkSetOutputCallback(CVDisplayLinkRef displayLink, CVDisplayLinkOutputCallback callback, void *userInfo)
{
	// TODO
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
		CGLContextObj ctxt = [cgw cglContext];
		if (ctxt == cglContext)
		{
			CGDirectDisplayID displayID = [window.screen cgDirectDisplayID];
			NSMutableDictionary* self = (NSMutableDictionary*) displayLink;

			[self setObject: @[[NSNumber numberWithInt: displayID]]
					forKey: kDirectDisplayArray];
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
	NSArray* ids = self[kDirectDisplayArray];

	if ([ids count] > 0)
		return (CGDirectDisplayID) [[ids firstObject] intValue];
	return kCGNullDirectDisplay;
}

CVReturn CVDisplayLinkCreateWithCGDisplay(
    CGDirectDisplayID displayID,
    CV_RETURNS_RETAINED_PARAMETER CVDisplayLinkRef CV_NULLABLE * CV_NONNULL displayLinkOut )
{
	if (displayID == kCGNullDirectDisplay)
		return kCVReturnInvalidArgument;

	NSMutableDictionary* self = [[NSMutableDictionary alloc] init];
	[self setObject: @[ [NSNumber numberWithInt: displayID] ]
			forKey: @"CGDirectDisplay"];

	*displayLinkOut = (CVDisplayLinkRef) self;
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
