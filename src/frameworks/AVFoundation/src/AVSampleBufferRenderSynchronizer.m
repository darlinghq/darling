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

#import <AVFoundation/AVSampleBufferRenderSynchronizer.h>
#include <mach/mach_time.h>

NSNotificationName const AVSampleBufferRenderSynchronizerRateDidChangeNotification =
    @"AVSampleBufferRenderSynchronizerRateDidChangeNotification";

static int64_t host_nanoseconds(void)
{
    static mach_timebase_info_data_t timebase;
    if (timebase.denom == 0)
        mach_timebase_info(&timebase);
    return (int64_t)(mach_absolute_time() * timebase.numer / timebase.denom);
}

// A time observer: a dispatch timer that reports the synchronizer's time, or calls its block as the
// time passes each boundary.
@interface _AVSampleBufferRenderSynchronizerObserver : NSObject {
@public
    dispatch_source_t _timer;
    NSArray<NSValue *> *_boundaries;
    CMTime _lastTime;
}
@end

@implementation _AVSampleBufferRenderSynchronizerObserver
- (void)dealloc
{
    if (_timer != NULL) {
        dispatch_source_cancel(_timer);
        dispatch_release(_timer);
    }
    [_boundaries release];
    [super dealloc];
}
@end

// The synchronizer keeps its own clock: a media time that advances at the rate, from the host time
// it was last anchored at.
@implementation AVSampleBufferRenderSynchronizer {
    NSLock *_lock;
    float _rate;
    CMTime _anchorTime;
    int64_t _anchorHostNanoseconds;
    BOOL _delaysRateChangeUntilHasSufficientMediaData;
    NSMutableArray *_renderers;
}

- (instancetype)init
{
    if ((self = [super init]) != nil) {
        _lock = [[NSLock alloc] init];
        _anchorTime = kCMTimeZero;
        _anchorHostNanoseconds = host_nanoseconds();
        _delaysRateChangeUntilHasSufficientMediaData = YES;
        _renderers = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)dealloc
{
    [_lock release];
    [_renderers release];
    [super dealloc];
}

- (CMTimebaseRef)timebase { return NULL; }

static CMTime current_time(CMTime anchor, int64_t anchorHost, float rate)
{
    if (!CMTIME_IS_NUMERIC(anchor))
        return anchor;
    Float64 elapsed = (host_nanoseconds() - anchorHost) / 1e9 * rate;
    int32_t timescale = anchor.timescale > 0 ? anchor.timescale : 1000000000;
    return CMTimeAdd(anchor, CMTimeMakeWithSeconds(elapsed, timescale));
}

- (CMTime)currentTime
{
    [_lock lock];
    CMTime time = current_time(_anchorTime, _anchorHostNanoseconds, _rate);
    [_lock unlock];
    return time;
}

- (float)rate
{
    [_lock lock];
    float rate = _rate;
    [_lock unlock];
    return rate;
}

- (void)setRate:(float)rate
{
    [self setRate:rate time:kCMTimeInvalid atHostTime:kCMTimeInvalid];
}

- (void)setRate:(float)rate time:(CMTime)time
{
    [self setRate:rate time:time atHostTime:kCMTimeInvalid];
}

- (void)setRate:(float)rate time:(CMTime)time atHostTime:(CMTime)hostTime
{
    int64_t now = host_nanoseconds();
    [_lock lock];
    // An invalid time keeps the current one; a host time sets when the given time is reached.
    CMTime anchor = CMTIME_IS_VALID(time) ? time : current_time(_anchorTime, _anchorHostNanoseconds, _rate);
    int64_t anchorHost = now;
    if (CMTIME_IS_NUMERIC(hostTime))
        anchorHost = (int64_t)(CMTimeGetSeconds(hostTime) * 1e9);
    BOOL changed = (_rate != rate);
    _rate = rate;
    _anchorTime = anchor;
    _anchorHostNanoseconds = anchorHost;
    [_lock unlock];

    if (changed)
        [[NSNotificationCenter defaultCenter]
            postNotificationName:AVSampleBufferRenderSynchronizerRateDidChangeNotification object:self];
}

@synthesize delaysRateChangeUntilHasSufficientMediaData = _delaysRateChangeUntilHasSufficientMediaData;

- (NSArray *)renderers
{
    [_lock lock];
    NSArray *renderers = [[_renderers copy] autorelease];
    [_lock unlock];
    return renderers;
}

- (void)addRenderer:(id<AVQueuedSampleBufferRendering>)renderer
{
    [_lock lock];
    if (![_renderers containsObject:renderer])
        [_renderers addObject:renderer];
    [_lock unlock];
}

- (void)removeRenderer:(id<AVQueuedSampleBufferRendering>)renderer atTime:(CMTime)time
     completionHandler:(void (^)(BOOL didRemoveRenderer))completionHandler
{
    [_lock lock];
    BOOL had = [_renderers containsObject:renderer];
    [_renderers removeObject:renderer];
    [_lock unlock];

    if (completionHandler != nil) {
        void (^handler)(BOOL) = [completionHandler copy];
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            handler(had);
            [handler release];
        });
    }
}

- (id)addPeriodicTimeObserverForInterval:(CMTime)interval queue:(dispatch_queue_t)queue
                              usingBlock:(void (^)(CMTime time))block
{
    Float64 seconds = CMTimeGetSeconds(interval);
    if (!(seconds > 0))
        [NSException raise:NSInvalidArgumentException format:@"time observer interval must be positive"];

    _AVSampleBufferRenderSynchronizerObserver *observer = [[_AVSampleBufferRenderSynchronizerObserver alloc] init];
    observer->_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue ?: dispatch_get_main_queue());
    uint64_t period = (uint64_t)(seconds * NSEC_PER_SEC);
    dispatch_source_set_timer(observer->_timer, dispatch_time(DISPATCH_TIME_NOW, period), period, period / 10);

    __block AVSampleBufferRenderSynchronizer *synchronizer = self;
    void (^report)(CMTime) = [[block copy] autorelease];
    dispatch_source_set_event_handler(observer->_timer, ^{
        if (synchronizer.rate != 0)
            report([synchronizer currentTime]);
    });
    dispatch_resume(observer->_timer);
    return [observer autorelease];
}

- (id)addBoundaryTimeObserverForTimes:(NSArray<NSValue *> *)times queue:(dispatch_queue_t)queue
                           usingBlock:(void (^)(void))block
{
    _AVSampleBufferRenderSynchronizerObserver *observer = [[_AVSampleBufferRenderSynchronizerObserver alloc] init];
    observer->_boundaries = [times copy];
    observer->_lastTime = [self currentTime];
    observer->_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue ?: dispatch_get_main_queue());
    uint64_t period = 10 * NSEC_PER_MSEC;
    dispatch_source_set_timer(observer->_timer, dispatch_time(DISPATCH_TIME_NOW, period), period, period / 10);

    __block AVSampleBufferRenderSynchronizer *synchronizer = self;
    __block _AVSampleBufferRenderSynchronizerObserver *watcher = observer;
    void (^crossed)(void) = [[block copy] autorelease];
    dispatch_source_set_event_handler(observer->_timer, ^{
        CMTime now = [synchronizer currentTime];
        for (NSValue *value in watcher->_boundaries) {
            CMTime boundary;
            [value getValue:&boundary];
            if (CMTimeCompare(watcher->_lastTime, boundary) < 0 && CMTimeCompare(now, boundary) >= 0)
                crossed();
        }
        watcher->_lastTime = now;
    });
    dispatch_resume(observer->_timer);
    return [observer autorelease];
}

- (void)removeTimeObserver:(id)observer
{
    if (![observer isKindOfClass:[_AVSampleBufferRenderSynchronizerObserver class]])
        return;
    _AVSampleBufferRenderSynchronizerObserver *o = observer;
    if (o->_timer != NULL)
        dispatch_source_cancel(o->_timer);
}

@end
