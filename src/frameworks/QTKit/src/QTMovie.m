/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel
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

#import <QTKit/QTMovie.h>
#import <QTKit/QTKit.h>
#import <QTKit/QTTimeValue.h>
#import "QTCMTime.h"
#import "QTMoviePrivate.h"
#import <AVFoundation/AVFoundation.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSBitmapImageRep.h>
#import <CoreVideo/CoreVideo.h>
#import <darling-stub-forwarding.h>
#include <stdlib.h>
#include <unistd.h>

// The error code QuickTime reports when a movie cannot be opened (couldNotResolveDataRef).
static const NSInteger QTMovieOpenErrorCode = -2000;

// How long opening a movie waits for the first frame to be decoded.
static const int64_t QTMovieFirstFrameTimeout = 3 * NSEC_PER_SEC;

static NSError *QTMovieOpenError(NSString *description)
{
    return [NSError errorWithDomain:NSOSStatusErrorDomain code:QTMovieOpenErrorCode
                           userInfo:@{ NSLocalizedDescriptionKey: description }];
}

// The state of a movie, in an object of its own because the instance layout of QTMovie is fixed.
@interface QTMovieState : NSObject {
@public
    AVPlayer *player;
    AVPlayerItem *item;
    AVPlayerItemVideoOutput *output;
    NSURL *url;
    NSString *filePath;
    NSString *temporaryPath; // a file made for a movie created from data
    float volume;
    BOOL muted;
    NSInteger loadState;
    CGImageRef lastImage; // the newest frame taken from the output
}
@end

@implementation QTMovieState

- (void)dealloc
{
    if (lastImage)
        CGImageRelease(lastImage);
    [player release];
    [item release];
    [output release];
    [url release];
    [filePath release];
    if (temporaryPath) {
        unlink([temporaryPath fileSystemRepresentation]);
        [temporaryPath release];
    }
    [super dealloc];
}

@end

static void QTReleasePixelBufferImageData(void *info, const void *data, size_t size)
{
    CVPixelBufferRef pixelBuffer = info;
    CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
    CVPixelBufferRelease(pixelBuffer);
}

// An image over the pixels of a 32BGRA pixel buffer; the buffer stays locked until the image is released.
static CGImageRef QTCreateImageFromPixelBuffer(CVPixelBufferRef pixelBuffer)
{
    if (CVPixelBufferGetPixelFormatType(pixelBuffer) != kCVPixelFormatType_32BGRA ||
        CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly) != kCVReturnSuccess)
        return NULL;
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);
    size_t height = CVPixelBufferGetHeight(pixelBuffer);
    const void *base = CVPixelBufferGetBaseAddress(pixelBuffer);
    CGDataProviderRef provider = NULL;
    if (base) {
        CVPixelBufferRetain(pixelBuffer);
        provider = CGDataProviderCreateWithData(pixelBuffer, base, bytesPerRow * height, QTReleasePixelBufferImageData);
    }
    if (!provider) {
        CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
        return NULL;
    }
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGImageRef image = CGImageCreate(CVPixelBufferGetWidth(pixelBuffer), height, 8, 32, bytesPerRow, space,
        kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipFirst, provider, NULL, false, kCGRenderingIntentDefault);
    CGColorSpaceRelease(space);
    CGDataProviderRelease(provider);
    return image;
}

#define STATE ((QTMovieState *)_movieState)

@implementation QTMovie

+ (BOOL)canInitWithFile:(NSString *)fileName
{
    if (!fileName)
        return NO;
    return [[AVURLAsset URLAssetWithURL:[NSURL fileURLWithPath:fileName] options:nil] isPlayable];
}

+ (BOOL)canInitWithURL:(NSURL *)url
{
    // Only local files can be played for now.
    return [url isFileURL] && [self canInitWithFile:[url path]];
}

+ (NSArray *)movieFileTypes:(QTIncludeFlags)types
{
    return [NSArray arrayWithObjects:@"mov", @"qt", @"mp4", @"m4v", @"m4a", @"mpg", @"mpeg", @"mp3", @"aac", @"wav", @"aif", @"aiff", nil];
}

+ (NSArray *)movieUnregisteredFileTypes:(QTIncludeFlags)types
{
    return [NSArray array];
}

+ (instancetype)movieWithFile:(NSString *)fileName error:(NSError **)errorPtr
{
    return [[[self alloc] initWithFile:fileName error:errorPtr] autorelease];
}

+ (instancetype)movieWithURL:(NSURL *)url error:(NSError **)errorPtr
{
    return [[[self alloc] initWithURL:url error:errorPtr] autorelease];
}

+ (instancetype)movieWithData:(NSData *)data error:(NSError **)errorPtr
{
    return [[[self alloc] initWithData:data error:errorPtr] autorelease];
}

+ (instancetype)movieWithAttributes:(NSDictionary *)attributes error:(NSError **)errorPtr
{
    return [[[self alloc] initWithAttributes:attributes error:errorPtr] autorelease];
}

+ (instancetype)movie
{
    return [[[self alloc] init] autorelease];
}

- (instancetype)init
{
    if ((self = [super init])) {
        _movieState = [[QTMovieState alloc] init];
        STATE->volume = 1.0f;
        STATE->loadState = QTMovieLoadStateComplete;
    }
    return self;
}

- (instancetype)initWithAttributes:(NSDictionary *)attributes error:(NSError **)errorPtr
{
    NSString *file = [attributes objectForKey:QTMovieFileNameAttribute];
    if (file)
        return [self initWithFile:file error:errorPtr];
    NSURL *url = [attributes objectForKey:QTMovieURLAttribute];
    if (url)
        return [self initWithURL:url error:errorPtr];
    NSData *data = [attributes objectForKey:QTMovieDataAttribute];
    if (data)
        return [self initWithData:data error:errorPtr];
    if (errorPtr)
        *errorPtr = QTMovieOpenError(@"The movie attributes have no file name, URL or data");
    [self release];
    return nil;
}

- (instancetype)initWithFile:(NSString *)fileName error:(NSError **)errorPtr
{
    if (!fileName) {
        if (errorPtr)
            *errorPtr = QTMovieOpenError(@"The movie file name is nil");
        [self release];
        return nil;
    }
    return [self initWithURL:[NSURL fileURLWithPath:fileName] error:errorPtr];
}

- (instancetype)initWithData:(NSData *)data error:(NSError **)errorPtr
{
    NSString *template = [NSTemporaryDirectory() stringByAppendingPathComponent:@"QTMovieData.XXXXXX"];
    char *path = data ? strdup([template fileSystemRepresentation]) : NULL;
    int fd = path ? mkstemp(path) : -1;
    BOOL written = NO;
    if (fd >= 0) {
        const uint8_t *bytes = [data bytes];
        size_t remaining = [data length];
        written = YES;
        while (remaining > 0) {
            ssize_t n = write(fd, bytes, remaining);
            if (n <= 0) {
                written = NO;
                break;
            }
            bytes += n;
            remaining -= (size_t)n;
        }
        close(fd);
    }
    NSString *temporaryPath = fd >= 0 ? [NSString stringWithUTF8String:path] : nil;
    free(path);
    if (!written) {
        if (temporaryPath)
            unlink([temporaryPath fileSystemRepresentation]);
        if (errorPtr)
            *errorPtr = QTMovieOpenError(data ? @"The movie data could not be stored" : @"The movie data is nil");
        [self release];
        return nil;
    }

    self = [self initWithFile:temporaryPath error:errorPtr];
    if (self)
        STATE->temporaryPath = [temporaryPath retain]; // removed with the movie
    else
        unlink([temporaryPath fileSystemRepresentation]);
    return self;
}

- (instancetype)initWithURL:(NSURL *)url error:(NSError **)errorPtr
{
    if ((self = [self init])) {
        AVURLAsset *asset = url ? [AVURLAsset URLAssetWithURL:url options:nil] : nil;
        if (![asset isPlayable]) {
            if (errorPtr)
                *errorPtr = QTMovieOpenError(@"The movie could not be opened");
            [self release];
            return nil;
        }

        STATE->url = [url copy];
        STATE->filePath = [[url path] copy];
        STATE->item = [[AVPlayerItem playerItemWithAsset:asset] retain];
        // The frames are always delivered as 32BGRA.
        STATE->output = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:
            @{ (NSString *)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA) }];
        [STATE->item addOutput:STATE->output];
        STATE->player = [[AVPlayer playerWithPlayerItem:STATE->item] retain];

        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_playerItemDidPlayToEnd:)
                                                     name:AVPlayerItemDidPlayToEndTimeNotification object:STATE->item];

        // A movie has its first frame once it is open.
        [self _waitForSeekToTime:kCMTimeZero];
    }
    return self;
}

- (void)dealloc
{
    // An object that failed to initialize before it made its state is released too.
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    if (_movieState)
        [STATE->player pause];
    [_movieState release];
    [super dealloc];
}

- (AVPlayer *)_player { return STATE->player; }

// Seeks and returns when the frame for the time is available (or after a timeout).
- (void)_waitForSeekToTime:(CMTime)time
{
    if (!STATE->player)
        return;
    dispatch_semaphore_t done = dispatch_semaphore_create(0);
    [STATE->player seekToTime:time completionHandler:^(BOOL finished) { dispatch_semaphore_signal(done); }];
    dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, QTMovieFirstFrameTimeout));
    dispatch_release(done);
}

- (void)_post:(NSString *)name userInfo:(NSDictionary *)userInfo
{
    [[NSNotificationCenter defaultCenter] postNotificationName:name object:self userInfo:userInfo];
}

- (void)_postRateChange:(float)rate
{
    [self _post:QTMovieRateDidChangeNotification
       userInfo:@{ QTMovieRateDidChangeNotificationParameter: [NSNumber numberWithFloat:rate] }];
}

- (void)_playerItemDidPlayToEnd:(NSNotification *)note
{
    [self _postRateChange:0.0f];
    [self _post:QTMovieDidEndNotification userInfo:nil];
}

#pragma mark - Attributes

- (BOOL)_hasTrackOfType:(AVMediaType)type
{
    return [[[STATE->item asset] tracksWithMediaType:type] count] > 0;
}

- (NSDictionary *)movieAttributes
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    if (STATE->filePath)
        dict[QTMovieFileNameAttribute] = STATE->filePath;
    if (STATE->url)
        dict[QTMovieURLAttribute] = STATE->url;
    if (STATE->item) {
        NSSize size = [self currentSize];
        dict[QTMovieDurationAttribute] = [NSValue valueWithQTTime:[self duration]];
        dict[QTMovieCurrentTimeAttribute] = [NSValue valueWithQTTime:[self currentTime]];
        dict[QTMovieNaturalSizeAttribute] = [NSValue valueWithSize:size];
        dict[QTMovieCurrentSizeAttribute] = [NSValue valueWithSize:size];
        dict[QTMovieHasVideoAttribute] = [NSNumber numberWithBool:[self _hasTrackOfType:AVMediaTypeVideo]];
        dict[QTMovieHasAudioAttribute] = [NSNumber numberWithBool:[self _hasTrackOfType:AVMediaTypeAudio]];
        dict[QTMovieHasDurationAttribute] = @YES;
    }
    dict[QTMovieRateAttribute] = [NSNumber numberWithFloat:[self rate]];
    dict[QTMovieVolumeAttribute] = [NSNumber numberWithFloat:[self volume]];
    dict[QTMovieMutedAttribute] = [NSNumber numberWithBool:[self muted]];
    dict[QTMoviePreferredRateAttribute] = @1.0f;
    dict[QTMoviePreferredVolumeAttribute] = @1.0f;
    dict[QTMovieLoadStateAttribute] = [NSNumber numberWithInteger:STATE->loadState];
    dict[QTMovieIsActiveAttribute] = @YES;
    dict[QTMovieIsLinearAttribute] = @YES;
    dict[QTMovieEditableAttribute] = @NO;
    return dict;
}

- (void)setMovieAttributes:(NSDictionary *)attributes
{
    for (NSString *key in attributes)
        [self setAttribute:attributes[key] forKey:key];
}

- (id)attributeForKey:(NSString *)attributeKey
{
    return [[self movieAttributes] objectForKey:attributeKey];
}

- (void)setAttribute:(id)value forKey:(NSString *)attributeKey
{
    if ([attributeKey isEqualToString:QTMovieCurrentTimeAttribute] && [value isKindOfClass:[NSValue class]]) {
        [self setCurrentTime:[(NSValue *)value QTTimeValue]];
    } else if ([attributeKey isEqualToString:QTMovieRateAttribute] && [value respondsToSelector:@selector(floatValue)]) {
        [self setRate:[value floatValue]];
    } else if ([attributeKey isEqualToString:QTMovieVolumeAttribute] && [value respondsToSelector:@selector(floatValue)]) {
        [self setVolume:[value floatValue]];
    } else if ([attributeKey isEqualToString:QTMovieMutedAttribute] && [value respondsToSelector:@selector(boolValue)]) {
        [self setMuted:[value boolValue]];
    }
    // Other attributes are not kept: the movie does not know them.
}

#pragma mark - Playback controls

- (void)play
{
    // Playing from the end starts again at the beginning.
    if (STATE->item && CMTimeCompare([STATE->item currentTime], [STATE->item duration]) >= 0 &&
        CMTimeGetSeconds([STATE->item duration]) > 0)
        [self gotoBeginning];
    [self setRate:1.0f];
}

- (void)stop
{
    [self setRate:0.0f];
}

- (float)rate
{
    return STATE->player ? [STATE->player rate] : 0.0f;
}

- (void)setRate:(float)rate
{
    [STATE->player setRate:rate];
    [self _postRateChange:rate];
}

- (float)volume
{
    return STATE->volume;
}

- (void)setVolume:(float)volume
{
    STATE->volume = volume;
    [STATE->player setVolume:volume];
    [self _post:QTMovieVolumeDidChangeNotification userInfo:nil];
}

- (BOOL)muted
{
    return STATE->muted;
}

- (void)setMuted:(BOOL)muted
{
    STATE->muted = muted;
    [STATE->player setMuted:muted];
    [self _post:QTMovieMutedDidChangeNotification userInfo:nil];
}

- (QTTime)duration
{
    return STATE->item ? QTTimeFromCMTime([STATE->item duration]) : QTZeroTime;
}

- (QTTime)currentTime
{
    return STATE->player ? QTTimeFromCMTime([STATE->player currentTime]) : QTZeroTime;
}

- (void)setCurrentTime:(QTTime)time
{
    [STATE->player seekToTime:CMTimeFromQTTime(time)];
    [self _post:QTMovieTimeDidChangeNotification userInfo:nil];
}

- (void)gotoBeginning
{
    [self setCurrentTime:QTZeroTime];
}

- (void)gotoEnd
{
    [self setRate:0.0f];
    [self setCurrentTime:[self duration]];
}

- (void)_step:(NSInteger)count
{
    // Stepping stops playback.
    [self setRate:0.0f];
    [STATE->item stepByCount:count];
    [self _post:QTMovieTimeDidChangeNotification userInfo:nil];
}

- (void)stepForward
{
    [self _step:1];
}

- (void)stepBackward
{
    [self _step:-1];
}

#pragma mark - Frame access and geometry

- (NSSize)currentSize
{
    CGSize size = STATE->item ? [STATE->item presentationSize] : CGSizeZero;
    return NSMakeSize(size.width, size.height);
}

- (NSImage *)currentFrameImage
{
    if (!STATE->output)
        return nil;

    // The output only hands out frames it has not given yet: keep the last one.
    CVPixelBufferRef pixelBuffer = [STATE->output copyPixelBufferForItemTime:kCMTimeZero itemTimeForDisplay:NULL];
    if (pixelBuffer) {
        CGImageRef image = QTCreateImageFromPixelBuffer(pixelBuffer);
        CVPixelBufferRelease(pixelBuffer);
        if (image) {
            if (STATE->lastImage)
                CGImageRelease(STATE->lastImage);
            STATE->lastImage = image;
        }
    }
    if (!STATE->lastImage)
        return nil;

    NSBitmapImageRep *representation = [[[NSBitmapImageRep alloc] initWithCGImage:STATE->lastImage] autorelease];
    if (!representation)
        return nil;
    NSImage *image = [[[NSImage alloc] initWithSize:NSMakeSize([representation pixelsWide], [representation pixelsHigh])] autorelease];
    [image addRepresentation:representation];
    return image;
}

- (NSImage *)posterImage
{
    return [self frameImageAtTime:QTZeroTime];
}

- (NSImage *)frameImageAtTime:(QTTime)time
{
    return [self frameImageAtTime:time withAttributes:nil error:NULL];
}

- (NSImage *)frameImageAtTime:(QTTime)time withAttributes:(NSDictionary *)attributes error:(NSError **)errorPtr
{
    // Gets the frame by moving the playback position, which is left there.
    // An indefinite time leaves the position alone: the frame is the current one.
    CMTime target = CMTimeFromQTTime(time);
    if (CMTIME_IS_NUMERIC(target))
        [self _waitForSeekToTime:target];
    NSImage *image = [self currentFrameImage];
    if (!image && errorPtr)
        *errorPtr = QTMovieOpenError(@"The movie has no frame");
    return image;
}

- (BOOL)isIdling { return YES; }
- (void)setIdling:(BOOL)idling {}
- (BOOL)autoIdles { return YES; }
- (void)setAutoIdles:(BOOL)autoIdles {}

- (NSInteger)loadState { return STATE->loadState; }
- (QTTimeRange)availableRanges { return QTMakeTimeRange(QTZeroTime, [self duration]); }
- (QTTimeRange)loadedRanges { return QTMakeTimeRange(QTZeroTime, [self duration]); }

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

@end
