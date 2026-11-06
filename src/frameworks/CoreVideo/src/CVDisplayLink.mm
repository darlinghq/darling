#include <CoreVideo/CVDisplayLink.h>
#include <memory>
#include <mach/mach_time.h>
#include <dispatch/dispatch.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSScreen.h>
#import <Foundation/Foundation.h>
#include <CoreGraphics/CGWindow.h>
#include <CoreGraphics/CGDirectDisplay.h>

// A display link ticks at the display's nominal refresh rate (60 Hz when the
// display does not report one) on a private dispatch queue, handing the
// output callback or handler timestamps for the current and the next frame.

static const int64_t kDefaultRefreshPeriodNs = 1000000000 / 60;

@interface CVDisplayLinkImpl : NSObject {
@public
	CGDirectDisplayID displayID;
	CVDisplayLinkOutputCallback callback;
	void* userInfo;
	CVDisplayLinkOutputHandler handler;
	dispatch_queue_t queue;
	dispatch_source_t timer;
	uint64_t frameCount;
	uint64_t startHostTime;
	CVTimeStamp lastTime;
}
@end

@implementation CVDisplayLinkImpl

- (instancetype) init {
	if ((self = [super init])) {
		queue = dispatch_queue_create("CVDisplayLink", DISPATCH_QUEUE_SERIAL);
		memset(&lastTime, 0, sizeof(lastTime));
	}
	return self;
}

- (void) dealloc {
	if (timer) {
		dispatch_source_cancel(timer);
		dispatch_release(timer);
	}
	dispatch_release(queue);
	[handler release];
	[super dealloc];
}

- (int64_t) refreshPeriodNs {
	CVTime time = CVDisplayLinkGetNominalOutputVideoRefreshPeriod((CVDisplayLinkRef) self);
	if (time.flags & kCVTimeIsIndefinite || time.timeScale <= 0 || time.timeValue <= 0)
		return kDefaultRefreshPeriodNs;
	return (int64_t) (1000000000.0 * time.timeValue / time.timeScale);
}

static uint64_t hostTimeToNs(uint64_t hostTime) {
	static mach_timebase_info_data_t info;
	if (info.denom == 0)
		mach_timebase_info(&info);
	return hostTime * info.numer / info.denom;
}

static uint64_t nsToHostTime(uint64_t ns) {
	static mach_timebase_info_data_t info;
	if (info.denom == 0)
		mach_timebase_info(&info);
	return ns * info.denom / info.numer;
}

- (void) fillTimeStamp: (CVTimeStamp*) stamp frame: (uint64_t) frame periodNs: (int64_t) periodNs {
	stamp->version = 0;
	stamp->videoTimeScale = 1000000000;
	stamp->videoTime = (int64_t) (frame * periodNs);
	stamp->hostTime = startHostTime + nsToHostTime(frame * periodNs);
	stamp->rateScalar = 1.0;
	stamp->videoRefreshPeriod = periodNs;
	memset(&stamp->smpteTime, 0, sizeof(stamp->smpteTime));
	stamp->flags = kCVTimeStampVideoTimeValid | kCVTimeStampHostTimeValid | kCVTimeStampVideoRefreshPeriodValid | kCVTimeStampRateScalarValid;
	stamp->reserved = 0;
}

- (void) tick {
	int64_t periodNs = [self refreshPeriodNs];
	CVTimeStamp now, output;
	CVOptionFlags flagsOut = 0;

	frameCount++;
	[self fillTimeStamp: &now frame: frameCount periodNs: periodNs];
	[self fillTimeStamp: &output frame: frameCount + 1 periodNs: periodNs];
	lastTime = now;

	if (handler)
		handler((CVDisplayLinkRef) self, &now, &output, 0, &flagsOut);
	else if (callback)
		callback((CVDisplayLinkRef) self, &now, &output, 0, &flagsOut, userInfo);
}

- (void) start {
	if (timer)
		return;

	startHostTime = mach_absolute_time();
	frameCount = 0;

	timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
	dispatch_source_set_timer(timer, dispatch_time(DISPATCH_TIME_NOW, [self refreshPeriodNs]), [self refreshPeriodNs], 100000);
	dispatch_source_set_event_handler(timer, ^{ [self tick]; });
	dispatch_resume(timer);
}

- (void) stop {
	if (!timer)
		return;

	dispatch_source_cancel(timer);
	dispatch_release(timer);
	timer = NULL;
}

@end

static CVDisplayLinkImpl* impl(CVDisplayLinkRef link) {
	return (CVDisplayLinkImpl*) link;
}

CFTypeID CVDisplayLinkGetTypeID(void)
{
	return (CFTypeID) [CVDisplayLinkImpl class];
}

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

	CVDisplayLinkImpl* link = [[CVDisplayLinkImpl alloc] init];
	link->displayID = displayCount > 0 ? displays[0] : kCGNullDirectDisplay;

	*displayLinkOut = (CVDisplayLinkRef) link;
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkCreateWithCGDisplays(CGDirectDisplayID* displayArray, int32_t count, CVDisplayLinkRef* displayLinkOut)
{
	if (count < 1 || displayArray == NULL)
		return kCVReturnInvalidArgument;

	return CVDisplayLinkCreateWithCGDisplay(displayArray[0], displayLinkOut);
}

CVReturn CVDisplayLinkCreateWithCGDisplay(CGDirectDisplayID displayID, CVDisplayLinkRef* displayLinkOut)
{
	if (displayID == kCGNullDirectDisplay)
		return kCVReturnInvalidArgument;

	CVDisplayLinkImpl* link = [[CVDisplayLinkImpl alloc] init];
	link->displayID = displayID;

	*displayLinkOut = (CVDisplayLinkRef) link;
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkSetCurrentCGDisplay(CVDisplayLinkRef displayLink, CGDirectDisplayID displayID)
{
	if (!displayLink || displayID == kCGNullDirectDisplay)
		return kCVReturnInvalidArgument;

	impl(displayLink)->displayID = displayID;
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkStart(CVDisplayLinkRef displayLink)
{
	if (!displayLink)
		return kCVReturnInvalidArgument;

	[impl(displayLink) start];
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkStop(CVDisplayLinkRef displayLink)
{
	if (!displayLink)
		return kCVReturnInvalidArgument;

	[impl(displayLink) stop];
	return kCVReturnSuccess;
}

Boolean CVDisplayLinkIsRunning(CVDisplayLinkRef displayLink)
{
	if (!displayLink)
		return false;

	return impl(displayLink)->timer != NULL;
}

CVDisplayLinkRef CVDisplayLinkRetain(CVDisplayLinkRef displayLink)
{
	return (CVDisplayLinkRef) [impl(displayLink) retain];
}

void CVDisplayLinkRelease(CVDisplayLinkRef displayLink)
{
	[impl(displayLink) release];
}

CVReturn CVDisplayLinkSetOutputCallback(CVDisplayLinkRef displayLink, CVDisplayLinkOutputCallback callback, void *userInfo)
{
	if (!displayLink)
		return kCVReturnInvalidArgument;

	impl(displayLink)->callback = callback;
	impl(displayLink)->userInfo = userInfo;
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkSetOutputHandler(CVDisplayLinkRef displayLink, CVDisplayLinkOutputHandler handler)
{
	if (!displayLink || !handler)
		return kCVReturnInvalidArgument;

	CVDisplayLinkImpl* link = impl(displayLink);
	CVDisplayLinkOutputHandler copy = [handler copy];
	[link->handler release];
	link->handler = copy;
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
			impl(displayLink)->displayID = [window.screen cgDirectDisplayID];
			return kCVReturnSuccess;
		}
	}

	return kCVReturnError;
}

CGDirectDisplayID CVDisplayLinkGetCurrentCGDisplay(CVDisplayLinkRef displayLink)
{
	if (!displayLink)
		return kCGNullDirectDisplay;

	return impl(displayLink)->displayID;
}

CVTime CVDisplayLinkGetNominalOutputVideoRefreshPeriod(CVDisplayLinkRef displayLink)
{
	// 60 Hz unless the display reports another rate
	CVTime time = { 1, 60, 0 };
	CGDirectDisplayID displayId = CVDisplayLinkGetCurrentCGDisplay(displayLink);
	if (displayId == kCGNullDirectDisplay)
		return time;

	CGDisplayModeRef mode = CGDisplayCopyDisplayMode(displayId);
	if (!mode)
		return time;

	double rate = CGDisplayModeGetRefreshRate(mode);
	if (rate >= 1.0)
		time.timeScale = (int32_t) rate;

	CGDisplayModeRelease(mode);
	return time;
}

CVTime CVDisplayLinkGetOutputVideoLatency(CVDisplayLinkRef displayLink)
{
	CVTime time = { 0, 0, kCVTimeIsIndefinite };
	return time;
}

double CVDisplayLinkGetActualOutputVideoRefreshPeriod(CVDisplayLinkRef displayLink)
{
	CVTime time = CVDisplayLinkGetNominalOutputVideoRefreshPeriod(displayLink);
	return (double) time.timeValue / (double) time.timeScale;
}

CVReturn CVDisplayLinkGetCurrentTime(CVDisplayLinkRef displayLink, CVTimeStamp* outTime)
{
	if (!displayLink || !outTime)
		return kCVReturnInvalidArgument;

	CVDisplayLinkImpl* link = impl(displayLink);
	if (!link->timer)
		return kCVReturnError;

	*outTime = link->lastTime;
	return kCVReturnSuccess;
}
