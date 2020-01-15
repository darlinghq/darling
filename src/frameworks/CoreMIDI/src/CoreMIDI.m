/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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


#include <CoreMIDI/CoreMIDI.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* MIDIClientCreate(void)
{
    if (verbose) puts("STUB: MIDIClientCreate called");
    return NULL;
}

void* MIDIClientCreateWithBlock(void)
{
    if (verbose) puts("STUB: MIDIClientCreateWithBlock called");
    return NULL;
}

void* MIDIClientDispose(void)
{
    if (verbose) puts("STUB: MIDIClientDispose called");
    return NULL;
}

void* MIDIConfigCheckVirginState(void)
{
    if (verbose) puts("STUB: MIDIConfigCheckVirginState called");
    return NULL;
}

void* MIDIConfigDelete(void)
{
    if (verbose) puts("STUB: MIDIConfigDelete called");
    return NULL;
}

void* MIDIConfigDuplicate(void)
{
    if (verbose) puts("STUB: MIDIConfigDuplicate called");
    return NULL;
}

void* MIDIConfigExists(void)
{
    if (verbose) puts("STUB: MIDIConfigExists called");
    return NULL;
}

void* MIDIConfigGetCurrent(void)
{
    if (verbose) puts("STUB: MIDIConfigGetCurrent called");
    return NULL;
}

void* MIDIConfigGetList(void)
{
    if (verbose) puts("STUB: MIDIConfigGetList called");
    return NULL;
}

void* MIDIConfigInstall(void)
{
    if (verbose) puts("STUB: MIDIConfigInstall called");
    return NULL;
}

void* MIDIConfigNew(void)
{
    if (verbose) puts("STUB: MIDIConfigNew called");
    return NULL;
}

void* MIDIConfigRename(void)
{
    if (verbose) puts("STUB: MIDIConfigRename called");
    return NULL;
}

void* MIDIDestinationCreate(void)
{
    if (verbose) puts("STUB: MIDIDestinationCreate called");
    return NULL;
}

void* MIDIDestinationCreateWithBlock(void)
{
    if (verbose) puts("STUB: MIDIDestinationCreateWithBlock called");
    return NULL;
}

void* MIDIDeviceAddEntity(void)
{
    if (verbose) puts("STUB: MIDIDeviceAddEntity called");
    return NULL;
}

void* MIDIDeviceCreate(void)
{
    if (verbose) puts("STUB: MIDIDeviceCreate called");
    return NULL;
}

void* MIDIDeviceDBGetManufModelProperties(void)
{
    if (verbose) puts("STUB: MIDIDeviceDBGetManufModelProperties called");
    return NULL;
}

void* MIDIDeviceDBGetManufacturers(void)
{
    if (verbose) puts("STUB: MIDIDeviceDBGetManufacturers called");
    return NULL;
}

void* MIDIDeviceDBGetModels(void)
{
    if (verbose) puts("STUB: MIDIDeviceDBGetModels called");
    return NULL;
}

void* MIDIDeviceDispose(void)
{
    if (verbose) puts("STUB: MIDIDeviceDispose called");
    return NULL;
}

void* MIDIDeviceGetEntity(void)
{
    if (verbose) puts("STUB: MIDIDeviceGetEntity called");
    return NULL;
}

void* MIDIDeviceGetNumberOfEntities(void)
{
    if (verbose) puts("STUB: MIDIDeviceGetNumberOfEntities called");
    return NULL;
}

void* MIDIDeviceListAddDevice(void)
{
    if (verbose) puts("STUB: MIDIDeviceListAddDevice called");
    return NULL;
}

void* MIDIDeviceListDispose(void)
{
    if (verbose) puts("STUB: MIDIDeviceListDispose called");
    return NULL;
}

void* MIDIDeviceListGetDevice(void)
{
    if (verbose) puts("STUB: MIDIDeviceListGetDevice called");
    return NULL;
}

void* MIDIDeviceListGetNumberOfDevices(void)
{
    if (verbose) puts("STUB: MIDIDeviceListGetNumberOfDevices called");
    return NULL;
}

void* MIDIDeviceRemoveEntity(void)
{
    if (verbose) puts("STUB: MIDIDeviceRemoveEntity called");
    return NULL;
}

void* MIDIDriverEnableMonitoring(void)
{
    if (verbose) puts("STUB: MIDIDriverEnableMonitoring called");
    return NULL;
}

void* MIDIEndpointDispose(void)
{
    if (verbose) puts("STUB: MIDIEndpointDispose called");
    return NULL;
}

void* MIDIEndpointGetEntity(void)
{
    if (verbose) puts("STUB: MIDIEndpointGetEntity called");
    return NULL;
}

void* MIDIEndpointGetRefCons(void)
{
    if (verbose) puts("STUB: MIDIEndpointGetRefCons called");
    return NULL;
}

void* MIDIEndpointSetRefCons(void)
{
    if (verbose) puts("STUB: MIDIEndpointSetRefCons called");
    return NULL;
}

void* MIDIEntityAddOrRemoveEndpoints(void)
{
    if (verbose) puts("STUB: MIDIEntityAddOrRemoveEndpoints called");
    return NULL;
}

void* MIDIEntityGetDestination(void)
{
    if (verbose) puts("STUB: MIDIEntityGetDestination called");
    return NULL;
}

void* MIDIEntityGetDevice(void)
{
    if (verbose) puts("STUB: MIDIEntityGetDevice called");
    return NULL;
}

void* MIDIEntityGetNumberOfDestinations(void)
{
    if (verbose) puts("STUB: MIDIEntityGetNumberOfDestinations called");
    return NULL;
}

void* MIDIEntityGetNumberOfSources(void)
{
    if (verbose) puts("STUB: MIDIEntityGetNumberOfSources called");
    return NULL;
}

void* MIDIEntityGetSource(void)
{
    if (verbose) puts("STUB: MIDIEntityGetSource called");
    return NULL;
}

void* MIDIExternalDeviceCreate(void)
{
    if (verbose) puts("STUB: MIDIExternalDeviceCreate called");
    return NULL;
}

void* MIDIFlushOutput(void)
{
    if (verbose) puts("STUB: MIDIFlushOutput called");
    return NULL;
}

void* MIDIGet32BitOnlyDrivers(void)
{
    if (verbose) puts("STUB: MIDIGet32BitOnlyDrivers called");
    return NULL;
}

void* MIDIGetDestination(void)
{
    if (verbose) puts("STUB: MIDIGetDestination called");
    return NULL;
}

void* MIDIGetDevice(void)
{
    if (verbose) puts("STUB: MIDIGetDevice called");
    return NULL;
}

void* MIDIGetDriverDeviceList(void)
{
    if (verbose) puts("STUB: MIDIGetDriverDeviceList called");
    return NULL;
}

void* MIDIGetDriverIORunLoop(void)
{
    if (verbose) puts("STUB: MIDIGetDriverIORunLoop called");
    return NULL;
}

void* MIDIGetExternalDevice(void)
{
    if (verbose) puts("STUB: MIDIGetExternalDevice called");
    return NULL;
}

void* MIDIGetNumberOfDestinations(void)
{
    if (verbose) puts("STUB: MIDIGetNumberOfDestinations called");
    return NULL;
}

void* MIDIGetNumberOfDevices(void)
{
    if (verbose) puts("STUB: MIDIGetNumberOfDevices called");
    return NULL;
}

void* MIDIGetNumberOfExternalDevices(void)
{
    if (verbose) puts("STUB: MIDIGetNumberOfExternalDevices called");
    return NULL;
}

void* MIDIGetNumberOfSources(void)
{
    if (verbose) puts("STUB: MIDIGetNumberOfSources called");
    return NULL;
}

void* MIDIGetSerialPortDrivers(void)
{
    if (verbose) puts("STUB: MIDIGetSerialPortDrivers called");
    return NULL;
}

void* MIDIGetSerialPortOwner(void)
{
    if (verbose) puts("STUB: MIDIGetSerialPortOwner called");
    return NULL;
}

void* MIDIGetSource(void)
{
    if (verbose) puts("STUB: MIDIGetSource called");
    return NULL;
}

void* MIDIInputPortCreate(void)
{
    if (verbose) puts("STUB: MIDIInputPortCreate called");
    return NULL;
}

void* MIDIInputPortCreateWithBlock(void)
{
    if (verbose) puts("STUB: MIDIInputPortCreateWithBlock called");
    return NULL;
}

void* MIDIObjectFindByUniqueID(void)
{
    if (verbose) puts("STUB: MIDIObjectFindByUniqueID called");
    return NULL;
}

void* MIDIObjectGetDataProperty(void)
{
    if (verbose) puts("STUB: MIDIObjectGetDataProperty called");
    return NULL;
}

void* MIDIObjectGetDictionaryProperty(void)
{
    if (verbose) puts("STUB: MIDIObjectGetDictionaryProperty called");
    return NULL;
}

void* MIDIObjectGetIntegerProperty(void)
{
    if (verbose) puts("STUB: MIDIObjectGetIntegerProperty called");
    return NULL;
}

void* MIDIObjectGetProperties(void)
{
    if (verbose) puts("STUB: MIDIObjectGetProperties called");
    return NULL;
}

void* MIDIObjectGetStringProperty(void)
{
    if (verbose) puts("STUB: MIDIObjectGetStringProperty called");
    return NULL;
}

void* MIDIObjectRemoveProperty(void)
{
    if (verbose) puts("STUB: MIDIObjectRemoveProperty called");
    return NULL;
}

void* MIDIObjectSetDataProperty(void)
{
    if (verbose) puts("STUB: MIDIObjectSetDataProperty called");
    return NULL;
}

void* MIDIObjectSetDictionaryProperty(void)
{
    if (verbose) puts("STUB: MIDIObjectSetDictionaryProperty called");
    return NULL;
}

void* MIDIObjectSetIntegerProperty(void)
{
    if (verbose) puts("STUB: MIDIObjectSetIntegerProperty called");
    return NULL;
}

void* MIDIObjectSetStringProperty(void)
{
    if (verbose) puts("STUB: MIDIObjectSetStringProperty called");
    return NULL;
}

void* MIDIOutputPortCreate(void)
{
    if (verbose) puts("STUB: MIDIOutputPortCreate called");
    return NULL;
}

void* MIDIPacketListAdd(void)
{
    if (verbose) puts("STUB: MIDIPacketListAdd called");
    return NULL;
}

void* MIDIPacketListInit(void)
{
    if (verbose) puts("STUB: MIDIPacketListInit called");
    return NULL;
}

void* MIDIPortConnectSource(void)
{
    if (verbose) puts("STUB: MIDIPortConnectSource called");
    return NULL;
}

void* MIDIPortDisconnectSource(void)
{
    if (verbose) puts("STUB: MIDIPortDisconnectSource called");
    return NULL;
}

void* MIDIPortDispose(void)
{
    if (verbose) puts("STUB: MIDIPortDispose called");
    return NULL;
}

void* MIDIReceived(void)
{
    if (verbose) puts("STUB: MIDIReceived called");
    return NULL;
}

void* MIDIRestart(void)
{
    if (verbose) puts("STUB: MIDIRestart called");
    return NULL;
}

void* MIDISend(void)
{
    if (verbose) puts("STUB: MIDISend called");
    return NULL;
}

void* MIDISendSysex(void)
{
    if (verbose) puts("STUB: MIDISendSysex called");
    return NULL;
}

void* MIDIServerIs32Bit(void)
{
    if (verbose) puts("STUB: MIDIServerIs32Bit called");
    return NULL;
}

void* MIDIServerRun(void)
{
    if (verbose) puts("STUB: MIDIServerRun called");
    return NULL;
}

void* MIDISetNotificationRunLoop(void)
{
    if (verbose) puts("STUB: MIDISetNotificationRunLoop called");
    return NULL;
}

void* MIDISetSerialPortOwner(void)
{
    if (verbose) puts("STUB: MIDISetSerialPortOwner called");
    return NULL;
}

void* MIDISetupAddDevice(void)
{
    if (verbose) puts("STUB: MIDISetupAddDevice called");
    return NULL;
}

void* MIDISetupAddExternalDevice(void)
{
    if (verbose) puts("STUB: MIDISetupAddExternalDevice called");
    return NULL;
}

void* MIDISetupCreate(void)
{
    if (verbose) puts("STUB: MIDISetupCreate called");
    return NULL;
}

void* MIDISetupDispose(void)
{
    if (verbose) puts("STUB: MIDISetupDispose called");
    return NULL;
}

void* MIDISetupFromData(void)
{
    if (verbose) puts("STUB: MIDISetupFromData called");
    return NULL;
}

void* MIDISetupGetCurrent(void)
{
    if (verbose) puts("STUB: MIDISetupGetCurrent called");
    return NULL;
}

void* MIDISetupInstall(void)
{
    if (verbose) puts("STUB: MIDISetupInstall called");
    return NULL;
}

void* MIDISetupRemoveDevice(void)
{
    if (verbose) puts("STUB: MIDISetupRemoveDevice called");
    return NULL;
}

void* MIDISetupRemoveExternalDevice(void)
{
    if (verbose) puts("STUB: MIDISetupRemoveExternalDevice called");
    return NULL;
}

void* MIDISetupToData(void)
{
    if (verbose) puts("STUB: MIDISetupToData called");
    return NULL;
}

void* MIDISourceCreate(void)
{
    if (verbose) puts("STUB: MIDISourceCreate called");
    return NULL;
}

void* MIDIThruConnectionCreate(void)
{
    if (verbose) puts("STUB: MIDIThruConnectionCreate called");
    return NULL;
}

void* MIDIThruConnectionDispose(void)
{
    if (verbose) puts("STUB: MIDIThruConnectionDispose called");
    return NULL;
}

void* MIDIThruConnectionFind(void)
{
    if (verbose) puts("STUB: MIDIThruConnectionFind called");
    return NULL;
}

void* MIDIThruConnectionGetParams(void)
{
    if (verbose) puts("STUB: MIDIThruConnectionGetParams called");
    return NULL;
}

void* MIDIThruConnectionParamsInitialize(void)
{
    if (verbose) puts("STUB: MIDIThruConnectionParamsInitialize called");
    return NULL;
}

void* MIDIThruConnectionSetParams(void)
{
    if (verbose) puts("STUB: MIDIThruConnectionSetParams called");
    return NULL;
}

void* MIDITimerTaskCreate(void)
{
    if (verbose) puts("STUB: MIDITimerTaskCreate called");
    return NULL;
}

void* MIDITimerTaskDispose(void)
{
    if (verbose) puts("STUB: MIDITimerTaskDispose called");
    return NULL;
}

void* MIDITimerTaskSetNextWakeTime(void)
{
    if (verbose) puts("STUB: MIDITimerTaskSetNextWakeTime called");
    return NULL;
}
