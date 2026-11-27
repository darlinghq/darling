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
#import <darling-stub-forwarding.h>

@implementation AVAssetTrack

- (instancetype)_initWithAsset:(AVAsset *)asset
                       trackID:(CMPersistentTrackID)trackID
                     mediaType:(AVMediaType)mediaType
                   naturalSize:(CGSize)naturalSize
              nominalFrameRate:(float)nominalFrameRate
                     timeRange:(CMTimeRange)timeRange
            formatDescriptions:(NSArray *)formatDescriptions
{
    if ((self = [super init])) {
        _assetTrack = [[AVAssetTrackInternal alloc] init];
        _assetTrack->asset = asset;
        _assetTrack->trackID = trackID;
        _assetTrack->mediaType = [mediaType copy];
        _assetTrack->naturalSize = naturalSize;
        _assetTrack->nominalFrameRate = nominalFrameRate;
        _assetTrack->timeRange = timeRange;
        _assetTrack->formatDescriptions = [formatDescriptions copy];
    }
    return self;
}

- (void)dealloc
{
    [_assetTrack release];
    [super dealloc];
}

- (AVAssetTrackInternal *)_internal { return _assetTrack; }
- (AVAsset *)asset { return _assetTrack->asset; }
- (CMPersistentTrackID)trackID { return _assetTrack->trackID; }
- (AVMediaType)mediaType { return _assetTrack->mediaType; }
- (CGSize)naturalSize { return _assetTrack->naturalSize; }
- (float)nominalFrameRate { return _assetTrack->nominalFrameRate; }
- (CMTimeRange)timeRange { return _assetTrack->timeRange; }
- (NSArray *)formatDescriptions { return _assetTrack->formatDescriptions ?: [NSArray array]; }
- (CMTime)minFrameDuration { return _assetTrack->nominalFrameRate > 0 ? CMTimeMake(1, (int32_t)(_assetTrack->nominalFrameRate + 0.5f)) : kCMTimeInvalid; }
- (CGAffineTransform)preferredTransform { return CGAffineTransformIdentity; }
- (BOOL)isEnabled { return YES; }
- (BOOL)isPlayable { return YES; }

- (AVKeyValueStatus)statusOfValueForKey:(NSString *)key error:(NSError **)outError
{
    return AVPlaybackStatusOfKey(self, key, outError);
}

- (void)loadValuesAsynchronouslyForKeys:(NSArray<NSString *> *)keys completionHandler:(void (^)(void))handler
{
    AVPlaybackLoadKeys(keys, handler);
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
