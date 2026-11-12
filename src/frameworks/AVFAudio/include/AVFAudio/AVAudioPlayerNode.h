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

#include <Foundation/Foundation.h>
#include <AVFAudio/AVAudioMixing.h>
#include <AVFAudio/AVAudioNode.h>

// The mixing properties (volume, pan and the 3D mixing properties) are stored on the node
// and clamped to their documented ranges; there is no engine graph that uses them yet.
@interface AVAudioPlayerNode : AVAudioNode <AVAudioMixing>
{
@private
	// Declared here because the 32-bit runtime has no ivars in the implementation block.
	float _volume;
	float _pan;
	float _rate;
	float _reverbBlend;
	float _obstruction;
	float _occlusion;
	AVAudio3DMixingRenderingAlgorithm _renderingAlgorithm;
	AVAudio3DMixingSourceMode _sourceMode;
	AVAudio3DMixingPointSourceInHeadMode _pointSourceInHeadMode;
	AVAudio3DPoint _position;
}

@end
