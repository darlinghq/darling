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

#import <AVFoundation/AVPlayer.h>
#import <AVFoundation/AVPlayerItem.h>
#import <AVFoundation/AVPlayerLayer.h>
#import <AVFoundation/AVURLAsset.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>
#import <darling-stub-forwarding.h>
#import "AVHostPath.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#pragma mark - Frame Conversion Helpers

static inline uint8_t clamp_u8(int v)
{
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}

static void convert_yuv420_to_bgra32(const AVFrame *frame, uint8_t *dst, size_t dstStride)
{
    bool full = (frame->format == AV_PIX_FMT_YUVJ420P || frame->color_range == AVCOL_RANGE_JPEG);
    bool bt709 = (frame->colorspace == AVCOL_SPC_BT709 ||
                  (frame->colorspace == AVCOL_SPC_UNSPECIFIED && frame->height >= 720));
    int yScale = full ? 256 : 298;
    int yOffset = full ? 0 : 16;
    int rv, gu, gv, bu;
    if (bt709) {
        rv = full ? 403 : 459; gu = full ? -48 : -55; gv = full ? -120 : -136; bu = full ? 475 : 541;
    } else {
        rv = full ? 359 : 409; gu = full ? -88 : -100; gv = full ? -183 : -208; bu = full ? 454 : 516;
    }

    for (int y = 0; y < frame->height; y++) {
        uint8_t *rowDst = dst + (size_t)y * dstStride;
        const uint8_t *rowY = frame->data[0] + (size_t)y * frame->linesize[0];
        const uint8_t *rowU = frame->data[1] + (size_t)(y / 2) * frame->linesize[1];
        const uint8_t *rowV = frame->data[2] + (size_t)(y / 2) * frame->linesize[2];
        for (int x = 0; x < frame->width; x++) {
            int c = yScale * ((int)rowY[x] - yOffset);
            int d = (int)rowU[x / 2] - 128;
            int e = (int)rowV[x / 2] - 128;
            rowDst[4 * x + 0] = clamp_u8((c + bu * d + 128) >> 8);          // B
            rowDst[4 * x + 1] = clamp_u8((c + gu * d + gv * e + 128) >> 8);  // G
            rowDst[4 * x + 2] = clamp_u8((c + rv * e + 128) >> 8);          // R
            rowDst[4 * x + 3] = 255;                                          // A
        }
    }
}

static CGImageRef create_cgimage_from_avframe(AVFrame *frame)
{
    if (!frame || frame->width <= 0 || frame->height <= 0)
        return NULL;

    size_t width = (size_t)frame->width;
    size_t height = (size_t)frame->height;
    size_t rowBytes = width * 4;
    size_t totalBytes = rowBytes * height;

    uint8_t *dst = (uint8_t *)malloc(totalBytes);
    if (!dst)
        return NULL;

    if (frame->format == AV_PIX_FMT_BGRA) {
        for (size_t y = 0; y < height; y++) {
            memcpy(dst + y * rowBytes, frame->data[0] + y * frame->linesize[0], rowBytes);
        }
    } else {
        convert_yuv420_to_bgra32(frame, dst, rowBytes);
    }

    CFDataRef data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, dst, totalBytes, kCFAllocatorMalloc);
    if (!data) {
        free(dst);
        return NULL;
    }

    CGDataProviderRef provider = CGDataProviderCreateWithCFData(data);
    CFRelease(data);
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo info = kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipFirst;
    CGImageRef image = CGImageCreate(width, height, 8, 32, rowBytes, space, info, provider, NULL, false, kCGRenderingIntentDefault);
    CGColorSpaceRelease(space);
    CGDataProviderRelease(provider);
    return image;
}

#pragma mark - Periodic Time Observer

@interface _AVPlayerPeriodicObserver : NSObject {
@public
    CMTime _interval;
    dispatch_queue_t _queue;
    void (^_block)(CMTime time);
    CMTime _lastFired;
}
@end
@implementation _AVPlayerPeriodicObserver
- (void)dealloc
{
    if (_queue) dispatch_release(_queue);
    [_block release];
    [super dealloc];
}
@end

#pragma mark - Playback Engine Context

typedef struct PlayerEngine {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t thread;
    bool threadCreated;
    bool shouldExit;
    bool playing;
    float rate;
    bool seekRequested;
    int64_t seekTargetUs;

    char *filePath;
    int videoStreamIdx;
    AVFormatContext *fmtCtx;
    AVCodecContext *videoCodecCtx;

    pthread_mutex_t frameMutex;
    CGImageRef currentFrameImage;

    int64_t currentPtsUs;
    int64_t durationUs;
    double timeBase;

    __unsafe_unretained AVPlayer *player;
    __unsafe_unretained AVPlayerItem *item;
} PlayerEngine;

static void *player_engine_worker(void *arg)
{
    PlayerEngine *engine = (PlayerEngine *)arg;

    while (1) {
        pthread_mutex_lock(&engine->mutex);
        while (!engine->playing && !engine->seekRequested && !engine->shouldExit) {
            pthread_cond_wait(&engine->cond, &engine->mutex);
        }
        if (engine->shouldExit) {
            pthread_mutex_unlock(&engine->mutex);
            break;
        }

        if (engine->seekRequested) {
            engine->seekRequested = false;
            int64_t targetUs = engine->seekTargetUs;
            pthread_mutex_unlock(&engine->mutex);

            if (engine->fmtCtx) {
                av_seek_frame(engine->fmtCtx, -1, targetUs, AVSEEK_FLAG_BACKWARD);
                if (engine->videoCodecCtx) {
                    avcodec_flush_buffers(engine->videoCodecCtx);
                }
            }
            engine->currentPtsUs = targetUs;
            continue;
        }

        float currentRate = engine->rate;
        pthread_mutex_unlock(&engine->mutex);

        if (!engine->fmtCtx) {
            usleep(10000);
            continue;
        }

        AVPacket *pkt = av_packet_alloc();
        if (!pkt) {
            usleep(5000);
            continue;
        }

        int ret = av_read_frame(engine->fmtCtx, pkt);
        if (ret < 0) {
            // EOF reached
            av_packet_free(&pkt);

            // Flush remaining decoder frames
            if (engine->videoCodecCtx) {
                avcodec_send_packet(engine->videoCodecCtx, NULL);
                AVFrame *flushFrame = av_frame_alloc();
                while (avcodec_receive_frame(engine->videoCodecCtx, flushFrame) == 0) {
                    CGImageRef img = create_cgimage_from_avframe(flushFrame);
                    if (img) {
                        pthread_mutex_lock(&engine->frameMutex);
                        if (engine->currentFrameImage)
                            CGImageRelease(engine->currentFrameImage);
                        engine->currentFrameImage = img;
                        pthread_mutex_unlock(&engine->frameMutex);

                        CGImageRetain(img);
                        dispatch_async(dispatch_get_main_queue(), ^{
                            [engine->player _deliverFrameToLayers:img];
                            CGImageRelease(img);
                        });
                    }
                }
                av_frame_free(&flushFrame);
            }

            pthread_mutex_lock(&engine->mutex);
            engine->playing = false;
            pthread_mutex_unlock(&engine->mutex);

            dispatch_async(dispatch_get_main_queue(), ^{
                [engine->player _didReachEOF];
            });
            continue;
        }

        if (pkt->stream_index == engine->videoStreamIdx && engine->videoCodecCtx) {
            int64_t framePtsUs = engine->currentPtsUs;
            if (pkt->pts != AV_NOPTS_VALUE) {
                framePtsUs = (int64_t)(pkt->pts * engine->timeBase * 1000000.0);
            }

            int sendRet = avcodec_send_packet(engine->videoCodecCtx, pkt);
            if (sendRet == 0) {
                AVFrame *frame = av_frame_alloc();
                while (avcodec_receive_frame(engine->videoCodecCtx, frame) == 0) {
                    CGImageRef img = create_cgimage_from_avframe(frame);
                    if (img) {
                        pthread_mutex_lock(&engine->frameMutex);
                        if (engine->currentFrameImage)
                            CGImageRelease(engine->currentFrameImage);
                        engine->currentFrameImage = img;
                        pthread_mutex_unlock(&engine->frameMutex);

                        CGImageRetain(img);
                        dispatch_async(dispatch_get_main_queue(), ^{
                            [engine->player _deliverFrameToLayers:img];
                            CGImageRelease(img);
                        });
                    }

                    // Pacing according to timestamps
                    int64_t diffUs = framePtsUs - engine->currentPtsUs;
                    if (diffUs > 0 && diffUs < 1000000 && currentRate > 0.0f) {
                        useconds_t sleepTime = (useconds_t)(diffUs / currentRate);
                        usleep(sleepTime);
                    }
                    engine->currentPtsUs = framePtsUs;

                    CMTime newTime = CMTimeMake(framePtsUs, 1000000);
                    dispatch_async(dispatch_get_main_queue(), ^{
                        [engine->item _setCurrentTime:newTime];
                    });
                    [engine->player _fireTimeObservers:newTime];
                }
                av_frame_free(&frame);
            }
        }

        av_packet_free(&pkt);
    }
    return NULL;
}

#pragma mark - AVPlayer Implementation

@implementation AVPlayer {
    NSMutableArray *_layers;
    NSMutableArray *_timeObservers;
}

+ (instancetype)playerWithURL:(NSURL *)URL
{
    AVPlayerItem *item = [AVPlayerItem playerItemWithURL:URL];
    return [self playerWithPlayerItem:item];
}

+ (instancetype)playerWithPlayerItem:(AVPlayerItem *)item
{
    return [[[self alloc] initWithPlayerItem:item] autorelease];
}

- (instancetype)initWithURL:(NSURL *)URL
{
    AVPlayerItem *item = [AVPlayerItem playerItemWithURL:URL];
    return [self initWithPlayerItem:item];
}

- (instancetype)initWithPlayerItem:(AVPlayerItem *)item
{
    if ((self = [super init])) {
        _rate = 0.0f;
        _volume = 1.0f;
        _muted = NO;
        _actionAtItemEnd = AVPlayerActionAtItemEndPause;
        _layers = [[NSMutableArray alloc] init];
        _timeObservers = [[NSMutableArray alloc] init];

        [self replaceCurrentItemWithPlayerItem:item];
    }
    return self;
}

- (void)dealloc
{
    [self _teardownEngine];
    [_currentItem release];
    [_error release];
    [_layers release];
    [_timeObservers release];
    [super dealloc];
}

- (void)_teardownEngine
{
    PlayerEngine *engine = (PlayerEngine *)_playerEngine;
    if (engine) {
        pthread_mutex_lock(&engine->mutex);
        engine->shouldExit = true;
        engine->playing = false;
        pthread_cond_broadcast(&engine->cond);
        pthread_mutex_unlock(&engine->mutex);

        if (engine->threadCreated) {
            pthread_join(engine->thread, NULL);
        }

        pthread_mutex_lock(&engine->frameMutex);
        if (engine->currentFrameImage) {
            CGImageRelease(engine->currentFrameImage);
            engine->currentFrameImage = NULL;
        }
        pthread_mutex_unlock(&engine->frameMutex);

        if (engine->videoCodecCtx) {
            avcodec_free_context(&engine->videoCodecCtx);
        }
        if (engine->fmtCtx) {
            avformat_close_input(&engine->fmtCtx);
        }
        if (engine->filePath) {
            free(engine->filePath);
        }

        pthread_mutex_destroy(&engine->mutex);
        pthread_cond_destroy(&engine->cond);
        pthread_mutex_destroy(&engine->frameMutex);
        free(engine);
        _playerEngine = NULL;
    }
}

- (void)replaceCurrentItemWithPlayerItem:(AVPlayerItem *)item
{
    [self _teardownEngine];

    if (_currentItem != item) {
        [_currentItem release];
        _currentItem = [item retain];
    }

    if (_currentItem == nil) {
        _status = AVPlayerStatusUnknown;
        return;
    }

    if ([_currentItem status] == AVPlayerItemStatusFailed) {
        _status = AVPlayerStatusFailed;
        _error = [[_currentItem error] retain];
        return;
    }

    _status = AVPlayerStatusReadyToPlay;

    NSString *path = nil;
    if ([_currentItem.asset isKindOfClass:[AVURLAsset class]]) {
        NSURL *url = [(AVURLAsset *)_currentItem.asset URL];
        path = [url isFileURL] ? [url path] : [url absoluteString];
    }

    if (!path)
        return;

    PlayerEngine *engine = (PlayerEngine *)calloc(1, sizeof(PlayerEngine));
    pthread_mutex_init(&engine->mutex, NULL);
    pthread_cond_init(&engine->cond, NULL);
    pthread_mutex_init(&engine->frameMutex, NULL);
    engine->videoStreamIdx = -1;
    engine->player = self;
    engine->item = _currentItem;
    engine->rate = _rate;

    if (AVFormatOpenInputHost(&engine->fmtCtx, [path fileSystemRepresentation], NULL, NULL, &engine->filePath) == 0) {
        if (avformat_find_stream_info(engine->fmtCtx, NULL) >= 0) {
            for (unsigned int i = 0; i < engine->fmtCtx->nb_streams; i++) {
                if (engine->fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                    engine->videoStreamIdx = (int)i;
                    break;
                }
            }

            if (engine->videoStreamIdx >= 0) {
                AVStream *st = engine->fmtCtx->streams[engine->videoStreamIdx];
                AVCodecParameters *codecpar = st->codecpar;
                const AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
                if (codec) {
                    engine->videoCodecCtx = avcodec_alloc_context3(codec);
                    if (engine->videoCodecCtx) {
                        avcodec_parameters_to_context(engine->videoCodecCtx, codecpar);
                        avcodec_open2(engine->videoCodecCtx, codec, NULL);
                    }
                }
                engine->timeBase = av_q2d(st->time_base);
                engine->durationUs = (int64_t)(st->duration * engine->timeBase * 1000000.0);
            }

            // Decode the very first video frame to have ready for display immediately
            if (engine->videoStreamIdx >= 0 && engine->videoCodecCtx) {
                AVPacket *firstPkt = av_packet_alloc();
                AVFrame *firstFrame = av_frame_alloc();
                bool gotFirst = false;

                while (av_read_frame(engine->fmtCtx, firstPkt) >= 0) {
                    if (firstPkt->stream_index == engine->videoStreamIdx) {
                        if (avcodec_send_packet(engine->videoCodecCtx, firstPkt) == 0) {
                            if (avcodec_receive_frame(engine->videoCodecCtx, firstFrame) == 0) {
                                CGImageRef img = create_cgimage_from_avframe(firstFrame);
                                if (img) {
                                    pthread_mutex_lock(&engine->frameMutex);
                                    engine->currentFrameImage = img;
                                    pthread_mutex_unlock(&engine->frameMutex);
                                    gotFirst = true;
                                }
                            }
                        }
                    }
                    av_packet_unref(firstPkt);
                    if (gotFirst) break;
                }
                av_packet_free(&firstPkt);
                av_frame_free(&firstFrame);

                // Rewind to beginning
                av_seek_frame(engine->fmtCtx, -1, 0, AVSEEK_FLAG_BACKWARD);
                avcodec_flush_buffers(engine->videoCodecCtx);
            }
        }
    }

    _playerEngine = engine;

    if (pthread_create(&engine->thread, NULL, player_engine_worker, engine) == 0) {
        engine->threadCreated = true;
    }

    // If we have a decoded first frame, display on any existing layers
    CGImageRef initialFrame = [self _copyCurrentFrameImage];
    if (initialFrame) {
        [self _deliverFrameToLayers:initialFrame];
        CGImageRelease(initialFrame);
    }
}

- (AVPlayerStatus)status { return _status; }
- (NSError *)error { return _error; }
- (AVPlayerItem *)currentItem { return _currentItem; }
- (float)rate { return _rate; }

- (void)setRate:(float)rate
{
    _rate = rate;
    PlayerEngine *engine = (PlayerEngine *)_playerEngine;
    if (engine) {
        pthread_mutex_lock(&engine->mutex);
        engine->rate = rate;
        engine->playing = (rate > 0.0f);
        pthread_cond_signal(&engine->cond);
        pthread_mutex_unlock(&engine->mutex);
    }
}

- (void)play
{
    [self setRate:1.0f];
}

- (void)pause
{
    [self setRate:0.0f];
}

- (float)volume { return _volume; }
- (void)setVolume:(float)volume { _volume = volume; }
- (BOOL)isMuted { return _muted; }
- (void)setMuted:(BOOL)muted { _muted = muted; }
- (AVPlayerActionAtItemEnd)actionAtItemEnd { return _actionAtItemEnd; }
- (void)setActionAtItemEnd:(AVPlayerActionAtItemEnd)action { _actionAtItemEnd = action; }

- (CMTime)currentTime
{
    return _currentItem ? [_currentItem currentTime] : kCMTimeZero;
}

- (void)seekToTime:(CMTime)time
{
    [self seekToTime:time completionHandler:nil];
}

- (void)seekToTime:(CMTime)time completionHandler:(void (^)(BOOL finished))completionHandler
{
    PlayerEngine *engine = (PlayerEngine *)_playerEngine;
    if (engine) {
        int64_t targetUs = (int64_t)(CMTimeGetSeconds(time) * 1000000.0);
        pthread_mutex_lock(&engine->mutex);
        engine->seekTargetUs = targetUs;
        engine->seekRequested = true;
        pthread_cond_signal(&engine->cond);
        pthread_mutex_unlock(&engine->mutex);
    }
    [_currentItem seekToTime:time completionHandler:completionHandler];
}

- (void)seekToTime:(CMTime)time toleranceBefore:(CMTime)toleranceBefore toleranceAfter:(CMTime)toleranceAfter
{
    [self seekToTime:time completionHandler:nil];
}

- (void)seekToTime:(CMTime)time toleranceBefore:(CMTime)toleranceBefore toleranceAfter:(CMTime)toleranceAfter completionHandler:(void (^)(BOOL finished))completionHandler
{
    [self seekToTime:time completionHandler:completionHandler];
}

- (id)addPeriodicTimeObserverForInterval:(CMTime)interval queue:(dispatch_queue_t)queue usingBlock:(void (^)(CMTime time))block
{
    _AVPlayerPeriodicObserver *obs = [[_AVPlayerPeriodicObserver alloc] init];
    obs->_interval = interval;
    if (queue) {
        dispatch_retain(queue);
        obs->_queue = queue;
    } else {
        obs->_queue = NULL;
    }
    obs->_block = [block copy];
    obs->_lastFired = kCMTimeInvalid;

    @synchronized(_timeObservers) {
        [_timeObservers addObject:obs];
    }
    return [obs autorelease];
}

- (void)removeTimeObserver:(id)observer
{
    @synchronized(_timeObservers) {
        [_timeObservers removeObject:observer];
    }
}

- (void)_registerPlayerLayer:(AVPlayerLayer *)layer
{
    @synchronized(_layers) {
        if (![_layers containsObject:layer]) {
            [_layers addObject:layer];
        }
    }
}

- (void)_unregisterPlayerLayer:(AVPlayerLayer *)layer
{
    @synchronized(_layers) {
        [_layers removeObject:layer];
    }
}

- (CGImageRef)_copyCurrentFrameImage
{
    PlayerEngine *engine = (PlayerEngine *)_playerEngine;
    if (!engine) return NULL;

    pthread_mutex_lock(&engine->frameMutex);
    CGImageRef img = engine->currentFrameImage ? CGImageRetain(engine->currentFrameImage) : NULL;
    pthread_mutex_unlock(&engine->frameMutex);
    return img;
}

- (void)_deliverFrameToLayers:(CGImageRef)image
{
    NSArray *layersCopy;
    @synchronized(_layers) {
        layersCopy = [_layers copy];
    }
    for (AVPlayerLayer *layer in layersCopy) {
        [layer _displayCGImage:image];
    }
    [layersCopy release];
}

- (void)_didReachEOF
{
    _rate = 0.0f;
    [_currentItem _didPlayToEnd];
}

- (void)_fireTimeObservers:(CMTime)time
{
    NSArray *observersCopy;
    @synchronized(_timeObservers) {
        observersCopy = [_timeObservers copy];
    }
    for (_AVPlayerPeriodicObserver *obs in observersCopy) {
        if (CMTIME_IS_INVALID(obs->_lastFired) ||
            CMTimeCompare(CMTimeSubtract(time, obs->_lastFired), obs->_interval) >= 0) {
            obs->_lastFired = time;
            void (^block)(CMTime) = [obs->_block retain];
            dispatch_queue_t q = obs->_queue ?: dispatch_get_main_queue();
            dispatch_async(q, ^{
                block(time);
                [block release];
            });
        }
    }
    [observersCopy release];
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
