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

#import <AVFoundation/AVAssetTrack.h>
#import <darling-stub-forwarding.h>

@implementation AVAssetTrack

- (instancetype)_initWithAsset:(AVAsset *)asset
                       trackID:(CMPersistentTrackID)trackID
                     mediaType:(AVMediaType)mediaType
                   naturalSize:(CGSize)naturalSize
              nominalFrameRate:(float)nominalFrameRate
                     timeRange:(CMTimeRange)timeRange
{
    if ((self = [super init])) {
        _asset = asset;
        _trackID = trackID;
        _mediaType = [mediaType copy];
        _naturalSize = naturalSize;
        _nominalFrameRate = nominalFrameRate;
        _timeRange = timeRange;
        _enabled = YES;
    }
    return self;
}

- (void)dealloc
{
    [_mediaType release];
    [super dealloc];
}

- (AVAsset *)asset { return _asset; }
- (CMPersistentTrackID)trackID { return _trackID; }
- (AVMediaType)mediaType { return _mediaType; }
- (CGSize)naturalSize { return _naturalSize; }
- (float)nominalFrameRate { return _nominalFrameRate; }
- (CMTimeRange)timeRange { return _timeRange; }
- (BOOL)isEnabled { return _enabled; }

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
