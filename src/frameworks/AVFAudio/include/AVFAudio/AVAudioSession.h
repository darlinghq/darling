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

#ifndef NS_TYPED_ENUM
#define NS_TYPED_ENUM
#endif

typedef NSString * AVAudioSessionCategory NS_TYPED_ENUM;
typedef NSString * AVAudioSessionMode NS_TYPED_ENUM;
typedef NSString * AVAudioSessionPort NS_TYPED_ENUM;

/* Categories */
extern NSString *const AVAudioSessionCategoryAmbient;
extern NSString *const AVAudioSessionCategorySoloAmbient;
extern NSString *const AVAudioSessionCategoryPlayback;
extern NSString *const AVAudioSessionCategoryRecord;
extern NSString *const AVAudioSessionCategoryPlayAndRecord;
extern NSString *const AVAudioSessionCategoryMultiRoute;

/* Modes */
extern NSString *const AVAudioSessionModeDefault;
extern NSString *const AVAudioSessionModeVoiceChat;
extern NSString *const AVAudioSessionModeGameChat;
extern NSString *const AVAudioSessionModeVideoRecording;
extern NSString *const AVAudioSessionModeMeasurement;
extern NSString *const AVAudioSessionModeMoviePlayback;
extern NSString *const AVAudioSessionModeVideoChat;
extern NSString *const AVAudioSessionModeSpokenAudio;
extern NSString *const AVAudioSessionModeSpatialCapture;
extern NSString *const AVAudioSessionModeVoicePrompt;

/* Notifications */
extern NSString *const AVAudioSessionInterruptionNotification;
extern NSString *const AVAudioSessionRouteChangeNotification;
extern NSString *const AVAudioSessionMediaServicesWereLostNotification;
extern NSString *const AVAudioSessionMediaServicesWereResetNotification;
extern NSString *const AVAudioSessionSilenceSecondaryAudioHintNotification;

/* Notification userInfo Keys */
extern NSString *const AVAudioSessionInterruptionTypeKey;
extern NSString *const AVAudioSessionInterruptionOptionKey;
extern NSString *const AVAudioSessionInterruptionReasonKey;
extern NSString *const AVAudioSessionRouteChangeReasonKey;
extern NSString *const AVAudioSessionRouteChangePreviousRouteKey;
extern NSString *const AVAudioSessionSilenceSecondaryAudioHintTypeKey;

/* Port types */
extern NSString *const AVAudioSessionPortBuiltInMic;
extern NSString *const AVAudioSessionPortBuiltInSpeaker;
extern NSString *const AVAudioSessionPortBuiltInReceiver;
extern NSString *const AVAudioSessionPortHeadphones;
extern NSString *const AVAudioSessionPortBluetoothHFP;
extern NSString *const AVAudioSessionPortBluetoothA2DP;
extern NSString *const AVAudioSessionPortBluetoothLE;
extern NSString *const AVAudioSessionPortCarAudio;
extern NSString *const AVAudioSessionPortAirPlay;
extern NSString *const AVAudioSessionPortLineIn;
extern NSString *const AVAudioSessionPortLineOut;
extern NSString *const AVAudioSessionPortHDMI;
extern NSString *const AVAudioSessionPortDisplayPort;

/* Orientations */
extern NSString *const AVAudioSessionOrientationBack;
extern NSString *const AVAudioSessionOrientationFront;
extern NSString *const AVAudioSessionOrientationTop;
extern NSString *const AVAudioSessionOrientationBottom;

/* Polar patterns */
extern NSString *const AVAudioSessionPolarPatternStereo;
extern NSString *const AVAudioSessionPolarPatternCardioid;
extern NSString *const AVAudioSessionPolarPatternOmnidirectional;
extern NSString *const AVAudioSessionPolarPatternSubcardioid;

@interface AVAudioSession : NSObject

+ (AVAudioSession *)sharedInstance;

- (BOOL)setActive:(BOOL)active error:(NSError **)outError;
- (BOOL)setActive:(BOOL)active withOptions:(NSUInteger)options error:(NSError **)outError;

- (BOOL)setCategory:(NSString *)category error:(NSError **)outError;
- (NSString *)category;

- (BOOL)setMode:(NSString *)mode error:(NSError **)outError;
- (NSString *)mode;

- (double)sampleRate;
- (NSInteger)inputNumberOfChannels;
- (NSInteger)outputNumberOfChannels;

@end
