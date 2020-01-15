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


#include <CoreAudio/CoreAudio.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AudioConvertHostTimeToNanos(void) {
    if (verbose) puts("STUB: AudioConvertHostTimeToNanos called");
    return NULL;
}

void* AudioConvertNanosToHostTime(void) {
    if (verbose) puts("STUB: AudioConvertNanosToHostTime called");
    return NULL;
}

void* AudioDeviceAddIOProc(void) {
    if (verbose) puts("STUB: AudioDeviceAddIOProc called");
    return NULL;
}

void* AudioDeviceAddPropertyListener(void) {
    if (verbose) puts("STUB: AudioDeviceAddPropertyListener called");
    return NULL;
}

void* AudioDeviceCreateIOProcID(void) {
    if (verbose) puts("STUB: AudioDeviceCreateIOProcID called");
    return NULL;
}

void* AudioDeviceCreateIOProcIDWithBlock(void) {
    if (verbose) puts("STUB: AudioDeviceCreateIOProcIDWithBlock called");
    return NULL;
}

void* AudioDeviceCreateIOProcIDWithClockedBlock(void) {
    if (verbose) puts("STUB: AudioDeviceCreateIOProcIDWithClockedBlock called");
    return NULL;
}

void* AudioDeviceCreateIOProcIDWithClockedProc(void) {
    if (verbose) puts("STUB: AudioDeviceCreateIOProcIDWithClockedProc called");
    return NULL;
}

void* AudioDeviceDestroyIOProcID(void) {
    if (verbose) puts("STUB: AudioDeviceDestroyIOProcID called");
    return NULL;
}

void* AudioDeviceDuck(void) {
    if (verbose) puts("STUB: AudioDeviceDuck called");
    return NULL;
}

void* AudioDeviceEnterMode(void) {
    if (verbose) puts("STUB: AudioDeviceEnterMode called");
    return NULL;
}

void* AudioDeviceExitMode(void) {
    if (verbose) puts("STUB: AudioDeviceExitMode called");
    return NULL;
}

void* AudioDeviceGetCurrentTime(void) {
    if (verbose) puts("STUB: AudioDeviceGetCurrentTime called");
    return NULL;
}

void* AudioDeviceGetNearestStartTime(void) {
    if (verbose) puts("STUB: AudioDeviceGetNearestStartTime called");
    return NULL;
}

void* AudioDeviceGetProperty(void) {
    if (verbose) puts("STUB: AudioDeviceGetProperty called");
    return NULL;
}

void* AudioDeviceGetPropertyInfo(void) {
    if (verbose) puts("STUB: AudioDeviceGetPropertyInfo called");
    return NULL;
}

void* AudioDeviceRead(void) {
    if (verbose) puts("STUB: AudioDeviceRead called");
    return NULL;
}

void* AudioDeviceRemoveIOProc(void) {
    if (verbose) puts("STUB: AudioDeviceRemoveIOProc called");
    return NULL;
}

void* AudioDeviceRemovePropertyListener(void) {
    if (verbose) puts("STUB: AudioDeviceRemovePropertyListener called");
    return NULL;
}

void* AudioDeviceRun(void) {
    if (verbose) puts("STUB: AudioDeviceRun called");
    return NULL;
}

void* AudioDeviceSetProperty(void) {
    if (verbose) puts("STUB: AudioDeviceSetProperty called");
    return NULL;
}

void* AudioDeviceStart(void) {
    if (verbose) puts("STUB: AudioDeviceStart called");
    return NULL;
}

void* AudioDeviceStartAtTime(void) {
    if (verbose) puts("STUB: AudioDeviceStartAtTime called");
    return NULL;
}

void* AudioDeviceStop(void) {
    if (verbose) puts("STUB: AudioDeviceStop called");
    return NULL;
}

void* AudioDeviceTranslateTime(void) {
    if (verbose) puts("STUB: AudioDeviceTranslateTime called");
    return NULL;
}

void* AudioDeviceWrite(void) {
    if (verbose) puts("STUB: AudioDeviceWrite called");
    return NULL;
}

void* AudioGetCurrentHostTime(void) {
    if (verbose) puts("STUB: AudioGetCurrentHostTime called");
    return NULL;
}

void* AudioGetHostClockFrequency(void) {
    if (verbose) puts("STUB: AudioGetHostClockFrequency called");
    return NULL;
}

void* AudioGetHostClockMinimumTimeDelta(void) {
    if (verbose) puts("STUB: AudioGetHostClockMinimumTimeDelta called");
    return NULL;
}

void* AudioHardwareAddPropertyListener(void) {
    if (verbose) puts("STUB: AudioHardwareAddPropertyListener called");
    return NULL;
}

void* AudioHardwareAddRunLoopSource(void) {
    if (verbose) puts("STUB: AudioHardwareAddRunLoopSource called");
    return NULL;
}

void* AudioHardwareBeMaster(void) {
    if (verbose) puts("STUB: AudioHardwareBeMaster called");
    return NULL;
}

void* AudioHardwareClaimAudioDeviceID(void) {
    if (verbose) puts("STUB: AudioHardwareClaimAudioDeviceID called");
    return NULL;
}

void* AudioHardwareClaimAudioStreamID(void) {
    if (verbose) puts("STUB: AudioHardwareClaimAudioStreamID called");
    return NULL;
}

void* AudioHardwareCreateAggregateDevice(void) {
    if (verbose) puts("STUB: AudioHardwareCreateAggregateDevice called");
    return NULL;
}

void* AudioHardwareDestroyAggregateDevice(void) {
    if (verbose) puts("STUB: AudioHardwareDestroyAggregateDevice called");
    return NULL;
}

void* AudioHardwareDevicePropertyChanged(void) {
    if (verbose) puts("STUB: AudioHardwareDevicePropertyChanged called");
    return NULL;
}

void* AudioHardwareDevicesCreated(void) {
    if (verbose) puts("STUB: AudioHardwareDevicesCreated called");
    return NULL;
}

void* AudioHardwareDevicesDied(void) {
    if (verbose) puts("STUB: AudioHardwareDevicesDied called");
    return NULL;
}

void* AudioHardwareGetProperty(void) {
    if (verbose) puts("STUB: AudioHardwareGetProperty called");
    return NULL;
}

void* AudioHardwareGetPropertyInfo(void) {
    if (verbose) puts("STUB: AudioHardwareGetPropertyInfo called");
    return NULL;
}

void* AudioHardwareRemovePropertyListener(void) {
    if (verbose) puts("STUB: AudioHardwareRemovePropertyListener called");
    return NULL;
}

void* AudioHardwareRemoveRunLoopSource(void) {
    if (verbose) puts("STUB: AudioHardwareRemoveRunLoopSource called");
    return NULL;
}

void* AudioHardwareSetProperty(void) {
    if (verbose) puts("STUB: AudioHardwareSetProperty called");
    return NULL;
}

void* AudioHardwareStreamPropertyChanged(void) {
    if (verbose) puts("STUB: AudioHardwareStreamPropertyChanged called");
    return NULL;
}

void* AudioHardwareStreamsCreated(void) {
    if (verbose) puts("STUB: AudioHardwareStreamsCreated called");
    return NULL;
}

void* AudioHardwareStreamsDied(void) {
    if (verbose) puts("STUB: AudioHardwareStreamsDied called");
    return NULL;
}

void* AudioHardwareUnload(void) {
    if (verbose) puts("STUB: AudioHardwareUnload called");
    return NULL;
}

void* AudioObjectAddPropertyListener(void) {
    if (verbose) puts("STUB: AudioObjectAddPropertyListener called");
    return NULL;
}

void* AudioObjectAddPropertyListenerBlock(void) {
    if (verbose) puts("STUB: AudioObjectAddPropertyListenerBlock called");
    return NULL;
}

void* AudioObjectAddPropertyListenerInternal(void) {
    if (verbose) puts("STUB: AudioObjectAddPropertyListenerInternal called");
    return NULL;
}

void* AudioObjectCreate(void) {
    if (verbose) puts("STUB: AudioObjectCreate called");
    return NULL;
}

void* AudioObjectExists(void) {
    if (verbose) puts("STUB: AudioObjectExists called");
    return NULL;
}

void* AudioObjectGetPropertyData(void) {
    if (verbose) puts("STUB: AudioObjectGetPropertyData called");
    return NULL;
}

void* AudioObjectGetPropertyDataSize(void) {
    if (verbose) puts("STUB: AudioObjectGetPropertyDataSize called");
    return NULL;
}

void* AudioObjectHasProperty(void) {
    if (verbose) puts("STUB: AudioObjectHasProperty called");
    return NULL;
}

void* AudioObjectIsPropertySettable(void) {
    if (verbose) puts("STUB: AudioObjectIsPropertySettable called");
    return NULL;
}

void* AudioObjectPropertiesChanged(void) {
    if (verbose) puts("STUB: AudioObjectPropertiesChanged called");
    return NULL;
}

void* AudioObjectRemovePropertyListener(void) {
    if (verbose) puts("STUB: AudioObjectRemovePropertyListener called");
    return NULL;
}

void* AudioObjectRemovePropertyListenerBlock(void) {
    if (verbose) puts("STUB: AudioObjectRemovePropertyListenerBlock called");
    return NULL;
}

void* AudioObjectSetPropertyData(void) {
    if (verbose) puts("STUB: AudioObjectSetPropertyData called");
    return NULL;
}

void* AudioObjectShow(void) {
    if (verbose) puts("STUB: AudioObjectShow called");
    return NULL;
}

void* AudioObjectsPublishedAndDied(void) {
    if (verbose) puts("STUB: AudioObjectsPublishedAndDied called");
    return NULL;
}

void* AudioStreamAddPropertyListener(void) {
    if (verbose) puts("STUB: AudioStreamAddPropertyListener called");
    return NULL;
}

void* AudioStreamGetProperty(void) {
    if (verbose) puts("STUB: AudioStreamGetProperty called");
    return NULL;
}

void* AudioStreamGetPropertyInfo(void) {
    if (verbose) puts("STUB: AudioStreamGetPropertyInfo called");
    return NULL;
}

void* AudioStreamRemovePropertyListener(void) {
    if (verbose) puts("STUB: AudioStreamRemovePropertyListener called");
    return NULL;
}

void* AudioStreamSetProperty(void) {
    if (verbose) puts("STUB: AudioStreamSetProperty called");
    return NULL;
}
