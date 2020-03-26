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
#include <CoreServices/MacErrors.h>

const CFStringRef kAudioComponentRegistrationsChangedNotification = CFSTR("AudioComponentRegistrationsChangedNotification");
const CFStringRef kAudioComponentInstanceInvalidationNotification = CFSTR("AudioComponentInstanceInvalidationNotification");

OSStatus AudioUnitInitialize(AudioUnit inUnit)
{

}

OSStatus AudioUnitUninitialize(AudioUnit inUnit)
{

}

OSStatus
AudioUnitGetPropertyInfo(AudioUnit inUnit,
         AudioUnitPropertyID inID,
         AudioUnitScope inScope,
         AudioUnitElement inElement,
         UInt32 * _Nullable outDataSize,
         Boolean * _Nullable outWritable)
{
}

OSStatus
AudioUnitGetProperty(AudioUnit inUnit,
         AudioUnitPropertyID inID,
         AudioUnitScope inScope,
         AudioUnitElement inElement,
         void * outData,
         UInt32 * ioDataSize)
{
}

OSStatus
AudioUnitSetProperty(AudioUnit inUnit,
         AudioUnitPropertyID inID,
         AudioUnitScope inScope,
         AudioUnitElement inElement,
         const void * _Nullable inData,
         UInt32 inDataSize)
{
}

OSStatus
AudioUnitAddPropertyListener(AudioUnit inUnit,
         AudioUnitPropertyID inID,
         AudioUnitPropertyListenerProc inProc,
         void * _Nullable inProcUserData)
{
}

OSStatus
AudioUnitRemovePropertyListenerWithUserData(
         AudioUnit inUnit,
         AudioUnitPropertyID inID,
         AudioUnitPropertyListenerProc inProc,
         void * _Nullable inProcUserData)
{
}

OSStatus
AudioUnitAddRenderNotify(AudioUnit inUnit,
         AURenderCallback inProc,
         void * _Nullable inProcUserData)
{

}

OSStatus
AudioUnitRemoveRenderNotify(AudioUnit inUnit,
         AURenderCallback inProc,
         void * _Nullable inProcUserData)
{

}

OSStatus
AudioUnitGetParameter(AudioUnit inUnit,
         AudioUnitParameterID inID,
         AudioUnitScope inScope,
         AudioUnitElement inElement,
         AudioUnitParameterValue * outValue)
{

}

OSStatus
AudioUnitSetParameter(AudioUnit inUnit,
         AudioUnitParameterID inID,
         AudioUnitScope inScope,
         AudioUnitElement inElement,
         AudioUnitParameterValue inValue,
         UInt32 inBufferOffsetInFrames)
{
}

OSStatus
AudioUnitScheduleParameters(AudioUnit inUnit,
         const AudioUnitParameterEvent * inParameterEvent,
         UInt32 inNumParamEvents)
{

}

OSStatus
AudioUnitRender(AudioUnit inUnit,
         AudioUnitRenderActionFlags * _Nullable ioActionFlags,
         const AudioTimeStamp * inTimeStamp,
         UInt32 inOutputBusNumber,
         UInt32 inNumberFrames,
         AudioBufferList * ioData)
{
}

OSStatus
AudioUnitProcess (AudioUnit inUnit,
         AudioUnitRenderActionFlags * _Nullable ioActionFlags,
         const AudioTimeStamp * inTimeStamp,
         UInt32 inNumberFrames,
         AudioBufferList * ioData)
{
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
}

OSStatus
AudioUnitReset(AudioUnit inUnit,
         AudioUnitScope inScope,
         AudioUnitElement inElement)
{

}

OSStatus
AudioUnitExtensionSetComponentList(CFStringRef extensionIdentifier, _Nullable CFArrayRef audioComponentInfo)
{

}

_Nullable CFArrayRef
AudioUnitExtensionCopyComponentList(CFStringRef extensionIdentifier)
{

}

