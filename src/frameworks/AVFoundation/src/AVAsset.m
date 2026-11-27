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
#import <AVFoundation/AVURLAsset.h>
#import <AVFoundation/AVError.h>
#import <darling-stub-forwarding.h>

AVKeyValueStatus AVPlaybackStatusOfKey(id object, NSString *key, NSError **outError)
{
    // The properties of these classes are all available as soon as the object exists.
    if (key && [object respondsToSelector:NSSelectorFromString(key)]) {
        if (outError)
            *outError = nil;
        return AVKeyValueStatusLoaded;
    }
    if (outError) {
        *outError = [NSError errorWithDomain:AVFoundationErrorDomain code:-11800
                                    userInfo:@{ NSLocalizedDescriptionKey: @"The operation could not be completed" }];
    }
    return AVKeyValueStatusFailed;
}

void AVPlaybackLoadKeys(NSArray *keys, void (^handler)(void))
{
    if (!handler)
        return;
    void (^copy)(void) = [handler copy];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        copy();
        [copy release];
    });
}

@implementation AVAsset

+ (instancetype)assetWithURL:(NSURL *)URL
{
    return [AVURLAsset URLAssetWithURL:URL options:nil];
}

- (instancetype)init
{
    if ((self = [super init])) {
        _asset = [[AVAssetInternal alloc] init];
        _asset->duration = kCMTimeZero;
        _asset->tracks = [[NSArray alloc] init];
    }
    return self;
}

- (void)dealloc
{
    [_asset release];
    [super dealloc];
}

- (AVAssetInternal *)_internal { return _asset; }

- (CMTime)duration { return _asset->duration; }
- (float)preferredRate { return 1.0f; }
- (float)preferredVolume { return 1.0f; }
- (CGSize)naturalSize { return _asset->naturalSize; }
- (BOOL)isPlayable { return _asset->playable; }
- (NSArray<AVAssetTrack *> *)tracks { return _asset->tracks; }

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
    if ([mediaCharacteristic isEqualToString:AVMediaCharacteristicVisual])
        return [self tracksWithMediaType:AVMediaTypeVideo];
    if ([mediaCharacteristic isEqualToString:AVMediaCharacteristicAudible])
        return [self tracksWithMediaType:AVMediaTypeAudio];
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
