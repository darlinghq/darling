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

#ifndef AVPlaybackPrivate_h
#define AVPlaybackPrivate_h

// Private interfaces between the playback classes of AVFoundation.

#import <AVFoundation/AVAsset.h>
#import <AVFoundation/AVAssetInternal.h>
#import <AVFoundation/AVAssetTrack.h>
#import <AVFoundation/AVAssetTrackInternal.h>
#import <AVFoundation/AVPlayer.h>
#import <AVFoundation/AVPlayerInternal.h>
#import <AVFoundation/AVPlayerItem.h>
#import <AVFoundation/AVPlayerItemInternal.h>
#import <AVFoundation/AVPlayerItemOutput.h>
#import <AVFoundation/AVPlayerItemVideoOutput.h>
#import <AVFoundation/AVPlayerItemVideoOutputInternal.h>
#import <AVFoundation/AVPlayerLayer.h>
#import <AVFoundation/AVPlayerLayerInternal.h>

AVKeyValueStatus AVPlaybackStatusOfKey(id object, NSString *key, NSError **outError);
void AVPlaybackLoadKeys(NSArray *keys, void (^handler)(void));

// A CGImage over the pixels of a 32BGRA pixel buffer, without copying them. NULL if the buffer
// cannot be mapped. The buffer stays locked and retained until the image is released.
CGImageRef AVPlaybackCreateImageFromPixelBuffer(CVPixelBufferRef pixelBuffer);

@interface AVAsset (AVPlaybackPrivate)
- (AVAssetInternal *)_internal;
@end

@interface AVAssetTrack (AVPlaybackPrivate)
- (instancetype)_initWithAsset:(AVAsset *)asset
                       trackID:(CMPersistentTrackID)trackID
                     mediaType:(AVMediaType)mediaType
                   naturalSize:(CGSize)naturalSize
              nominalFrameRate:(float)nominalFrameRate
                     timeRange:(CMTimeRange)timeRange;
@end

@interface AVPlayerItem (AVPlaybackPrivate)
- (AVPlayerItemInternal *)_internal;
// Main thread: the item's state after a player took it.
- (void)_setStatus:(AVPlayerItemStatus)status error:(NSError *)error;
@end

@interface AVPlayerItemVideoOutput (AVPlaybackPrivate)
- (void)_setItem:(AVPlayerItem *)item;
@end

@interface AVPlayer (AVPlaybackPrivate)
- (AVPlayerInternal *)_internal;
@end

@interface AVPlayerLayer (AVPlaybackPrivate)
// Main thread.
- (void)_showImage:(CGImageRef)image;
@end

@interface AVPlayerInternal (Playback)
- (instancetype)initWithPlayer:(AVPlayer *)owner;
// Stops playback and detaches from the player. Idempotent.
- (void)invalidate;
- (void)replaceItem:(AVPlayerItem *)newItem;
- (float)rate;
- (void)setRate:(float)rate;
// Applies the player's volume and muted state to the audio output.
- (void)updateVolume;
// Applies the item's audioTimePitchAlgorithm to the audio output.
- (void)updateAudioTimePitchAlgorithm;
- (NSInteger)timeControlStatus;
- (CMTime)currentTime;
- (void)seekToTime:(CMTime)time completionHandler:(void (^)(BOOL finished))completionHandler;
- (void)registerLayer:(AVPlayerLayer *)layer;
- (void)unregisterLayer:(AVPlayerLayer *)layer;
// The newest frame as an image or pixel buffer (retained), with its serial number. NULL before the first frame.
- (CGImageRef)copyLatestImage;
- (CVPixelBufferRef)copyLatestPixelBuffer:(uint64_t *)serial;
- (id)addPeriodicObserverWithInterval:(CMTime)interval queue:(dispatch_queue_t)queue block:(void (^)(CMTime))block;
- (void)removePeriodicObserver:(id)observer;
@end

#endif
