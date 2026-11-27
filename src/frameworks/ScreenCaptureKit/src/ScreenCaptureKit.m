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
#import <CoreGraphics/CGDisplayStream.h>
#import <CoreVideo/CoreVideo.h>
#import <IOSurface/IOSurfaceRef.h>

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
    CGDisplayStreamRef _displayStream;
    dispatch_queue_t _captureQueue;
}
- (NSError *)startCaptureSynchronously;
- (void)stopCaptureSynchronously;
- (BOOL)restartCaptureWithError:(NSError **)error;
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

- (id)copyWithZone:(NSZone *)zone
{
    SCContentFilter *copy = [[[self class] allocWithZone: zone] initWithDisplay: _targetDisplay
        window: _targetWindow style: _style includeMenu: _includeMenuBar];

    if (copy == nil)
        return nil;
    copy->_contentRect = _contentRect;
    copy->_pointPixelScale = _pointPixelScale;
    copy->_streamType = _streamType;
    copy->_filterIncludedWindows = [_filterIncludedWindows copy];
    copy->_filterExcludedWindows = [_filterExcludedWindows copy];
    copy->_filterIncludedApplications = [_filterIncludedApplications copy];
    copy->_filterExcludedApplications = [_filterExcludedApplications copy];
    copy->_unsupportedFilter = _unsupportedFilter;
    copy->_includedApplications = [_includedApplications retain];
    copy->_includedDisplays = [_includedDisplays retain];
    copy->_includedWindows = [_includedWindows retain];
    copy->_cameraEnabled = _cameraEnabled;
    copy->_microphoneEnabled = _microphoneEnabled;
    return copy;
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

- (id)copyWithZone:(NSZone *)zone
{
    SCStreamConfiguration *copy = [[[self class] allocWithZone: zone] init];

    if (copy == nil)
        return nil;
    copy->_width = _width;
    copy->_height = _height;
    copy->_minimumFrameInterval = _minimumFrameInterval;
    copy->_pixelFormat = _pixelFormat;
    copy->_scalesToFit = _scalesToFit;
    copy->_sourceRect = _sourceRect;
    copy->_destinationRect = _destinationRect;
    copy->_queueDepth = _queueDepth;
    copy->_showsCursor = _showsCursor;
    copy->_capturesAudio = _capturesAudio;
    copy->_sampleRate = _sampleRate;
    copy->_channelCount = _channelCount;
    copy->_excludesCurrentProcessAudio = _excludesCurrentProcessAudio;
    copy->_streamName = [_streamName copy];
    copy->_backgroundColor = _backgroundColor;
    copy->_colorSpaceName = _colorSpaceName;
    copy->_colorMatrix = _colorMatrix;
    copy->_captureResolution = _captureResolution;
    copy->_captureDynamicRange = _captureDynamicRange;
    copy->_preservesAspectRatio = _preservesAspectRatio;
    copy->_shouldBeOpaque = _shouldBeOpaque;
    copy->_ignoreShadowsDisplay = _ignoreShadowsDisplay;
    copy->_ignoreShadowsSingleWindow = _ignoreShadowsSingleWindow;
    copy->_ignoreGlobalClipDisplay = _ignoreGlobalClipDisplay;
    copy->_ignoreGlobalClipSingleWindow = _ignoreGlobalClipSingleWindow;
    copy->_includeChildWindows = _includeChildWindows;
    copy->_captureMicrophone = _captureMicrophone;
    copy->_microphoneCaptureDeviceID = [_microphoneCaptureDeviceID copy];
    copy->_showMouseClicks = _showMouseClicks;
    return copy;
}

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

static CGSize SCOutputSize(SCStreamConfiguration *configuration, CGRect source)
{
    CGFloat width = configuration.width > 0 ? configuration.width : source.size.width;
    CGFloat height = configuration.height > 0 ? configuration.height : source.size.height;

    if (!(width > 0) || !(height > 0) || width > 16384 || height > 16384)
        return CGSizeZero;
    return CGSizeMake(width, height);
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

static CGRect SCSourceRect(SCStream *stream, CGRect *displayBounds,
                           CGDirectDisplayID *displayIDOut)
{
    SCContentFilter *filter = stream->_filter;
    SCStreamConfiguration *configuration = stream->_configuration;
    CGRect bounds = CGRectZero;
    CGRect source = [filter contentRect];
    CGDirectDisplayID displayID = CGMainDisplayID();

    if (filter->_targetWindow != nil) {
        CGRect windowFrame = filter->_targetWindow.frame;
        CGDirectDisplayID displays[32];
        uint32_t count = 0;

        if (CGGetDisplaysWithRect(windowFrame, 32, displays, &count) == kCGErrorSuccess && count > 0)
            displayID = displays[0];
    } else if (filter->_targetDisplay != nil) {
        displayID = filter->_targetDisplay.displayID;
    }
    bounds = CGDisplayBounds(displayID);
    if (CGRectIsEmpty(bounds)) {
        *displayBounds = CGRectZero;
        if (displayIDOut != NULL)
            *displayIDOut = displayID;
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
    if (displayIDOut != NULL)
        *displayIDOut = displayID;
    return source;
}

static CGRect SCDestinationRect(SCStream *stream, CGRect source, CGSize outputSize)
{
    SCStreamConfiguration *configuration = stream->_configuration;
    CGRect destination = CGRectMake(0, 0, outputSize.width, outputSize.height);

    if (!CGRectIsEmpty(configuration.destinationRect))
        destination = CGRectIntersection(configuration.destinationRect, destination);
    else if (configuration.scalesToFit || configuration.preservesAspectRatio)
        destination = SCFitRect(source, destination, configuration.preservesAspectRatio);
    return CGRectIsNull(destination) ? CGRectZero : destination;
}

static CMSampleBufferRef SCCreateScreenSampleBuffer(SCStream *stream,
                                                    IOSurfaceRef surface,
                                                    CGDisplayStreamUpdateRef update,
                                                    CGRect source,
                                                    CGSize outputSize,
                                                    uint64_t displayTime)
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
    int64_t durationNs;
    int64_t ptsNs = displayTime <= (uint64_t) INT64_MAX
        ? (int64_t) displayTime : -1;
    size_t width = (size_t) outputSize.width;
    size_t height = (size_t) outputSize.height;
    uint8_t *surfaceBase;
    uint32_t surfaceStride, destinationStride;
    uint8_t *destination;
    const CGRect *rects;
    size_t rectCount = 0;

    if (CGRectIsEmpty(source) || surface == NULL
            || IOSurfaceGetPixelFormat(surface) != configuration.pixelFormat)
        return NULL;
    if ((size_t) IOSurfaceGetWidth(surface) != width
            || (size_t) IOSurfaceGetHeight(surface) != height)
        return NULL;
    if (CVPixelBufferCreate(NULL, width, height, configuration.pixelFormat, NULL, &pixelBuffer)
            != kCVReturnSuccess)
        return NULL;
    if (CVPixelBufferLockBaseAddress(pixelBuffer, 0) != kCVReturnSuccess) {
        CVPixelBufferRelease(pixelBuffer);
        return NULL;
    }
    if (IOSurfaceLock(surface, kIOSurfaceLockReadOnly, NULL) != 0) {
        CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
        CVPixelBufferRelease(pixelBuffer);
        return NULL;
    }

    surfaceBase = (uint8_t *) IOSurfaceGetBaseAddress(surface);
    surfaceStride = (uint32_t) IOSurfaceGetBytesPerRow(surface);
    destination = (uint8_t *) CVPixelBufferGetBaseAddress(pixelBuffer);
    destinationStride = (uint32_t) CVPixelBufferGetBytesPerRow(pixelBuffer);
    if (surfaceStride < width * 4 || destinationStride < width * 4) {
        IOSurfaceUnlock(surface, kIOSurfaceLockReadOnly, NULL);
        CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
        CVPixelBufferRelease(pixelBuffer);
        return NULL;
    }
    for (size_t y = 0; y < height; y++)
        memcpy(destination + y * destinationStride, surfaceBase + y * surfaceStride, width * 4);
    IOSurfaceUnlock(surface, kIOSurfaceLockReadOnly, NULL);
    CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);

    if (CMVideoFormatDescriptionCreateForImageBuffer(NULL, pixelBuffer, &description) != noErr) {
        CVPixelBufferRelease(pixelBuffer);
        return NULL;
    }

    memset(&timing, 0, sizeof(timing));
    timing.presentationTimeStamp = ptsNs >= 0
        ? CMTimeMake(ptsNs, 1000000000)
        : CMClockGetTime(CMClockGetHostTimeClock());
    if (CMTimeGetSeconds(configuration.minimumFrameInterval) > 0)
        frameRate = 1 / CMTimeGetSeconds(configuration.minimumFrameInterval);
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
        attachment = (CFMutableDictionaryRef) CFArrayGetValueAtIndex(attachments, 0);
        number = CFNumberCreate(NULL, kCFNumberSInt64Type, &ptsNs);
        CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoDisplayTime, number);
        CFRelease(number);

        number = CFNumberCreate(NULL, kCFNumberSInt64Type, &(int64_t){ SCFrameStatusComplete });
        CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoStatus, number);
        CFRelease(number);

        number = CFNumberCreate(NULL, kCFNumberSInt64Type, &(int64_t){ 1 });
        CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoScaleFactor, number);
        CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoContentScale, number);
        CFRelease(number);

        CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoContentRect,
                             [NSValue valueWithRect: source]);
        CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoBoundingRect,
                             [NSValue valueWithRect: source]);
        CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoScreenRect,
                             [NSValue valueWithRect: source]);

        rects = CGDisplayStreamUpdateGetRects(update, kCGDisplayStreamUpdateRefreshedRects, &rectCount);
        if (rects != NULL && rectCount > 0) {
            NSMutableArray *dirty = [NSMutableArray arrayWithCapacity: rectCount];
            for (size_t i = 0; i < rectCount; i++)
                [dirty addObject: [NSValue valueWithRect: rects[i]]];
            CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoDirtyRects, dirty);
        } else {
            CFDictionarySetValue(attachment, (CFStringRef) SCStreamFrameInfoDirtyRects,
                                 [NSArray arrayWithObject: [NSValue valueWithRect: source]]);
        }
    }
    return sample;
}

static void SCDispatchUnexpectedStopError(SCStream *stream)
{
    [stream retain];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        id<SCStreamDelegate> delegate = stream->_delegate;
        if (delegate != nil && [delegate respondsToSelector: @selector(stream:didStopWithError:)])
            [delegate stream: stream
             didStopWithError: SCMakeError(SCStreamErrorFailedToStart,
                 @"The display stream stopped unexpectedly.")];
        [stream release];
    });
}

static void SCDisplayFrame(SCStream *stream, CGDisplayStreamFrameStatus status,
                           uint64_t displayTime, IOSurfaceRef surface,
                           CGDisplayStreamUpdateRef update)
{
    CGRect displayBounds = CGRectZero;
    CGRect source;
    CGSize outputSize;
    CMSampleBufferRef sample;
    NSArray *outputs;

    if (status == kCGDisplayStreamFrameStatusStopped) {
        if (__atomic_exchange_n(&stream->_capturing, NO, __ATOMIC_ACQ_REL))
            SCDispatchUnexpectedStopError(stream);
        return;
    }
    if (!__atomic_load_n(&stream->_capturing, __ATOMIC_ACQUIRE)
            || status != kCGDisplayStreamFrameStatusFrameComplete || surface == NULL)
        return;

    source = SCSourceRect(stream, &displayBounds, NULL);
    outputSize = SCOutputSize(stream->_configuration, source);
    sample = SCCreateScreenSampleBuffer(stream, surface, update, source, outputSize, displayTime);
    if (sample == NULL)
        return;

    @synchronized (stream->_outputs) {
        outputs = [stream->_outputs copy];
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
            [stream retain];
            dispatch_async(entry.queue, ^{
                [entry.output stream: stream didOutputSampleBuffer: sample
                              ofType: SCStreamOutputTypeScreen];
                CFRelease(sample);
                [entry release];
                [stream release];
            });
        } else {
            [entry.output stream: stream didOutputSampleBuffer: sample
                          ofType: SCStreamOutputTypeScreen];
        }
    }
    [outputs release];
    CFRelease(sample);
}

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

- (NSError *)startCaptureSynchronously
{
    CGRect bounds = CGRectZero;
    CGRect source;
    CGSize outputSize;
    CGRect destination;
    CGDirectDisplayID displayID = CGMainDisplayID();
    CFMutableDictionaryRef properties;
    CGDisplayStreamRef stream;
    SCStream *capturedStream = self;
    double minimumFrameTime;
    int64_t queueDepth;
    CGError result;

    if (__atomic_load_n(&_capturing, __ATOMIC_ACQUIRE))
        return nil;
    if (_filter->_unsupportedFilter)
        return SCMakeError(SCStreamErrorFailedToStart,
            @"This content filter cannot be enforced by the CoreGraphics display stream.");
    if (!SCPixelFormatIsSupported(_configuration.pixelFormat))
        return SCMakeError(SCStreamErrorFailedToStart,
            @"The requested pixel format is not supported by this backend.");

    source = SCSourceRect(self, &bounds, &displayID);
    outputSize = SCOutputSize(_configuration, source);
    destination = SCDestinationRect(self, source, outputSize);
    if (CGRectIsEmpty(bounds) || CGRectIsEmpty(source)
            || CGSizeEqualToSize(outputSize, CGSizeZero) || CGRectIsEmpty(destination))
        return SCMakeError(SCStreamErrorFailedToStart, @"The requested content is not on a display.");

    _captureQueue = dispatch_queue_create("com.darling.screencapturekit.display-stream",
                                          DISPATCH_QUEUE_SERIAL);
    if (_captureQueue == NULL)
        return SCMakeError(SCStreamErrorFailedToStart, @"The capture queue could not be created.");

    properties = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
                                           &kCFTypeDictionaryValueCallBacks);
    if (properties != NULL) {
        CFNumberRef number;

        CFDictionarySetValue(properties, kCGDisplayStreamSourceRect,
                             [NSValue valueWithRect: source]);
        CFDictionarySetValue(properties, kCGDisplayStreamDestinationRect,
                             [NSValue valueWithRect: destination]);
        CFDictionarySetValue(properties, kCGDisplayStreamPreserveAspectRatio,
                             _configuration.preservesAspectRatio ? kCFBooleanTrue : kCFBooleanFalse);
        CFDictionarySetValue(properties, kCGDisplayStreamShowCursor,
                             _configuration.showsCursor ? kCFBooleanTrue : kCFBooleanFalse);

        queueDepth = _configuration.queueDepth > 0 ? _configuration.queueDepth : 3;
        number = CFNumberCreate(NULL, kCFNumberSInt64Type, &queueDepth);
        if (number != NULL) {
            CFDictionarySetValue(properties, kCGDisplayStreamQueueDepth, number);
            CFRelease(number);
        }
        minimumFrameTime = CMTimeGetSeconds(_configuration.minimumFrameInterval);
        if (minimumFrameTime > 0) {
            number = CFNumberCreate(NULL, kCFNumberDoubleType, &minimumFrameTime);
            if (number != NULL) {
                CFDictionarySetValue(properties, kCGDisplayStreamMinimumFrameTime, number);
                CFRelease(number);
            }
        }
    }

    stream = CGDisplayStreamCreateWithDispatchQueue(displayID, (size_t) outputSize.width,
        (size_t) outputSize.height, _configuration.pixelFormat, properties, _captureQueue,
        ^(CGDisplayStreamFrameStatus status, uint64_t displayTime, IOSurfaceRef surface,
          CGDisplayStreamUpdateRef update) {
            @autoreleasepool {
                SCDisplayFrame(capturedStream, status, displayTime, surface, update);
            }
        });
    if (properties != NULL)
        CFRelease(properties);
    if (stream == NULL) {
        dispatch_release(_captureQueue);
        _captureQueue = NULL;
        return SCMakeError(SCStreamErrorFailedToStart, @"The display stream could not be created.");
    }

    __atomic_store_n(&_capturing, YES, __ATOMIC_RELEASE);
    result = CGDisplayStreamStart(stream);
    if (result != kCGErrorSuccess) {
        __atomic_store_n(&_capturing, NO, __ATOMIC_RELEASE);
        CFRelease(stream);
        dispatch_release(_captureQueue);
        _captureQueue = NULL;
        return SCMakeError(SCStreamErrorFailedToStart,
            [NSString stringWithFormat:@"The display stream could not be started (%d).", result]);
    }
    _displayStream = stream;
    return nil;
}

- (void)stopCaptureSynchronously
{
    CGDisplayStreamRef stream;

    __atomic_store_n(&_capturing, NO, __ATOMIC_RELEASE);
    stream = _displayStream;
    _displayStream = NULL;
    if (stream != NULL) {
        CGDisplayStreamStop(stream);
        CFRelease(stream);
    }
    if (_captureQueue != NULL) {
        dispatch_release(_captureQueue);
        _captureQueue = NULL;
    }
}

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
