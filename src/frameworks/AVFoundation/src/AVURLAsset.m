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

#import <AVFoundation/AVURLAsset.h>
#import <AVFoundation/AVAssetTrack.h>
#import <darling-stub-forwarding.h>

#import "AVHostPath.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>

@implementation AVURLAsset

+ (instancetype)URLAssetWithURL:(NSURL *)URL options:(NSDictionary<NSString *,id> *)options
{
    return [[[self alloc] initWithURL:URL options:options] autorelease];
}

- (instancetype)initWithURL:(NSURL *)URL options:(NSDictionary<NSString *,id> *)options
{
    if ((self = [super init])) {
        _URL = [URL copy];
        _options = [options copy];
        _tracks = [[NSMutableArray alloc] init];
        _duration = kCMTimeZero;
        _naturalSize = CGSizeZero;
        _preferredRate = 1.0f;
        _preferredVolume = 1.0f;
        _playable = NO;

        NSString *path = [URL isFileURL] ? [URL path] : [URL absoluteString];
        if (path) {
            AVFormatContext *fmt_ctx = NULL;
            char *hostPath = NULL;
            if (AVFormatOpenInputHost(&fmt_ctx, [path fileSystemRepresentation], NULL, NULL, &hostPath) == 0) {
                if (hostPath) {
                    free(hostPath);
                }
                if (avformat_find_stream_info(fmt_ctx, NULL) >= 0) {
                    if (fmt_ctx->duration > 0) {
                        _duration = CMTimeMake(fmt_ctx->duration, AV_TIME_BASE);
                    }
                    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
                        AVStream *st = fmt_ctx->streams[i];
                        AVCodecParameters *codecpar = st->codecpar;
                        AVMediaType mtype = nil;
                        CGSize size = CGSizeZero;
                        float fps = 0.0f;

                        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                            mtype = AVMediaTypeVideo;
                            size = CGSizeMake(codecpar->width, codecpar->height);
                            if (CGSizeEqualToSize(_naturalSize, CGSizeZero)) {
                                _naturalSize = size;
                            }
                            if (st->avg_frame_rate.den > 0) {
                                fps = (float)av_q2d(st->avg_frame_rate);
                            }
                        } else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                            mtype = AVMediaTypeAudio;
                        }

                        if (mtype != nil) {
                            CMTime streamDuration = _duration;
                            if (st->duration > 0 && st->time_base.den > 0) {
                                streamDuration = CMTimeMake((int64_t)(st->duration * av_q2d(st->time_base) * 1000000.0), 1000000);
                            }
                            CMTimeRange trange = CMTimeRangeMake(kCMTimeZero, streamDuration);
                            AVAssetTrack *track = [[AVAssetTrack alloc] _initWithAsset:self
                                                                               trackID:st->id
                                                                             mediaType:mtype
                                                                           naturalSize:size
                                                                      nominalFrameRate:fps
                                                                             timeRange:trange];
                            [_tracks addObject:track];
                            [track release];
                        }
                    }

                    if (CMTIME_IS_VALID(_duration) && _duration.value == 0) {
                        // Check if any stream has duration
                        for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
                            AVStream *st = fmt_ctx->streams[i];
                            if (st->duration > 0 && st->time_base.den > 0) {
                                int64_t dur_us = (int64_t)(st->duration * av_q2d(st->time_base) * 1000000.0);
                                if (dur_us > _duration.value) {
                                    _duration = CMTimeMake(dur_us, 1000000);
                                }
                            }
                        }
                    }

                    _playable = YES;
                }
                avformat_close_input(&fmt_ctx);
            }
        }
    }
    return self;
}

- (void)dealloc
{
    [_URL release];
    [_options release];
    [_tracks release];
    [super dealloc];
}

- (NSURL *)URL { return _URL; }
- (CMTime)duration { return _duration; }
- (CGSize)naturalSize { return _naturalSize; }
- (float)preferredRate { return _preferredRate; }
- (float)preferredVolume { return _preferredVolume; }
- (BOOL)isPlayable { return _playable; }
- (NSArray<AVAssetTrack *> *)tracks { return _tracks; }

+ (NSArray<NSString *> *)audiovisualTypes
{
    return [NSArray arrayWithObjects:
        @"public.movie",
        @"public.audio",
        @"com.apple.quicktime-movie",
        @"public.mpeg-4",
        nil];
}

+ (NSArray<NSString *> *)audiovisualMIMETypes
{
    return [NSArray arrayWithObjects:
        @"video/quicktime",
        @"video/mp4",
        @"audio/mp4",
        @"audio/mpeg",
        @"audio/wav",
        nil];
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
