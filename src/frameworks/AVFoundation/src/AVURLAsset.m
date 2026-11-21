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

#import "AVPlaybackPrivate.h"
#import "AVPlaybackEngine.h"
#import <AVFoundation/AVURLAsset.h>
#import <darling-stub-forwarding.h>

@implementation AVURLAsset

+ (instancetype)URLAssetWithURL:(NSURL *)URL options:(NSDictionary<NSString *, id> *)options
{
    return [[[self alloc] initWithURL:URL options:options] autorelease];
}

- (instancetype)initWithURL:(NSURL *)URL options:(NSDictionary<NSString *, id> *)options
{
    if ((self = [super init])) {
        AVAssetInternal *state = [self _internal];
        state->URL = [URL copy];
        state->options = [options copy];

        // Only local files can be read for now.
        AVPEMedia *media = [URL isFileURL] ? AVPEMediaOpen([[URL path] fileSystemRepresentation]) : NULL;
        if (media) {
            state->duration = CMTimeMake(AVPEMediaDurationUs(media), 1000000);
            NSMutableArray *tracks = [[NSMutableArray alloc] init];
            BOOL hasVideo = NO, hasDecodableVideo = NO, hasDecodableAudio = NO;
            for (int i = 0; i < AVPEMediaTrackCount(media); i++) {
                const AVPETrackInfo *info = AVPEMediaTrack(media, i);
                AVMediaType type = info->type == AVPETrackVideo ? AVMediaTypeVideo : AVMediaTypeAudio;
                if (info->type == AVPETrackVideo) {
                    hasVideo = YES;
                    hasDecodableVideo |= info->decodable;
                    if (CGSizeEqualToSize(state->naturalSize, CGSizeZero) && info->decodable)
                        state->naturalSize = CGSizeMake(info->width, info->height);
                } else if (info->type == AVPETrackAudio) {
                    hasDecodableAudio |= info->decodable;
                }
                AVAssetTrack *track = [[AVAssetTrack alloc] _initWithAsset:self
                                                                    trackID:(CMPersistentTrackID)info->trackID
                                                                  mediaType:type
                                                                naturalSize:CGSizeMake(info->width, info->height)
                                                           nominalFrameRate:info->frameRate
                                                                  timeRange:CMTimeRangeMake(kCMTimeZero, CMTimeMake(info->durationUs, 1000000))];
                [tracks addObject:track];
                [track release];
            }
            [state->tracks release];
            state->tracks = tracks;
            // Video needs a decoder; media without video is playable if its audio can be decoded.
            state->playable = hasVideo ? hasDecodableVideo : hasDecodableAudio;
            AVPEMediaClose(media);
        }
    }
    return self;
}

- (NSURL *)URL { return [self _internal]->URL; }

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
