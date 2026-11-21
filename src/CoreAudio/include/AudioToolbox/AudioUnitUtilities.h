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

#ifndef AUDIOTOOLBOX_AUDIOUNITUTILITIES_H
#define AUDIOTOOLBOX_AUDIOUNITUTILITIES_H

#include <AudioToolbox/AUComponent.h>
#include <AudioToolbox/AudioUnitProperties.h>
#include <CoreFoundation/CoreFoundation.h>
#include <dispatch/dispatch.h>

#ifdef __cplusplus
extern "C" {
#endif

// Parameter and property change notifications between the clients of an
// audio unit (hosts, views, controllers), and helpers for parameter values.

CF_ENUM(AudioUnitParameterID) {
	// Wildcard parameter ID; valid only when sending a notification.
	kAUParameterListener_AnyParameter = 0xFFFFFFFF
};

typedef CF_ENUM(UInt32, AudioUnitEventType) {
	kAudioUnitEvent_ParameterValueChange = 0,
	kAudioUnitEvent_BeginParameterChangeGesture = 1,
	kAudioUnitEvent_EndParameterChangeGesture = 2,
	kAudioUnitEvent_PropertyChange = 3
};

typedef struct AUListenerBase* AUParameterListenerRef;
typedef AUParameterListenerRef AUEventListenerRef;

typedef struct AudioUnitEvent {
	AudioUnitEventType mEventType;
	union {
		AudioUnitParameter mParameter;
		AudioUnitProperty mProperty;
	} mArgument;
} AudioUnitEvent;

#ifdef __BLOCKS__
typedef void (^AUParameterListenerBlock)(void* _Nullable inObject,
	const AudioUnitParameter* inParameter, AudioUnitParameterValue inValue);
typedef void (^AUEventListenerBlock)(void* _Nullable inObject,
	const AudioUnitEvent* inEvent, UInt64 inEventHostTime,
	AudioUnitParameterValue inParameterValue);
#endif

typedef void (*AUParameterListenerProc)(void* _Nullable inUserData,
	void* _Nullable inObject, const AudioUnitParameter* inParameter,
	AudioUnitParameterValue inValue);
typedef void (*AUEventListenerProc)(void* _Nullable inUserData,
	void* _Nullable inObject, const AudioUnitEvent* inEvent,
	UInt64 inEventHostTime, AudioUnitParameterValue inParameterValue);

#ifdef __BLOCKS__
extern OSStatus AUListenerCreateWithDispatchQueue(AUParameterListenerRef _Nullable* _Nonnull outListener,
	Float32 inNotificationInterval, dispatch_queue_t inDispatchQueue,
	AUParameterListenerBlock inBlock);
#endif

extern OSStatus AUListenerCreate(AUParameterListenerProc inProc,
	void* _Nullable inUserData, CFRunLoopRef _Nullable inRunLoop,
	CFStringRef _Nullable inRunLoopMode, Float32 inNotificationInterval,
	AUParameterListenerRef _Nullable* _Nonnull outListener);

extern OSStatus AUListenerDispose(AUParameterListenerRef inListener);

extern OSStatus AUListenerAddParameter(AUParameterListenerRef inListener,
	void* _Nullable inObject, const AudioUnitParameter* inParameter);

extern OSStatus AUListenerRemoveParameter(AUParameterListenerRef inListener,
	void* _Nullable inObject, const AudioUnitParameter* inParameter);

extern OSStatus AUParameterSet(AUParameterListenerRef _Nullable inSendingListener,
	void* _Nullable inSendingObject, const AudioUnitParameter* inParameter,
	AudioUnitParameterValue inValue, UInt32 inBufferOffsetInFrames);

extern OSStatus AUParameterListenerNotify(AUParameterListenerRef _Nullable inSendingListener,
	void* _Nullable inSendingObject, const AudioUnitParameter* inParameter);

#ifdef __BLOCKS__
extern OSStatus AUEventListenerCreateWithDispatchQueue(AUEventListenerRef _Nullable* _Nonnull outListener,
	Float32 inNotificationInterval, Float32 inValueChangeGranularity,
	dispatch_queue_t inDispatchQueue, AUEventListenerBlock inBlock);
#endif

extern OSStatus AUEventListenerCreate(AUEventListenerProc inProc,
	void* _Nullable inUserData, CFRunLoopRef inRunLoop,
	CFStringRef inRunLoopMode, Float32 inNotificationInterval,
	Float32 inValueChangeGranularity,
	AUEventListenerRef _Nullable* _Nonnull outListener);

extern OSStatus AUEventListenerAddEventType(AUEventListenerRef inListener,
	void* _Nullable inObject, const AudioUnitEvent* inEvent);

extern OSStatus AUEventListenerRemoveEventType(AUEventListenerRef inListener,
	void* _Nullable inObject, const AudioUnitEvent* inEvent);

extern OSStatus AUEventListenerNotify(AUEventListenerRef _Nullable inSendingListener,
	void* _Nullable inSendingObject, const AudioUnitEvent* inEvent);

extern AudioUnitParameterValue AUParameterValueFromLinear(Float32 inLinearValue,
	const AudioUnitParameter* inParameter);

extern Float32 AUParameterValueToLinear(AudioUnitParameterValue inParameterValue,
	const AudioUnitParameter* inParameter);

extern char* AUParameterFormatValue(Float64 inParameterValue,
	const AudioUnitParameter* inParameter, char* inTextBuffer, UInt32 inDigits);

#ifdef __cplusplus
}
#endif

#endif
