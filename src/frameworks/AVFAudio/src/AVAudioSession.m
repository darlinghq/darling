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

#import <AVFAudio/AVAudioSession.h>
#import <objc/runtime.h>
#include <stdio.h>

/* Categories */
NSString *const AVAudioSessionCategoryAmbient = @"AVAudioSessionCategoryAmbient";
NSString *const AVAudioSessionCategorySoloAmbient = @"AVAudioSessionCategorySoloAmbient";
NSString *const AVAudioSessionCategoryPlayback = @"AVAudioSessionCategoryPlayback";
NSString *const AVAudioSessionCategoryRecord = @"AVAudioSessionCategoryRecord";
NSString *const AVAudioSessionCategoryPlayAndRecord = @"AVAudioSessionCategoryPlayAndRecord";
NSString *const AVAudioSessionCategoryMultiRoute = @"AVAudioSessionCategoryMultiRoute";

/* Modes */
NSString *const AVAudioSessionModeDefault = @"AVAudioSessionModeDefault";
NSString *const AVAudioSessionModeVoiceChat = @"AVAudioSessionModeVoiceChat";
NSString *const AVAudioSessionModeGameChat = @"AVAudioSessionModeGameChat";
NSString *const AVAudioSessionModeVideoRecording = @"AVAudioSessionModeVideoRecording";
NSString *const AVAudioSessionModeMeasurement = @"AVAudioSessionModeMeasurement";
NSString *const AVAudioSessionModeMoviePlayback = @"AVAudioSessionModeMoviePlayback";
NSString *const AVAudioSessionModeVideoChat = @"AVAudioSessionModeVideoChat";
NSString *const AVAudioSessionModeSpokenAudio = @"AVAudioSessionModeSpokenAudio";
NSString *const AVAudioSessionModeSpatialCapture = @"AVAudioSessionModeSpatialCapture";
NSString *const AVAudioSessionModeVoicePrompt = @"AVAudioSessionModeVoicePrompt";

/* Notifications */
NSString *const AVAudioSessionInterruptionNotification = @"AVAudioSessionInterruptionNotification";
NSString *const AVAudioSessionRouteChangeNotification = @"AVAudioSessionRouteChangeNotification";
NSString *const AVAudioSessionMediaServicesWereLostNotification = @"AVAudioSessionMediaServicesWereLostNotification";
NSString *const AVAudioSessionMediaServicesWereResetNotification = @"AVAudioSessionMediaServicesWereResetNotification";
NSString *const AVAudioSessionSilenceSecondaryAudioHintNotification = @"AVAudioSessionSilenceSecondaryAudioHintNotification";

/* Notification userInfo Keys */
NSString *const AVAudioSessionInterruptionTypeKey = @"AVAudioSessionInterruptionTypeKey";
NSString *const AVAudioSessionInterruptionOptionKey = @"AVAudioSessionInterruptionOptionKey";
NSString *const AVAudioSessionInterruptionReasonKey = @"AVAudioSessionInterruptionReasonKey";
NSString *const AVAudioSessionRouteChangeReasonKey = @"AVAudioSessionRouteChangeReasonKey";
NSString *const AVAudioSessionRouteChangePreviousRouteKey = @"AVAudioSessionRouteChangePreviousRouteKey";
NSString *const AVAudioSessionSilenceSecondaryAudioHintTypeKey = @"AVAudioSessionSilenceSecondaryAudioHintTypeKey";

/* Port types */
NSString *const AVAudioSessionPortBuiltInMic = @"AVAudioSessionPortBuiltInMic";
NSString *const AVAudioSessionPortBuiltInSpeaker = @"AVAudioSessionPortBuiltInSpeaker";
NSString *const AVAudioSessionPortBuiltInReceiver = @"AVAudioSessionPortBuiltInReceiver";
NSString *const AVAudioSessionPortHeadphones = @"AVAudioSessionPortHeadphones";
NSString *const AVAudioSessionPortBluetoothHFP = @"AVAudioSessionPortBluetoothHFP";
NSString *const AVAudioSessionPortBluetoothA2DP = @"AVAudioSessionPortBluetoothA2DP";
NSString *const AVAudioSessionPortBluetoothLE = @"AVAudioSessionPortBluetoothLE";
NSString *const AVAudioSessionPortCarAudio = @"AVAudioSessionPortCarAudio";
NSString *const AVAudioSessionPortAirPlay = @"AVAudioSessionPortAirPlay";
NSString *const AVAudioSessionPortLineIn = @"AVAudioSessionPortLineIn";
NSString *const AVAudioSessionPortLineOut = @"AVAudioSessionPortLineOut";
NSString *const AVAudioSessionPortHDMI = @"AVAudioSessionPortHDMI";
NSString *const AVAudioSessionPortDisplayPort = @"AVAudioSessionPortDisplayPort";

/* Orientations */
NSString *const AVAudioSessionOrientationBack = @"AVAudioSessionOrientationBack";
NSString *const AVAudioSessionOrientationFront = @"AVAudioSessionOrientationFront";
NSString *const AVAudioSessionOrientationTop = @"AVAudioSessionOrientationTop";
NSString *const AVAudioSessionOrientationBottom = @"AVAudioSessionOrientationBottom";

/* Polar patterns */
NSString *const AVAudioSessionPolarPatternStereo = @"AVAudioSessionPolarPatternStereo";
NSString *const AVAudioSessionPolarPatternCardioid = @"AVAudioSessionPolarPatternCardioid";
NSString *const AVAudioSessionPolarPatternOmnidirectional = @"AVAudioSessionPolarPatternOmnidirectional";
NSString *const AVAudioSessionPolarPatternSubcardioid = @"AVAudioSessionPolarPatternSubcardioid";

/*
 * Singleton session state.
 * Note: Access is kept simple and static here; on modern macOS AVAudioSession
 * is thread-safe. Audio playback is single-threaded in Darling's current stubs.
 */
static NSString *g_category = nil;
static NSString *g_mode = nil;
static BOOL g_active = NO;

@implementation AVAudioSession

+ (AVAudioSession *)sharedInstance
{
    static AVAudioSession *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
        g_category = [AVAudioSessionCategoryPlayback copy];
        g_mode = [AVAudioSessionModeDefault copy];
        g_active = NO;
    });
    return instance;
}

- (BOOL)setActive:(BOOL)active error:(NSError **)outError
{
    g_active = active;
    if (outError) *outError = nil;
    return YES;
}

- (BOOL)setActive:(BOOL)active withOptions:(NSUInteger)options error:(NSError **)outError
{
    (void)options;
    return [self setActive:active error:outError];
}

- (BOOL)setCategory:(NSString *)category error:(NSError **)outError
{
    if (g_category != category) {
        [g_category release];
        g_category = [category copy];
    }
    if (outError) *outError = nil;
    return YES;
}

- (NSString *)category
{
    return g_category ?: AVAudioSessionCategoryPlayback;
}

- (BOOL)setMode:(NSString *)mode error:(NSError **)outError
{
    if (g_mode != mode) {
        [g_mode release];
        g_mode = [mode copy];
    }
    if (outError) *outError = nil;
    return YES;
}

- (NSString *)mode
{
    return g_mode ?: AVAudioSessionModeDefault;
}

- (double)sampleRate
{
    return 44100.0;
}

- (NSInteger)inputNumberOfChannels
{
    return 1;
}

- (NSInteger)outputNumberOfChannels
{
    return 2;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    static bool should_log = false;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        should_log = (getenv("DARLING_LOG_STUBS") != NULL);
    });

    if (should_log) {
        fprintf(stderr, "[AVFAudio stub] %s called on %s\n",
                sel_getName([anInvocation selector]),
                class_getName([self class]));
    }
}

@end
