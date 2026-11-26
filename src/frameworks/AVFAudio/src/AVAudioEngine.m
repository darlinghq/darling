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

#import <AVFAudio/AVAudioEngine.h>
#import <AVFAudio/AVAudioInputNode.h>
#import <AVFAudio/AVAudioMixerNode.h>
#import <AVFAudio/AVAudioOutputNode.h>
#import "AVFAudioStub.h"

NSString *const AVAudioEngineConfigurationChangeNotification = @"AVAudioEngineConfigurationChangeNotification";

@implementation AVAudioEngine {
    AVAudioOutputNode *_outputNode;
    AVAudioInputNode *_inputNode;
    AVAudioMixerNode *_mainMixerNode;
    NSMutableSet<AVAudioNode *> *_attachedNodes;
    BOOL _running;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _attachedNodes = [[NSMutableSet alloc] init];
    }
    return self;
}

- (void)dealloc
{
    [_outputNode release];
    [_inputNode release];
    [_mainMixerNode release];
    [_attachedNodes release];
    [super dealloc];
}

- (AVAudioOutputNode *)outputNode
{
    @synchronized (self) {
        if (!_outputNode) {
            _outputNode = [[AVAudioOutputNode alloc] init];
        }
        return _outputNode;
    }
}

- (AVAudioInputNode *)inputNode
{
    @synchronized (self) {
        if (!_inputNode) {
            _inputNode = [[AVAudioInputNode alloc] init];
        }
        return _inputNode;
    }
}

- (AVAudioMixerNode *)mainMixerNode
{
    @synchronized (self) {
        if (!_mainMixerNode) {
            _mainMixerNode = [[AVAudioMixerNode alloc] init];
        }
        return _mainMixerNode;
    }
}

- (BOOL)isRunning
{
    return _running;
}

- (void)attachNode:(AVAudioNode *)node
{
    if (node) {
        [_attachedNodes addObject:node];
    }
}

- (void)detachNode:(AVAudioNode *)node
{
    if (node) {
        [_attachedNodes removeObject:node];
    }
}

- (void)connect:(AVAudioNode *)node1 to:(AVAudioNode *)node2 format:(AVAudioFormat *)format
{
}

- (void)disconnectNodeInput:(AVAudioNode *)node
{
}

- (void)disconnectNodeOutput:(AVAudioNode *)node
{
}

- (BOOL)startAndReturnError:(NSError **)outError
{
    if (outError) {
        *outError = nil;
    }
    _running = YES;
    return YES;
}

- (void)pause
{
    _running = NO;
}

- (void)reset
{
    _running = NO;
}

- (void)stop
{
    _running = NO;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return AVAudioStubSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
