/*
 This file is part of Darling.

 Copyright (C) 2025 Darling Developers

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

#import <AVFAudio/AVAudioPlayerNode.h>
#import "AVFAudioStub.h"

static float clampValue(float value, float low, float high)
{
	if (value < low)
	{
		return low;
	}
	if (value > high)
	{
		return high;
	}
	return value;
}

@implementation AVAudioPlayerNode

- (instancetype)init
{
	self = [super init];
	if (self != nil)
	{
		_volume = 1.0f;
		_rate = 1.0f;
	}
	return self;
}

- (float)volume
{
	return _volume;
}

- (void)setVolume:(float)volume
{
	// Only negative values are clamped; gain above 1.0 is accepted.
	_volume = volume < 0.0f ? 0.0f : volume;
}

- (float)pan
{
	return _pan;
}

- (void)setPan:(float)pan
{
	_pan = clampValue(pan, -1.0f, 1.0f);
}

- (float)rate
{
	return _rate;
}

- (void)setRate:(float)rate
{
	_rate = clampValue(rate, 0.5f, 2.0f);
}

- (float)reverbBlend
{
	return _reverbBlend;
}

- (void)setReverbBlend:(float)reverbBlend
{
	_reverbBlend = clampValue(reverbBlend, 0.0f, 1.0f);
}

- (float)obstruction
{
	return _obstruction;
}

- (void)setObstruction:(float)obstruction
{
	_obstruction = clampValue(obstruction, -100.0f, 0.0f);
}

- (float)occlusion
{
	return _occlusion;
}

- (void)setOcclusion:(float)occlusion
{
	_occlusion = clampValue(occlusion, -100.0f, 0.0f);
}

- (AVAudio3DMixingRenderingAlgorithm)renderingAlgorithm
{
	return _renderingAlgorithm;
}

- (void)setRenderingAlgorithm:(AVAudio3DMixingRenderingAlgorithm)renderingAlgorithm
{
	_renderingAlgorithm = renderingAlgorithm;
}

- (AVAudio3DMixingSourceMode)sourceMode
{
	return _sourceMode;
}

- (void)setSourceMode:(AVAudio3DMixingSourceMode)sourceMode
{
	_sourceMode = sourceMode;
}

- (AVAudio3DMixingPointSourceInHeadMode)pointSourceInHeadMode
{
	return _pointSourceInHeadMode;
}

- (void)setPointSourceInHeadMode:(AVAudio3DMixingPointSourceInHeadMode)pointSourceInHeadMode
{
	_pointSourceInHeadMode = pointSourceInHeadMode;
}

- (AVAudio3DPoint)position
{
	return _position;
}

- (void)setPosition:(AVAudio3DPoint)position
{
	_position = position;
}

- (AVAudioMixingDestination *)destinationForMixer:(AVAudioNode *)mixer bus:(AVAudioNodeBus)bus
{
	// Nodes are not connected to each other in this implementation, so there is never a
	// destination; an unconnected node answers nil.
	return nil;
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
