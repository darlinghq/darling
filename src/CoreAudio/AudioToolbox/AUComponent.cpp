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


#include <AudioToolbox/AUComponent.h>
#include <AudioToolbox/AudioOutputUnit.h>
#include <CarbonCore/MacErrors.h>
#include "ComponentDispatch.h"

const CFStringRef kAudioComponentRegistrationsChangedNotification = CFSTR("AudioComponentRegistrationsChangedNotification");
const CFStringRef kAudioComponentInstanceInvalidationNotification = CFSTR("AudioComponentInstanceInvalidationNotification");

OSStatus AudioOutputUnitStart(AudioUnit ci)
{
    return dispatchCall(ci, kAudioOutputUnitStartSelect);
}

OSStatus AudioOutputUnitStop(AudioUnit ci)
{
    return dispatchCall(ci, kAudioOutputUnitStopSelect);
}

OSStatus AudioUnitInitialize(AudioUnit inUnit)
{
    return dispatchCall(inUnit, kAudioUnitInitializeSelect);
}

OSStatus AudioUnitUninitialize(AudioUnit inUnit)
{
    return dispatchCall(inUnit, kAudioUnitUninitializeSelect);
}

OSStatus
AudioUnitGetPropertyInfo(AudioUnit inUnit,
    AudioUnitPropertyID inID,
    AudioUnitScope inScope,
    AudioUnitElement inElement,
    UInt32 * _Nullable outDataSize,
    Boolean * _Nullable outWritable)
{
    return dispatchCall(inUnit, kAudioUnitGetPropertyInfoSelect, inID, inScope, inElement, outDataSize, outWritable);
}

OSStatus
AudioUnitGetProperty(AudioUnit inUnit,
    AudioUnitPropertyID inID,
    AudioUnitScope inScope,
    AudioUnitElement inElement,
    void * outData,
    UInt32 * ioDataSize)
{
    return dispatchCall(inUnit, kAudioUnitGetPropertySelect, inID, inScope, inElement, outData, ioDataSize);
}

OSStatus
AudioUnitSetProperty(AudioUnit inUnit,
    AudioUnitPropertyID inID,
    AudioUnitScope inScope,
    AudioUnitElement inElement,
    const void * _Nullable inData,
    UInt32 inDataSize)
{
    return dispatchCall(inUnit, kAudioUnitSetPropertySelect, inID, inScope, inElement, inData, inDataSize);
}

OSStatus
AudioUnitAddPropertyListener(AudioUnit inUnit,
    AudioUnitPropertyID inID,
    AudioUnitPropertyListenerProc inProc,
    void * _Nullable inProcUserData)
{
    return dispatchCall(inUnit, kAudioUnitAddPropertyListenerSelect, inID, inProc, inProcUserData);
}

OSStatus
AudioUnitRemovePropertyListenerWithUserData(
    AudioUnit inUnit,
    AudioUnitPropertyID inID,
    AudioUnitPropertyListenerProc inProc,
    void * _Nullable inProcUserData)
{
    return dispatchCall(inUnit, kAudioUnitRemovePropertyListenerWithUserDataSelect, inID, inProc, inProcUserData);
}

OSStatus
AudioUnitAddRenderNotify(AudioUnit inUnit,
         AURenderCallback inProc,
         void * _Nullable inProcUserData)
{
    return dispatchCall(inUnit, kAudioUnitAddRenderNotifySelect, inProc, inProcUserData);
}

OSStatus
AudioUnitRemoveRenderNotify(AudioUnit inUnit,
         AURenderCallback inProc,
         void * _Nullable inProcUserData)
{
    return dispatchCall(inUnit, kAudioUnitRemoveRenderNotifySelect, inProc, inProcUserData);
}

OSStatus
AudioUnitGetParameter(AudioUnit inUnit,
         AudioUnitParameterID inID,
         AudioUnitScope inScope,
         AudioUnitElement inElement,
         AudioUnitParameterValue * outValue)
{
    return dispatchCall(inUnit, kAudioUnitGetParameterSelect, inID, inScope, inElement, outValue);
}

OSStatus
AudioUnitSetParameter(AudioUnit inUnit,
         AudioUnitParameterID inID,
         AudioUnitScope inScope,
         AudioUnitElement inElement,
         AudioUnitParameterValue inValue,
         UInt32 inBufferOffsetInFrames)
{
    return dispatchCall(inUnit, kAudioUnitSetParameterSelect, inID, inScope, inElement, inValue, inBufferOffsetInFrames);
}

OSStatus
AudioUnitScheduleParameters(AudioUnit inUnit,
         const AudioUnitParameterEvent * inParameterEvent,
         UInt32 inNumParamEvents)
{
    return dispatchCall(inUnit, kAudioUnitScheduleParametersSelect, inParameterEvent, inNumParamEvents);
}

OSStatus
AudioUnitRender(AudioUnit inUnit,
         AudioUnitRenderActionFlags * _Nullable ioActionFlags,
         const AudioTimeStamp * inTimeStamp,
         UInt32 inOutputBusNumber,
         UInt32 inNumberFrames,
         AudioBufferList * ioData)
{
    return dispatchCall(inUnit, kAudioUnitRenderSelect, ioActionFlags, inTimeStamp, inOutputBusNumber, inNumberFrames, ioData);
}

OSStatus
AudioUnitProcess (AudioUnit inUnit,
         AudioUnitRenderActionFlags * _Nullable ioActionFlags,
         const AudioTimeStamp * inTimeStamp,
         UInt32 inNumberFrames,
         AudioBufferList * ioData)
{
    // TODO
    return unimpErr;
}

OSStatus
AudioUnitProcessMultiple(AudioUnit inUnit,
         AudioUnitRenderActionFlags * _Nullable ioActionFlags,
         const AudioTimeStamp * inTimeStamp,
         UInt32 inNumberFrames,
         UInt32 inNumberInputBufferLists,
         const AudioBufferList * _Nonnull * _Nonnull inInputBufferLists,
         UInt32 inNumberOutputBufferLists,
         AudioBufferList * _Nonnull * _Nonnull ioOutputBufferLists)
{
    // TODO
    return unimpErr;
}

OSStatus
AudioUnitReset(AudioUnit inUnit,
         AudioUnitScope inScope,
         AudioUnitElement inElement)
{
    return dispatchCall(inUnit, kAudioUnitResetSelect, inScope, inElement);
}

OSStatus
AudioUnitExtensionSetComponentList(CFStringRef extensionIdentifier, _Nullable CFArrayRef audioComponentInfo)
{
    // TODO
    return unimpErr;
}

_Nullable CFArrayRef
AudioUnitExtensionCopyComponentList(CFStringRef extensionIdentifier)
{
    // TODO
    return nullptr;
}

