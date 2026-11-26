/*
 This file is part of Darling.

 Copyright (C) 2025-2026 Darling Developers

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

@class AVAudioFormat;
@class AVAudioInputNode;
@class AVAudioMixerNode;
@class AVAudioNode;
@class AVAudioOutputNode;

extern NSString *const AVAudioEngineConfigurationChangeNotification;

@interface AVAudioEngine : NSObject

@property (nonatomic, readonly) AVAudioOutputNode *outputNode;
@property (nonatomic, readonly) AVAudioInputNode *inputNode;
@property (nonatomic, readonly) AVAudioMixerNode *mainMixerNode;
@property (nonatomic, readonly, getter=isRunning) BOOL running;

- (void)attachNode:(AVAudioNode *)node;
- (void)detachNode:(AVAudioNode *)node;
- (void)connect:(AVAudioNode *)node1 to:(AVAudioNode *)node2 format:(AVAudioFormat *)format;
- (void)disconnectNodeInput:(AVAudioNode *)node;
- (void)disconnectNodeOutput:(AVAudioNode *)node;
- (BOOL)startAndReturnError:(NSError **)outError;
- (void)pause;
- (void)reset;
- (void)stop;

@end
