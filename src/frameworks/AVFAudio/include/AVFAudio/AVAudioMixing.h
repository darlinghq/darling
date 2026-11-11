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
#include <AVFAudio/AVAudio3DMixing.h>
#include <AVFAudio/AVAudioStereoMixing.h>
#include <AVFAudio/AVAudioTypes.h>

@class AVAudioMixingDestination;
@class AVAudioNode;

@protocol AVAudioMixing <AVAudioStereoMixing, AVAudio3DMixing>

// The destination that represents this source's connection to `mixer` on `bus`, or nil
// when the source is not connected to that mixer.
- (AVAudioMixingDestination *)destinationForMixer:(AVAudioNode *)mixer bus:(AVAudioNodeBus)bus;

// Linear gain of the source. The default is 1.0; negative values are clamped to 0.0 and
// values above 1.0 are accepted.
@property (nonatomic) float volume;

@end
