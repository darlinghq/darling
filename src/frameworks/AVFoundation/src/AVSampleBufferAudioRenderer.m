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
@interface _AVSampleBufferAudioRendererState : NSObject {
@public
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
@end
@implementation _AVSampleBufferAudioRendererState
@end

#define STATE(self_) (((_AVSampleBufferAudioRendererState *)(self_)->_internal))

@implementation AVSampleBufferAudioRenderer

- (instancetype)init
{
    if ((self = [super init]) != nil) {
        _internal = [[_AVSampleBufferAudioRendererState alloc] init];
        STATE(self)->_volume = 1.0f;
        STATE(self)->_audioTimePitchAlgorithm = [@"AVAudioTimePitchAlgorithmTimeDomain" copy];
        STATE(self)->_chunks = [[NSMutableArray alloc] init];
        pthread_mutex_init(&STATE(self)->_lock, NULL);
    }
    return self;
}

- (void)dealloc
{
    [self stopRequestingMediaData];
    [self disposeUnit];
    pthread_mutex_destroy(&STATE(self)->_lock);
    [STATE(self)->_chunks release];
    [STATE(self)->_error release];
    [STATE(self)->_audioOutputDeviceUniqueID release];
    [STATE(self)->_audioTimePitchAlgorithm release];
    [_internal release];
    [super dealloc];
}

- (AVQueuedSampleBufferRenderingStatus)status { return STATE(self)->_status; }
- (NSError *)error { return STATE(self)->_error; }

- (float)volume { return STATE(self)->_volume; }
- (void)setVolume:(float)volume { STATE(self)->_volume = volume; }
- (BOOL)isMuted { return STATE(self)->_muted; }
- (void)setMuted:(BOOL)muted { STATE(self)->_muted = muted; }

- (void)_setSynchronizer:(AVSampleBufferRenderSynchronizer *)synchronizer
{
    STATE(self)->_synchronizer = synchronizer;
}

- (NSString *)audioOutputDeviceUniqueID { return STATE(self)->_audioOutputDeviceUniqueID; }
- (void)setAudioOutputDeviceUniqueID:(NSString *)uid
{
    NSString *old = STATE(self)->_audioOutputDeviceUniqueID;
    STATE(self)->_audioOutputDeviceUniqueID = [uid copy];
    [old release];
}

- (NSString *)audioTimePitchAlgorithm { return STATE(self)->_audioTimePitchAlgorithm; }
- (void)setAudioTimePitchAlgorithm:(NSString *)algorithm
{
    NSString *old = STATE(self)->_audioTimePitchAlgorithm;
    STATE(self)->_audioTimePitchAlgorithm = [algorithm copy];
    [old release];
}

- (CMTimebaseRef)timebase { return NULL; }

- (size_t)targetBytes
{
    size_t bytesPerSecond = (size_t)(STATE(self)->_format.mSampleRate * STATE(self)->_format.mBytesPerFrame);
    return bytesPerSecond > 0 ? bytesPerSecond / 2 : 64 * 1024;
}

- (BOOL)isReadyForMoreMediaData
{
    if (STATE(self)->_status == AVQueuedSampleBufferRenderingStatusFailed)
        return NO;
    pthread_mutex_lock(&STATE(self)->_lock);
    BOOL ready = STATE(self)->_queuedBytes < [self targetBytes];
    pthread_mutex_unlock(&STATE(self)->_lock);
    return ready;
}

- (BOOL)hasSufficientMediaDataForReliablePlaybackStart
{
    pthread_mutex_lock(&STATE(self)->_lock);
    BOOL enough = STATE(self)->_queuedBytes >= [self targetBytes] / 2;
    pthread_mutex_unlock(&STATE(self)->_lock);
    return enough;
}

- (void)failWithDescription:(NSString *)description status:(OSStatus)status
{
    [self willChangeValueForKey:@"status"];
    STATE(self)->_status = AVQueuedSampleBufferRenderingStatusFailed;
    [STATE(self)->_error release];
    STATE(self)->_error = [[NSError alloc] initWithDomain:AVFoundationErrorDomain code:-11800 /* AVErrorUnknown */
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
    BOOL playing = STATE(self)->_synchronizer == nil || STATE(self)->_synchronizer.rate != 0;
    pthread_mutex_lock(&STATE(self)->_lock);
    while (playing && copied < wanted && STATE(self)->_chunks.count > 0) {
        NSData *chunk = STATE(self)->_chunks[0];
        size_t n = MIN(wanted - copied, chunk.length - STATE(self)->_chunkOffset);
        memcpy((char *)out->mData + copied, (const char *)chunk.bytes + STATE(self)->_chunkOffset, n);
        copied += n;
        STATE(self)->_chunkOffset += n;
        STATE(self)->_queuedBytes -= n;
        if (STATE(self)->_chunkOffset == chunk.length) {
            [STATE(self)->_chunks removeObjectAtIndex:0];
            STATE(self)->_chunkOffset = 0;
        }
    }
    BOOL wantMore = STATE(self)->_queuedBytes < [self targetBytes] && STATE(self)->_requestBlock != nil && !STATE(self)->_requestPending;
    if (wantMore)
        STATE(self)->_requestPending = YES;
    pthread_mutex_unlock(&STATE(self)->_lock);
    memset((char *)out->mData + copied, 0, wanted - copied);
    if (copied == 0)
        *flags |= kAudioUnitRenderAction_OutputIsSilence;

    float gain = STATE(self)->_muted ? 0.0f : STATE(self)->_volume;
    if (gain != 1.0f && copied > 0) {
        if (STATE(self)->_format.mFormatFlags & kAudioFormatFlagIsFloat && STATE(self)->_format.mBitsPerChannel == 32) {
            float *samples = out->mData;
            for (size_t i = 0; i < copied / sizeof(float); i++)
                samples[i] *= gain;
        } else if (STATE(self)->_format.mBitsPerChannel == 16) {
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
    dispatch_queue_t queue = STATE(self)->_requestQueue;
    void (^block)(void) = [STATE(self)->_requestBlock retain];
    if (queue == NULL || block == nil) {
        [block release];
        return;
    }
    [self retain];
    dispatch_async(queue, ^{
        pthread_mutex_lock(&STATE(self)->_lock);
        STATE(self)->_requestPending = NO;
        pthread_mutex_unlock(&STATE(self)->_lock);
        block();
        [block release];
        [self release];
    });
}

- (void)disposeUnit
{
    if (STATE(self)->_unit == NULL)
        return;
    if (STATE(self)->_started)
        AudioOutputUnitStop(STATE(self)->_unit);
    AudioUnitUninitialize(STATE(self)->_unit);
    AudioComponentInstanceDispose(STATE(self)->_unit);
    STATE(self)->_unit = NULL;
    STATE(self)->_started = NO;
}

- (OSStatus)openUnitWithFormat:(const AudioStreamBasicDescription *)format
{
    [self disposeUnit];
    AudioComponentDescription description = { kAudioUnitType_Output, kAudioUnitSubType_DefaultOutput,
                                              kAudioUnitManufacturer_Apple, 0, 0 };
    AudioComponent component = AudioComponentFindNext(NULL, &description);
    if (component == NULL)
        return kAudioUnitErr_InvalidElement;
    OSStatus status = AudioComponentInstanceNew(component, &STATE(self)->_unit);
    if (status != noErr)
        return status;
    AURenderCallbackStruct callback = { renderCallback, self };
    status = AudioUnitSetProperty(STATE(self)->_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, format, sizeof(*format));
    if (status == noErr) {
        status = AudioUnitSetProperty(STATE(self)->_unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0,
                                      &callback, sizeof(callback));
    }
    if (status == noErr) {
        status = AudioUnitInitialize(STATE(self)->_unit);
    }
    if (status == noErr) {
        status = AudioOutputUnitStart(STATE(self)->_unit);
    }
    if (status != noErr) {
        AudioComponentInstanceDispose(STATE(self)->_unit);
        STATE(self)->_unit = NULL;
        return status;
    }
    STATE(self)->_started = YES;
    STATE(self)->_format = *format;
    return noErr;
}

- (void)enqueueSampleBuffer:(CMSampleBufferRef)sampleBuffer
{
    if (sampleBuffer == NULL || STATE(self)->_status == AVQueuedSampleBufferRenderingStatusFailed)
        return;
    const AudioStreamBasicDescription *format =
        CMAudioFormatDescriptionGetStreamBasicDescription(CMSampleBufferGetFormatDescription(sampleBuffer));
    if (format == NULL || format->mFormatID != kAudioFormatLinearPCM
        || (format->mFormatFlags & kAudioFormatFlagIsNonInterleaved)) {
        [self failWithDescription:@"Only interleaved linear PCM can be rendered" status:kAudioFormatUnsupportedDataFormatError];
        return;
    }
    if (STATE(self)->_unit == NULL || memcmp(format, &STATE(self)->_format, sizeof(*format)) != 0) {
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
    pthread_mutex_lock(&STATE(self)->_lock);
    [STATE(self)->_chunks addObject:chunk];
    STATE(self)->_queuedBytes += length;
    pthread_mutex_unlock(&STATE(self)->_lock);
    [chunk release];
    if (STATE(self)->_status != AVQueuedSampleBufferRenderingStatusRendering) {
        [self willChangeValueForKey:@"status"];
        STATE(self)->_status = AVQueuedSampleBufferRenderingStatusRendering;
        [self didChangeValueForKey:@"status"];
    }
}

- (void)flush
{
    pthread_mutex_lock(&STATE(self)->_lock);
    [STATE(self)->_chunks removeAllObjects];
    STATE(self)->_chunkOffset = 0;
    STATE(self)->_queuedBytes = 0;
    pthread_mutex_unlock(&STATE(self)->_lock);
    // A failed renderer stays failed after a flush (macOS 26.7.3 and 10.14.4 probe: status and error kept).
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
    pthread_mutex_lock(&STATE(self)->_lock);
    STATE(self)->_requestQueue = queue;
    dispatch_retain(queue);
    STATE(self)->_requestBlock = [block copy];
    STATE(self)->_requestPending = YES;
    pthread_mutex_unlock(&STATE(self)->_lock);
    [self scheduleRequest];
}

- (void)stopRequestingMediaData
{
    pthread_mutex_lock(&STATE(self)->_lock);
    if (STATE(self)->_requestQueue != NULL) {
        dispatch_release(STATE(self)->_requestQueue);
        STATE(self)->_requestQueue = NULL;
    }
    [STATE(self)->_requestBlock release];
    STATE(self)->_requestBlock = nil;
    STATE(self)->_requestPending = NO;
    pthread_mutex_unlock(&STATE(self)->_lock);
}

@end
