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

#import <AVFoundation/AVAsset.h>
#import <AVFoundation/AVURLAsset.h>
#import <AVFoundation/AVAssetTrack.h>
#import <darling-stub-forwarding.h>

@implementation AVAsset

+ (instancetype)assetWithURL:(NSURL *)URL
{
    return [AVURLAsset URLAssetWithURL:URL options:nil];
}

- (CMTime)duration { return kCMTimeZero; }
- (float)preferredRate { return 1.0f; }
- (float)preferredVolume { return 1.0f; }
- (CGSize)naturalSize { return CGSizeZero; }
- (BOOL)isPlayable { return NO; }
- (NSArray<AVAssetTrack *> *)tracks { return [NSArray array]; }

- (NSArray<AVAssetTrack *> *)tracksWithMediaType:(AVMediaType)mediaType
{
    NSMutableArray *result = [NSMutableArray array];
    for (AVAssetTrack *track in [self tracks]) {
        if ([[track mediaType] isEqualToString:mediaType])
            [result addObject:track];
    }
    return result;
}

- (NSArray<AVAssetTrack *> *)tracksWithMediaCharacteristic:(NSString *)mediaCharacteristic
{
    return [NSArray array];
}

- (AVAssetTrack *)trackWithTrackID:(CMPersistentTrackID)trackID
{
    for (AVAssetTrack *track in [self tracks]) {
        if ([track trackID] == trackID)
            return track;
    }
    return nil;
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
