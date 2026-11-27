/*
  This file is part of Darling.

  Copyright (C) 2026 Darling Developers

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

#import <ScreenCaptureKit/ScreenCaptureKit.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreGraphics/CGDisplayStream.h>
#import <CoreVideo/CoreVideo.h>

#include <dispatch/dispatch.h>
#include <math.h>
#include <string.h>

NSString *const SCStreamErrorDomain = @"com.apple.ScreenCaptureKit.SCStreamErrorDomain";

SCStreamFrameInfo const SCStreamFrameInfoStatus = @"Status";
SCStreamFrameInfo const SCStreamFrameInfoDisplayTime = @"DisplayTime";
SCStreamFrameInfo const SCStreamFrameInfoScaleFactor = @"ScaleFactor";
SCStreamFrameInfo const SCStreamFrameInfoContentScale = @"ContentScale";
SCStreamFrameInfo const SCStreamFrameInfoContentRect = @"ContentRect";
SCStreamFrameInfo const SCStreamFrameInfoBoundingRect = @"BoundingRect";
SCStreamFrameInfo const SCStreamFrameInfoScreenRect = @"ScreenRect";
SCStreamFrameInfo const SCStreamFrameInfoDirtyRects = @"DirtyRects";

@interface SCRunningApplication ()
@property (readwrite, copy) NSString *bundleIdentifier;
@property (readwrite, copy) NSString *applicationName;
@property (readwrite) pid_t processID;
@end

@interface SCWindow ()
@property (readwrite) CGWindowID windowID;
@property (readwrite) CGRect frame;
@property (readwrite, copy) NSString *title;
@property (readwrite) NSInteger windowLayer;
@property (readwrite, retain) SCRunningApplication *owningApplication;
@property (readwrite, getter=isOnScreen) BOOL onScreen;
@end

@interface SCDisplay ()
@property (readwrite) CGDirectDisplayID displayID;
@property (readwrite) NSInteger width;
@property (readwrite) NSInteger height;
@property (readwrite) CGRect frame;
@end

@interface SCShareableContent ()
@property (readwrite, retain) NSArray<SCWindow *> *windows;
@property (readwrite, retain) NSArray<SCDisplay *> *displays;
@property (readwrite, retain) NSArray<SCRunningApplication *> *applications;
@end

@interface SCContentFilter () {
@public
    SCDisplay *_targetDisplay;
    SCWindow *_targetWindow;
    NSArray *_filterIncludedWindows;
    NSArray *_filterExcludedWindows;
    NSArray *_filterIncludedApplications;
    NSArray *_filterExcludedApplications;
    BOOL _unsupportedFilter;
}
@property (readwrite) CGRect contentRect;
@property (readwrite) CGFloat pointPixelScale;
@property (readwrite) SCShareableContentStyle style;
@property (readwrite) SCStreamType streamType;
@property (readwrite, retain) NSArray<SCRunningApplication *> *includedApplications;
@property (readwrite, retain) NSArray<SCDisplay *> *includedDisplays;
@property (readwrite, retain) NSArray<SCWindow *> *includedWindows;
- (instancetype)initWithDisplay:(SCDisplay *)display
                          window:(SCWindow *)window
                           style:(SCShareableContentStyle)style
                     includeMenu:(BOOL)includeMenu;
@end

@interface SCStreamConfiguration ()
- (void)setDefaults;
@end

@class SCCaptureContext;

@interface DarlingScreenCaptureOutput : NSObject
@property (nonatomic, retain) id<SCStreamOutput> output;
@property (nonatomic) SCStreamOutputType type;
@property (nonatomic, retain) dispatch_queue_t queue;
@end

@implementation DarlingScreenCaptureOutput

- (void)dealloc
{
    [_output release];
    if (_queue != NULL)
        dispatch_release(_queue);
    [super dealloc];
}

@end

@interface SCStream () {
@public
    SCContentFilter *_filter;
    SCStreamConfiguration *_configuration;
    id<SCStreamDelegate> _delegate;
    NSMutableArray *_outputs;
    BOOL _capturing;
#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
    CGDisplayStreamRef _displayStream;
    dispatch_queue_t _captureQueue;
    SCCaptureContext *_captureContext;
#endif
}
- (NSError *)startCaptureSynchronously;
- (void)stopCaptureSynchronously;
- (BOOL)restartCaptureWithError:(NSError **)error;
#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
- (void)deliverSurface:(IOSurfaceRef)surface
            displayTime:(uint64_t)displayTime
                 update:(CGDisplayStreamUpdateRef)updateRef;
#endif
@end

static NSError *SCMakeError(NSInteger code, NSString *description)
{
    return [NSError errorWithDomain:SCStreamErrorDomain
                               code:code
                           userInfo:description != nil
                               ? @{ NSLocalizedDescriptionKey: description }
                               : nil];
}

static NSError *SCUnavailableError(void)
{
    return SCMakeError(SCStreamErrorUserDeclined,
        @"Screen capture is not available in this Darling build.");
}

static void SCDispatchErrorCompletion(void (^handler)(NSError *), NSError *error)
{
    if (handler == nil)
        return;
    void (^block)(NSError *) = [handler copy];
    NSError *copiedError = [error retain];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        block(copiedError);
        [copiedError release];
        [block release];
    });
}

static void SCDispatchContentCompletion(void (^handler)(SCShareableContent *, NSError *),
                                        SCShareableContent *content, NSError *error)
{
    if (handler == nil)
        return;
    void (^block)(SCShareableContent *, NSError *) = [handler copy];
    SCShareableContent *copiedContent = [content retain];
    NSError *copiedError = [error retain];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        block(copiedContent, copiedError);
        [copiedContent release];
        [copiedError release];
        [block release];
    });
}

static void SCDispatchScreenshotErrorCompletion(void (^handler)(id, NSError *), NSError *error)
{
    if (handler == nil)
        return;
    void (^block)(id, NSError *) = [handler copy];
    NSError *copiedError = [error retain];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        block(nil, copiedError);
        [copiedError release];
        [block release];
    });
}

static SCRunningApplication *SCAppplicationForPID(pid_t pid)
{
    SCRunningApplication *application = [[[SCRunningApplication alloc] init] autorelease];

    application.processID = pid;
    if (pid == [[NSProcessInfo processInfo] processIdentifier]) {
        application.applicationName = [[NSProcessInfo processInfo] processName];
        application.bundleIdentifier = [[NSBundle mainBundle] bundleIdentifier];
    }
    return application;
}

static NSArray<SCRunningApplication *> *SCApplicationsForPIDs(NSArray *pids)
{
    NSMutableArray *applications = [NSMutableArray array];

    for (NSNumber *pid in pids) {
        BOOL duplicate = NO;
        for (SCRunningApplication *existing in applications) {
            if (existing.processID == (pid_t) [pid integerValue]) {
                duplicate = YES;
                break;
            }
        }
        if (!duplicate)
            [applications addObject: SCAppplicationForPID((pid_t) [pid integerValue])];
    }
    return applications;
}

static SCDisplay *SCDisplayForID(CGDirectDisplayID displayID)
{
    CGRect bounds = CGDisplayBounds(displayID);
    SCDisplay *display;

    if (CGRectIsEmpty(bounds))
        return nil;
    display = [[[SCDisplay alloc] init] autorelease];
    display.displayID = displayID;
    display.frame = bounds;
    display.width = (NSInteger) bounds.size.width;
    display.height = (NSInteger) bounds.size.height;
    return display;
}

static SCDisplay *SCDisplayContainingRect(CGRect rect)
{
    CGDirectDisplayID displays[32];
    uint32_t count = 0;

    if (CGGetDisplaysWithRect(rect, 32, displays, &count) != kCGErrorSuccess || count == 0)
        return SCDisplayForID(CGMainDisplayID());
    return SCDisplayForID(displays[0]);
}

static NSArray<SCDisplay *> *SCActiveDisplays(void)
{
    CGDirectDisplayID displays[32];
    uint32_t count = 0;
    NSMutableArray *result = [NSMutableArray array];

    if (CGGetActiveDisplayList(32, displays, &count) != kCGErrorSuccess)
        return result;
    for (uint32_t i = 0; i < count; i++) {
        SCDisplay *display = SCDisplayForID(displays[i]);
        if (display != nil)
            [result addObject: display];
    }
    if ([result count] == 0) {
        SCDisplay *main = SCDisplayForID(CGMainDisplayID());
        if (main != nil)
            [result addObject: main];
    }
    return result;
}

static SCShareableContent *SCBuildContent(BOOL excludeDesktopWindows, BOOL onScreenWindowsOnly,
                                          pid_t onlyProcess)
{
    CGWindowListOption options = kCGWindowListOptionAll;
    CFArrayRef rawWindows;
    NSArray *windowInfo;
    NSMutableArray *windows = [NSMutableArray array];
    NSMutableArray *pids = [NSMutableArray array];
    SCShareableContent *content;

    if (onScreenWindowsOnly)
        options |= kCGWindowListOptionOnScreenOnly;
    if (excludeDesktopWindows)
        options |= kCGWindowListExcludeDesktopElements;

    rawWindows = CGWindowListCopyWindowInfo(options, kCGNullWindowID);
    windowInfo = rawWindows != NULL ? [(NSArray *) rawWindows autorelease] : [NSArray array];
    for (NSDictionary *info in windowInfo) {
        NSNumber *number = [info objectForKey: (id) kCGWindowNumber];
        NSNumber *owner = [info objectForKey: (id) kCGWindowOwnerPID];
        NSDictionary *bounds = [info objectForKey: (id) kCGWindowBounds];
        SCWindow *window;
        CGRect frame = CGRectZero;

        if (![number isKindOfClass: [NSNumber class]])
            continue;
        if (onlyProcess != 0 && (![owner isKindOfClass: [NSNumber class]]
                || [owner integerValue] != onlyProcess))
            continue;
        if (bounds != nil)
            CGRectMakeWithDictionaryRepresentation((CFDictionaryRef) bounds, &frame);

        window = [[[SCWindow alloc] init] autorelease];
        window.windowID = (CGWindowID) [number unsignedLongLongValue];
        window.frame = frame;
        window.title = [info objectForKey: (id) kCGWindowName];
        {
            NSNumber *layer = [info objectForKey: (id) kCGWindowLayer];
            window.windowLayer = [layer isKindOfClass: [NSNumber class]] ? [layer integerValue] : 0;
        }
        window.onScreen = (options & kCGWindowListOptionOnScreenOnly) != 0;
        if ([owner isKindOfClass: [NSNumber class]])
            window.owningApplication = SCAppplicationForPID((pid_t) [owner integerValue]);
        [windows addObject: window];
        if ([owner isKindOfClass: [NSNumber class]])
            [pids addObject: owner];
    }

    content = [[[SCShareableContent alloc] init] autorelease];
    content.windows = windows;
    content.displays = SCActiveDisplays();
    content.applications = SCApplicationsForPIDs(pids);
    return content;
}

@implementation SCRunningApplication

@synthesize bundleIdentifier = _bundleIdentifier, applicationName = _applicationName, processID = _processID;

- (void)dealloc
{
    [_bundleIdentifier release];
    [_applicationName release];
    [super dealloc];
}

@end

@implementation SCWindow

@synthesize windowID = _windowID, frame = _frame, title = _title, windowLayer = _windowLayer,
    owningApplication = _owningApplication, onScreen = _onScreen;

- (void)dealloc
{
    [_title release];
    [_owningApplication release];
    [super dealloc];
}

@end

@implementation SCDisplay

@synthesize displayID = _displayID, width = _width, height = _height, frame = _frame;

@end

@implementation SCShareableContent

@synthesize windows = _windows, displays = _displays, applications = _applications;

- (void)dealloc
{
    [_windows release];
    [_displays release];
    [_applications release];
    [super dealloc];
}

+ (void)getShareableContentWithCompletionHandler:(void (^)(SCShareableContent *, NSError *))completionHandler
{
    SCDispatchContentCompletion(completionHandler, SCBuildContent(NO, NO, 0), nil);
}

+ (void)getCurrentProcessShareableContentWithCompletionHandler:(void (^)(SCShareableContent *, NSError *))completionHandler
{
    SCDispatchContentCompletion(completionHandler,
        SCBuildContent(NO, NO, [[NSProcessInfo processInfo] processIdentifier]), nil);
}

+ (void)getShareableContentExcludingDesktopWindows:(BOOL)excludeDesktopWindows
                                onScreenWindowsOnly:(BOOL)onScreenWindowsOnly
                                  completionHandler:(void (^)(SCShareableContent *, NSError *))completionHandler
{
    SCDispatchContentCompletion(completionHandler,
        SCBuildContent(excludeDesktopWindows, onScreenWindowsOnly, 0), nil);
}

@end

@implementation SCContentFilter

@synthesize contentRect = _contentRect, pointPixelScale = _pointPixelScale, style = _style,
    streamType = _streamType, includeMenuBar = _includeMenuBar,
    includedApplications = _includedApplications, includedDisplays = _includedDisplays,
    includedWindows = _includedWindows, cameraEnabled = _cameraEnabled,
    microphoneEnabled = _microphoneEnabled;

- (instancetype)initWithDisplay:(SCDisplay *)display
                          window:(SCWindow *)window
                           style:(SCShareableContentStyle)style
                     includeMenu:(BOOL)includeMenu
{
    self = [super init];
    if (self == nil)
        return nil;

    _targetDisplay = [display retain];
    _targetWindow = [window retain];
    _style = style;
    _streamType = window != nil ? SCStreamTypeWindow : SCStreamTypeDisplay;
    _includeMenuBar = includeMenu;
    _pointPixelScale = 1;
    _contentRect = window != nil ? window.frame : display.frame;
    return self;
}

- (instancetype)initWithDesktopIndependentWindow:(SCWindow *)window
{
    SCDisplay *display = nil;

    if (window != nil)
        display = SCDisplayContainingRect(window.frame);
    return [self initWithDisplay: display window: window style: SCShareableContentStyleWindow
                    includeMenu: NO];
}

- (instancetype)initWithDisplay:(SCDisplay *)display excludingWindows:(NSArray *)excluded
{
    self = [self initWithDisplay: display window: nil style: SCShareableContentStyleDisplay
                    includeMenu: NO];
    if (self != nil && [excluded count] > 0) {
        _filterExcludedWindows = [excluded copy];
        _unsupportedFilter = YES;
    }
    return self;
}

- (instancetype)initWithDisplay:(SCDisplay *)display includingWindows:(NSArray *)included
{
    self = [self initWithDisplay: display window: nil style: SCShareableContentStyleDisplay
                    includeMenu: NO];
    if (self == nil)
        return nil;
    _filterIncludedWindows = [included copy];
    self.includedWindows = included;
    if ([included count] == 1)
        _targetWindow = [(SCWindow *) [included objectAtIndex: 0] retain];
    else
        _unsupportedFilter = [included count] != 0;
    return self;
}

- (instancetype)initWithDisplay:(SCDisplay *)display
             includingApplications:(NSArray *)applications
                  exceptingWindows:(NSArray *)exceptingWindows
{
    self = [self initWithDisplay: display window: nil style: SCShareableContentStyleApplication
                    includeMenu: NO];
    if (self != nil) {
        _filterIncludedApplications = [applications copy];
        _filterExcludedWindows = [exceptingWindows copy];
        _unsupportedFilter = YES;
    }
    return self;
}

- (instancetype)initWithDisplay:(SCDisplay *)display
             excludingApplications:(NSArray *)applications
                  exceptingWindows:(NSArray *)exceptingWindows
{
    self = [self initWithDisplay: display window: nil style: SCShareableContentStyleDisplay
                    includeMenu: NO];
    if (self != nil) {
        _filterExcludedApplications = [applications copy];
        _filterExcludedWindows = [exceptingWindows copy];
        _unsupportedFilter = YES;
    }
    return self;
}

- (void)dealloc
{
    [_targetDisplay release];
    [_targetWindow release];
    [_filterIncludedWindows release];
    [_filterExcludedWindows release];
    [_filterIncludedApplications release];
    [_filterExcludedApplications release];
    [_includedApplications release];
    [_includedDisplays release];
    [_includedWindows release];
    [super dealloc];
}

@end

@implementation SCStreamConfiguration

- (instancetype)init
{
    self = [super init];
    if (self != nil)
        [self setDefaults];
    return self;
}

- (void)setDefaults
{
    _pixelFormat = kCVPixelFormatType_32BGRA;
    _scalesToFit = YES;
    _preservesAspectRatio = YES;
    _queueDepth = 3;
    _minimumFrameInterval = kCMTimeInvalid;
    _captureResolution = SCCaptureResolutionAutomatic;
    _captureDynamicRange = SCCaptureDynamicRangeSDR;
}

@synthesize width = _width, height = _height, minimumFrameInterval = _minimumFrameInterval,
    pixelFormat = _pixelFormat, scalesToFit = _scalesToFit, sourceRect = _sourceRect,
    destinationRect = _destinationRect, queueDepth = _queueDepth, showsCursor = _showsCursor,
    capturesAudio = _capturesAudio, sampleRate = _sampleRate, channelCount = _channelCount,
    excludesCurrentProcessAudio = _excludesCurrentProcessAudio, streamName = _streamName,
    backgroundColor = _backgroundColor, colorSpaceName = _colorSpaceName,
    colorMatrix = _colorMatrix, captureResolution = _captureResolution,
    captureDynamicRange = _captureDynamicRange, preservesAspectRatio = _preservesAspectRatio,
    shouldBeOpaque = _shouldBeOpaque, ignoreShadowsDisplay = _ignoreShadowsDisplay,
    ignoreShadowsSingleWindow = _ignoreShadowsSingleWindow,
    ignoreGlobalClipDisplay = _ignoreGlobalClipDisplay,
    ignoreGlobalClipSingleWindow = _ignoreGlobalClipSingleWindow,
    includeChildWindows = _includeChildWindows, captureMicrophone = _captureMicrophone,
    microphoneCaptureDeviceID = _microphoneCaptureDeviceID, showMouseClicks = _showMouseClicks;

- (void)dealloc
{
    [_streamName release];
    [_microphoneCaptureDeviceID release];
    [super dealloc];
}

@end

@implementation SCRecordingOutputConfiguration

@synthesize outputURL = _outputURL, videoCodecType = _videoCodecType, outputFileType = _outputFileType;

- (void)dealloc
{
    [_outputURL release];
    [_videoCodecType release];
    [_outputFileType release];
    [super dealloc];
}

@end

@implementation SCRecordingOutput

- (instancetype)initWithConfiguration:(SCRecordingOutputConfiguration *)recordingOutputConfiguration delegate:(id<SCRecordingOutputDelegate>)delegate
{
    (void) recordingOutputConfiguration;
    (void) delegate;
    return [super init];
}

@end

#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE

static BOOL SCPixelFormatIsSupported(OSType format)
{
    switch (format) {
        case kCVPixelFormatType_32BGRA:
        case kCVPixelFormatType_32RGBA:
        case kCVPixelFormatType_32ARGB:
        case kCVPixelFormatType_32ABGR:
            return YES;
        default:
            return NO;
    }
}

/* The display stream owns its handler block. The block keeps this context,
 * never the SCStream itself; stopCaptureSynchronously clears the pointer before
 * releasing the stream. */
@interface SCCaptureContext : NSObject {
@public
    SCStream *_stream;
}
- (instancetype)initWithStream:(SCStream *)stream;
- (SCStream *)strongStream;
- (void)clearStream;
@end

@implementation SCCaptureContext

- (instancetype)initWithStream:(SCStream *)stream
{
    self = [super init];
    if (self != nil)
        _stream = [stream retain];
    return self;
}

- (SCStream *)strongStream
{
    @synchronized (self) {
        return [_stream retain];
    }
}

- (void)clearStream
{
    @synchronized (self) {
        [_stream release];
        _stream = nil;
    }
}

- (void)dealloc
{
    [_stream release];
    [super dealloc];
}

@end

static CGRect SCFitRect(CGRect source, CGRect destination, BOOL preserveAspectRatio);

static CGDirectDisplayID SCDisplayIDForStream(SCStream *stream)
{
    SCContentFilter *filter = stream->_filter;

    if (filter->_targetDisplay != nil)
        return filter->_targetDisplay.displayID;
    if (filter->_targetWindow != nil) {
        CGDirectDisplayID displays[32];
        uint32_t count = 0;
        CGRect frame = filter->_targetWindow.frame;

        if (CGGetDisplaysWithRect(frame, 32, displays, &count) == kCGErrorSuccess && count > 0)
            return displays[0];
    }
    return CGMainDisplayID();
}

static CGSize SCOutputSize(SCStream *stream, CGRect source)
{
    SCStreamConfiguration *configuration = stream->_configuration;

    return CGSizeMake(configuration.width > 0 ? configuration.width : source.size.width,
        configuration.height > 0 ? configuration.height : source.size.height);
}

/* CGDisplayStream receives the destination in output-surface coordinates. It
 * applies the documented fit and crop itself, so the sample buffer only wraps
 * the surface delivered by CoreGraphics. */
static CGRect SCDestinationRect(SCStream *stream, CGRect source, CGSize outputSize)
{
    SCStreamConfiguration *configuration = stream->_configuration;
    CGRect full = CGRectMake(0, 0, outputSize.width, outputSize.height);
    CGRect destination;

    if (!CGRectIsEmpty(configuration.destinationRect))
        destination = CGRectIntersection(configuration.destinationRect, full);
    else if (configuration.scalesToFit || configuration.preservesAspectRatio)
        destination = SCFitRect(source, full, configuration.preservesAspectRatio);
    else
        destination = full;
    return CGRectIsNull(destination) ? CGRectZero : destination;
}

/* kCGDisplayStreamMinimumFrameTime is a CFNumber of seconds. */
static double SCMinimumFrameTime(SCStreamConfiguration *configuration)
{
    if (!CMTimeIsValid(configuration.minimumFrameInterval))
        return 0;
    double seconds = CMTimeGetSeconds(configuration.minimumFrameInterval);
    return seconds > 0 ? seconds : 0;
}

static CGRect SCFitRect(CGRect source, CGRect destination, BOOL preserveAspectRatio)
{
    CGFloat scaleX, scaleY, scale, width, height;

    if (!preserveAspectRatio || CGRectIsEmpty(source) || CGRectIsEmpty(destination))
        return destination;
    scaleX = destination.size.width / source.size.width;
    scaleY = destination.size.height / source.size.height;
    scale = scaleX < scaleY ? scaleX : scaleY;
    width = source.size.width * scale;
    height = source.size.height * scale;
    return CGRectMake(destination.origin.x + (destination.size.width - width) / 2,
        destination.origin.y + (destination.size.height - height) / 2, width, height);
}

static CGRect SCSourceRect(SCStream *stream, CGRect *displayBounds)
{
    SCContentFilter *filter = stream->_filter;
    SCStreamConfiguration *configuration = stream->_configuration;
    CGRect bounds = CGDisplayBounds(SCDisplayIDForStream(stream));
    CGRect source = [filter contentRect];

    if (CGRectIsEmpty(bounds)) {
        *displayBounds = CGRectZero;
        return CGRectZero;
    }
    if (!CGRectIsEmpty(configuration.sourceRect)) {
        source = CGRectMake(source.origin.x + configuration.sourceRect.origin.x,
            source.origin.y + configuration.sourceRect.origin.y,
            configuration.sourceRect.size.width, configuration.sourceRect.size.height);
    }
    source = CGRectIntersection(source, bounds);
    if (CGRectIsNull(source))
        source = CGRectZero;

    *displayBounds = bounds;
    return source;
}

static NSDictionary *SCStreamProperties(SCStream *stream, CGRect source, CGRect destination)
{
    SCStreamConfiguration *configuration = stream->_configuration;
    NSMutableDictionary *properties = [NSMutableDictionary dictionary];
    double minimumFrameTime = SCMinimumFrameTime(configuration);

    if (!CGRectIsEmpty(source))
        [properties setObject: [NSValue valueWithRect: source]
            forKey: (id) kCGDisplayStreamSourceRect];
    if (!CGRectIsEmpty(destination))
        [properties setObject: [NSValue valueWithRect: destination]
            forKey: (id) kCGDisplayStreamDestinationRect];
    [properties setObject: [NSNumber numberWithBool: configuration.preservesAspectRatio]
        forKey: (id) kCGDisplayStreamPreserveAspectRatio];
    [properties setObject: [NSNumber numberWithBool: configuration.showsCursor]
        forKey: (id) kCGDisplayStreamShowCursor];
    if (minimumFrameTime > 0)
        [properties setObject: [NSNumber numberWithDouble: minimumFrameTime]
            forKey: (id) kCGDisplayStreamMinimumFrameTime];
    if (configuration.queueDepth > 0)
        [properties setObject: [NSNumber numberWithInteger: configuration.queueDepth]
            forKey: (id) kCGDisplayStreamQueueDepth];
    return properties;
}

static CMSampleBufferRef SCCreateScreenSampleBuffer(SCStream *stream, IOSurfaceRef surface,
    uint64_t displayTime, CGDisplayStreamUpdateRef updateRef, CGRect source, CGRect destination)
{
    SCStreamConfiguration *configuration = stream->_configuration;
    CVPixelBufferRef pixelBuffer = NULL;
    CMVideoFormatDescriptionRef description = NULL;
    CMSampleBufferRef sample = NULL;
    CMSampleTimingInfo timing;
    CFArrayRef attachments;
    CFMutableDictionaryRef attachment;
    CFNumberRef number;
    double frameRate = 30;
    int64_t pts = (int64_t) displayTime;
    int64_t durationNs;

    if (surface == NULL || CGRectIsEmpty(source) || CGRectIsEmpty(destination))
        return NULL;
    if ((OSType) IOSurfaceGetPixelFormat(surface) != configuration.pixelFormat)
        return NULL;
    if (CVPixelBufferCreateWithIOSurface(NULL, surface, NULL, &pixelBuffer) != kCVReturnSuccess)
        return NULL;

    if (CMVideoFormatDescriptionCreateForImageBuffer(NULL, pixelBuffer, &description) != noErr) {
        CVPixelBufferRelease(pixelBuffer);
        return NULL;
    }

    memset(&timing, 0, sizeof(timing));
    timing.presentationTimeStamp = pts >= 0
        ? CMTimeMake(pts, 1000000000)
        : CMTimeConvertScale(CMClockGetTime(CMClockGetHostTimeClock()), 1000000000,
            kCMTimeRoundingMethod_RoundHalfAwayFromZero);
    {
        double minimum = SCMinimumFrameTime(configuration);
        if (minimum > 0)
            frameRate = 1 / minimum;
    }
    durationNs = (int64_t) (1000000000 / frameRate);
    if (durationNs <= 0)
        durationNs = 33333333;
    timing.duration = CMTimeMake(durationNs, 1000000000);

    if (CMSampleBufferCreateReadyWithImageBuffer(NULL, pixelBuffer, description, &timing, &sample)
            != noErr) {
        CFRelease(description);
        CVPixelBufferRelease(pixelBuffer);
        return NULL;
    }
    CFRelease(description);
    CVPixelBufferRelease(pixelBuffer);

    attachments = CMSampleBufferGetSampleAttachmentsArray(sample, true);
    if (attachments != NULL && CFArrayGetCount(attachments) > 0) {
        NSMutableArray *dirty = [NSMutableArray array];
        size_t rectCount = 0, i;
        const CGRect *rects = CGDisplayStreamUpdateGetRects(updateRef,
            kCGDisplayStreamUpdateReducedDirtyRects, &rectCount);

        attachment = (CFMutableDictionaryRef) CFArrayGetValueAtIndex(attachments, 0);
        number = CFNumberCreate(NULL, kCFNumberSInt64Type, &pts);
        CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoDisplayTime, number);
        CFRelease(number);

        {
            int64_t status = SCFrameStatusComplete;
            number = CFNumberCreate(NULL, kCFNumberSInt64Type, &status);
            CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoStatus, number);
            CFRelease(number);
        }

        {
            int64_t one = 1;
            CFNumberRef scale = CFNumberCreate(NULL, kCFNumberSInt64Type, &one);
            CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoScaleFactor, scale);
            CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoContentScale, scale);
            CFRelease(scale);
        }
        CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoContentRect,
            [NSValue valueWithRect: source]);
        CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoBoundingRect,
            [NSValue valueWithRect: destination]);
        CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoScreenRect,
            [NSValue valueWithRect: source]);

        for (i = 0; i < rectCount && rects != NULL; i++) {
            CGRect rect = CGRectIntersection(rects[i], destination);
            if (!CGRectIsNull(rect) && !CGRectIsEmpty(rect))
                [dirty addObject: [NSValue valueWithRect: rect]];
        }
        if ([dirty count] == 0)
            [dirty addObject: [NSValue valueWithRect: destination]];
        CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoDirtyRects, dirty);
    }
    return sample;
}

static NSError *SCDisplayStreamError(CGError error)
{
    if (error == kCGErrorIllegalArgument)
        return SCMakeError(SCStreamErrorFailedToStart,
            @"CoreGraphics rejected the requested display stream configuration.");
    return SCMakeError(SCStreamErrorFailedToStart,
        [NSString stringWithFormat: @"The display stream could not be started (%d).", error]);
}

static void SCDisplayFrameAvailable(SCCaptureContext *context,
    CGDisplayStreamFrameStatus status, uint64_t displayTime, IOSurfaceRef surface,
    CGDisplayStreamUpdateRef updateRef)
{
    SCStream *stream;

    if (status == kCGDisplayStreamFrameStatusStopped || surface == NULL)
        return;
    stream = [context strongStream];
    if (stream == nil)
        return;
    [stream deliverSurface: surface displayTime: displayTime update: updateRef];
    [stream release];
}
#endif

@implementation SCStream

- (instancetype)initWithFilter:(SCContentFilter *)contentFilter
                 configuration:(SCStreamConfiguration *)streamConfig
                      delegate:(id<SCStreamDelegate>)delegate
{
    self = [super init];
    if (self == nil || contentFilter == nil || streamConfig == nil)
        return nil;

    _filter = [contentFilter copy];
    _configuration = [streamConfig copy];
    _delegate = delegate;
    _outputs = [[NSMutableArray alloc] init];
    return self;
}

- (void)dealloc
{
    [self stopCaptureSynchronously];
    [_filter release];
    [_configuration release];
    [_outputs release];
    [super dealloc];
}

- (BOOL)isCapturing
{
    return __atomic_load_n(&_capturing, __ATOMIC_ACQUIRE);
}

- (CMClockRef)synchronizationClock
{
    return CMClockGetHostTimeClock();
}

- (BOOL)addStreamOutput:(id<SCStreamOutput>)output
                   type:(SCStreamOutputType)type
       sampleHandlerQueue:(dispatch_queue_t)sampleHandlerQueue
                   error:(NSError **)error
{
    DarlingScreenCaptureOutput *entry;

    if (output == nil) {
        if (error != NULL)
            *error = SCMakeError(SCStreamErrorFailedToStart, @"A stream output is required.");
        return NO;
    }
    if (type != SCStreamOutputTypeScreen) {
        if (error != NULL)
            *error = SCMakeError(SCStreamErrorFailedToStart,
                @"Audio and microphone stream outputs are not supported by this backend.");
        return NO;
    }

    entry = [[[DarlingScreenCaptureOutput alloc] init] autorelease];
    entry.output = output;
    entry.type = type;
    if (sampleHandlerQueue != NULL) {
        dispatch_retain(sampleHandlerQueue);
        entry.queue = sampleHandlerQueue;
    }

    @synchronized (_outputs) {
        for (DarlingScreenCaptureOutput *existing in _outputs) {
            if (existing.output == output && existing.type == type) {
                if (error != NULL)
                    *error = SCMakeError(SCStreamErrorFailedToStart,
                        @"The stream output is already registered.");
                return NO;
            }
        }
        [_outputs addObject: entry];
    }
    return YES;
}

- (BOOL)removeStreamOutput:(id<SCStreamOutput>)output
                      type:(SCStreamOutputType)type
                     error:(NSError **)error
{
    BOOL removed = NO;

    @synchronized (_outputs) {
        for (DarlingScreenCaptureOutput *entry in [[_outputs copy] autorelease]) {
            if (entry.output == output && entry.type == type) {
                [_outputs removeObject: entry];
                removed = YES;
                break;
            }
        }
    }
    if (!removed && error != NULL)
        *error = SCMakeError(SCStreamErrorFailedToStart, @"The stream output is not registered.");
    return removed;
}

- (BOOL)addRecordingOutput:(SCRecordingOutput *)recordingOutput error:(NSError **)error
{
    (void) recordingOutput;
    if (error != NULL)
        *error = SCMakeError(SCStreamErrorFailedToStart,
            @"Recording outputs are not supported by this backend.");
    return NO;
}

- (BOOL)removeRecordingOutput:(SCRecordingOutput *)recordingOutput error:(NSError **)error
{
    (void) recordingOutput;
    if (error != NULL)
        *error = SCMakeError(SCStreamErrorFailedToStart,
            @"Recording outputs are not supported by this backend.");
    return NO;
}

- (void)updateConfiguration:(SCStreamConfiguration *)streamConfig
          completionHandler:(void (^)(NSError *))completionHandler
{
    if (streamConfig == nil) {
        SCDispatchErrorCompletion(completionHandler,
            SCMakeError(SCStreamErrorFailedToStart, @"A stream configuration is required."));
        return;
    }

    [self retain];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        SCStreamConfiguration *copy = [streamConfig copy];
        NSError *error = nil;

        [self stopCaptureSynchronously];
        [_configuration release];
        _configuration = copy;
        if ([_outputs count] > 0)
            error = [self startCaptureSynchronously];
        SCDispatchErrorCompletion(completionHandler, error);
        [self release];
    });
}

- (void)updateContentFilter:(SCContentFilter *)contentFilter
          completionHandler:(void (^)(NSError *))completionHandler
{
    if (contentFilter == nil) {
        SCDispatchErrorCompletion(completionHandler,
            SCMakeError(SCStreamErrorFailedToStart, @"A content filter is required."));
        return;
    }

    [self retain];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        SCContentFilter *copy = [contentFilter copy];
        NSError *error = nil;

        [self stopCaptureSynchronously];
        [_filter release];
        _filter = copy;
        if ([_outputs count] > 0)
            error = [self startCaptureSynchronously];
        SCDispatchErrorCompletion(completionHandler, error);
        [self release];
    });
}

- (void)startCaptureWithCompletionHandler:(void (^)(NSError *))completionHandler
{
    [self retain];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSError *error = [self startCaptureSynchronously];
        SCDispatchErrorCompletion(completionHandler, error);
        [self release];
    });
}

- (void)stopCaptureWithCompletionHandler:(void (^)(NSError *))completionHandler
{
    [self retain];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [self stopCaptureSynchronously];
        SCDispatchErrorCompletion(completionHandler, nil);
        [self release];
    });
}

#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
- (void)deliverSurface:(IOSurfaceRef)surface
            displayTime:(uint64_t)displayTime
                 update:(CGDisplayStreamUpdateRef)updateRef
{
    CGRect bounds = CGRectZero;
    CGRect source;
    CGSize outputSize;
    CGRect destination;
    CMSampleBufferRef sample;
    NSArray *outputs;

    if (!__atomic_load_n(&_capturing, __ATOMIC_ACQUIRE))
        return;
    source = SCSourceRect(self, &bounds);
    outputSize = SCOutputSize(self, source);
    destination = SCDestinationRect(self, source, outputSize);
    sample = SCCreateScreenSampleBuffer(self, surface, displayTime, updateRef,
        source, destination);
    if (sample == NULL)
        return;

    @synchronized (_outputs) {
        outputs = [self->_outputs copy];
    }
    for (DarlingScreenCaptureOutput *entry in outputs) {
        if (entry.type != SCStreamOutputTypeScreen)
            continue;
        if (![entry.output respondsToSelector:
                @selector(stream:didOutputSampleBuffer:ofType:)])
            continue;
        if (entry.queue != NULL) {
            CFRetain(sample);
            [entry retain];
            [self retain];
            dispatch_async(entry.queue, ^{
                [entry.output stream: self didOutputSampleBuffer: sample
                              ofType: SCStreamOutputTypeScreen];
                CFRelease(sample);
                [entry release];
                [self release];
            });
        } else {
            [entry.output stream: self didOutputSampleBuffer: sample
                          ofType: SCStreamOutputTypeScreen];
        }
    }
    [outputs release];
    CFRelease(sample);
}

- (NSError *)startCaptureSynchronously
{
    CGRect bounds = CGRectZero;
    CGRect source;
    CGSize outputSize;
    CGRect destination;
    CGDirectDisplayID displayID;
    NSDictionary *properties;
    SCCaptureContext *context;
    CGDisplayStreamRef displayStream;
    CGError result;

    if (__atomic_load_n(&_capturing, __ATOMIC_ACQUIRE))
        return nil;
    if (_filter->_unsupportedFilter)
        return SCMakeError(SCStreamErrorFailedToStart,
            @"This content filter cannot be enforced by CoreGraphics display capture.");
    if (!SCPixelFormatIsSupported(_configuration.pixelFormat))
        return SCMakeError(SCStreamErrorFailedToStart,
            @"The requested pixel format is not supported by this backend.");

    displayID = SCDisplayIDForStream(self);
    source = SCSourceRect(self, &bounds);
    if (CGRectIsEmpty(bounds) || CGRectIsEmpty(source))
        return SCMakeError(SCStreamErrorFailedToStart, @"The requested content is not on a display.");
    outputSize = SCOutputSize(self, source);
    if (outputSize.width <= 0 || outputSize.height <= 0)
        return SCMakeError(SCStreamErrorFailedToStart, @"The stream has no output area.");
    destination = SCDestinationRect(self, source, outputSize);
    if (CGRectIsEmpty(destination))
        return SCMakeError(SCStreamErrorFailedToStart, @"The destination rect is empty.");

    _captureQueue = dispatch_queue_create("Darling.ScreenCaptureKit.stream", DISPATCH_QUEUE_SERIAL);
    if (_captureQueue == NULL)
        return SCMakeError(SCStreamErrorFailedToStart, @"A capture queue could not be created.");
    context = [[SCCaptureContext alloc] initWithStream: self];
    _captureContext = context;
    properties = SCStreamProperties(self, source, destination);
    displayStream = CGDisplayStreamCreateWithDispatchQueue(displayID,
        (size_t) outputSize.width, (size_t) outputSize.height,
        _configuration.pixelFormat, (CFDictionaryRef) properties, _captureQueue,
        ^(CGDisplayStreamFrameStatus status, uint64_t displayTime,
            IOSurfaceRef surface, CGDisplayStreamUpdateRef updateRef)
        {
            SCDisplayFrameAvailable(context, status, displayTime, surface, updateRef);
        });
    if (displayStream == NULL) {
        [self stopCaptureSynchronously];
        return SCMakeError(SCStreamErrorFailedToStart, @"The display stream could not be created.");
    }
    _displayStream = displayStream;
    __atomic_store_n(&_capturing, YES, __ATOMIC_RELEASE);

    result = CGDisplayStreamStart(displayStream);
    if (result != kCGErrorSuccess) {
        [self stopCaptureSynchronously];
        return SCDisplayStreamError(result);
    }
    return nil;
}

- (void)stopCaptureSynchronously
{
    SCCaptureContext *context = _captureContext;

    __atomic_store_n(&_capturing, NO, __ATOMIC_RELEASE);
    [context clearStream];
    if (_displayStream != NULL) {
        CGDisplayStreamStop(_displayStream);
        CFRelease(_displayStream);
        _displayStream = NULL;
    }
    _captureContext = nil;
    [context release];
    if (_captureQueue != NULL) {
        dispatch_release(_captureQueue);
        _captureQueue = NULL;
    }
}
#else
- (NSError *)startCaptureSynchronously
{
    return SCUnavailableError();
}

- (void)stopCaptureSynchronously
{
    __atomic_store_n(&_capturing, NO, __ATOMIC_RELEASE);
}
#endif

- (BOOL)restartCaptureWithError:(NSError **)error
{
    NSError *localError;

    [self stopCaptureSynchronously];
    localError = [self startCaptureSynchronously];
    if (error != NULL)
        *error = localError;
    return localError == nil;
}

@end

@implementation SCScreenshotConfiguration

@synthesize width = _width, height = _height, showsCursor = _showsCursor, sourceRect = _sourceRect,
    destinationRect = _destinationRect, ignoreShadows = _ignoreShadows, ignoreClipping = _ignoreClipping,
    includeChildWindows = _includeChildWindows, displayIntent = _displayIntent,
    dynamicRange = _dynamicRange, contentType = _contentType, fileURL = _fileURL;

- (void)dealloc
{
    [_contentType release];
    [_fileURL release];
    [super dealloc];
}

@end

@implementation SCScreenshotOutput
@synthesize sdrImage = _sdrImage, hdrImage = _hdrImage, fileURL = _fileURL;
@end

@implementation SCScreenshotManager

+ (void)captureImageWithFilter:(SCContentFilter *)contentFilter
                 configuration:(SCStreamConfiguration *)config
             completionHandler:(void (^)(CGImageRef, NSError *))completionHandler
{
    (void) contentFilter;
    (void) config;
    SCDispatchScreenshotErrorCompletion((void (^)(id, NSError *)) completionHandler,
        SCUnavailableError());
}

+ (void)captureSampleBufferWithFilter:(SCContentFilter *)contentFilter
                        configuration:(SCStreamConfiguration *)config
                    completionHandler:(void (^)(void *, NSError *))completionHandler
{
    (void) contentFilter;
    (void) config;
    SCDispatchScreenshotErrorCompletion((void (^)(id, NSError *)) completionHandler,
        SCUnavailableError());
}

+ (void)captureImageInRect:(CGRect)rect completionHandler:(void (^)(CGImageRef, NSError *))completionHandler
{
    (void) rect;
    SCDispatchScreenshotErrorCompletion((void (^)(id, NSError *)) completionHandler,
        SCUnavailableError());
}

+ (void)captureScreenshotWithFilter:(SCContentFilter *)contentFilter
                      configuration:(SCScreenshotConfiguration *)config
                  completionHandler:(void (^)(id, NSError *))completionHandler
{
    (void) contentFilter;
    (void) config;
    SCDispatchScreenshotErrorCompletion(completionHandler, SCUnavailableError());
}

+ (void)captureScreenshotWithRect:(CGRect)rect
                    configuration:(SCScreenshotConfiguration *)config
                completionHandler:(void (^)(id, NSError *))completionHandler
{
    (void) rect;
    (void) config;
    SCDispatchScreenshotErrorCompletion(completionHandler, SCUnavailableError());
}

@end
