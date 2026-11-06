#include <CoreVideo/CVDisplayLink.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreGraphics/CGWindow.h>
#include <CoreGraphics/CGDirectDisplay.h>
#include <Block.h>
#include <mach/mach_time.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSScreen.h>

// A display link calls its output callback on a thread of its own, once per
// refresh period of its current display, from CVDisplayLinkStart until
// CVDisplayLinkStop.
//
// Everything the thread uses lives in a separately reference-counted state
// object, so the link itself can be destroyed while its thread is still
// winding down. Releasing the last reference to a running link stops it.
//
// Times are reported in the host time base (mach_absolute_time) and, for
// video time, in nanoseconds of the same clock.

static const double kDefaultRefreshRate = 60.0;
static const int32_t kVideoTimeScale = 1000000000;

struct LinkState
{
	pthread_mutex_t lock;
	pthread_cond_t cond;
	_Atomic int32_t refs;

	CVDisplayLinkRef link;
	CVDisplayLinkOutputCallback callback;
	void* userInfo;
	CVDisplayLinkOutputHandler handler;

	bool running;
	// Changed by every start and stop, so that a thread left over from an
	// earlier start exits instead of delivering callbacks.
	uint64_t generation;
	bool inCallback;
	pthread_t callbackThread;

	uint64_t periodTicks;
	uint64_t lastCallback;
	double measuredPeriodTicks;
};

struct __CVDisplayLink
{
	CFRuntimeBase base;
	LinkState* state;
	CGDirectDisplayID* displays;
	CFIndex displayCount;
	CGDirectDisplayID currentDisplay;
};

static mach_timebase_info_data_t timebase()
{
	static mach_timebase_info_data_t info;
	static pthread_once_t once = PTHREAD_ONCE_INIT;
	pthread_once(&once, [] {
		mach_timebase_info(&info);
		if (info.numer == 0 || info.denom == 0)
			info.numer = info.denom = 1;
	});
	return info;
}

static uint64_t ticksToNanos(uint64_t ticks)
{
	mach_timebase_info_data_t tb = timebase();
	return (ticks / tb.denom) * tb.numer + (ticks % tb.denom) * tb.numer / tb.denom;
}

static uint64_t nanosToTicks(uint64_t nanos)
{
	mach_timebase_info_data_t tb = timebase();
	return (nanos / tb.numer) * tb.denom + (nanos % tb.numer) * tb.denom / tb.numer;
}

static double refreshRateOf(CGDirectDisplayID display)
{
	double rate = 0;
	if (display != kCGNullDirectDisplay)
	{
		CGDisplayModeRef mode = CGDisplayCopyDisplayMode(display);
		if (mode)
		{
			rate = CGDisplayModeGetRefreshRate(mode);
			CGDisplayModeRelease(mode);
		}
	}
	// Displays that do not report a rate are treated as the common 60 Hz.
	return rate >= 1.0 ? rate : kDefaultRefreshRate;
}

static LinkState* stateCreate()
{
	LinkState* state = (LinkState*) calloc(1, sizeof(LinkState));
	if (!state)
		return NULL;
	pthread_mutex_init(&state->lock, NULL);
	pthread_cond_init(&state->cond, NULL);
	state->refs = 1;
	return state;
}

static void stateRetain(LinkState* state)
{
	atomic_fetch_add(&state->refs, 1);
}

static void stateRelease(LinkState* state)
{
	if (atomic_fetch_sub(&state->refs, 1) != 1)
		return;
	if (state->handler)
		Block_release(state->handler);
	pthread_cond_destroy(&state->cond);
	pthread_mutex_destroy(&state->lock);
	free(state);
}

static void fillTimeStamp(CVTimeStamp* ts, uint64_t hostTime, uint64_t periodTicks, double rateScalar)
{
	memset(ts, 0, sizeof(*ts));
	ts->version = 0;
	ts->videoTimeScale = kVideoTimeScale;
	ts->videoTime = (int64_t) ticksToNanos(hostTime);
	ts->hostTime = hostTime;
	ts->rateScalar = rateScalar;
	ts->videoRefreshPeriod = (int64_t) ticksToNanos(periodTicks);
	ts->flags = kCVTimeStampVideoHostTimeValid | kCVTimeStampVideoRefreshPeriodValid | kCVTimeStampRateScalarValid;
}

static void waitUntil(LinkState* state, uint64_t deadline)
{
	uint64_t now = mach_absolute_time();
	if (now >= deadline)
		return;
	uint64_t nanos = ticksToNanos(deadline - now);
	struct timespec rel = { (time_t) (nanos / 1000000000ull), (long) (nanos % 1000000000ull) };
	pthread_cond_timedwait_relative_np(&state->cond, &state->lock, &rel);
}

struct ThreadStart
{
	LinkState* state;
	uint64_t generation;
};

static void* linkThread(void* arg)
{
	ThreadStart start = *(ThreadStart*) arg;
	free(arg);
	LinkState* state = start.state;

	pthread_setname_np("CVDisplayLink");

	pthread_mutex_lock(&state->lock);
	uint64_t deadline = mach_absolute_time();
	while (state->running && state->generation == start.generation)
	{
		uint64_t now = mach_absolute_time();
		if (now < deadline)
		{
			waitUntil(state, deadline);
			continue;
		}
		// A callback from an earlier start may still be returning.
		if (state->inCallback)
		{
			pthread_cond_wait(&state->cond, &state->lock);
			continue;
		}

		uint64_t period = state->periodTicks;
		// Frames missed because a callback ran long are skipped, not
		// delivered in a burst.
		if (now - deadline >= period)
			deadline = now - (now - deadline) % period;

		if (state->lastCallback != 0)
		{
			double interval = (double) (now - state->lastCallback);
			state->measuredPeriodTicks = state->measuredPeriodTicks > 0
				? state->measuredPeriodTicks * 0.9 + interval * 0.1
				: interval;
		}
		state->lastCallback = now;

		CVDisplayLinkRef link = state->link;
		CVDisplayLinkOutputCallback callback = state->callback;
		void* userInfo = state->userInfo;
		CVDisplayLinkOutputHandler handler = NULL;
		if (state->handler)
			handler = Block_copy(state->handler);
		state->inCallback = true;
		state->callbackThread = pthread_self();
		pthread_mutex_unlock(&state->lock);

		CVTimeStamp inNow, inOutputTime;
		fillTimeStamp(&inNow, now, period, 1.0);
		// The frame being prepared is shown at the next refresh.
		fillTimeStamp(&inOutputTime, deadline + period, period, 1.0);
		CVOptionFlags flagsOut = 0;
		@autoreleasepool
		{
			if (handler)
				handler(link, &inNow, &inOutputTime, 0, &flagsOut);
			else if (callback)
				callback(link, &inNow, &inOutputTime, 0, &flagsOut, userInfo);
		}
		if (handler)
			Block_release(handler);

		pthread_mutex_lock(&state->lock);
		state->inCallback = false;
		pthread_cond_broadcast(&state->cond);
		deadline += period;
	}
	pthread_mutex_unlock(&state->lock);
	stateRelease(state);
	return NULL;
}

// Stops the link's thread. Unless this is the link's own thread inside a
// callback, waits for a callback in progress to return, so that none runs
// after this returns.
static void stopLink(LinkState* state)
{
	pthread_mutex_lock(&state->lock);
	if (state->running)
	{
		state->running = false;
		state->generation++;
		pthread_cond_broadcast(&state->cond);
	}
	while (state->inCallback && !pthread_equal(state->callbackThread, pthread_self()))
		pthread_cond_wait(&state->cond, &state->lock);
	state->lastCallback = 0;
	state->measuredPeriodTicks = 0;
	pthread_mutex_unlock(&state->lock);
}

static void linkFinalize(CFTypeRef cf)
{
	CVDisplayLinkRef link = (CVDisplayLinkRef) cf;
	if (link->state)
	{
		stopLink(link->state);
		pthread_mutex_lock(&link->state->lock);
		link->state->link = NULL;
		pthread_mutex_unlock(&link->state->lock);
		stateRelease(link->state);
	}
	free(link->displays);
}

static CFStringRef linkCopyDescription(CFTypeRef cf)
{
	CVDisplayLinkRef link = (CVDisplayLinkRef) cf;
	return CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("<CVDisplayLink %p [display %u]>"),
		cf, (unsigned) link->currentDisplay);
}

static CFTypeID g_linkTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass kLinkClass = {
	0,
	"CVDisplayLink",
	NULL,
	NULL,
	linkFinalize,
	NULL,
	NULL,
	NULL,
	linkCopyDescription,
};

CFTypeID CVDisplayLinkGetTypeID(void)
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;
	pthread_once(&once, [] {
		g_linkTypeID = _CFRuntimeRegisterClass(&kLinkClass);
	});
	return g_linkTypeID;
}

static bool isValidLink(CVDisplayLinkRef link)
{
	return link && CFGetTypeID(link) == CVDisplayLinkGetTypeID();
}

static void setPeriod(CVDisplayLinkRef link)
{
	double rate = refreshRateOf(link->currentDisplay);
	uint64_t ticks = nanosToTicks((uint64_t) (1e9 / rate));
	pthread_mutex_lock(&link->state->lock);
	link->state->periodTicks = ticks ? ticks : 1;
	pthread_cond_broadcast(&link->state->cond);
	pthread_mutex_unlock(&link->state->lock);
}

static CVReturn createLink(const CGDirectDisplayID* displays, CFIndex count, CVDisplayLinkRef* displayLinkOut)
{
	CFIndex extra = sizeof(struct __CVDisplayLink) - sizeof(CFRuntimeBase);
	CVDisplayLinkRef link = (CVDisplayLinkRef) _CFRuntimeCreateInstance(kCFAllocatorDefault,
		CVDisplayLinkGetTypeID(), extra, NULL);
	if (!link)
		return kCVReturnAllocationFailed;

	link->state = stateCreate();
	if (count > 0)
		link->displays = (CGDirectDisplayID*) malloc(sizeof(CGDirectDisplayID) * count);
	if (!link->state || (count > 0 && !link->displays))
	{
		CFRelease(link);
		return kCVReturnAllocationFailed;
	}
	if (count > 0)
		memcpy(link->displays, displays, sizeof(CGDirectDisplayID) * count);
	link->displayCount = count;
	link->currentDisplay = count > 0 ? displays[0] : kCGNullDirectDisplay;
	link->state->link = link;
	setPeriod(link);

	*displayLinkOut = link;
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkCreateWithCGDisplays(
    CGDirectDisplayID * CV_NONNULL displayArray,
    CFIndex count,
    CV_RETURNS_RETAINED_PARAMETER CVDisplayLinkRef CV_NULLABLE * CV_NONNULL displayLinkOut )
{
	if (!displayLinkOut)
		return kCVReturnInvalidArgument;
	*displayLinkOut = NULL;
	if (!displayArray || count <= 0)
		return kCVReturnInvalidArgument;
	for (CFIndex i = 0; i < count; i++)
	{
		if (displayArray[i] == kCGNullDirectDisplay)
			return kCVReturnInvalidDisplay;
	}
	return createLink(displayArray, count, displayLinkOut);
}

CVReturn CVDisplayLinkCreateWithCGDisplay(
    CGDirectDisplayID displayID,
    CV_RETURNS_RETAINED_PARAMETER CVDisplayLinkRef CV_NULLABLE * CV_NONNULL displayLinkOut )
{
	if (!displayLinkOut)
		return kCVReturnInvalidArgument;
	*displayLinkOut = NULL;
	if (displayID == kCGNullDirectDisplay)
		return kCVReturnInvalidDisplay;
	return createLink(&displayID, 1, displayLinkOut);
}

CVReturn CVDisplayLinkCreateWithActiveCGDisplays(CVDisplayLinkRef* displayLinkOut)
{
	if (!displayLinkOut)
		return kCVReturnInvalidArgument;
	*displayLinkOut = NULL;

	uint32_t count = 0;
	if (CGGetActiveDisplayList(0, NULL, &count) != kCGErrorSuccess)
		count = 0;

	CGDirectDisplayID* displays = NULL;
	if (count > 0)
	{
		displays = (CGDirectDisplayID*) malloc(sizeof(CGDirectDisplayID) * count);
		if (!displays)
			return kCVReturnAllocationFailed;
		if (CGGetActiveDisplayList(count, displays, &count) != kCGErrorSuccess)
			count = 0;
	}

	// Without displays the link still runs, at the default rate.
	CVReturn ret = createLink(displays, count, displayLinkOut);
	free(displays);
	return ret;
}

CVReturn CVDisplayLinkCreateWithOpenGLDisplayMask(
    CGOpenGLDisplayMask mask,
    CV_RETURNS_RETAINED_PARAMETER CVDisplayLinkRef CV_NULLABLE * CV_NONNULL displayLinkOut )
{
	if (!displayLinkOut)
		return kCVReturnInvalidArgument;
	*displayLinkOut = NULL;
	if (mask == 0)
		return kCVReturnInvalidDisplay;

	CGDirectDisplayID display = CGOpenGLDisplayMaskToDisplayID(mask);
	if (display == kCGNullDirectDisplay)
		return kCVReturnInvalidDisplay;
	return createLink(&display, 1, displayLinkOut);
}

CVDisplayLinkRef CVDisplayLinkRetain(CVDisplayLinkRef displayLink)
{
	if (displayLink)
		CFRetain(displayLink);
	return displayLink;
}

void CVDisplayLinkRelease(CVDisplayLinkRef displayLink)
{
	if (displayLink)
		CFRelease(displayLink);
}

CVReturn CVDisplayLinkSetCurrentCGDisplay(CVDisplayLinkRef CV_NONNULL displayLink, CGDirectDisplayID displayID)
{
	if (!isValidLink(displayLink))
		return kCVReturnInvalidArgument;
	if (displayID == kCGNullDirectDisplay)
		return kCVReturnInvalidDisplay;

	// A link created for some displays only follows those.
	if (displayLink->displayCount > 0)
	{
		CFIndex i;
		for (i = 0; i < displayLink->displayCount; i++)
		{
			if (displayLink->displays[i] == displayID)
				break;
		}
		if (i == displayLink->displayCount)
			return kCVReturnInvalidDisplay;
	}

	displayLink->currentDisplay = displayID;
	setPeriod(displayLink);
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(CVDisplayLinkRef displayLink, CGLContextObj cglContext, CGLPixelFormatObj cglPixelFormat)
{
	if (!isValidLink(displayLink) || !cglContext)
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
			if (displayID == kCGNullDirectDisplay)
				return kCVReturnInvalidDisplay;
			displayLink->currentDisplay = displayID;
			setPeriod(displayLink);
			return kCVReturnSuccess;
		}
	}

	return kCVReturnError;
}

CGDirectDisplayID CVDisplayLinkGetCurrentCGDisplay(CVDisplayLinkRef CV_NONNULL displayLink)
{
	if (!isValidLink(displayLink))
		return kCGNullDirectDisplay;
	return displayLink->currentDisplay;
}

CVReturn CVDisplayLinkSetOutputCallback(CVDisplayLinkRef displayLink, CVDisplayLinkOutputCallback callback, void *userInfo)
{
	if (!isValidLink(displayLink))
		return kCVReturnInvalidArgument;

	LinkState* state = displayLink->state;
	pthread_mutex_lock(&state->lock);
	CVDisplayLinkOutputHandler old = state->handler;
	state->handler = NULL;
	state->callback = callback;
	state->userInfo = userInfo;
	pthread_mutex_unlock(&state->lock);
	if (old)
		Block_release(old);
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkSetOutputHandler(CVDisplayLinkRef displayLink, CVDisplayLinkOutputHandler handler)
{
	if (!isValidLink(displayLink) || !handler)
		return kCVReturnInvalidArgument;

	CVDisplayLinkOutputHandler copy = Block_copy(handler);
	LinkState* state = displayLink->state;
	pthread_mutex_lock(&state->lock);
	CVDisplayLinkOutputHandler old = state->handler;
	state->handler = copy;
	state->callback = NULL;
	state->userInfo = NULL;
	pthread_mutex_unlock(&state->lock);
	if (old)
		Block_release(old);
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkStart(CVDisplayLinkRef displayLink)
{
	if (!isValidLink(displayLink))
		return kCVReturnInvalidArgument;

	LinkState* state = displayLink->state;
	pthread_mutex_lock(&state->lock);
	if (state->running)
	{
		pthread_mutex_unlock(&state->lock);
		return kCVReturnSuccess;
	}

	ThreadStart* start = (ThreadStart*) malloc(sizeof(ThreadStart));
	if (!start)
	{
		pthread_mutex_unlock(&state->lock);
		return kCVReturnAllocationFailed;
	}
	state->generation++;
	start->state = state;
	start->generation = state->generation;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	stateRetain(state);
	pthread_t thread;
	int err = pthread_create(&thread, &attr, linkThread, start);
	pthread_attr_destroy(&attr);
	if (err != 0)
	{
		free(start);
		pthread_mutex_unlock(&state->lock);
		stateRelease(state);
		return kCVReturnError;
	}
	state->running = true;
	pthread_mutex_unlock(&state->lock);
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkStop(CVDisplayLinkRef displayLink)
{
	if (!isValidLink(displayLink))
		return kCVReturnInvalidArgument;
	stopLink(displayLink->state);
	return kCVReturnSuccess;
}

Boolean CVDisplayLinkIsRunning(CVDisplayLinkRef displayLink)
{
	if (!isValidLink(displayLink))
		return false;
	LinkState* state = displayLink->state;
	pthread_mutex_lock(&state->lock);
	bool running = state->running;
	pthread_mutex_unlock(&state->lock);
	return running;
}

CVTime CVDisplayLinkGetNominalOutputVideoRefreshPeriod(CVDisplayLinkRef CV_NONNULL displayLink)
{
	CVTime time = { 0, 0, kCVTimeIsIndefinite };
	if (!isValidLink(displayLink))
		return time;

	// Expressed in thousandths of a refresh so that rates such as 59.94 Hz
	// stay exact.
	double rate = refreshRateOf(displayLink->currentDisplay);
	time.timeValue = 1000;
	time.timeScale = (int32_t) (rate * 1000.0 + 0.5);
	time.flags = 0;
	return time;
}

CVTime CVDisplayLinkGetOutputVideoLatency(CVDisplayLinkRef displayLink)
{
	CVTime time = { 0, 0, kCVTimeIsIndefinite };
	if (!isValidLink(displayLink))
		return time;
	time.timeScale = kVideoTimeScale;
	time.flags = 0;
	return time;
}

double CVDisplayLinkGetActualOutputVideoRefreshPeriod(CVDisplayLinkRef displayLink)
{
	if (!isValidLink(displayLink))
		return 0.0;

	LinkState* state = displayLink->state;
	pthread_mutex_lock(&state->lock);
	bool running = state->running;
	double measured = state->measuredPeriodTicks;
	uint64_t nominal = state->periodTicks;
	pthread_mutex_unlock(&state->lock);

	// Only a running link has a measured period.
	if (!running)
		return 0.0;
	double ticks = measured > 0 ? measured : (double) nominal;
	mach_timebase_info_data_t tb = timebase();
	return ticks * tb.numer / tb.denom / 1e9;
}

CVReturn CVDisplayLinkGetCurrentTime(CVDisplayLinkRef displayLink, CVTimeStamp * CV_NONNULL outTime)
{
	if (!isValidLink(displayLink) || !outTime)
		return kCVReturnInvalidArgument;

	LinkState* state = displayLink->state;
	pthread_mutex_lock(&state->lock);
	uint64_t period = state->periodTicks;
	pthread_mutex_unlock(&state->lock);

	fillTimeStamp(outTime, mach_absolute_time(), period, 1.0);
	return kCVReturnSuccess;
}

CVReturn CVDisplayLinkTranslateTime(CVDisplayLinkRef displayLink, const CVTimeStamp * CV_NONNULL inTime, CVTimeStamp * CV_NONNULL outTime)
{
	if (!isValidLink(displayLink) || !inTime || !outTime)
		return kCVReturnInvalidArgument;

	uint64_t hostTime;
	if (inTime->flags & kCVTimeStampHostTimeValid)
	{
		hostTime = inTime->hostTime;
	}
	else if ((inTime->flags & kCVTimeStampVideoTimeValid) && inTime->videoTimeScale > 0 && inTime->videoTime >= 0)
	{
		// Convert the video time to nanoseconds without overflowing.
		uint64_t scale = (uint64_t) inTime->videoTimeScale;
		uint64_t video = (uint64_t) inTime->videoTime;
		uint64_t nanos = (video / scale) * 1000000000ull + (video % scale) * 1000000000ull / scale;
		hostTime = nanosToTicks(nanos);
	}
	else
	{
		return kCVReturnError;
	}

	// The fields requested through outTime->flags are filled in; with no
	// request, both host and video time are.
	uint64_t requested = outTime->flags & kCVTimeStampVideoHostTimeValid;
	if (!requested)
		requested = kCVTimeStampVideoHostTimeValid;

	LinkState* state = displayLink->state;
	pthread_mutex_lock(&state->lock);
	uint64_t period = state->periodTicks;
	pthread_mutex_unlock(&state->lock);

	CVTimeStamp result;
	fillTimeStamp(&result, hostTime, period, 1.0);
	result.flags &= ~(uint64_t) kCVTimeStampVideoHostTimeValid;
	result.flags |= requested;
	if (!(requested & kCVTimeStampHostTimeValid))
		result.hostTime = 0;
	if (!(requested & kCVTimeStampVideoTimeValid))
		result.videoTime = 0;
	*outTime = result;
	return kCVReturnSuccess;
}
