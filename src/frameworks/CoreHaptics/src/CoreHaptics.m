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


#import <CoreHaptics/CoreHaptics.h>
#include <dispatch/dispatch.h>

// The classes keep their state in synthesized instance variables, which the
// legacy 32-bit Objective-C ABI does not support, and the framework exists
// only for 64-bit processes on macOS. The 32-bit slice is therefore empty.
#ifdef __LP64__

#define CH_STRING(name, value) NSString *name = @value

CH_STRING(CHHapticEventParameterIDHapticIntensity, "HapticIntensity");
CH_STRING(CHHapticEventParameterIDHapticSharpness, "HapticSharpness");
CH_STRING(CHHapticEventParameterIDAttackTime, "AttackTime");
CH_STRING(CHHapticEventParameterIDDecayTime, "DecayTime");
CH_STRING(CHHapticEventParameterIDReleaseTime, "ReleaseTime");
CH_STRING(CHHapticEventParameterIDSustained, "Sustained");
CH_STRING(CHHapticEventParameterIDAudioVolume, "AudioVolume");
CH_STRING(CHHapticEventParameterIDAudioPitch, "AudioPitch");
CH_STRING(CHHapticEventParameterIDAudioPan, "AudioPan");
CH_STRING(CHHapticEventParameterIDAudioBrightness, "AudioBrightness");

CH_STRING(CHHapticDynamicParameterIDHapticIntensityControl, "HapticIntensityControl");
CH_STRING(CHHapticDynamicParameterIDHapticSharpnessControl, "HapticSharpnessControl");
CH_STRING(CHHapticDynamicParameterIDHapticAttackTimeControl, "HapticAttackTimeControl");
CH_STRING(CHHapticDynamicParameterIDHapticDecayTimeControl, "HapticDecayTimeControl");
CH_STRING(CHHapticDynamicParameterIDHapticReleaseTimeControl, "HapticReleaseTimeControl");
CH_STRING(CHHapticDynamicParameterIDAudioVolumeControl, "AudioVolumeControl");
CH_STRING(CHHapticDynamicParameterIDAudioPanControl, "AudioPanControl");
CH_STRING(CHHapticDynamicParameterIDAudioBrightnessControl, "AudioBrightnessControl");
CH_STRING(CHHapticDynamicParameterIDAudioPitchControl, "AudioPitchControl");
CH_STRING(CHHapticDynamicParameterIDAudioAttackTimeControl, "AudioAttackTimeControl");
CH_STRING(CHHapticDynamicParameterIDAudioDecayTimeControl, "AudioDecayTimeControl");
CH_STRING(CHHapticDynamicParameterIDAudioReleaseTimeControl, "AudioReleaseTimeControl");

CH_STRING(CHHapticEventTypeHapticTransient, "HapticTransient");
CH_STRING(CHHapticEventTypeHapticContinuous, "HapticContinuous");
CH_STRING(CHHapticEventTypeAudioContinuous, "AudioContinuous");
CH_STRING(CHHapticEventTypeAudioCustom, "AudioCustom");

CH_STRING(CHHapticPatternKeyVersion, "Version");
CH_STRING(CHHapticPatternKeyPattern, "Pattern");
CH_STRING(CHHapticPatternKeyEvent, "Event");
CH_STRING(CHHapticPatternKeyEventType, "EventType");
CH_STRING(CHHapticPatternKeyTime, "Time");
CH_STRING(CHHapticPatternKeyEventDuration, "EventDuration");
CH_STRING(CHHapticPatternKeyEventWaveformPath, "EventWaveformPath");
CH_STRING(CHHapticPatternKeyEventParameters, "EventParameters");
CH_STRING(CHHapticPatternKeyEventWaveformUseVolumeEnvelope, "EventWaveformUseVolumeEnvelope");
CH_STRING(CHHapticPatternKeyEventWaveformLoopEnabled, "EventWaveformLoopEnabled");
CH_STRING(CHHapticPatternKeyParameter, "Parameter");
CH_STRING(CHHapticPatternKeyParameterID, "ParameterID");
CH_STRING(CHHapticPatternKeyParameterValue, "ParameterValue");
CH_STRING(CHHapticPatternKeyParameterCurve, "ParameterCurve");
CH_STRING(CHHapticPatternKeyParameterCurveControlPoints, "ParameterCurveControlPoints");

CH_STRING(CHHapticAudioResourceKeyUseVolumeEnvelope, "UseVolumeEnvelope");
CH_STRING(CHHapticAudioResourceKeyLoopEnabled, "LoopEnabled");

static NSError *notSupportedError(void)
{
	return [NSError errorWithDomain:CoreHapticsErrorDomain
	                           code:CHHapticErrorCodeNotSupported
	                       userInfo:@{ NSLocalizedDescriptionKey: @"Haptic feedback is not supported on this device" }];
}

static BOOL failNotSupported(NSError **error)
{
	if (error != NULL)
		*error = notSupportedError();
	return NO;
}

#pragma mark Parameters

@implementation CHHapticEventParameter

- (instancetype)initWithParameterID:(CHHapticEventParameterID)parameterID value:(float)value
{
	if ((self = [super init]) != nil) {
		_parameterID = [parameterID copy];
		_value = value;
	}
	return self;
}

- (void)dealloc
{
	[_parameterID release];
	[super dealloc];
}

@end

@implementation CHHapticDynamicParameter

- (instancetype)initWithParameterID:(CHHapticDynamicParameterID)parameterID
                              value:(float)value
                       relativeTime:(NSTimeInterval)time
{
	if ((self = [super init]) != nil) {
		_parameterID = [parameterID copy];
		_value = value;
		_relativeTime = time;
	}
	return self;
}

- (void)dealloc
{
	[_parameterID release];
	[super dealloc];
}

@end

@implementation CHHapticParameterCurveControlPoint

- (instancetype)initWithRelativeTime:(NSTimeInterval)time value:(float)value
{
	if ((self = [super init]) != nil) {
		_relativeTime = time;
		_value = value;
	}
	return self;
}

@end

@implementation CHHapticParameterCurve

- (instancetype)initWithParameterID:(CHHapticDynamicParameterID)parameterID
                      controlPoints:(NSArray<CHHapticParameterCurveControlPoint *> *)controlPoints
                       relativeTime:(NSTimeInterval)relativeTime
{
	if ((self = [super init]) != nil) {
		_parameterID = [parameterID copy];
		_controlPoints = [controlPoints copy];
		_relativeTime = relativeTime;
	}
	return self;
}

- (void)dealloc
{
	[_parameterID release];
	[_controlPoints release];
	[super dealloc];
}

@end

#pragma mark Events

@implementation CHHapticEvent

- (instancetype)initWithEventType:(CHHapticEventType)type
                       parameters:(NSArray<CHHapticEventParameter *> *)eventParams
                     relativeTime:(NSTimeInterval)time
                         duration:(NSTimeInterval)duration
{
	if ((self = [super init]) != nil) {
		_type = [type copy];
		_eventParameters = [eventParams copy];
		_relativeTime = time;
		_duration = duration;
	}
	return self;
}

- (instancetype)initWithEventType:(CHHapticEventType)type
                       parameters:(NSArray<CHHapticEventParameter *> *)eventParams
                     relativeTime:(NSTimeInterval)time
{
	return [self initWithEventType:type parameters:eventParams relativeTime:time duration:0];
}

- (instancetype)initWithAudioResourceID:(CHHapticAudioResourceID)resID
                             parameters:(NSArray<CHHapticEventParameter *> *)eventParams
                           relativeTime:(NSTimeInterval)time
                               duration:(NSTimeInterval)duration
{
	return [self initWithEventType:CHHapticEventTypeAudioCustom parameters:eventParams relativeTime:time duration:duration];
}

- (instancetype)initWithAudioResourceID:(CHHapticAudioResourceID)resID
                             parameters:(NSArray<CHHapticEventParameter *> *)eventParams
                           relativeTime:(NSTimeInterval)time
{
	return [self initWithAudioResourceID:resID parameters:eventParams relativeTime:time duration:0];
}

- (void)dealloc
{
	[_type release];
	[_eventParameters release];
	[super dealloc];
}

@end

#pragma mark Patterns

@implementation CHHapticPattern {
	NSArray *_events;
	NSArray *_parameters;
	NSArray *_curves;
}

- (instancetype)initWithPrivateEvents:(NSArray<CHHapticEvent *> *)events
                           parameters:(NSArray<CHHapticDynamicParameter *> *)parameters
                               curves:(NSArray<CHHapticParameterCurve *> *)curves
{
	if ((self = [super init]) != nil) {
		_events = [events copy];
		_parameters = [parameters copy];
		_curves = [curves copy];
	}
	return self;
}

- (instancetype)initWithEvents:(NSArray<CHHapticEvent *> *)events
                    parameters:(NSArray<CHHapticDynamicParameter *> *)parameters
                         error:(NSError **)outError
{
	return [self initWithPrivateEvents:events parameters:parameters curves:@[]];
}

- (instancetype)initWithEvents:(NSArray<CHHapticEvent *> *)events
               parameterCurves:(NSArray<CHHapticParameterCurve *> *)parameterCurves
                         error:(NSError **)outError
{
	return [self initWithPrivateEvents:events parameters:@[] curves:parameterCurves];
}

- (instancetype)initWithDictionary:(NSDictionary<CHHapticPatternKey, id> *)patternDict error:(NSError **)outError
{
	failNotSupported(outError);
	[self release];
	return nil;
}

- (instancetype)initWithContentsOfURL:(NSURL *)ahapURL error:(NSError **)outError
{
	failNotSupported(outError);
	[self release];
	return nil;
}

- (NSDictionary<CHHapticPatternKey, id> *)exportDictionaryAndReturnError:(NSError **)outError
{
	failNotSupported(outError);
	return nil;
}

// The end of the last event or parameter curve.
- (NSTimeInterval)duration
{
	NSTimeInterval end = 0;
	for (CHHapticEvent *event in _events) {
		NSTimeInterval eventEnd = event.relativeTime + event.duration;
		if (eventEnd > end)
			end = eventEnd;
	}
	for (CHHapticDynamicParameter *parameter in _parameters) {
		if (parameter.relativeTime > end)
			end = parameter.relativeTime;
	}
	for (CHHapticParameterCurve *curve in _curves) {
		for (CHHapticParameterCurveControlPoint *point in curve.controlPoints) {
			NSTimeInterval pointTime = curve.relativeTime + point.relativeTime;
			if (pointTime > end)
				end = pointTime;
		}
	}
	return end;
}

- (void)dealloc
{
	[_events release];
	[_parameters release];
	[_curves release];
	[super dealloc];
}

@end

#pragma mark Capabilities

// What +[CHHapticEngine capabilitiesForHardware] returns: no haptic and no
// audio support, and no parameter attributes.
@interface CHHapticDeviceCapabilityNone : NSObject <CHHapticDeviceCapability>
@end

@implementation CHHapticDeviceCapabilityNone

- (BOOL)supportsHaptics
{
	return NO;
}

- (BOOL)supportsAudio
{
	return NO;
}

- (id<CHHapticParameterAttributes>)attributesForEventParameter:(CHHapticEventParameterID)inParameter
                                                      eventType:(CHHapticEventType)type
                                                          error:(NSError **)outError
{
	failNotSupported(outError);
	return nil;
}

- (id<CHHapticParameterAttributes>)attributesForDynamicParameter:(CHHapticDynamicParameterID)inParameter
                                                            error:(NSError **)outError
{
	failNotSupported(outError);
	return nil;
}

@end

#pragma mark Engine

// Completion handlers run asynchronously, as they do on macOS.
static void completeNotSupportedLater(CHHapticCompletionHandler handler)
{
	if (handler == nil)
		return;
	handler = [handler copy];
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		handler(notSupportedError());
		[handler release];
	});
}

@implementation CHHapticEngine

+ (id<CHHapticDeviceCapability>)capabilitiesForHardware
{
	static id<CHHapticDeviceCapability> capability;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		capability = [[CHHapticDeviceCapabilityNone alloc] init];
	});
	return capability;
}

- (instancetype)initAndReturnError:(NSError **)error
{
	failNotSupported(error);
	[self release];
	return nil;
}

- (instancetype)initWithAudioSession:(AVAudioSession *)audioSession error:(NSError **)error
{
	failNotSupported(error);
	[self release];
	return nil;
}

- (NSTimeInterval)currentTime
{
	return 0;
}

- (void)startWithCompletionHandler:(CHHapticCompletionHandler)completionHandler
{
	completeNotSupportedLater(completionHandler);
}

- (BOOL)startAndReturnError:(NSError **)outError
{
	return failNotSupported(outError);
}

- (void)stopWithCompletionHandler:(CHHapticCompletionHandler)completionHandler
{
	completeNotSupportedLater(completionHandler);
}

- (void)notifyWhenPlayersFinished:(CHHapticEngineFinishedHandler)finishedHandler
{
}

- (id<CHHapticPatternPlayer>)createPlayerWithPattern:(CHHapticPattern *)pattern error:(NSError **)outError
{
	failNotSupported(outError);
	return nil;
}

- (id<CHHapticAdvancedPatternPlayer>)createAdvancedPlayerWithPattern:(CHHapticPattern *)pattern
                                                               error:(NSError **)outError
{
	failNotSupported(outError);
	return nil;
}

- (CHHapticAudioResourceID)registerAudioResource:(NSURL *)resourceURL
                                         options:(NSDictionary *)options
                                           error:(NSError **)outError
{
	failNotSupported(outError);
	return 0;
}

- (BOOL)unregisterAudioResource:(CHHapticAudioResourceID)resourceID error:(NSError **)outError
{
	return failNotSupported(outError);
}

- (BOOL)playPatternFromURL:(NSURL *)fileURL error:(NSError **)outError
{
	return failNotSupported(outError);
}

- (BOOL)playPatternFromData:(NSData *)data error:(NSError **)outError
{
	return failNotSupported(outError);
}

@end

#endif /* __LP64__ */
