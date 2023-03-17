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


#ifndef _CoreMIDI_H_
#define _CoreMIDI_H_

#import <Foundation/Foundation.h>

#import <CoreMIDI/MIDIServices.h>
#import <CoreMIDI/MIDICIProfile.h>
#import <CoreMIDI/MIDICIProfileState.h>
#import <CoreMIDI/MIDICISession.h>
#import <CoreMIDI/BLEMIDIAccessor.h>
#import <CoreMIDI/MIDINetworkSession.h>
#import <CoreMIDI/MIDINetworkHost.h>
#import <CoreMIDI/MIDINetworkConnection.h>

void* MIDIClientCreate(void);
void* MIDIClientCreateWithBlock(void);
void* MIDIClientDispose(void);
void* MIDIConfigCheckVirginState(void);
void* MIDIConfigDelete(void);
void* MIDIConfigDuplicate(void);
void* MIDIConfigExists(void);
void* MIDIConfigGetCurrent(void);
void* MIDIConfigGetList(void);
void* MIDIConfigInstall(void);
void* MIDIConfigNew(void);
void* MIDIConfigRename(void);
void* MIDIDestinationCreate(void);
void* MIDIDestinationCreateWithBlock(void);
void* MIDIDeviceAddEntity(void);
void* MIDIDeviceCreate(void);
void* MIDIDeviceDBGetManufModelProperties(void);
void* MIDIDeviceDBGetManufacturers(void);
void* MIDIDeviceDBGetModels(void);
void* MIDIDeviceDispose(void);
void* MIDIDeviceGetEntity(void);
void* MIDIDeviceGetNumberOfEntities(void);
void* MIDIDeviceListAddDevice(void);
void* MIDIDeviceListDispose(void);
void* MIDIDeviceListGetDevice(void);
void* MIDIDeviceListGetNumberOfDevices(void);
void* MIDIDeviceRemoveEntity(void);
void* MIDIDriverEnableMonitoring(void);
void* MIDIEndpointDispose(void);
void* MIDIEndpointGetEntity(void);
void* MIDIEndpointGetRefCons(void);
void* MIDIEndpointSetRefCons(void);
void* MIDIEntityAddOrRemoveEndpoints(void);
void* MIDIEntityGetDestination(void);
void* MIDIEntityGetDevice(void);
void* MIDIEntityGetNumberOfDestinations(void);
void* MIDIEntityGetNumberOfSources(void);
void* MIDIEntityGetSource(void);
void* MIDIExternalDeviceCreate(void);
void* MIDIFlushOutput(void);
void* MIDIGet32BitOnlyDrivers(void);
void* MIDIGetDestination(void);
void* MIDIGetDevice(void);
void* MIDIGetDriverDeviceList(void);
void* MIDIGetDriverIORunLoop(void);
void* MIDIGetExternalDevice(void);
void* MIDIGetNumberOfDestinations(void);
void* MIDIGetNumberOfDevices(void);
void* MIDIGetNumberOfExternalDevices(void);
void* MIDIGetNumberOfSources(void);
void* MIDIGetSerialPortDrivers(void);
void* MIDIGetSerialPortOwner(void);
void* MIDIGetSource(void);
void* MIDIInputPortCreate(void);
void* MIDIInputPortCreateWithBlock(void);
void* MIDIObjectFindByUniqueID(void);
void* MIDIObjectGetDataProperty(void);
void* MIDIObjectGetDictionaryProperty(void);
void* MIDIObjectGetIntegerProperty(void);
void* MIDIObjectGetProperties(void);
void* MIDIObjectGetStringProperty(void);
void* MIDIObjectRemoveProperty(void);
void* MIDIObjectSetDataProperty(void);
void* MIDIObjectSetDictionaryProperty(void);
void* MIDIObjectSetIntegerProperty(void);
void* MIDIObjectSetStringProperty(void);
void* MIDIOutputPortCreate(void);
void* MIDIPacketListAdd(void);
void* MIDIPacketListInit(void);
void* MIDIPortConnectSource(void);
void* MIDIPortDisconnectSource(void);
void* MIDIPortDispose(void);
void* MIDIReceived(void);
void* MIDIRestart(void);
void* MIDISend(void);
void* MIDISendSysex(void);
void* MIDIServerIs32Bit(void);
void* MIDIServerRun(void);
void* MIDISetNotificationRunLoop(void);
void* MIDISetSerialPortOwner(void);
void* MIDISetupAddDevice(void);
void* MIDISetupAddExternalDevice(void);
void* MIDISetupCreate(void);
void* MIDISetupDispose(void);
void* MIDISetupFromData(void);
void* MIDISetupGetCurrent(void);
void* MIDISetupInstall(void);
void* MIDISetupRemoveDevice(void);
void* MIDISetupRemoveExternalDevice(void);
void* MIDISetupToData(void);
void* MIDISourceCreate(void);
void* MIDIThruConnectionCreate(void);
void* MIDIThruConnectionDispose(void);
void* MIDIThruConnectionFind(void);
void* MIDIThruConnectionGetParams(void);
void* MIDIThruConnectionParamsInitialize(void);
void* MIDIThruConnectionSetParams(void);
void* MIDITimerTaskCreate(void);
void* MIDITimerTaskDispose(void);
void* MIDITimerTaskSetNextWakeTime(void);

#endif
