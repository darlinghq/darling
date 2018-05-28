/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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


#ifndef _CoreAudio_H_
#define _CoreAudio_H_

#import <Foundation/Foundation.h>


void* AudioConvertHostTimeToNanos(void);
void* AudioConvertNanosToHostTime(void);
void* AudioDeviceAddIOProc(void);
void* AudioDeviceAddPropertyListener(void);
void* AudioDeviceCreateIOProcID(void);
void* AudioDeviceCreateIOProcIDWithBlock(void);
void* AudioDeviceCreateIOProcIDWithClockedBlock(void);
void* AudioDeviceCreateIOProcIDWithClockedProc(void);
void* AudioDeviceDestroyIOProcID(void);
void* AudioDeviceDuck(void);
void* AudioDeviceEnterMode(void);
void* AudioDeviceExitMode(void);
void* AudioDeviceGetCurrentTime(void);
void* AudioDeviceGetNearestStartTime(void);
void* AudioDeviceGetProperty(void);
void* AudioDeviceGetPropertyInfo(void);
void* AudioDeviceRead(void);
void* AudioDeviceRemoveIOProc(void);
void* AudioDeviceRemovePropertyListener(void);
void* AudioDeviceRun(void);
void* AudioDeviceSetProperty(void);
void* AudioDeviceStart(void);
void* AudioDeviceStartAtTime(void);
void* AudioDeviceStop(void);
void* AudioDeviceTranslateTime(void);
void* AudioDeviceWrite(void);
void* AudioGetCurrentHostTime(void);
void* AudioGetHostClockFrequency(void);
void* AudioGetHostClockMinimumTimeDelta(void);
void* AudioHardwareAddPropertyListener(void);
void* AudioHardwareAddRunLoopSource(void);
void* AudioHardwareBeMaster(void);
void* AudioHardwareClaimAudioDeviceID(void);
void* AudioHardwareClaimAudioStreamID(void);
void* AudioHardwareCreateAggregateDevice(void);
void* AudioHardwareDestroyAggregateDevice(void);
void* AudioHardwareDevicePropertyChanged(void);
void* AudioHardwareDevicesCreated(void);
void* AudioHardwareDevicesDied(void);
void* AudioHardwareGetProperty(void);
void* AudioHardwareGetPropertyInfo(void);
void* AudioHardwareRemovePropertyListener(void);
void* AudioHardwareRemoveRunLoopSource(void);
void* AudioHardwareSetProperty(void);
void* AudioHardwareStreamPropertyChanged(void);
void* AudioHardwareStreamsCreated(void);
void* AudioHardwareStreamsDied(void);
void* AudioHardwareUnload(void);
void* AudioObjectAddPropertyListener(void);
void* AudioObjectAddPropertyListenerBlock(void);
void* AudioObjectAddPropertyListenerInternal(void);
void* AudioObjectCreate(void);
void* AudioObjectExists(void);
void* AudioObjectGetPropertyData(void);
void* AudioObjectGetPropertyDataSize(void);
void* AudioObjectHasProperty(void);
void* AudioObjectIsPropertySettable(void);
void* AudioObjectPropertiesChanged(void);
void* AudioObjectRemovePropertyListener(void);
void* AudioObjectRemovePropertyListenerBlock(void);
void* AudioObjectSetPropertyData(void);
void* AudioObjectShow(void);
void* AudioObjectsPublishedAndDied(void);
void* AudioStreamAddPropertyListener(void);
void* AudioStreamGetProperty(void);
void* AudioStreamGetPropertyInfo(void);
void* AudioStreamRemovePropertyListener(void);
void* AudioStreamSetProperty(void);

#endif
