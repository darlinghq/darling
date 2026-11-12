/*
 This file is part of Darling.

 Copyright (C) 2017-2026 Darling Developers

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

#import <AVFoundation/AVSampleBufferAudioRenderer.h>
#import <AVFoundation/AVError.h>

NSString *const AVSampleBufferAudioRendererFlushTimeKey = @"AVSampleBufferAudioRendererFlushTimeKey";
NSNotificationName const AVSampleBufferAudioRendererOutputConfigurationDidChangeNotification =
    @"AVSampleBufferAudioRendererOutputConfigurationDidChangeNotification";

// No audio device renders queued sample buffers yet. The renderer takes requests for data like
// Apple's, and the first buffer enqueued fails it with an error, the way a renderer that lost its
// output does, so a client moves to another audio path.
@implementation AVSampleBufferAudioRenderer {
    AVQueuedSampleBufferRenderingStatus _status;
    NSError *_error;
    NSString *_audioOutputDeviceUniqueID;
    NSString *_audioTimePitchAlgorithm;
    float _volume;
    BOOL _muted;
    dispatch_queue_t _requestQueue;
    void (^_requestBlock)(void);
}

- (instancetype)init
{
    if ((self = [super init]) != nil) {
        _volume = 1.0f;
        _audioTimePitchAlgorithm = [@"AVAudioTimePitchAlgorithmTimeDomain" copy];
    }
    return self;
}

- (void)dealloc
{
    [self stopRequestingMediaData];
    [_error release];
    [_audioOutputDeviceUniqueID release];
    [_audioTimePitchAlgorithm release];
    [super dealloc];
}

@synthesize status = _status, error = _error, volume = _volume, muted = _muted;

- (NSString *)audioOutputDeviceUniqueID { return _audioOutputDeviceUniqueID; }
- (void)setAudioOutputDeviceUniqueID:(NSString *)uid
{
    NSString *old = _audioOutputDeviceUniqueID;
    _audioOutputDeviceUniqueID = [uid copy];
    [old release];
}

- (NSString *)audioTimePitchAlgorithm { return _audioTimePitchAlgorithm; }
- (void)setAudioTimePitchAlgorithm:(NSString *)algorithm
{
    NSString *old = _audioTimePitchAlgorithm;
    _audioTimePitchAlgorithm = [algorithm copy];
    [old release];
}

- (CMTimebaseRef)timebase { return NULL; }

- (BOOL)isReadyForMoreMediaData
{
    return _status != AVQueuedSampleBufferRenderingStatusFailed;
}

- (BOOL)hasSufficientMediaDataForReliablePlaybackStart { return NO; }

- (void)enqueueSampleBuffer:(CMSampleBufferRef)sampleBuffer
{
    if (_status == AVQueuedSampleBufferRenderingStatusFailed)
        return;
    [self willChangeValueForKey:@"status"];
    _status = AVQueuedSampleBufferRenderingStatusFailed;
    _error = [[NSError alloc] initWithDomain:AVFoundationErrorDomain code:-11800 /* AVErrorUnknown */
                                    userInfo:@{ NSLocalizedDescriptionKey: @"No audio output renders sample buffers" }];
    [self didChangeValueForKey:@"status"];
}

- (void)flush
{
}

- (void)flushFromSourceTime:(CMTime)time completionHandler:(void (^)(BOOL flushSucceeded))completionHandler
{
    if (completionHandler != nil) {
        void (^handler)(BOOL) = [completionHandler copy];
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            handler(YES);
            [handler release];
        });
    }
}

- (void)requestMediaDataWhenReadyOnQueue:(dispatch_queue_t)queue usingBlock:(void (^)(void))block
{
    [self stopRequestingMediaData];
    _requestQueue = queue;
    dispatch_retain(queue);
    _requestBlock = [block copy];
    if (self.readyForMoreMediaData) {
        void (^request)(void) = [_requestBlock retain];
        dispatch_async(queue, ^{
            request();
            [request release];
        });
    }
}

- (void)stopRequestingMediaData
{
    if (_requestQueue != NULL) {
        dispatch_release(_requestQueue);
        _requestQueue = NULL;
    }
    [_requestBlock release];
    _requestBlock = nil;
}

@end
