/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel
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

#import <AVFoundation/AVPlayerItem.h>
#import <AVFoundation/AVURLAsset.h>
#import <AVFoundation/AVError.h>
#import <darling-stub-forwarding.h>

@implementation AVPlayerItem

+ (instancetype)playerItemWithURL:(NSURL *)URL
{
    AVAsset *asset = [AVURLAsset URLAssetWithURL:URL options:nil];
    return [self playerItemWithAsset:asset];
}

+ (instancetype)playerItemWithAsset:(AVAsset *)asset
{
    return [[[self alloc] initWithAsset:asset] autorelease];
}

- (instancetype)initWithURL:(NSURL *)URL
{
    AVAsset *asset = [AVURLAsset URLAssetWithURL:URL options:nil];
    return [self initWithAsset:asset];
}

- (instancetype)initWithAsset:(AVAsset *)asset
{
    if ((self = [super init])) {
        _asset = [asset retain];
        _tracks = [[NSMutableArray alloc] init];
        _currentTime = kCMTimeZero;

        if ([_asset isPlayable]) {
            _status = AVPlayerItemStatusReadyToPlay;
            _duration = [_asset duration];
            _presentationSize = [_asset naturalSize];
        } else {
            _status = AVPlayerItemStatusFailed;
            _error = [[NSError alloc] initWithDomain:@"AVFoundationErrorDomain"
                                                code:-11828 /* AVErrorFileFormatUnrecognized */
                                            userInfo:nil];
            _duration = kCMTimeZero;
            _presentationSize = CGSizeZero;
        }
    }
    return self;
}

- (void)dealloc
{
    [_asset release];
    [_error release];
    [_tracks release];
    [super dealloc];
}

- (AVPlayerItemStatus)status { return _status; }
- (NSError *)error { return _error; }
- (AVAsset *)asset { return _asset; }
- (CMTime)duration { return _duration; }
- (CGSize)presentationSize { return _presentationSize; }
- (NSArray *)tracks { return _tracks; }
- (CMTime)currentTime { return _currentTime; }

- (void)_setCurrentTime:(CMTime)time
{
    _currentTime = time;
}

- (void)seekToTime:(CMTime)time
{
    [self seekToTime:time completionHandler:nil];
}

- (void)seekToTime:(CMTime)time completionHandler:(void (^)(BOOL finished))completionHandler
{
    _currentTime = time;
    if (completionHandler) {
        completionHandler(YES);
    }
    [[NSNotificationCenter defaultCenter] postNotificationName:AVPlayerItemTimeJumpedNotification object:self];
}

- (void)seekToTime:(CMTime)time toleranceBefore:(CMTime)toleranceBefore toleranceAfter:(CMTime)toleranceAfter
{
    [self seekToTime:time completionHandler:nil];
}

- (void)seekToTime:(CMTime)time toleranceBefore:(CMTime)toleranceBefore toleranceAfter:(CMTime)toleranceAfter completionHandler:(void (^)(BOOL finished))completionHandler
{
    [self seekToTime:time completionHandler:completionHandler];
}

- (void)stepByCount:(NSInteger)stepCount
{
    // Step roughly 1/30s per step
    CMTime stepDuration = CMTimeMake(stepCount, 30);
    CMTime newTime = CMTimeAdd(_currentTime, stepDuration);
    if (CMTimeCompare(newTime, kCMTimeZero) < 0) {
        newTime = kCMTimeZero;
    }
    if (CMTimeCompare(newTime, _duration) > 0) {
        newTime = _duration;
    }
    [self seekToTime:newTime completionHandler:nil];
}

- (void)_didPlayToEnd
{
    _currentTime = _duration;
    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] postNotificationName:AVPlayerItemDidPlayToEndTimeNotification object:self];
    });
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

@end
