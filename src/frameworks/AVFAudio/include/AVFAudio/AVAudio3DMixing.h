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
#include <AVFAudio/AVAudioTypes.h>

typedef NS_ENUM(NSInteger, AVAudio3DMixingRenderingAlgorithm) {
	AVAudio3DMixingRenderingAlgorithmEqualPowerPanning = 0,
	AVAudio3DMixingRenderingAlgorithmSphericalHead = 1,
	AVAudio3DMixingRenderingAlgorithmHRTF = 2,
	AVAudio3DMixingRenderingAlgorithmSoundField = 3,
	AVAudio3DMixingRenderingAlgorithmStereoPassThrough = 5,
	AVAudio3DMixingRenderingAlgorithmHRTFHQ = 6,
	AVAudio3DMixingRenderingAlgorithmAuto = 7,
};

typedef NS_ENUM(NSInteger, AVAudio3DMixingSourceMode) {
	AVAudio3DMixingSourceModeSpatializeIfMono = 0,
	AVAudio3DMixingSourceModeBypass = 1,
	AVAudio3DMixingSourceModePointSource = 2,
	AVAudio3DMixingSourceModeAmbienceBed = 3,
};

typedef NS_ENUM(NSInteger, AVAudio3DMixingPointSourceInHeadMode) {
	AVAudio3DMixingPointSourceInHeadModeMono = 0,
	AVAudio3DMixingPointSourceInHeadModeBypass = 1,
};

@protocol AVAudio3DMixing

// Playback rate multiplier, 0.5 to 2.0 (clamped). The default is 1.0.
@property (nonatomic) float rate;
@property (nonatomic) AVAudio3DMixingRenderingAlgorithm renderingAlgorithm;
@property (nonatomic) AVAudio3DMixingSourceMode sourceMode;
@property (nonatomic) AVAudio3DMixingPointSourceInHeadMode pointSourceInHeadMode;
// Amount of the signal sent to the reverb, 0.0 to 1.0 (clamped). The default is 0.0.
@property (nonatomic) float reverbBlend;
// Attenuation in decibels, -100.0 to 0.0 (clamped). The default is 0.0.
@property (nonatomic) float obstruction;
@property (nonatomic) float occlusion;
@property (nonatomic) AVAudio3DPoint position;

@end
