/*
 This file is part of Darling.

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

#ifndef AVFAUDIO_AVAUDIOSESSIONTYPES_H
#define AVFAUDIO_AVAUDIOSESSIONTYPES_H

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSUInteger, AVAudioSessionPortOverride) {
    AVAudioSessionPortOverrideNone = 0,
    AVAudioSessionPortOverrideSpeaker = 'spkr',
};

typedef NS_ENUM(NSUInteger, AVAudioSessionRouteChangeReason) {
    AVAudioSessionRouteChangeReasonUnknown = 0,
    AVAudioSessionRouteChangeReasonNewDeviceAvailable = 1,
    AVAudioSessionRouteChangeReasonOldDeviceUnavailable = 2,
    AVAudioSessionRouteChangeReasonCategoryChange = 3,
    AVAudioSessionRouteChangeReasonOverride = 4,
    AVAudioSessionRouteChangeReasonWakeFromSleep = 6,
    AVAudioSessionRouteChangeReasonNoSuitableRouteForCategory = 7,
    AVAudioSessionRouteChangeReasonRouteConfigurationChange = 8,
};

typedef NS_OPTIONS(NSUInteger, AVAudioSessionCategoryOptions) {
    AVAudioSessionCategoryOptionMixWithOthers = 0x1,
    AVAudioSessionCategoryOptionDuckOthers = 0x2,
    AVAudioSessionCategoryOptionAllowBluetooth = 0x4,
    AVAudioSessionCategoryOptionDefaultToSpeaker = 0x8,
    AVAudioSessionCategoryOptionInterruptSpokenAudioAndMixWithOthers = 0x11,
    AVAudioSessionCategoryOptionAllowBluetoothA2DP = 0x20,
    AVAudioSessionCategoryOptionAllowAirPlay = 0x40,
    AVAudioSessionCategoryOptionOverrideMutedMicrophoneInterruption = 0x80,
};

typedef NS_ENUM(NSUInteger, AVAudioSessionInterruptionType) {
    AVAudioSessionInterruptionTypeBegan = 1,
    AVAudioSessionInterruptionTypeEnded = 0,
};

typedef NS_OPTIONS(NSUInteger, AVAudioSessionInterruptionOptions) {
    AVAudioSessionInterruptionOptionShouldResume = 1,
};

typedef NS_ENUM(NSUInteger, AVAudioSessionInterruptionReason) {
    AVAudioSessionInterruptionReasonDefault = 0,
    AVAudioSessionInterruptionReasonAppWasSuspended = 1,
    AVAudioSessionInterruptionReasonBuiltInMicMuted = 2,
    AVAudioSessionInterruptionReasonRouteDisconnected = 4,
};

typedef NS_OPTIONS(NSUInteger, AVAudioSessionSetActiveOptions) {
    AVAudioSessionSetActiveOptionNotifyOthersOnDeactivation = 1,
};

typedef NS_ENUM(NSUInteger, AVAudioSessionSilenceSecondaryAudioHintType) {
    AVAudioSessionSilenceSecondaryAudioHintTypeBegin = 1,
    AVAudioSessionSilenceSecondaryAudioHintTypeEnd = 0,
};

typedef NS_ENUM(NSUInteger, AVAudioSessionIOType) {
    AVAudioSessionIOTypeNotSpecified = 0,
    AVAudioSessionIOTypeAggregated = 1,
};

typedef NS_ENUM(NSUInteger, AVAudioSessionRouteSharingPolicy) {
    AVAudioSessionRouteSharingPolicyDefault = 0,
    AVAudioSessionRouteSharingPolicyLongFormAudio = 1,
    AVAudioSessionRouteSharingPolicyLongForm = AVAudioSessionRouteSharingPolicyLongFormAudio,
    AVAudioSessionRouteSharingPolicyIndependent = 2,
    AVAudioSessionRouteSharingPolicyLongFormVideo = 3,
};

typedef NS_ENUM(NSInteger, AVAudioSessionPromptStyle) {
    AVAudioSessionPromptStyleNone = 'none',
    AVAudioSessionPromptStyleShort = 'shrt',
    AVAudioSessionPromptStyleNormal = 'nrml',
};

typedef NS_ENUM(NSInteger, AVAudioStereoOrientation) {
    AVAudioStereoOrientationNone = 0,
    AVAudioStereoOrientationPortrait = 1,
    AVAudioStereoOrientationPortraitUpsideDown = 2,
    AVAudioStereoOrientationLandscapeRight = 3,
    AVAudioStereoOrientationLandscapeLeft = 4,
};

#endif
