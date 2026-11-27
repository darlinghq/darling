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

#import <Foundation/Foundation.h>
#import <CoreVideo/CVPixelBuffer.h>
#import <CoreGraphics/CGImage.h>
#include <pthread.h>

@class AVPlayer;
@class AVPlayerItem;
struct AVPEEngine;

// Private state of AVPlayer.
@interface AVPlayerInternal : NSObject {
@public
    // The owning player. Cleared, under ownerLock, when the player goes away; main-queue work that
    // reports to the player takes the lock too.
    __unsafe_unretained AVPlayer *player;
    pthread_mutex_t ownerLock;

    AVPlayerItem *item;
    struct AVPEEngine *engine;
    int status;
    NSError *error;
    float volume;
    BOOL muted;
    int actionAtItemEnd;
    BOOL invalidated;
    uint64_t preferredVideoDecoderGPURegistryID;
    BOOL automaticallyWaitsToMinimizeStalling;

    // Frames, seeks, layers and time observers. Guarded by lock.
    pthread_mutex_t lock;
    CVPixelBufferRef latestFrame;
    uint64_t frameSerial;
    BOOL deliveryPending;
    CGImageRef latestImage;
    uint64_t latestImageSerial;
    uint64_t lastToken;
    NSMutableDictionary *seekHandlers;
    NSMutableArray *layers;
    NSMutableArray *observers;
}

@end
