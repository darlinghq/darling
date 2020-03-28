/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

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

#ifndef _AT_AUDIO_COMPONENT_H
#define _AT_AUDIO_COMPONENT_H
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreServices/MacTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	kAudioComponentFlag_Unsearchable = 1,
	kAudioComponentFlag_SandboxSafe = 2,
	kAudioComponentFlag_IsV3AudioUnit = 3,
	kAudioComponentFlag_RequiresAsyncInstantiation = 4,
	kAudioComponentFlag_CanLoadInProcess = 8,
};
typedef UInt32 AudioComponentFlags;

enum {
	kAudioComponentInstantiation_LoadOutOfProcess = 1,
	kAudioComponentInstantiation_LoadInProcess = 2,
};
typedef UInt32 AudioComponentInstantiationOptions;

#pragma pack(push, 4)
typedef struct {
	OSType componentType;
	OSType componentSubType;
	OSType componentManufacturer;
	UInt32 componentFlags;
	UInt32 componentFlagsMask;
} AudioComponentDescription;
#pragma pack(pop)

typedef struct OpaqueAudioComponent* AudioComponent;
typedef struct ComponentInstanceRecord* AudioComponentInstance;

typedef OSStatus (*AudioComponentMethod)(void* self, ...);

typedef struct AudioComponentPlugInInterface {
	OSStatus (*Open)(void* self, AudioComponentInstance inst);
	OSStatus (*Close)(void* self);
	AudioComponentMethod (*Lookup)(SInt16 sel);
	void* reserved;
} AudioComponentPlugInInterface;

typedef AudioComponentPlugInInterface * (*AudioComponentFactoryFunction)(const AudioComponentDescription*);

AudioComponent AudioComponentRegister(const AudioComponentDescription *inDesc, CFStringRef inName,
	UInt32 inVersion, AudioComponentFactoryFunction inFactory);

AudioComponent AudioComponentFindNext(AudioComponent inComponent, const AudioComponentDescription* desc);

UInt32 AudioComponentCount(const AudioComponentDescription* desc);

OSStatus AudioComponentCopyName(AudioComponent component, CFStringRef* outName);

OSStatus AudioComponentGetDescription(AudioComponent component, AudioComponentDescription* outDesc);

OSStatus AudioComponentGetVersion(AudioComponent component, UInt32* outVersion);

#if defined(__OBJC__)
#if !TARGET_OS_IPHONE

@class NSImage;
NSImage* AudioComponentGetIcon(AudioComponent component);

#else

@class UIImage;
UIImage* AudioComponentGetIcon(AudioComponent component, float desiredPointSize);

#endif
#endif

OSStatus AudioComponentInstanceNew(AudioComponent component, AudioComponentInstance* outInstance);
void AudioComponentInstantiate(AudioComponent component, AudioComponentInstantiationOptions opts, void(^handler)(AudioComponentInstance, OSStatus));

OSStatus AudioComponentInstanceDispose(AudioComponentInstance inst);
AudioComponent AudioComponentInstanceGetComponent(AudioComponentInstance inst);

Boolean AudioComponentInstanceCanDo(AudioComponentInstance inst, SInt16 sel);

OSStatus AudioComponentCopyConfigurationInfo(AudioComponent component, CFDictionaryRef* outInfo);

enum {
	kAudioComponentValidationResult_Unknown = 0,
	kAudioComponentValidationResult_Passed,
	kAudioComponentValidationResult_Failed,
	kAudioComponentValidationResult_TimedOut,
	kAudioComponentValidationResult_UnauthorizedError_Open,
	kAudioComponentValidationResult_UnauthorizedError_Init,
};
typedef UInt32 AudioComponentValidationResult;

#define kAudioComponentConfigurationInfo_ValidationResult "ValidationResult"
#define kAudioComponentValidationParameter_TimeOut "TimeOut"
#define kAudioComponentValidationParameter_ForceValidation "ForceValidation"

OSStatus AudioComponentValidate(AudioComponent component, CFDictionaryRef validationParams, AudioComponentValidationResult* result);

#ifdef __cplusplus
}
#endif

#endif
