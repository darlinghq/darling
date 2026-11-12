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
#import "AVFAudioStub.h"

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
NSString *const AVAudioSessionPortBuiltInMic = @"MicrophoneBuiltIn";
NSString *const AVAudioSessionPortBuiltInSpeaker = @"Speaker";
NSString *const AVAudioSessionPortBuiltInReceiver = @"Receiver";
NSString *const AVAudioSessionPortHeadphones = @"Headphones";
NSString *const AVAudioSessionPortBluetoothHFP = @"BluetoothHFP";
NSString *const AVAudioSessionPortBluetoothA2DP = @"BluetoothA2DPOutput";
NSString *const AVAudioSessionPortBluetoothLE = @"BluetoothLE";
NSString *const AVAudioSessionPortCarAudio = @"CarAudio";
NSString *const AVAudioSessionPortAirPlay = @"AirPlay";
NSString *const AVAudioSessionPortLineIn = @"LineIn";
NSString *const AVAudioSessionPortLineOut = @"LineOut";
NSString *const AVAudioSessionPortHDMI = @"HDMIOutput";
NSString *const AVAudioSessionPortDisplayPort = @"DisplayPort";

/* Orientations */
NSString *const AVAudioSessionOrientationBack = @"Back";
NSString *const AVAudioSessionOrientationFront = @"Front";
NSString *const AVAudioSessionOrientationTop = @"Top";
NSString *const AVAudioSessionOrientationBottom = @"Bottom";

/* Polar patterns */
NSString *const AVAudioSessionPolarPatternStereo = @"Stereo";
NSString *const AVAudioSessionPolarPatternCardioid = @"Cardioid";
NSString *const AVAudioSessionPolarPatternOmnidirectional = @"Omnidirectional";
NSString *const AVAudioSessionPolarPatternSubcardioid = @"Subcardioid";

/*
 * Singleton session state.
 * Note: Access is kept simple and static here; on modern macOS AVAudioSession
 * is thread-safe. Audio playback is single-threaded in Darling's current stubs.
 */
static NSString *g_category = nil;
static NSString *g_mode = nil;
static BOOL g_active = NO;
static AVAudioSessionCategoryOptions g_categoryOptions = 0;
static AVAudioSessionRouteSharingPolicy g_routeSharingPolicy = AVAudioSessionRouteSharingPolicyDefault;
static BOOL g_supportsMultichannelContent = NO;

@implementation AVAudioSession

+ (AVAudioSession *)sharedInstance
{
    static AVAudioSession *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
        g_category = [AVAudioSessionCategorySoloAmbient copy];
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

- (BOOL)setActive:(BOOL)active withOptions:(AVAudioSessionSetActiveOptions)options error:(NSError **)outError
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

/* iOS's: the options (mixing with others, ducking, the speaker) change nothing here. */
- (BOOL)setCategory:(NSString *)category withOptions:(AVAudioSessionCategoryOptions)options error:(NSError **)outError
{
    g_categoryOptions = options;
    return [self setCategory:category error:outError];
}

- (BOOL)setCategory:(NSString *)category mode:(NSString *)mode options:(AVAudioSessionCategoryOptions)options error:(NSError **)outError
{
    g_categoryOptions = options;
    return [self setCategory:category error:outError] && [self setMode:mode error:outError];
}

/* The policy is recorded and reported back; there is no AirPlay to route long-form audio to. */
- (BOOL)setCategory:(NSString *)category mode:(NSString *)mode routeSharingPolicy:(AVAudioSessionRouteSharingPolicy)policy options:(AVAudioSessionCategoryOptions)options error:(NSError **)outError
{
    g_routeSharingPolicy = policy;
    return [self setCategory:category mode:mode options:options error:outError];
}

- (AVAudioSessionCategoryOptions)categoryOptions
{
    return g_categoryOptions;
}

- (AVAudioSessionRouteSharingPolicy)routeSharingPolicy
{
    return g_routeSharingPolicy;
}

- (NSString *)category
{
    return g_category ?: AVAudioSessionCategorySoloAmbient;
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

/* The built-in route is a mono microphone and a stereo speaker. Like iOS, a preference beyond
   what the route has succeeds but leaves the channel count as it is. */
- (BOOL)setPreferredInputNumberOfChannels:(NSInteger)count error:(NSError **)outError
{
    if (outError) *outError = nil;
    return YES;
}

- (NSInteger)preferredInputNumberOfChannels { return 1; }

- (BOOL)setPreferredOutputNumberOfChannels:(NSInteger)count error:(NSError **)outError
{
    if (outError) *outError = nil;
    return YES;
}

- (NSInteger)preferredOutputNumberOfChannels { return 2; }
- (NSInteger)maximumInputNumberOfChannels { return 1; }
- (NSInteger)maximumOutputNumberOfChannels { return 2; }

- (BOOL)setSupportsMultichannelContent:(BOOL)inValue error:(NSError **)outError
{
    g_supportsMultichannelContent = inValue;
    if (outError) *outError = nil;
    return YES;
}

- (BOOL)supportsMultichannelContent
{
    return g_supportsMultichannelContent;
}
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return AVAudioStubSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    // Methods that are not implemented are accepted and ignored.
}

@end
