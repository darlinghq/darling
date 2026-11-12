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
#import <AVFoundation/AVSampleBufferRenderSynchronizer.h>
#import <AVFoundation/AVError.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioToolbox/AudioOutputUnit.h>
#include <pthread.h>
#include <string.h>

NSString *const AVSampleBufferAudioRendererFlushTimeKey = @"AVSampleBufferAudioRendererFlushTimeKey";
NSNotificationName const AVSampleBufferAudioRendererOutputConfigurationDidChangeNotification =
    @"AVSampleBufferAudioRendererOutputConfigurationDidChangeNotification";

@interface AVSampleBufferAudioRenderer (AVSampleBufferRenderSynchronizer)
- (void)_setSynchronizer:(AVSampleBufferRenderSynchronizer *)synchronizer;
@end

@interface AVSampleBufferAudioRenderer ()
- (OSStatus)renderInto:(AudioBufferList *)ioData frames:(UInt32)frames flags:(AudioUnitRenderActionFlags *)flags;
- (void)scheduleRequest;
- (void)disposeUnit;
@end

/*
 * Plays the linear PCM of the sample buffers enqueued, in order, through the default output unit.
 * It plays while its synchronizer's rate is not zero, and silence otherwise; a renderer without a
 * synchronizer plays at once. About half a second is queued before it stops asking for more data.
 * Formats other than interleaved linear PCM fail it with an error.
 */
@implementation AVSampleBufferAudioRenderer {
    AVQueuedSampleBufferRenderingStatus _status;
    NSError *_error;
    NSString *_audioOutputDeviceUniqueID;
    NSString *_audioTimePitchAlgorithm;
    float _volume;
    BOOL _muted;
    dispatch_queue_t _requestQueue;
    void (^_requestBlock)(void);
    BOOL _requestPending;

    pthread_mutex_t _lock;
    NSMutableArray<NSData *> *_chunks;
    size_t _chunkOffset;
    size_t _queuedBytes;
    AudioStreamBasicDescription _format;
    AudioUnit _unit;
    BOOL _started;
    AVSampleBufferRenderSynchronizer *_synchronizer; /* not retained: the synchronizer keeps us */
}

- (instancetype)init
{
    if ((self = [super init]) != nil) {
        _volume = 1.0f;
        _audioTimePitchAlgorithm = [@"AVAudioTimePitchAlgorithmTimeDomain" copy];
        _chunks = [[NSMutableArray alloc] init];
        pthread_mutex_init(&_lock, NULL);
    }
    return self;
}

- (void)dealloc
{
    [self stopRequestingMediaData];
    [self disposeUnit];
    pthread_mutex_destroy(&_lock);
    [_chunks release];
    [_error release];
    [_audioOutputDeviceUniqueID release];
    [_audioTimePitchAlgorithm release];
    [super dealloc];
}

@synthesize status = _status, error = _error;

- (float)volume { return _volume; }
- (void)setVolume:(float)volume { _volume = volume; }
- (BOOL)isMuted { return _muted; }
- (void)setMuted:(BOOL)muted { _muted = muted; }

- (void)_setSynchronizer:(AVSampleBufferRenderSynchronizer *)synchronizer
{
    _synchronizer = synchronizer;
}

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

- (size_t)targetBytes
{
    size_t bytesPerSecond = (size_t)(_format.mSampleRate * _format.mBytesPerFrame);
    return bytesPerSecond > 0 ? bytesPerSecond / 2 : 64 * 1024;
}

- (BOOL)isReadyForMoreMediaData
{
    if (_status == AVQueuedSampleBufferRenderingStatusFailed)
        return NO;
    pthread_mutex_lock(&_lock);
    BOOL ready = _queuedBytes < [self targetBytes];
    pthread_mutex_unlock(&_lock);
    return ready;
}

- (BOOL)hasSufficientMediaDataForReliablePlaybackStart
{
    pthread_mutex_lock(&_lock);
    BOOL enough = _queuedBytes >= [self targetBytes] / 2;
    pthread_mutex_unlock(&_lock);
    return enough;
}

- (void)failWithDescription:(NSString *)description status:(OSStatus)status
{
    NSLog(@"%@ failed: %@ (%d)", [self class], description, (int)status);
    [self willChangeValueForKey:@"status"];
    _status = AVQueuedSampleBufferRenderingStatusFailed;
    [_error release];
    _error = [[NSError alloc] initWithDomain:AVFoundationErrorDomain code:-11800 /* AVErrorUnknown */
                                    userInfo:@{ NSLocalizedDescriptionKey: description,
                                                NSUnderlyingErrorKey: [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil] }];
    [self didChangeValueForKey:@"status"];
}

static OSStatus renderCallback(void *refCon, AudioUnitRenderActionFlags *flags, const AudioTimeStamp *timestamp,
                               UInt32 bus, UInt32 frames, AudioBufferList *ioData)
{
    return [(AVSampleBufferAudioRenderer *)refCon renderInto:ioData frames:frames flags:flags];
}

- (OSStatus)renderInto:(AudioBufferList *)ioData frames:(UInt32)frames flags:(AudioUnitRenderActionFlags *)flags
{
    AudioBuffer *out = &ioData->mBuffers[0];
    size_t wanted = out->mDataByteSize, copied = 0;
    BOOL playing = _synchronizer == nil || _synchronizer.rate != 0;
    pthread_mutex_lock(&_lock);
    while (playing && copied < wanted && _chunks.count > 0) {
        NSData *chunk = _chunks[0];
        size_t n = MIN(wanted - copied, chunk.length - _chunkOffset);
        memcpy((char *)out->mData + copied, (const char *)chunk.bytes + _chunkOffset, n);
        copied += n;
        _chunkOffset += n;
        _queuedBytes -= n;
        if (_chunkOffset == chunk.length) {
            [_chunks removeObjectAtIndex:0];
            _chunkOffset = 0;
        }
    }
    BOOL wantMore = _queuedBytes < [self targetBytes] && _requestBlock != nil && !_requestPending;
    if (wantMore)
        _requestPending = YES;
    pthread_mutex_unlock(&_lock);
    memset((char *)out->mData + copied, 0, wanted - copied);
    if (copied == 0)
        *flags |= kAudioUnitRenderAction_OutputIsSilence;

    float gain = _muted ? 0.0f : _volume;
    if (gain != 1.0f && copied > 0) {
        if (_format.mFormatFlags & kAudioFormatFlagIsFloat && _format.mBitsPerChannel == 32) {
            float *samples = out->mData;
            for (size_t i = 0; i < copied / sizeof(float); i++)
                samples[i] *= gain;
        } else if (_format.mBitsPerChannel == 16) {
            int16_t *samples = out->mData;
            for (size_t i = 0; i < copied / sizeof(int16_t); i++)
                samples[i] = (int16_t)(samples[i] * gain);
        }
    }
    if (wantMore)
        [self scheduleRequest];
    return noErr;
}

- (void)scheduleRequest
{
    dispatch_queue_t queue = _requestQueue;
    void (^block)(void) = [_requestBlock retain];
    if (queue == NULL || block == nil) {
        [block release];
        return;
    }
    [self retain];
    dispatch_async(queue, ^{
        pthread_mutex_lock(&_lock);
        _requestPending = NO;
        pthread_mutex_unlock(&_lock);
        block();
        [block release];
        [self release];
    });
}

- (void)disposeUnit
{
    if (_unit == NULL)
        return;
    if (_started)
        AudioOutputUnitStop(_unit);
    AudioUnitUninitialize(_unit);
    AudioComponentInstanceDispose(_unit);
    _unit = NULL;
    _started = NO;
}

- (OSStatus)openUnitWithFormat:(const AudioStreamBasicDescription *)format
{
    [self disposeUnit];
    AudioComponentDescription description = { kAudioUnitType_Output, kAudioUnitSubType_DefaultOutput,
                                              kAudioUnitManufacturer_Apple, 0, 0 };
    AudioComponent component = AudioComponentFindNext(NULL, &description);
    if (component == NULL)
        return kAudioUnitErr_InvalidElement;
    OSStatus status = AudioComponentInstanceNew(component, &_unit);
    if (status != noErr)
        return status;
    AURenderCallbackStruct callback = { renderCallback, self };
    const char *step = "stream format";
    status = AudioUnitSetProperty(_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, format, sizeof(*format));
    if (status == noErr) {
        step = "render callback";
        status = AudioUnitSetProperty(_unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0,
                                      &callback, sizeof(callback));
    }
    if (status == noErr) {
        step = "initialize";
        status = AudioUnitInitialize(_unit);
    }
    if (status == noErr) {
        step = "start";
        status = AudioOutputUnitStart(_unit);
    }
    if (status != noErr) {
        NSLog(@"AVSampleBufferAudioRenderer: output unit %s failed (%d) for %.0f Hz, %u channels, %u bits, flags 0x%x",
              step, (int)status, format->mSampleRate, (unsigned)format->mChannelsPerFrame, (unsigned)format->mBitsPerChannel,
              (unsigned)format->mFormatFlags);
        AudioComponentInstanceDispose(_unit);
        _unit = NULL;
        return status;
    }
    _started = YES;
    _format = *format;
    return noErr;
}

- (void)enqueueSampleBuffer:(CMSampleBufferRef)sampleBuffer
{
    if (sampleBuffer == NULL || _status == AVQueuedSampleBufferRenderingStatusFailed)
        return;
    const AudioStreamBasicDescription *format =
        CMAudioFormatDescriptionGetStreamBasicDescription(CMSampleBufferGetFormatDescription(sampleBuffer));
    if (format == NULL || format->mFormatID != kAudioFormatLinearPCM
        || (format->mFormatFlags & kAudioFormatFlagIsNonInterleaved)) {
        [self failWithDescription:@"Only interleaved linear PCM can be rendered" status:kAudioFormatUnsupportedDataFormatError];
        return;
    }
    if (_unit == NULL || memcmp(format, &_format, sizeof(*format)) != 0) {
        OSStatus status = [self openUnitWithFormat:format];
        if (status != noErr) {
            [self failWithDescription:@"The audio output could not be started" status:status];
            return;
        }
    }
    CMBlockBufferRef block = CMSampleBufferGetDataBuffer(sampleBuffer);
    size_t length = CMBlockBufferGetDataLength(block);
    char *bytes = NULL;
    if (length == 0 || CMBlockBufferGetDataPointer(block, 0, NULL, NULL, &bytes) != noErr)
        return;
    NSData *chunk = [[NSData alloc] initWithBytes:bytes length:length];
    pthread_mutex_lock(&_lock);
    [_chunks addObject:chunk];
    _queuedBytes += length;
    pthread_mutex_unlock(&_lock);
    [chunk release];
    if (_status != AVQueuedSampleBufferRenderingStatusRendering) {
        [self willChangeValueForKey:@"status"];
        _status = AVQueuedSampleBufferRenderingStatusRendering;
        [self didChangeValueForKey:@"status"];
    }
}

- (void)flush
{
    pthread_mutex_lock(&_lock);
    [_chunks removeAllObjects];
    _chunkOffset = 0;
    _queuedBytes = 0;
    pthread_mutex_unlock(&_lock);
    if (_status == AVQueuedSampleBufferRenderingStatusFailed) {
        [self willChangeValueForKey:@"status"];
        _status = AVQueuedSampleBufferRenderingStatusUnknown;
        [_error release];
        _error = nil;
        [self didChangeValueForKey:@"status"];
    }
}

- (void)flushFromSourceTime:(CMTime)time completionHandler:(void (^)(BOOL flushSucceeded))completionHandler
{
    [self flush];
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
    pthread_mutex_lock(&_lock);
    _requestQueue = queue;
    dispatch_retain(queue);
    _requestBlock = [block copy];
    _requestPending = YES;
    pthread_mutex_unlock(&_lock);
    [self scheduleRequest];
}

- (void)stopRequestingMediaData
{
    pthread_mutex_lock(&_lock);
    if (_requestQueue != NULL) {
        dispatch_release(_requestQueue);
        _requestQueue = NULL;
    }
    [_requestBlock release];
    _requestBlock = nil;
    _requestPending = NO;
    pthread_mutex_unlock(&_lock);
}

@end
