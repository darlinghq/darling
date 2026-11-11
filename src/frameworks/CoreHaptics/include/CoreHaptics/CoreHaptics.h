/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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


#ifndef _CoreHaptics_H_
#define _CoreHaptics_H_

#import <Foundation/Foundation.h>

// Core Haptics for Darling: the classes, constants and error codes of the
// public API are present so that apps linking the framework load. No haptic or
// audio-haptic hardware is reachable, so the hardware capabilities report no
// support and creating an engine fails with CHHapticErrorCodeNotSupported.
// Patterns and their events, parameters and curves are plain value objects.

#ifndef NS_TYPED_ENUM
#define NS_TYPED_ENUM
#endif

#define CH_EXPORT __attribute__((visibility("default"))) extern

NS_ASSUME_NONNULL_BEGIN

#define CHHapticTimeImmediate ((NSTimeInterval)0.0f)

#pragma mark Errors

static NSString *const CoreHapticsErrorDomain = @"com.apple.CoreHaptics";

typedef NS_ENUM(NSInteger, CHHapticErrorCode) {
	CHHapticErrorCodeEngineNotRunning = -4805,
	CHHapticErrorCodeOperationNotPermitted = -4806,
	CHHapticErrorCodeEngineStartTimeout = -4808,
	CHHapticErrorCodeNotSupported = -4809,
	CHHapticErrorCodeServerInitFailed = -4810,
	CHHapticErrorCodeServerInterrupted = -4811,
	CHHapticErrorCodeInvalidPatternPlayer = -4812,
	CHHapticErrorCodeInvalidPatternData = -4813,
	CHHapticErrorCodeInvalidPatternDictionary = -4814,
	CHHapticErrorCodeInvalidAudioSession = -4815,
	CHHapticErrorCodeInvalidEngineParameter = -4816,
	CHHapticErrorCodeInvalidParameterType = -4820,
	CHHapticErrorCodeInvalidEventType = -4821,
	CHHapticErrorCodeInvalidEventTime = -4822,
	CHHapticErrorCodeInvalidEventDuration = -4823,
	CHHapticErrorCodeInvalidAudioResource = -4824,
	CHHapticErrorCodeResourceNotAvailable = -4825,
	CHHapticErrorCodeBadEventEntry = -4830,
	CHHapticErrorCodeBadParameterEntry = -4831,
	CHHapticErrorCodeInvalidTime = -4840,
	CHHapticErrorCodeFileNotFound = -4851,
	CHHapticErrorCodeInsufficientPower = -4897,
	CHHapticErrorCodeUnknownError = -4898,
	CHHapticErrorCodeMemoryError = -4899
};

#pragma mark Parameters

typedef NSString *CHHapticEventParameterID NS_TYPED_ENUM;
CH_EXPORT CHHapticEventParameterID CHHapticEventParameterIDHapticIntensity;
CH_EXPORT CHHapticEventParameterID CHHapticEventParameterIDHapticSharpness;
CH_EXPORT CHHapticEventParameterID CHHapticEventParameterIDAttackTime;
CH_EXPORT CHHapticEventParameterID CHHapticEventParameterIDDecayTime;
CH_EXPORT CHHapticEventParameterID CHHapticEventParameterIDReleaseTime;
CH_EXPORT CHHapticEventParameterID CHHapticEventParameterIDSustained;
CH_EXPORT CHHapticEventParameterID CHHapticEventParameterIDAudioVolume;
CH_EXPORT CHHapticEventParameterID CHHapticEventParameterIDAudioPitch;
CH_EXPORT CHHapticEventParameterID CHHapticEventParameterIDAudioPan;
CH_EXPORT CHHapticEventParameterID CHHapticEventParameterIDAudioBrightness;

typedef NSString *CHHapticDynamicParameterID NS_TYPED_ENUM;
CH_EXPORT CHHapticDynamicParameterID CHHapticDynamicParameterIDHapticIntensityControl;
CH_EXPORT CHHapticDynamicParameterID CHHapticDynamicParameterIDHapticSharpnessControl;
CH_EXPORT CHHapticDynamicParameterID CHHapticDynamicParameterIDHapticAttackTimeControl;
CH_EXPORT CHHapticDynamicParameterID CHHapticDynamicParameterIDHapticDecayTimeControl;
CH_EXPORT CHHapticDynamicParameterID CHHapticDynamicParameterIDHapticReleaseTimeControl;
CH_EXPORT CHHapticDynamicParameterID CHHapticDynamicParameterIDAudioVolumeControl;
CH_EXPORT CHHapticDynamicParameterID CHHapticDynamicParameterIDAudioPanControl;
CH_EXPORT CHHapticDynamicParameterID CHHapticDynamicParameterIDAudioBrightnessControl;
CH_EXPORT CHHapticDynamicParameterID CHHapticDynamicParameterIDAudioPitchControl;
CH_EXPORT CHHapticDynamicParameterID CHHapticDynamicParameterIDAudioAttackTimeControl;
CH_EXPORT CHHapticDynamicParameterID CHHapticDynamicParameterIDAudioDecayTimeControl;
CH_EXPORT CHHapticDynamicParameterID CHHapticDynamicParameterIDAudioReleaseTimeControl;

CH_EXPORT
@interface CHHapticEventParameter : NSObject
@property (readonly) CHHapticEventParameterID parameterID;
@property (readwrite) float value;
- (nullable instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithParameterID:(CHHapticEventParameterID)parameterID value:(float)value NS_DESIGNATED_INITIALIZER;
@end

CH_EXPORT
@interface CHHapticDynamicParameter : NSObject
@property (readonly) CHHapticDynamicParameterID parameterID;
@property (readwrite) float value;
@property (readwrite) NSTimeInterval relativeTime;
- (nullable instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithParameterID:(CHHapticDynamicParameterID)parameterID
                              value:(float)value
                       relativeTime:(NSTimeInterval)time NS_DESIGNATED_INITIALIZER;
@end

CH_EXPORT
@interface CHHapticParameterCurveControlPoint : NSObject
@property (readwrite) NSTimeInterval relativeTime;
@property (readwrite) float value;
- (nullable instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithRelativeTime:(NSTimeInterval)time value:(float)value NS_DESIGNATED_INITIALIZER;
@end

CH_EXPORT
@interface CHHapticParameterCurve : NSObject
@property (readonly) CHHapticDynamicParameterID parameterID;
@property (readwrite) NSTimeInterval relativeTime;
@property (readonly) NSArray<CHHapticParameterCurveControlPoint *> *controlPoints;
- (nullable instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithParameterID:(CHHapticDynamicParameterID)parameterID
                      controlPoints:(NSArray<CHHapticParameterCurveControlPoint *> *)controlPoints
                       relativeTime:(NSTimeInterval)relativeTime NS_DESIGNATED_INITIALIZER;
@end

#pragma mark Events

typedef NSString *CHHapticEventType NS_TYPED_ENUM;
CH_EXPORT CHHapticEventType CHHapticEventTypeHapticTransient;
CH_EXPORT CHHapticEventType CHHapticEventTypeHapticContinuous;
CH_EXPORT CHHapticEventType CHHapticEventTypeAudioContinuous;
CH_EXPORT CHHapticEventType CHHapticEventTypeAudioCustom;

typedef NSUInteger CHHapticAudioResourceID;

CH_EXPORT
@interface CHHapticEvent : NSObject
@property (readonly) CHHapticEventType type;
@property (readonly) NSArray<CHHapticEventParameter *> *eventParameters;
@property (readwrite) NSTimeInterval relativeTime;
@property (readwrite) NSTimeInterval duration;
- (nullable instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithEventType:(CHHapticEventType)type
                       parameters:(NSArray<CHHapticEventParameter *> *)eventParams
                     relativeTime:(NSTimeInterval)time;
- (instancetype)initWithEventType:(CHHapticEventType)type
                       parameters:(NSArray<CHHapticEventParameter *> *)eventParams
                     relativeTime:(NSTimeInterval)time
                         duration:(NSTimeInterval)duration;
- (instancetype)initWithAudioResourceID:(CHHapticAudioResourceID)resID
                             parameters:(NSArray<CHHapticEventParameter *> *)eventParams
                           relativeTime:(NSTimeInterval)time;
- (instancetype)initWithAudioResourceID:(CHHapticAudioResourceID)resID
                             parameters:(NSArray<CHHapticEventParameter *> *)eventParams
                           relativeTime:(NSTimeInterval)time
                               duration:(NSTimeInterval)duration;
@end

#pragma mark Patterns

typedef NSString *CHHapticPatternKey NS_TYPED_ENUM;
CH_EXPORT CHHapticPatternKey CHHapticPatternKeyVersion;
CH_EXPORT CHHapticPatternKey CHHapticPatternKeyPattern;
CH_EXPORT CHHapticPatternKey CHHapticPatternKeyEvent;
CH_EXPORT CHHapticPatternKey CHHapticPatternKeyEventType;
CH_EXPORT CHHapticPatternKey CHHapticPatternKeyTime;
CH_EXPORT CHHapticPatternKey CHHapticPatternKeyEventDuration;
CH_EXPORT CHHapticPatternKey CHHapticPatternKeyEventWaveformPath;
CH_EXPORT CHHapticPatternKey CHHapticPatternKeyEventParameters;
CH_EXPORT CHHapticPatternKey CHHapticPatternKeyEventWaveformUseVolumeEnvelope;
CH_EXPORT CHHapticPatternKey CHHapticPatternKeyEventWaveformLoopEnabled;
CH_EXPORT CHHapticPatternKey CHHapticPatternKeyParameter;
CH_EXPORT CHHapticPatternKey CHHapticPatternKeyParameterID;
CH_EXPORT CHHapticPatternKey CHHapticPatternKeyParameterValue;
CH_EXPORT CHHapticPatternKey CHHapticPatternKeyParameterCurve;
CH_EXPORT CHHapticPatternKey CHHapticPatternKeyParameterCurveControlPoints;

CH_EXPORT
@interface CHHapticPattern : NSObject
@property (readonly, nonatomic) NSTimeInterval duration;
- (nullable instancetype)init NS_UNAVAILABLE;
- (nullable instancetype)initWithEvents:(NSArray<CHHapticEvent *> *)events
                             parameters:(NSArray<CHHapticDynamicParameter *> *)parameters
                                  error:(NSError **)outError;
- (nullable instancetype)initWithEvents:(NSArray<CHHapticEvent *> *)events
                        parameterCurves:(NSArray<CHHapticParameterCurve *> *)parameterCurves
                                  error:(NSError **)outError;
// Dictionary and file based patterns are not parsed: these fail with
// CHHapticErrorCodeNotSupported.
- (nullable instancetype)initWithDictionary:(NSDictionary<CHHapticPatternKey, id> *)patternDict
                                      error:(NSError **)outError;
- (nullable instancetype)initWithContentsOfURL:(NSURL *)ahapURL error:(NSError **)outError;
- (nullable NSDictionary<CHHapticPatternKey, id> *)exportDictionaryAndReturnError:(NSError **)outError;
@end

#pragma mark Players

@protocol CHHapticPatternPlayer <NSObject>
- (BOOL)startAtTime:(NSTimeInterval)time error:(NSError **)outError;
- (BOOL)stopAtTime:(NSTimeInterval)time error:(NSError **)outError;
- (BOOL)sendParameters:(NSArray<CHHapticDynamicParameter *> *)parameters
                atTime:(NSTimeInterval)time
                 error:(NSError **)outError;
- (BOOL)scheduleParameterCurve:(CHHapticParameterCurve *)parameterCurve
                        atTime:(NSTimeInterval)time
                         error:(NSError **)outError;
- (BOOL)cancelAndReturnError:(NSError **)outError;
@property (readwrite) BOOL isMuted;
@end

typedef void (^CHHapticAdvancedPatternPlayerCompletionHandler)(NSError *_Nullable error);

@protocol CHHapticAdvancedPatternPlayer <CHHapticPatternPlayer>
- (BOOL)pauseAtTime:(NSTimeInterval)time error:(NSError **)outError;
- (BOOL)resumeAtTime:(NSTimeInterval)time error:(NSError **)outError;
- (BOOL)seekToOffset:(NSTimeInterval)offsetTime error:(NSError **)outError;
@property (readwrite) BOOL loopEnabled;
@property (readwrite) NSTimeInterval loopEnd;
@property (readwrite) float playbackRate;
@property (readwrite) BOOL isMuted;
@property (readwrite) CHHapticAdvancedPatternPlayerCompletionHandler completionHandler;
@end

#pragma mark Device capability

@protocol CHHapticParameterAttributes <NSObject>
@property (readonly) float minValue;
@property (readonly) float maxValue;
@property (readonly) float defaultValue;
@end

@protocol CHHapticDeviceCapability
@property (readonly) BOOL supportsHaptics;
@property (readonly) BOOL supportsAudio;
- (nullable id<CHHapticParameterAttributes>)attributesForEventParameter:(CHHapticEventParameterID)inParameter
                                                              eventType:(CHHapticEventType)type
                                                                  error:(NSError **)outError;
- (nullable id<CHHapticParameterAttributes>)attributesForDynamicParameter:(CHHapticDynamicParameterID)inParameter
                                                                    error:(NSError **)outError;
@end

#pragma mark Engine

@class AVAudioSession;

typedef void (^CHHapticCompletionHandler)(NSError *_Nullable error);

typedef NS_ENUM(NSInteger, CHHapticEngineFinishedAction) {
	CHHapticEngineFinishedActionStopEngine = 1,
	CHHapticEngineFinishedActionLeaveEngineRunning = 2
};

typedef CHHapticEngineFinishedAction (^CHHapticEngineFinishedHandler)(NSError *_Nullable error);

typedef NS_ENUM(NSInteger, CHHapticEngineStoppedReason) {
	CHHapticEngineStoppedReasonAudioSessionInterrupt = 1,
	CHHapticEngineStoppedReasonApplicationSuspended = 2,
	CHHapticEngineStoppedReasonIdleTimeout = 3,
	CHHapticEngineStoppedReasonNotifyWhenFinished = 4,
	CHHapticEngineStoppedReasonEngineDestroyed = 5,
	CHHapticEngineStoppedReasonGameControllerDisconnect = 6,
	CHHapticEngineStoppedReasonSystemError = -1
};

typedef void (^CHHapticEngineStoppedHandler)(CHHapticEngineStoppedReason stoppedReason);
typedef void (^CHHapticEngineResetHandler)(void);

typedef NSString *CHHapticAudioResourceKey;
CH_EXPORT CHHapticAudioResourceKey CHHapticAudioResourceKeyUseVolumeEnvelope;
CH_EXPORT CHHapticAudioResourceKey CHHapticAudioResourceKeyLoopEnabled;

CH_EXPORT
@interface CHHapticEngine : NSObject
+ (id<CHHapticDeviceCapability>)capabilitiesForHardware;
@property (readonly) NSTimeInterval currentTime;
@property (readwrite, atomic) CHHapticEngineStoppedHandler stoppedHandler;
@property (readwrite, atomic) CHHapticEngineResetHandler resetHandler;
@property (readwrite, nonatomic) BOOL playsHapticsOnly;
@property (readwrite, nonatomic) BOOL playsAudioOnly;
@property (readwrite, nonatomic) BOOL isMutedForAudio;
@property (readwrite, nonatomic) BOOL isMutedForHaptics;
@property (nonatomic, getter=isAutoShutdownEnabled) BOOL autoShutdownEnabled;
- (nullable instancetype)init NS_UNAVAILABLE;
// Always returns nil and sets CHHapticErrorCodeNotSupported.
- (nullable instancetype)initAndReturnError:(NSError **)error NS_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithAudioSession:(nullable AVAudioSession *)audioSession
                                        error:(NSError **)error NS_DESIGNATED_INITIALIZER;
- (void)startWithCompletionHandler:(CHHapticCompletionHandler _Nullable)completionHandler;
- (BOOL)startAndReturnError:(NSError **)outError;
- (void)stopWithCompletionHandler:(CHHapticCompletionHandler _Nullable)completionHandler;
- (void)notifyWhenPlayersFinished:(CHHapticEngineFinishedHandler)finishedHandler;
- (nullable id<CHHapticPatternPlayer>)createPlayerWithPattern:(CHHapticPattern *)pattern
                                                        error:(NSError **)outError;
- (nullable id<CHHapticAdvancedPatternPlayer>)createAdvancedPlayerWithPattern:(CHHapticPattern *)pattern
                                                                        error:(NSError **)outError;
- (CHHapticAudioResourceID)registerAudioResource:(NSURL *)resourceURL
                                         options:(NSDictionary *)options
                                           error:(NSError **)outError;
- (BOOL)unregisterAudioResource:(CHHapticAudioResourceID)resourceID error:(NSError **)outError;
- (BOOL)playPatternFromURL:(NSURL *)fileURL error:(NSError **)outError;
- (BOOL)playPatternFromData:(NSData *)data error:(NSError **)outError;
@end

NS_ASSUME_NONNULL_END

#endif
