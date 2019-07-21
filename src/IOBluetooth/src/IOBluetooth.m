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


#include <IOBluetooth/IOBluetooth.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AddEmptyDataRefToDictionary(void)
{
    if (verbose) puts("STUB: AddEmptyDataRefToDictionary called");
    return NULL;
}

void* AddEmptyStringToDictionary(void)
{
    if (verbose) puts("STUB: AddEmptyStringToDictionary called");
    return NULL;
}

void* AppleArchiveCreateUniquelyFromPath(void)
{
    if (verbose) puts("STUB: AppleArchiveCreateUniquelyFromPath called");
    return NULL;
}

void* AppleArchiveExpandAtPath(void)
{
    if (verbose) puts("STUB: AppleArchiveExpandAtPath called");
    return NULL;
}

void* AppleSingleToUTCDateTime(void)
{
    if (verbose) puts("STUB: AppleSingleToUTCDateTime called");
    return NULL;
}

void* BCCMD16ToUInt16(void)
{
    if (verbose) puts("STUB: BCCMD16ToUInt16 called");
    return NULL;
}

void* BCCMD16ToUInt8(void)
{
    if (verbose) puts("STUB: BCCMD16ToUInt8 called");
    return NULL;
}

void* BCCMD32ToUInt32(void)
{
    if (verbose) puts("STUB: BCCMD32ToUInt32 called");
    return NULL;
}

void* BTNSObjectFromXPCObject(void)
{
    if (verbose) puts("STUB: BTNSObjectFromXPCObject called");
    return NULL;
}

void* BTXPCObjectFromNSObject(void)
{
    if (verbose) puts("STUB: BTXPCObjectFromNSObject called");
    return NULL;
}

void* BluetoothClearIdleTimerInNVRAM(void)
{
    if (verbose) puts("STUB: BluetoothClearIdleTimerInNVRAM called");
    return NULL;
}

void* BluetoothDisableIdleTimer(void)
{
    if (verbose) puts("STUB: BluetoothDisableIdleTimer called");
    return NULL;
}

void* BluetoothDoNotTickleDisplay(void)
{
    if (verbose) puts("STUB: BluetoothDoNotTickleDisplay called");
    return NULL;
}

void* BluetoothDoReset(void)
{
    if (verbose) puts("STUB: BluetoothDoReset called");
    return NULL;
}

void* BluetoothEnableIdleTimer(void)
{
    if (verbose) puts("STUB: BluetoothEnableIdleTimer called");
    return NULL;
}

void* BluetoothHCIAcceptSynchronousConnectionRequest(void)
{
    if (verbose) puts("STUB: BluetoothHCIAcceptSynchronousConnectionRequest called");
    return NULL;
}

void* BluetoothHCIChangeConnectionPacketType(void)
{
    if (verbose) puts("STUB: BluetoothHCIChangeConnectionPacketType called");
    return NULL;
}

void* BluetoothHCIControllerGeneralInterestNotification(void)
{
    if (verbose) puts("STUB: BluetoothHCIControllerGeneralInterestNotification called");
    return NULL;
}

void* BluetoothHCIControllerPublished(void)
{
    if (verbose) puts("STUB: BluetoothHCIControllerPublished called");
    return NULL;
}

void* BluetoothHCIControllerSupportsSleep(void)
{
    if (verbose) puts("STUB: BluetoothHCIControllerSupportsSleep called");
    return NULL;
}

void* BluetoothHCIDecodeError(void)
{
    if (verbose) puts("STUB: BluetoothHCIDecodeError called");
    return NULL;
}

void* BluetoothHCIDecodeIOReturnError(void)
{
    if (verbose) puts("STUB: BluetoothHCIDecodeIOReturnError called");
    return NULL;
}

void* BluetoothHCIDisconnect(void)
{
    if (verbose) puts("STUB: BluetoothHCIDisconnect called");
    return NULL;
}

void* BluetoothHCIDispatchUserClientRoutine(void)
{
    if (verbose) puts("STUB: BluetoothHCIDispatchUserClientRoutine called");
    return NULL;
}

void* BluetoothHCIDumpStats(void)
{
    if (verbose) puts("STUB: BluetoothHCIDumpStats called");
    return NULL;
}

void* BluetoothHCIEnhancedAcceptSynchronousConnectionRequest(void)
{
    if (verbose) puts("STUB: BluetoothHCIEnhancedAcceptSynchronousConnectionRequest called");
    return NULL;
}

void* BluetoothHCIEnhancedSetupSynchronousConnection(void)
{
    if (verbose) puts("STUB: BluetoothHCIEnhancedSetupSynchronousConnection called");
    return NULL;
}

void* BluetoothHCIGetAsyncNotificationPort(void)
{
    if (verbose) puts("STUB: BluetoothHCIGetAsyncNotificationPort called");
    return NULL;
}

void* BluetoothHCIGetSynchronousConnectionPacketType(void)
{
    if (verbose) puts("STUB: BluetoothHCIGetSynchronousConnectionPacketType called");
    return NULL;
}

void* BluetoothHCIGetTransportHardwareStatus(void)
{
    if (verbose) puts("STUB: BluetoothHCIGetTransportHardwareStatus called");
    return NULL;
}

void* BluetoothHCIGetTransportInfo(void)
{
    if (verbose) puts("STUB: BluetoothHCIGetTransportInfo called");
    return NULL;
}

void* BluetoothHCIKillAllPendingRequests(void)
{
    if (verbose) puts("STUB: BluetoothHCIKillAllPendingRequests called");
    return NULL;
}

void* BluetoothHCILMPLoggingEnabled(void)
{
    if (verbose) puts("STUB: BluetoothHCILMPLoggingEnabled called");
    return NULL;
}

void* BluetoothHCIRejectSynchronousConnectionRequest(void)
{
    if (verbose) puts("STUB: BluetoothHCIRejectSynchronousConnectionRequest called");
    return NULL;
}

void* BluetoothHCIRequestCreate(void)
{
    if (verbose) puts("STUB: BluetoothHCIRequestCreate called");
    return NULL;
}

void* BluetoothHCIRequestDelete(void)
{
    if (verbose) puts("STUB: BluetoothHCIRequestDelete called");
    return NULL;
}

void* BluetoothHCIResetHappened(void)
{
    if (verbose) puts("STUB: BluetoothHCIResetHappened called");
    return NULL;
}

void* BluetoothHCIResetTransportHardwareStatus(void)
{
    if (verbose) puts("STUB: BluetoothHCIResetTransportHardwareStatus called");
    return NULL;
}

void* BluetoothHCISendRawACLData(void)
{
    if (verbose) puts("STUB: BluetoothHCISendRawACLData called");
    return NULL;
}

void* BluetoothHCISendRawCommand(void)
{
    if (verbose) puts("STUB: BluetoothHCISendRawCommand called");
    return NULL;
}

void* BluetoothHCISetAllowedChannels(void)
{
    if (verbose) puts("STUB: BluetoothHCISetAllowedChannels called");
    return NULL;
}

void* BluetoothHCISetDefaultChannelSettings(void)
{
    if (verbose) puts("STUB: BluetoothHCISetDefaultChannelSettings called");
    return NULL;
}

void* BluetoothHCISetDefaultL2CAPChannelSettings(void)
{
    if (verbose) puts("STUB: BluetoothHCISetDefaultL2CAPChannelSettings called");
    return NULL;
}

void* BluetoothHCISetupAsyncNotifications(void)
{
    if (verbose) puts("STUB: BluetoothHCISetupAsyncNotifications called");
    return NULL;
}

void* BluetoothHCISetupSynchronousConnection(void)
{
    if (verbose) puts("STUB: BluetoothHCISetupSynchronousConnection called");
    return NULL;
}

void* BluetoothHCISetupUserClient(void)
{
    if (verbose) puts("STUB: BluetoothHCISetupUserClient called");
    return NULL;
}

void* BluetoothHCISwitchToSelectedHostController(void)
{
    if (verbose) puts("STUB: BluetoothHCISwitchToSelectedHostController called");
    return NULL;
}

void* BluetoothHCITearDownUserClient(void)
{
    if (verbose) puts("STUB: BluetoothHCITearDownUserClient called");
    return NULL;
}

void* BluetoothHCIToggleLMPLogging(void)
{
    if (verbose) puts("STUB: BluetoothHCIToggleLMPLogging called");
    return NULL;
}

void* BluetoothHCIToggleSCOConnection(void)
{
    if (verbose) puts("STUB: BluetoothHCIToggleSCOConnection called");
    return NULL;
}

void* BluetoothHCIToggleeSCOEV3Connection(void)
{
    if (verbose) puts("STUB: BluetoothHCIToggleeSCOEV3Connection called");
    return NULL;
}

void* BluetoothHCIToggleeSCOEV4Connection(void)
{
    if (verbose) puts("STUB: BluetoothHCIToggleeSCOEV4Connection called");
    return NULL;
}

void* BluetoothHCIToggleeSCOEV5Connection(void)
{
    if (verbose) puts("STUB: BluetoothHCIToggleeSCOEV5Connection called");
    return NULL;
}

void* BluetoothHCIVendorCommand(void)
{
    if (verbose) puts("STUB: BluetoothHCIVendorCommand called");
    return NULL;
}

void* BluetoothHCIWriteLinkPolicySettings(void)
{
    if (verbose) puts("STUB: BluetoothHCIWriteLinkPolicySettings called");
    return NULL;
}

void* BluetoothHCIWritePageTimeout(void)
{
    if (verbose) puts("STUB: BluetoothHCIWritePageTimeout called");
    return NULL;
}

void* BluetoothLECreateConnection(void)
{
    if (verbose) puts("STUB: BluetoothLECreateConnection called");
    return NULL;
}

void* BluetoothSetBTLP(void)
{
    if (verbose) puts("STUB: BluetoothSetBTLP called");
    return NULL;
}

void* BluetoothSetBTPD(void)
{
    if (verbose) puts("STUB: BluetoothSetBTPD called");
    return NULL;
}

void* BluetoothSetBTPU(void)
{
    if (verbose) puts("STUB: BluetoothSetBTPU called");
    return NULL;
}

void* BluetoothSetBTRB(void)
{
    if (verbose) puts("STUB: BluetoothSetBTRB called");
    return NULL;
}

void* BluetoothSetBTRS(void)
{
    if (verbose) puts("STUB: BluetoothSetBTRS called");
    return NULL;
}

void* BluetoothSetIdleTimer(void)
{
    if (verbose) puts("STUB: BluetoothSetIdleTimer called");
    return NULL;
}

void* BluetoothSetIdleTimerInNVRAM(void)
{
    if (verbose) puts("STUB: BluetoothSetIdleTimerInNVRAM called");
    return NULL;
}

void* BluetoothUpdateFirmware(void)
{
    if (verbose) puts("STUB: BluetoothUpdateFirmware called");
    return NULL;
}

void* BluetoothValidSleepType(void)
{
    if (verbose) puts("STUB: BluetoothValidSleepType called");
    return NULL;
}

void* CStringAppend(void)
{
    if (verbose) puts("STUB: CStringAppend called");
    return NULL;
}

void* ClearBluetoothHCIResetHappened(void)
{
    if (verbose) puts("STUB: ClearBluetoothHCIResetHappened called");
    return NULL;
}

void* ConvertPStringToUnicode(void)
{
    if (verbose) puts("STUB: ConvertPStringToUnicode called");
    return NULL;
}

void* ConvertUnicodeToPString(void)
{
    if (verbose) puts("STUB: ConvertUnicodeToPString called");
    return NULL;
}

void* CopyCToPascalString(void)
{
    if (verbose) puts("STUB: CopyCToPascalString called");
    return NULL;
}

void* CreateHIDDeviceInterface(void)
{
    if (verbose) puts("STUB: CreateHIDDeviceInterface called");
    return NULL;
}

void* DeviceConnectedCallback(void)
{
    if (verbose) puts("STUB: DeviceConnectedCallback called");
    return NULL;
}

void* DeviceDisconnectedCallback(void)
{
    if (verbose) puts("STUB: DeviceDisconnectedCallback called");
    return NULL;
}

void* DumpHeaderBytes(void)
{
    if (verbose) puts("STUB: DumpHeaderBytes called");
    return NULL;
}

void* ElementCookieForUsage(void)
{
    if (verbose) puts("STUB: ElementCookieForUsage called");
    return NULL;
}

void* EventServiceConnectedCallback(void)
{
    if (verbose) puts("STUB: EventServiceConnectedCallback called");
    return NULL;
}

void* EventServiceDisconnectedCallback(void)
{
    if (verbose) puts("STUB: EventServiceDisconnectedCallback called");
    return NULL;
}

void* EventServiceInterestCallback(void)
{
    if (verbose) puts("STUB: EventServiceInterestCallback called");
    return NULL;
}

void* FSAppendAppleSingleEncoded(void)
{
    if (verbose) puts("STUB: FSAppendAppleSingleEncoded called");
    return NULL;
}

void* FSAppleSingleDecode(void)
{
    if (verbose) puts("STUB: FSAppleSingleDecode called");
    return NULL;
}

void* FSAppleSingleDecodeData(void)
{
    if (verbose) puts("STUB: FSAppleSingleDecodeData called");
    return NULL;
}

void* FSAppleSingleEncode(void)
{
    if (verbose) puts("STUB: FSAppleSingleEncode called");
    return NULL;
}

void* FSArchiveAppendItem(void)
{
    if (verbose) puts("STUB: FSArchiveAppendItem called");
    return NULL;
}

void* FSCopyForkData(void)
{
    if (verbose) puts("STUB: FSCopyForkData called");
    return NULL;
}

void* FSCreateAppleArchive(void)
{
    if (verbose) puts("STUB: FSCreateAppleArchive called");
    return NULL;
}

void* FSCreateArchive(void)
{
    if (verbose) puts("STUB: FSCreateArchive called");
    return NULL;
}

void* FSCreateFileBuffer(void)
{
    if (verbose) puts("STUB: FSCreateFileBuffer called");
    return NULL;
}

void* FSCreateUniqueArchive(void)
{
    if (verbose) puts("STUB: FSCreateUniqueArchive called");
    return NULL;
}

void* FSCreateUniqueFile(void)
{
    if (verbose) puts("STUB: FSCreateUniqueFile called");
    return NULL;
}

void* FSCreateUniqueSpecialFolderArchive(void)
{
    if (verbose) puts("STUB: FSCreateUniqueSpecialFolderArchive called");
    return NULL;
}

void* FSCreateUniquelyNamedAppleArchive(void)
{
    if (verbose) puts("STUB: FSCreateUniquelyNamedAppleArchive called");
    return NULL;
}

void* FSDeleteFileBuffer(void)
{
    if (verbose) puts("STUB: FSDeleteFileBuffer called");
    return NULL;
}

void* FSExists(void)
{
    if (verbose) puts("STUB: FSExists called");
    return NULL;
}

void* FSExpandAppleArchive(void)
{
    if (verbose) puts("STUB: FSExpandAppleArchive called");
    return NULL;
}

void* FSExpandArchive(void)
{
    if (verbose) puts("STUB: FSExpandArchive called");
    return NULL;
}

void* FSMakeFSRef(void)
{
    if (verbose) puts("STUB: FSMakeFSRef called");
    return NULL;
}

void* FSMakeUniqueName(void)
{
    if (verbose) puts("STUB: FSMakeUniqueName called");
    return NULL;
}

void* FSNeedsArchiving(void)
{
    if (verbose) puts("STUB: FSNeedsArchiving called");
    return NULL;
}

void* FSReadForkSkip(void)
{
    if (verbose) puts("STUB: FSReadForkSkip called");
    return NULL;
}

void* FSRefToNSStringPath(void)
{
    if (verbose) puts("STUB: FSRefToNSStringPath called");
    return NULL;
}

void* FileHasResourceFork(void)
{
    if (verbose) puts("STUB: FileHasResourceFork called");
    return NULL;
}

void* FileHasTypeAndCreator(void)
{
    if (verbose) puts("STUB: FileHasTypeAndCreator called");
    return NULL;
}

void* GetGMTDelta(void)
{
    if (verbose) puts("STUB: GetGMTDelta called");
    return NULL;
}

void* HCreateFile(void)
{
    if (verbose) puts("STUB: HCreateFile called");
    return NULL;
}

void* HExists(void)
{
    if (verbose) puts("STUB: HExists called");
    return NULL;
}

void* HIDDeviceForIOBluetoothDevice(void)
{
    if (verbose) puts("STUB: HIDDeviceForIOBluetoothDevice called");
    return NULL;
}

void* HMakeFSRef(void)
{
    if (verbose) puts("STUB: HMakeFSRef called");
    return NULL;
}

void* HMakeUniqueName(void)
{
    if (verbose) puts("STUB: HMakeUniqueName called");
    return NULL;
}

void* HostController_AsyncHCIEventsNotificationHandler(void)
{
    if (verbose) puts("STUB: HostController_AsyncHCIEventsNotificationHandler called");
    return NULL;
}

void* IOBluetoothAddAllowedIncomingL2CAPChannel(void)
{
    if (verbose) puts("STUB: IOBluetoothAddAllowedIncomingL2CAPChannel called");
    return NULL;
}

void* IOBluetoothAddAllowedIncomingRFCOMMChannel(void)
{
    if (verbose) puts("STUB: IOBluetoothAddAllowedIncomingRFCOMMChannel called");
    return NULL;
}

void* IOBluetoothAddPANDevice(void)
{
    if (verbose) puts("STUB: IOBluetoothAddPANDevice called");
    return NULL;
}

void* IOBluetoothAddSCOAudioDevice(void)
{
    if (verbose) puts("STUB: IOBluetoothAddSCOAudioDevice called");
    return NULL;
}

void* IOBluetoothAddSecureIncomingRFCOMMChannel(void)
{
    if (verbose) puts("STUB: IOBluetoothAddSecureIncomingRFCOMMChannel called");
    return NULL;
}

void* IOBluetoothAddServiceDict(void)
{
    if (verbose) puts("STUB: IOBluetoothAddServiceDict called");
    return NULL;
}

void* IOBluetoothAllowRoleSwitchForDevice(void)
{
    if (verbose) puts("STUB: IOBluetoothAllowRoleSwitchForDevice called");
    return NULL;
}

void* IOBluetoothAudioChange(void)
{
    if (verbose) puts("STUB: IOBluetoothAudioChange called");
    return NULL;
}

void* IOBluetoothBroadcomSchedulerWorkaround(void)
{
    if (verbose) puts("STUB: IOBluetoothBroadcomSchedulerWorkaround called");
    return NULL;
}

void* IOBluetoothCFStringFromDeviceAddress(void)
{
    if (verbose) puts("STUB: IOBluetoothCFStringFromDeviceAddress called");
    return NULL;
}

void* IOBluetoothCFStringToDeviceAddress(void)
{
    if (verbose) puts("STUB: IOBluetoothCFStringToDeviceAddress called");
    return NULL;
}

void* IOBluetoothCSRLibAccessToPSKey(void)
{
    if (verbose) puts("STUB: IOBluetoothCSRLibAccessToPSKey called");
    return NULL;
}

void* IOBluetoothCSRLibBCCMDGetNewSequenceNumber(void)
{
    if (verbose) puts("STUB: IOBluetoothCSRLibBCCMDGetNewSequenceNumber called");
    return NULL;
}

void* IOBluetoothCSRLibClearAllPSKeys(void)
{
    if (verbose) puts("STUB: IOBluetoothCSRLibClearAllPSKeys called");
    return NULL;
}

void* IOBluetoothCSRLibClearPSKey(void)
{
    if (verbose) puts("STUB: IOBluetoothCSRLibClearPSKey called");
    return NULL;
}

void* IOBluetoothCSRLibGetBCCMDMessageLen(void)
{
    if (verbose) puts("STUB: IOBluetoothCSRLibGetBCCMDMessageLen called");
    return NULL;
}

void* IOBluetoothCSRLibGetBCCMDMessageNumberOfWords(void)
{
    if (verbose) puts("STUB: IOBluetoothCSRLibGetBCCMDMessageNumberOfWords called");
    return NULL;
}

void* IOBluetoothCSRLibHCISendBCCMDMessage(void)
{
    if (verbose) puts("STUB: IOBluetoothCSRLibHCISendBCCMDMessage called");
    return NULL;
}

void* IOBluetoothCSRLibHandleBCCMDData(void)
{
    if (verbose) puts("STUB: IOBluetoothCSRLibHandleBCCMDData called");
    return NULL;
}

void* IOBluetoothCSRLibReadPSKey(void)
{
    if (verbose) puts("STUB: IOBluetoothCSRLibReadPSKey called");
    return NULL;
}

void* IOBluetoothCSRLibSendBCCMDMessage(void)
{
    if (verbose) puts("STUB: IOBluetoothCSRLibSendBCCMDMessage called");
    return NULL;
}

void* IOBluetoothCSRLibSetPSCallback(void)
{
    if (verbose) puts("STUB: IOBluetoothCSRLibSetPSCallback called");
    return NULL;
}

void* IOBluetoothCSRLibWritePSKey(void)
{
    if (verbose) puts("STUB: IOBluetoothCSRLibWritePSKey called");
    return NULL;
}

void* IOBluetoothClearCachedValues(void)
{
    if (verbose) puts("STUB: IOBluetoothClearCachedValues called");
    return NULL;
}

void* IOBluetoothClearIncomingChannelRules(void)
{
    if (verbose) puts("STUB: IOBluetoothClearIncomingChannelRules called");
    return NULL;
}

void* IOBluetoothClearRunInactivityTimerInNVRAM(void)
{
    if (verbose) puts("STUB: IOBluetoothClearRunInactivityTimerInNVRAM called");
    return NULL;
}

void* IOBluetoothConfigureHIDDevice(void)
{
    if (verbose) puts("STUB: IOBluetoothConfigureHIDDevice called");
    return NULL;
}

void* IOBluetoothConsoleUserID(void)
{
    if (verbose) puts("STUB: IOBluetoothConsoleUserID called");
    return NULL;
}

void* IOBluetoothConsoleUserName(void)
{
    if (verbose) puts("STUB: IOBluetoothConsoleUserName called");
    return NULL;
}

void* IOBluetoothDeviceCloseConnection(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceCloseConnection called");
    return NULL;
}

void* IOBluetoothDeviceCreateWithAddress(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceCreateWithAddress called");
    return NULL;
}

void* IOBluetoothDeviceCreateWithService(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceCreateWithService called");
    return NULL;
}

void* IOBluetoothDeviceGetAddress(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceGetAddress called");
    return NULL;
}

void* IOBluetoothDeviceGetAddressString(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceGetAddressString called");
    return NULL;
}

void* IOBluetoothDeviceGetConfigurableTypes(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceGetConfigurableTypes called");
    return NULL;
}

void* IOBluetoothDeviceGetConfiguredTypes(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceGetConfiguredTypes called");
    return NULL;
}

void* IOBluetoothDeviceGetDeviceClassMajor(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceGetDeviceClassMajor called");
    return NULL;
}

void* IOBluetoothDeviceGetLastServicesUpdate(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceGetLastServicesUpdate called");
    return NULL;
}

void* IOBluetoothDeviceGetName(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceGetName called");
    return NULL;
}

void* IOBluetoothDeviceGetNameOrAddress(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceGetNameOrAddress called");
    return NULL;
}

void* IOBluetoothDeviceGetServiceRecordForUUID(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceGetServiceRecordForUUID called");
    return NULL;
}

void* IOBluetoothDeviceGetServices(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceGetServices called");
    return NULL;
}

void* IOBluetoothDeviceInquiryClearFoundDevices(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquiryClearFoundDevices called");
    return NULL;
}

void* IOBluetoothDeviceInquiryCreateWithCallbackRefCon(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquiryCreateWithCallbackRefCon called");
    return NULL;
}

void* IOBluetoothDeviceInquiryDelete(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquiryDelete called");
    return NULL;
}

void* IOBluetoothDeviceInquiryGetFoundDevices(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquiryGetFoundDevices called");
    return NULL;
}

void* IOBluetoothDeviceInquiryGetInquiryLength(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquiryGetInquiryLength called");
    return NULL;
}

void* IOBluetoothDeviceInquiryGetUpdateNewDeviceNames(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquiryGetUpdateNewDeviceNames called");
    return NULL;
}

void* IOBluetoothDeviceInquiryGetUserRefCon(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquiryGetUserRefCon called");
    return NULL;
}

void* IOBluetoothDeviceInquirySetCompleteCallback(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquirySetCompleteCallback called");
    return NULL;
}

void* IOBluetoothDeviceInquirySetDeviceFoundCallback(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquirySetDeviceFoundCallback called");
    return NULL;
}

void* IOBluetoothDeviceInquirySetDeviceNameUpdatedCallback(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquirySetDeviceNameUpdatedCallback called");
    return NULL;
}

void* IOBluetoothDeviceInquirySetInquiryLength(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquirySetInquiryLength called");
    return NULL;
}

void* IOBluetoothDeviceInquirySetSearchCriteria(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquirySetSearchCriteria called");
    return NULL;
}

void* IOBluetoothDeviceInquirySetStartedCallback(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquirySetStartedCallback called");
    return NULL;
}

void* IOBluetoothDeviceInquirySetUpdateNewDeviceNames(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquirySetUpdateNewDeviceNames called");
    return NULL;
}

void* IOBluetoothDeviceInquirySetUpdatingNamesStartedCallback(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquirySetUpdatingNamesStartedCallback called");
    return NULL;
}

void* IOBluetoothDeviceInquirySetUserRefCon(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquirySetUserRefCon called");
    return NULL;
}

void* IOBluetoothDeviceInquiryStart(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquiryStart called");
    return NULL;
}

void* IOBluetoothDeviceInquiryStop(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceInquiryStop called");
    return NULL;
}

void* IOBluetoothDeviceIsConnected(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceIsConnected called");
    return NULL;
}

void* IOBluetoothDeviceOpenConnection(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceOpenConnection called");
    return NULL;
}

void* IOBluetoothDeviceOpenConnectionWithOptions(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceOpenConnectionWithOptions called");
    return NULL;
}

void* IOBluetoothDeviceOpenL2CAPChannelAsync(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceOpenL2CAPChannelAsync called");
    return NULL;
}

void* IOBluetoothDeviceOpenL2CAPChannelAsyncWithConfiguration(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceOpenL2CAPChannelAsyncWithConfiguration called");
    return NULL;
}

void* IOBluetoothDeviceOpenL2CAPChannelSync(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceOpenL2CAPChannelSync called");
    return NULL;
}

void* IOBluetoothDeviceOpenL2CAPChannelSyncWithConfiguration(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceOpenL2CAPChannelSyncWithConfiguration called");
    return NULL;
}

void* IOBluetoothDevicePerformSDPQuery(void)
{
    if (verbose) puts("STUB: IOBluetoothDevicePerformSDPQuery called");
    return NULL;
}

void* IOBluetoothDeviceRegisterForDisconnectNotification(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceRegisterForDisconnectNotification called");
    return NULL;
}

void* IOBluetoothDeviceRemoteNameRequest(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceRemoteNameRequest called");
    return NULL;
}

void* IOBluetoothDeviceRemoteNameRequestWithTimeout(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceRemoteNameRequestWithTimeout called");
    return NULL;
}

void* IOBluetoothDeviceServiceAdded(void)
{
    if (verbose) puts("STUB: IOBluetoothDeviceServiceAdded called");
    return NULL;
}

void* IOBluetoothDisallowRoleSwitchForDevice(void)
{
    if (verbose) puts("STUB: IOBluetoothDisallowRoleSwitchForDevice called");
    return NULL;
}

void* IOBluetoothEnableConsoleUserNotifications(void)
{
    if (verbose) puts("STUB: IOBluetoothEnableConsoleUserNotifications called");
    return NULL;
}

void* IOBluetoothFileLogHelper(void)
{
    if (verbose) puts("STUB: IOBluetoothFileLogHelper called");
    return NULL;
}

void* IOBluetoothFileLogHelperFlushFileCache(void)
{
    if (verbose) puts("STUB: IOBluetoothFileLogHelperFlushFileCache called");
    return NULL;
}

void* IOBluetoothFindNumberOfRegistryEntriesOfClassName(void)
{
    if (verbose) puts("STUB: IOBluetoothFindNumberOfRegistryEntriesOfClassName called");
    return NULL;
}

void* IOBluetoothFrameworkInit(void)
{
    if (verbose) puts("STUB: IOBluetoothFrameworkInit called");
    return NULL;
}

void* IOBluetoothFrameworkNotificationPort(void)
{
    if (verbose) puts("STUB: IOBluetoothFrameworkNotificationPort called");
    return NULL;
}

void* IOBluetoothGainAdministrationPrivileges(void)
{
    if (verbose) puts("STUB: IOBluetoothGainAdministrationPrivileges called");
    return NULL;
}

void* IOBluetoothGetAddressFromService(void)
{
    if (verbose) puts("STUB: IOBluetoothGetAddressFromService called");
    return NULL;
}

void* IOBluetoothGetAllOpenL2CAPChannelsForPSM(void)
{
    if (verbose) puts("STUB: IOBluetoothGetAllOpenL2CAPChannelsForPSM called");
    return NULL;
}

void* IOBluetoothGetAllOpenRFCOMMChannelsForID(void)
{
    if (verbose) puts("STUB: IOBluetoothGetAllOpenRFCOMMChannelsForID called");
    return NULL;
}

void* IOBluetoothGetAllowedIncomingRFCOMMChannels(void)
{
    if (verbose) puts("STUB: IOBluetoothGetAllowedIncomingRFCOMMChannels called");
    return NULL;
}

void* IOBluetoothGetBluetoothHCIVersionString(void)
{
    if (verbose) puts("STUB: IOBluetoothGetBluetoothHCIVersionString called");
    return NULL;
}

void* IOBluetoothGetBluetoothStackVersionString(void)
{
    if (verbose) puts("STUB: IOBluetoothGetBluetoothStackVersionString called");
    return NULL;
}

void* IOBluetoothGetCachedValues(void)
{
    if (verbose) puts("STUB: IOBluetoothGetCachedValues called");
    return NULL;
}

void* IOBluetoothGetChipsetID(void)
{
    if (verbose) puts("STUB: IOBluetoothGetChipsetID called");
    return NULL;
}

void* IOBluetoothGetChipsetString(void)
{
    if (verbose) puts("STUB: IOBluetoothGetChipsetString called");
    return NULL;
}

void* IOBluetoothGetConfiguredDeviceAddresses(void)
{
    if (verbose) puts("STUB: IOBluetoothGetConfiguredDeviceAddresses called");
    return NULL;
}

void* IOBluetoothGetConfiguredDevices(void)
{
    if (verbose) puts("STUB: IOBluetoothGetConfiguredDevices called");
    return NULL;
}

void* IOBluetoothGetLocalServices(void)
{
    if (verbose) puts("STUB: IOBluetoothGetLocalServices called");
    return NULL;
}

void* IOBluetoothGetObjectIDFromArguments(void)
{
    if (verbose) puts("STUB: IOBluetoothGetObjectIDFromArguments called");
    return NULL;
}

void* IOBluetoothGetObjectIDFromIOService(void)
{
    if (verbose) puts("STUB: IOBluetoothGetObjectIDFromIOService called");
    return NULL;
}

void* IOBluetoothGetPANConnectionState(void)
{
    if (verbose) puts("STUB: IOBluetoothGetPANConnectionState called");
    return NULL;
}

void* IOBluetoothGetPairedDevices(void)
{
    if (verbose) puts("STUB: IOBluetoothGetPairedDevices called");
    return NULL;
}

void* IOBluetoothGetUniqueFileNameAndPath(void)
{
    if (verbose) puts("STUB: IOBluetoothGetUniqueFileNameAndPath called");
    return NULL;
}

void* IOBluetoothGetUniqueFileNameAndWithCFStringPath(void)
{
    if (verbose) puts("STUB: IOBluetoothGetUniqueFileNameAndWithCFStringPath called");
    return NULL;
}

void* IOBluetoothGetVersion(void)
{
    if (verbose) puts("STUB: IOBluetoothGetVersion called");
    return NULL;
}

void* IOBluetoothHCIControllerDisableL2CAPKernelDrivers(void)
{
    if (verbose) puts("STUB: IOBluetoothHCIControllerDisableL2CAPKernelDrivers called");
    return NULL;
}

void* IOBluetoothIgnoreHIDDevice(void)
{
    if (verbose) puts("STUB: IOBluetoothIgnoreHIDDevice called");
    return NULL;
}

void* IOBluetoothIsBluetoothSecured(void)
{
    if (verbose) puts("STUB: IOBluetoothIsBluetoothSecured called");
    return NULL;
}

void* IOBluetoothIsConfiguredHIDDevice(void)
{
    if (verbose) puts("STUB: IOBluetoothIsConfiguredHIDDevice called");
    return NULL;
}

void* IOBluetoothIsFileAppleDesignatedPIMData(void)
{
    if (verbose) puts("STUB: IOBluetoothIsFileAppleDesignatedPIMData called");
    return NULL;
}

void* IOBluetoothIsFileAppleDesignatedPIMDataAtCFStringPath(void)
{
    if (verbose) puts("STUB: IOBluetoothIsFileAppleDesignatedPIMDataAtCFStringPath called");
    return NULL;
}

void* IOBluetoothIsMachinePortable(void)
{
    if (verbose) puts("STUB: IOBluetoothIsMachinePortable called");
    return NULL;
}

void* IOBluetoothL2CAPChannelCloseChannel(void)
{
    if (verbose) puts("STUB: IOBluetoothL2CAPChannelCloseChannel called");
    return NULL;
}

void* IOBluetoothL2CAPChannelGetIncomingMTU(void)
{
    if (verbose) puts("STUB: IOBluetoothL2CAPChannelGetIncomingMTU called");
    return NULL;
}

void* IOBluetoothL2CAPChannelGetOutgoingMTU(void)
{
    if (verbose) puts("STUB: IOBluetoothL2CAPChannelGetOutgoingMTU called");
    return NULL;
}

void* IOBluetoothL2CAPChannelRegisterForChannelCloseNotification(void)
{
    if (verbose) puts("STUB: IOBluetoothL2CAPChannelRegisterForChannelCloseNotification called");
    return NULL;
}

void* IOBluetoothL2CAPChannelRegisterIncomingEventListener(void)
{
    if (verbose) puts("STUB: IOBluetoothL2CAPChannelRegisterIncomingEventListener called");
    return NULL;
}

void* IOBluetoothL2CAPChannelRegisterIncomingEventListenerWithConfiguration(void)
{
    if (verbose) puts("STUB: IOBluetoothL2CAPChannelRegisterIncomingEventListenerWithConfiguration called");
    return NULL;
}

void* IOBluetoothL2CAPChannelWrite(void)
{
    if (verbose) puts("STUB: IOBluetoothL2CAPChannelWrite called");
    return NULL;
}

void* IOBluetoothL2CAPChannelWriteAsync(void)
{
    if (verbose) puts("STUB: IOBluetoothL2CAPChannelWriteAsync called");
    return NULL;
}

void* IOBluetoothLocalDeviceAvailable(void)
{
    if (verbose) puts("STUB: IOBluetoothLocalDeviceAvailable called");
    return NULL;
}

void* IOBluetoothLocalDeviceGetConfigState(void)
{
    if (verbose) puts("STUB: IOBluetoothLocalDeviceGetConfigState called");
    return NULL;
}

void* IOBluetoothLocalDeviceGetPowerState(void)
{
    if (verbose) puts("STUB: IOBluetoothLocalDeviceGetPowerState called");
    return NULL;
}

void* IOBluetoothLocalDeviceGetUSBProductID(void)
{
    if (verbose) puts("STUB: IOBluetoothLocalDeviceGetUSBProductID called");
    return NULL;
}

void* IOBluetoothLocalDeviceGetUSBVendorID(void)
{
    if (verbose) puts("STUB: IOBluetoothLocalDeviceGetUSBVendorID called");
    return NULL;
}

void* IOBluetoothLocalDeviceReadName(void)
{
    if (verbose) puts("STUB: IOBluetoothLocalDeviceReadName called");
    return NULL;
}

void* IOBluetoothLocalDeviceSetDiscoverable(void)
{
    if (verbose) puts("STUB: IOBluetoothLocalDeviceSetDiscoverable called");
    return NULL;
}

void* IOBluetoothLocalDeviceSetPowerState(void)
{
    if (verbose) puts("STUB: IOBluetoothLocalDeviceSetPowerState called");
    return NULL;
}

void* IOBluetoothLocalDeviceSupportsPowerOff(void)
{
    if (verbose) puts("STUB: IOBluetoothLocalDeviceSupportsPowerOff called");
    return NULL;
}

void* IOBluetoothLogHelper(void)
{
    if (verbose) puts("STUB: IOBluetoothLogHelper called");
    return NULL;
}

void* IOBluetoothNSStringFromDeviceAddress(void)
{
    if (verbose) puts("STUB: IOBluetoothNSStringFromDeviceAddress called");
    return NULL;
}

void* IOBluetoothNSStringFromDeviceAddressColon(void)
{
    if (verbose) puts("STUB: IOBluetoothNSStringFromDeviceAddressColon called");
    return NULL;
}

void* IOBluetoothNSStringToDeviceAddress(void)
{
    if (verbose) puts("STUB: IOBluetoothNSStringToDeviceAddress called");
    return NULL;
}

void* IOBluetoothNotificationLibAddHCIControllerRunLoopNotification(void)
{
    if (verbose) puts("STUB: IOBluetoothNotificationLibAddHCIControllerRunLoopNotification called");
    return NULL;
}

void* IOBluetoothNotificationLibAddNotificationForClassName(void)
{
    if (verbose) puts("STUB: IOBluetoothNotificationLibAddNotificationForClassName called");
    return NULL;
}

void* IOBluetoothNotificationLibCheckForExistingHCIControllersAndNotify(void)
{
    if (verbose) puts("STUB: IOBluetoothNotificationLibCheckForExistingHCIControllersAndNotify called");
    return NULL;
}

void* IOBluetoothNotificationLibDoHCIClientNotification(void)
{
    if (verbose) puts("STUB: IOBluetoothNotificationLibDoHCIClientNotification called");
    return NULL;
}

void* IOBluetoothNotificationLibHCIControllerInterestedCallback(void)
{
    if (verbose) puts("STUB: IOBluetoothNotificationLibHCIControllerInterestedCallback called");
    return NULL;
}

void* IOBluetoothNotificationLibHCIControllerPublishedCallback(void)
{
    if (verbose) puts("STUB: IOBluetoothNotificationLibHCIControllerPublishedCallback called");
    return NULL;
}

void* IOBluetoothNotificationLibHCIRawEventHandler(void)
{
    if (verbose) puts("STUB: IOBluetoothNotificationLibHCIRawEventHandler called");
    return NULL;
}

void* IOBluetoothNotificationLibHCIRawEventsSetup(void)
{
    if (verbose) puts("STUB: IOBluetoothNotificationLibHCIRawEventsSetup called");
    return NULL;
}

void* IOBluetoothNotificationLibNotificationCreate(void)
{
    if (verbose) puts("STUB: IOBluetoothNotificationLibNotificationCreate called");
    return NULL;
}

void* IOBluetoothNotificationLibNotificationDelete(void)
{
    if (verbose) puts("STUB: IOBluetoothNotificationLibNotificationDelete called");
    return NULL;
}

void* IOBluetoothNotificationLibSetup(void)
{
    if (verbose) puts("STUB: IOBluetoothNotificationLibSetup called");
    return NULL;
}

void* IOBluetoothNumberOfAvailableHIDDevices(void)
{
    if (verbose) puts("STUB: IOBluetoothNumberOfAvailableHIDDevices called");
    return NULL;
}

void* IOBluetoothNumberOfKeyboardHIDDevices(void)
{
    if (verbose) puts("STUB: IOBluetoothNumberOfKeyboardHIDDevices called");
    return NULL;
}

void* IOBluetoothNumberOfPointingHIDDevices(void)
{
    if (verbose) puts("STUB: IOBluetoothNumberOfPointingHIDDevices called");
    return NULL;
}

void* IOBluetoothNumberOfTabletHIDDevices(void)
{
    if (verbose) puts("STUB: IOBluetoothNumberOfTabletHIDDevices called");
    return NULL;
}

void* IOBluetoothOBEXSessionCreateWithIOBluetoothDeviceRefAndChannelNumber(void)
{
    if (verbose) puts("STUB: IOBluetoothOBEXSessionCreateWithIOBluetoothDeviceRefAndChannelNumber called");
    return NULL;
}

void* IOBluetoothOBEXSessionCreateWithIOBluetoothSDPServiceRecordRef(void)
{
    if (verbose) puts("STUB: IOBluetoothOBEXSessionCreateWithIOBluetoothSDPServiceRecordRef called");
    return NULL;
}

void* IOBluetoothOBEXSessionCreateWithIncomingIOBluetoothRFCOMMChannel(void)
{
    if (verbose) puts("STUB: IOBluetoothOBEXSessionCreateWithIncomingIOBluetoothRFCOMMChannel called");
    return NULL;
}

void* IOBluetoothOBEXSessionOpenTransportConnection(void)
{
    if (verbose) puts("STUB: IOBluetoothOBEXSessionOpenTransportConnection called");
    return NULL;
}

void* IOBluetoothOSLogHelper(void)
{
    if (verbose) puts("STUB: IOBluetoothOSLogHelper called");
    return NULL;
}

void* IOBluetoothObjectRelease(void)
{
    if (verbose) puts("STUB: IOBluetoothObjectRelease called");
    return NULL;
}

void* IOBluetoothObjectRetain(void)
{
    if (verbose) puts("STUB: IOBluetoothObjectRetain called");
    return NULL;
}

void* IOBluetoothObjectServiceTerminated(void)
{
    if (verbose) puts("STUB: IOBluetoothObjectServiceTerminated called");
    return NULL;
}

void* IOBluetoothPackData(void)
{
    if (verbose) puts("STUB: IOBluetoothPackData called");
    return NULL;
}

void* IOBluetoothPackDataList(void)
{
    if (verbose) puts("STUB: IOBluetoothPackDataList called");
    return NULL;
}

void* IOBluetoothPreferenceAddFavoriteAddressString(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceAddFavoriteAddressString called");
    return NULL;
}

void* IOBluetoothPreferenceBuildPrefsServerConnection(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceBuildPrefsServerConnection called");
    return NULL;
}

void* IOBluetoothPreferenceGetAutoSeekForKeyboard(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceGetAutoSeekForKeyboard called");
    return NULL;
}

void* IOBluetoothPreferenceGetAutoSeekForPointingDevice(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceGetAutoSeekForPointingDevice called");
    return NULL;
}

void* IOBluetoothPreferenceGetBrowseConnectionHandling(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceGetBrowseConnectionHandling called");
    return NULL;
}

void* IOBluetoothPreferenceGetControllerPowerState(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceGetControllerPowerState called");
    return NULL;
}

void* IOBluetoothPreferenceGetDiscoverableState(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceGetDiscoverableState called");
    return NULL;
}

void* IOBluetoothPreferenceGetFTPRequiresPairing(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceGetFTPRequiresPairing called");
    return NULL;
}

void* IOBluetoothPreferenceGetFavoriteAddressStrings(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceGetFavoriteAddressStrings called");
    return NULL;
}

void* IOBluetoothPreferenceGetOBEXFTPRootFolderLocation(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceGetOBEXFTPRootFolderLocation called");
    return NULL;
}

void* IOBluetoothPreferenceGetOBEXFileExchangeSaveToLocation(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceGetOBEXFileExchangeSaveToLocation called");
    return NULL;
}

void* IOBluetoothPreferenceGetOBEXFileHandling(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceGetOBEXFileHandling called");
    return NULL;
}

void* IOBluetoothPreferenceGetOBEXFileTransferEnabled(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceGetOBEXFileTransferEnabled called");
    return NULL;
}

void* IOBluetoothPreferenceGetOBEXOtherDataDisposition(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceGetOBEXOtherDataDisposition called");
    return NULL;
}

void* IOBluetoothPreferenceGetOPPRequiresPairing(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceGetOPPRequiresPairing called");
    return NULL;
}

void* IOBluetoothPreferenceGetRemoteWakeEnabled(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceGetRemoteWakeEnabled called");
    return NULL;
}

void* IOBluetoothPreferenceGetServicesEnabled(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceGetServicesEnabled called");
    return NULL;
}

void* IOBluetoothPreferenceIsFavoriteAddressString(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceIsFavoriteAddressString called");
    return NULL;
}

void* IOBluetoothPreferenceRemoveFavoriteAddressString(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceRemoveFavoriteAddressString called");
    return NULL;
}

void* IOBluetoothPreferenceSetAutoSeekForKeyboard(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceSetAutoSeekForKeyboard called");
    return NULL;
}

void* IOBluetoothPreferenceSetAutoSeekForPointingDevice(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceSetAutoSeekForPointingDevice called");
    return NULL;
}

void* IOBluetoothPreferenceSetBluetoothPrefPaneOpen(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceSetBluetoothPrefPaneOpen called");
    return NULL;
}

void* IOBluetoothPreferenceSetBrowseConnectionHandling(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceSetBrowseConnectionHandling called");
    return NULL;
}

void* IOBluetoothPreferenceSetControllerPowerState(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceSetControllerPowerState called");
    return NULL;
}

void* IOBluetoothPreferenceSetDiscoverableState(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceSetDiscoverableState called");
    return NULL;
}

void* IOBluetoothPreferenceSetFTPRequiresPairing(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceSetFTPRequiresPairing called");
    return NULL;
}

void* IOBluetoothPreferenceSetOBEXFTPRootFolderLocation(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceSetOBEXFTPRootFolderLocation called");
    return NULL;
}

void* IOBluetoothPreferenceSetOBEXFileExchangeSaveToLocation(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceSetOBEXFileExchangeSaveToLocation called");
    return NULL;
}

void* IOBluetoothPreferenceSetOBEXFileHandling(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceSetOBEXFileHandling called");
    return NULL;
}

void* IOBluetoothPreferenceSetOBEXFileTransferEnabled(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceSetOBEXFileTransferEnabled called");
    return NULL;
}

void* IOBluetoothPreferenceSetOBEXOtherDataDisposition(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceSetOBEXOtherDataDisposition called");
    return NULL;
}

void* IOBluetoothPreferenceSetOPPRequiresPairing(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceSetOPPRequiresPairing called");
    return NULL;
}

void* IOBluetoothPreferenceSetRemoteWakeEnabled(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceSetRemoteWakeEnabled called");
    return NULL;
}

void* IOBluetoothPreferenceSetServicesEnabled(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferenceSetServicesEnabled called");
    return NULL;
}

void* IOBluetoothPreferencesAvailable(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferencesAvailable called");
    return NULL;
}

void* IOBluetoothPreferencesGetValueForKey(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferencesGetValueForKey called");
    return NULL;
}

void* IOBluetoothPreferencesSetValueForKey(void)
{
    if (verbose) puts("STUB: IOBluetoothPreferencesSetValueForKey called");
    return NULL;
}

void* IOBluetoothRFCOMMChannelIsInitiatorLocal(void)
{
    if (verbose) puts("STUB: IOBluetoothRFCOMMChannelIsInitiatorLocal called");
    return NULL;
}

void* IOBluetoothRFCOMMChannelRegisterForChannelCloseNotification(void)
{
    if (verbose) puts("STUB: IOBluetoothRFCOMMChannelRegisterForChannelCloseNotification called");
    return NULL;
}

void* IOBluetoothRecentDevices(void)
{
    if (verbose) puts("STUB: IOBluetoothRecentDevices called");
    return NULL;
}

void* IOBluetoothRegisterForDeviceConnectNotifications(void)
{
    if (verbose) puts("STUB: IOBluetoothRegisterForDeviceConnectNotifications called");
    return NULL;
}

void* IOBluetoothRegisterForFilteredL2CAPChannelOpenNotifications(void)
{
    if (verbose) puts("STUB: IOBluetoothRegisterForFilteredL2CAPChannelOpenNotifications called");
    return NULL;
}

void* IOBluetoothRegisterForFilteredRFCOMMChannelOpenNotifications(void)
{
    if (verbose) puts("STUB: IOBluetoothRegisterForFilteredRFCOMMChannelOpenNotifications called");
    return NULL;
}

void* IOBluetoothRegisterForL2CAPChannelOpenNotifications(void)
{
    if (verbose) puts("STUB: IOBluetoothRegisterForL2CAPChannelOpenNotifications called");
    return NULL;
}

void* IOBluetoothRegisterForNotifications(void)
{
    if (verbose) puts("STUB: IOBluetoothRegisterForNotifications called");
    return NULL;
}

void* IOBluetoothRegisterForRFCOMMChannelOpenNotifications(void)
{
    if (verbose) puts("STUB: IOBluetoothRegisterForRFCOMMChannelOpenNotifications called");
    return NULL;
}

void* IOBluetoothRemoveAllowedIncomingL2CAPChannel(void)
{
    if (verbose) puts("STUB: IOBluetoothRemoveAllowedIncomingL2CAPChannel called");
    return NULL;
}

void* IOBluetoothRemoveAllowedIncomingRFCOMMChannel(void)
{
    if (verbose) puts("STUB: IOBluetoothRemoveAllowedIncomingRFCOMMChannel called");
    return NULL;
}

void* IOBluetoothRemoveCachedValue(void)
{
    if (verbose) puts("STUB: IOBluetoothRemoveCachedValue called");
    return NULL;
}

void* IOBluetoothRemoveConfiguredDevice(void)
{
    if (verbose) puts("STUB: IOBluetoothRemoveConfiguredDevice called");
    return NULL;
}

void* IOBluetoothRemoveIgnoredHIDDevice(void)
{
    if (verbose) puts("STUB: IOBluetoothRemoveIgnoredHIDDevice called");
    return NULL;
}

void* IOBluetoothRemoveLinkKeyForDevice(void)
{
    if (verbose) puts("STUB: IOBluetoothRemoveLinkKeyForDevice called");
    return NULL;
}

void* IOBluetoothRemovePANDevice(void)
{
    if (verbose) puts("STUB: IOBluetoothRemovePANDevice called");
    return NULL;
}

void* IOBluetoothRemoveRegistrationForNotifications(void)
{
    if (verbose) puts("STUB: IOBluetoothRemoveRegistrationForNotifications called");
    return NULL;
}

void* IOBluetoothRemoveSCOAudioDevice(void)
{
    if (verbose) puts("STUB: IOBluetoothRemoveSCOAudioDevice called");
    return NULL;
}

void* IOBluetoothRemoveServiceWithRecordHandle(void)
{
    if (verbose) puts("STUB: IOBluetoothRemoveServiceWithRecordHandle called");
    return NULL;
}

void* IOBluetoothRingBufferInit(void)
{
    if (verbose) puts("STUB: IOBluetoothRingBufferInit called");
    return NULL;
}

void* IOBluetoothRingBufferRead(void)
{
    if (verbose) puts("STUB: IOBluetoothRingBufferRead called");
    return NULL;
}

void* IOBluetoothRingBufferReadAtOffset(void)
{
    if (verbose) puts("STUB: IOBluetoothRingBufferReadAtOffset called");
    return NULL;
}

void* IOBluetoothRingBufferRelease(void)
{
    if (verbose) puts("STUB: IOBluetoothRingBufferRelease called");
    return NULL;
}

void* IOBluetoothRingBufferWrite(void)
{
    if (verbose) puts("STUB: IOBluetoothRingBufferWrite called");
    return NULL;
}

void* IOBluetoothRingBufferWriteAtOffset(void)
{
    if (verbose) puts("STUB: IOBluetoothRingBufferWriteAtOffset called");
    return NULL;
}

void* IOBluetoothSDPDataElementContainsDataElement(void)
{
    if (verbose) puts("STUB: IOBluetoothSDPDataElementContainsDataElement called");
    return NULL;
}

void* IOBluetoothSDPDataElementGetArrayValue(void)
{
    if (verbose) puts("STUB: IOBluetoothSDPDataElementGetArrayValue called");
    return NULL;
}

void* IOBluetoothSDPDataElementGetDataValue(void)
{
    if (verbose) puts("STUB: IOBluetoothSDPDataElementGetDataValue called");
    return NULL;
}

void* IOBluetoothSDPDataElementGetNumberValue(void)
{
    if (verbose) puts("STUB: IOBluetoothSDPDataElementGetNumberValue called");
    return NULL;
}

void* IOBluetoothSDPDataElementGetSize(void)
{
    if (verbose) puts("STUB: IOBluetoothSDPDataElementGetSize called");
    return NULL;
}

void* IOBluetoothSDPDataElementGetSizeDescriptor(void)
{
    if (verbose) puts("STUB: IOBluetoothSDPDataElementGetSizeDescriptor called");
    return NULL;
}

void* IOBluetoothSDPDataElementGetStringValue(void)
{
    if (verbose) puts("STUB: IOBluetoothSDPDataElementGetStringValue called");
    return NULL;
}

void* IOBluetoothSDPDataElementGetTypeDescriptor(void)
{
    if (verbose) puts("STUB: IOBluetoothSDPDataElementGetTypeDescriptor called");
    return NULL;
}

void* IOBluetoothSDPDataElementGetUUIDValue(void)
{
    if (verbose) puts("STUB: IOBluetoothSDPDataElementGetUUIDValue called");
    return NULL;
}

void* IOBluetoothSDPDataElementIsEqualToDataElement(void)
{
    if (verbose) puts("STUB: IOBluetoothSDPDataElementIsEqualToDataElement called");
    return NULL;
}

void* IOBluetoothSDPServiceRecordGetAttributeDataElement(void)
{
    if (verbose) puts("STUB: IOBluetoothSDPServiceRecordGetAttributeDataElement called");
    return NULL;
}

void* IOBluetoothSDPUUIDCreateUUID16(void)
{
    if (verbose) puts("STUB: IOBluetoothSDPUUIDCreateUUID16 called");
    return NULL;
}

void* IOBluetoothSDPUUIDGetBytes(void)
{
    if (verbose) puts("STUB: IOBluetoothSDPUUIDGetBytes called");
    return NULL;
}

void* IOBluetoothSDPUUIDGetLength(void)
{
    if (verbose) puts("STUB: IOBluetoothSDPUUIDGetLength called");
    return NULL;
}

void* IOBluetoothSDPUUIDIsEqualToUUID(void)
{
    if (verbose) puts("STUB: IOBluetoothSDPUUIDIsEqualToUUID called");
    return NULL;
}

void* IOBluetoothSecureBluetooth(void)
{
    if (verbose) puts("STUB: IOBluetoothSecureBluetooth called");
    return NULL;
}

void* IOBluetoothSelectBTPD(void)
{
    if (verbose) puts("STUB: IOBluetoothSelectBTPD called");
    return NULL;
}

void* IOBluetoothSelectBTPU(void)
{
    if (verbose) puts("STUB: IOBluetoothSelectBTPU called");
    return NULL;
}

void* IOBluetoothSelectBTRS(void)
{
    if (verbose) puts("STUB: IOBluetoothSelectBTRS called");
    return NULL;
}

void* IOBluetoothSelectReset(void)
{
    if (verbose) puts("STUB: IOBluetoothSelectReset called");
    return NULL;
}

void* IOBluetoothSetPANConnectionState(void)
{
    if (verbose) puts("STUB: IOBluetoothSetPANConnectionState called");
    return NULL;
}

void* IOBluetoothSetRunInactivityTimer(void)
{
    if (verbose) puts("STUB: IOBluetoothSetRunInactivityTimer called");
    return NULL;
}

void* IOBluetoothSetRunInactivityTimerInNVRAM(void)
{
    if (verbose) puts("STUB: IOBluetoothSetRunInactivityTimerInNVRAM called");
    return NULL;
}

void* IOBluetoothShouldRunInactivityTimer(void)
{
    if (verbose) puts("STUB: IOBluetoothShouldRunInactivityTimer called");
    return NULL;
}

void* IOBluetoothSystemHasOnlyBluetoothPointingDevices(void)
{
    if (verbose) puts("STUB: IOBluetoothSystemHasOnlyBluetoothPointingDevices called");
    return NULL;
}

void* IOBluetoothSystemModelIdentifier(void)
{
    if (verbose) puts("STUB: IOBluetoothSystemModelIdentifier called");
    return NULL;
}

void* IOBluetoothTearDownPreferencesServer(void)
{
    if (verbose) puts("STUB: IOBluetoothTearDownPreferencesServer called");
    return NULL;
}

void* IOBluetoothUnpackData(void)
{
    if (verbose) puts("STUB: IOBluetoothUnpackData called");
    return NULL;
}

void* IOBluetoothUnpackDataList(void)
{
    if (verbose) puts("STUB: IOBluetoothUnpackDataList called");
    return NULL;
}

void* IOBluetoothUnregisterHIDDevice(void)
{
    if (verbose) puts("STUB: IOBluetoothUnregisterHIDDevice called");
    return NULL;
}

void* IOBluetoothUpdateSystemActivity(void)
{
    if (verbose) puts("STUB: IOBluetoothUpdateSystemActivity called");
    return NULL;
}

void* IOBluetoothUserNotificationUnregister(void)
{
    if (verbose) puts("STUB: IOBluetoothUserNotificationUnregister called");
    return NULL;
}

void* ItemAtPathNeedsArchiving(void)
{
    if (verbose) puts("STUB: ItemAtPathNeedsArchiving called");
    return NULL;
}

void* NSStringPathResolveAliasesAndLinks(void)
{
    if (verbose) puts("STUB: NSStringPathResolveAliasesAndLinks called");
    return NULL;
}

void* NSStringPathToCFURL(void)
{
    if (verbose) puts("STUB: NSStringPathToCFURL called");
    return NULL;
}

void* NSStringPathToFSRef(void)
{
    if (verbose) puts("STUB: NSStringPathToFSRef called");
    return NULL;
}

void* OBEXAddApplicationParameterHeader(void)
{
    if (verbose) puts("STUB: OBEXAddApplicationParameterHeader called");
    return NULL;
}

void* OBEXAddAuthorizationChallengeHeader(void)
{
    if (verbose) puts("STUB: OBEXAddAuthorizationChallengeHeader called");
    return NULL;
}

void* OBEXAddAuthorizationResponseHeader(void)
{
    if (verbose) puts("STUB: OBEXAddAuthorizationResponseHeader called");
    return NULL;
}

void* OBEXAddBodyHeader(void)
{
    if (verbose) puts("STUB: OBEXAddBodyHeader called");
    return NULL;
}

void* OBEXAddByteSequenceHeader(void)
{
    if (verbose) puts("STUB: OBEXAddByteSequenceHeader called");
    return NULL;
}

void* OBEXAddConnectionIDHeader(void)
{
    if (verbose) puts("STUB: OBEXAddConnectionIDHeader called");
    return NULL;
}

void* OBEXAddCountHeader(void)
{
    if (verbose) puts("STUB: OBEXAddCountHeader called");
    return NULL;
}

void* OBEXAddDescriptionHeader(void)
{
    if (verbose) puts("STUB: OBEXAddDescriptionHeader called");
    return NULL;
}

void* OBEXAddHTTPHeader(void)
{
    if (verbose) puts("STUB: OBEXAddHTTPHeader called");
    return NULL;
}

void* OBEXAddImageDescriptorHeader(void)
{
    if (verbose) puts("STUB: OBEXAddImageDescriptorHeader called");
    return NULL;
}

void* OBEXAddImageHandleHeader(void)
{
    if (verbose) puts("STUB: OBEXAddImageHandleHeader called");
    return NULL;
}

void* OBEXAddLengthHeader(void)
{
    if (verbose) puts("STUB: OBEXAddLengthHeader called");
    return NULL;
}

void* OBEXAddNameHeader(void)
{
    if (verbose) puts("STUB: OBEXAddNameHeader called");
    return NULL;
}

void* OBEXAddObjectClassHeader(void)
{
    if (verbose) puts("STUB: OBEXAddObjectClassHeader called");
    return NULL;
}

void* OBEXAddTargetHeader(void)
{
    if (verbose) puts("STUB: OBEXAddTargetHeader called");
    return NULL;
}

void* OBEXAddTime4ByteHeader(void)
{
    if (verbose) puts("STUB: OBEXAddTime4ByteHeader called");
    return NULL;
}

void* OBEXAddTimeISOHeader(void)
{
    if (verbose) puts("STUB: OBEXAddTimeISOHeader called");
    return NULL;
}

void* OBEXAddTypeHeader(void)
{
    if (verbose) puts("STUB: OBEXAddTypeHeader called");
    return NULL;
}

void* OBEXAddUserDefinedHeader(void)
{
    if (verbose) puts("STUB: OBEXAddUserDefinedHeader called");
    return NULL;
}

void* OBEXAddWhoHeader(void)
{
    if (verbose) puts("STUB: OBEXAddWhoHeader called");
    return NULL;
}

void* OBEXCreateVCalendar(void)
{
    if (verbose) puts("STUB: OBEXCreateVCalendar called");
    return NULL;
}

void* OBEXCreateVCard(void)
{
    if (verbose) puts("STUB: OBEXCreateVCard called");
    return NULL;
}

void* OBEXCreateVEvent(void)
{
    if (verbose) puts("STUB: OBEXCreateVEvent called");
    return NULL;
}

void* OBEXFolderListingCreateFromFile(void)
{
    if (verbose) puts("STUB: OBEXFolderListingCreateFromFile called");
    return NULL;
}

void* OBEXFolderListingGetObjectArray(void)
{
    if (verbose) puts("STUB: OBEXFolderListingGetObjectArray called");
    return NULL;
}

void* OBEXFolderListingPrintDictionary(void)
{
    if (verbose) puts("STUB: OBEXFolderListingPrintDictionary called");
    return NULL;
}

void* OBEXGetHeaders(void)
{
    if (verbose) puts("STUB: OBEXGetHeaders called");
    return NULL;
}

void* OBEXHeadersToBytes(void)
{
    if (verbose) puts("STUB: OBEXHeadersToBytes called");
    return NULL;
}

void* OBEXPatchXMLData(void)
{
    if (verbose) puts("STUB: OBEXPatchXMLData called");
    return NULL;
}

void* OBEXSessionAbort(void)
{
    if (verbose) puts("STUB: OBEXSessionAbort called");
    return NULL;
}

void* OBEXSessionAbortResponse(void)
{
    if (verbose) puts("STUB: OBEXSessionAbortResponse called");
    return NULL;
}

void* OBEXSessionConnect(void)
{
    if (verbose) puts("STUB: OBEXSessionConnect called");
    return NULL;
}

void* OBEXSessionConnectResponse(void)
{
    if (verbose) puts("STUB: OBEXSessionConnectResponse called");
    return NULL;
}

void* OBEXSessionDelete(void)
{
    if (verbose) puts("STUB: OBEXSessionDelete called");
    return NULL;
}

void* OBEXSessionDisconnect(void)
{
    if (verbose) puts("STUB: OBEXSessionDisconnect called");
    return NULL;
}

void* OBEXSessionDisconnectResponse(void)
{
    if (verbose) puts("STUB: OBEXSessionDisconnectResponse called");
    return NULL;
}

void* OBEXSessionGet(void)
{
    if (verbose) puts("STUB: OBEXSessionGet called");
    return NULL;
}

void* OBEXSessionGetAvailableCommandPayloadLength(void)
{
    if (verbose) puts("STUB: OBEXSessionGetAvailableCommandPayloadLength called");
    return NULL;
}

void* OBEXSessionGetAvailableCommandResponsePayloadLength(void)
{
    if (verbose) puts("STUB: OBEXSessionGetAvailableCommandResponsePayloadLength called");
    return NULL;
}

void* OBEXSessionGetMaxPacketLength(void)
{
    if (verbose) puts("STUB: OBEXSessionGetMaxPacketLength called");
    return NULL;
}

void* OBEXSessionGetResponse(void)
{
    if (verbose) puts("STUB: OBEXSessionGetResponse called");
    return NULL;
}

void* OBEXSessionHasOpenOBEXConnection(void)
{
    if (verbose) puts("STUB: OBEXSessionHasOpenOBEXConnection called");
    return NULL;
}

void* OBEXSessionPut(void)
{
    if (verbose) puts("STUB: OBEXSessionPut called");
    return NULL;
}

void* OBEXSessionPutResponse(void)
{
    if (verbose) puts("STUB: OBEXSessionPutResponse called");
    return NULL;
}

void* OBEXSessionSetPath(void)
{
    if (verbose) puts("STUB: OBEXSessionSetPath called");
    return NULL;
}

void* OBEXSessionSetPathResponse(void)
{
    if (verbose) puts("STUB: OBEXSessionSetPathResponse called");
    return NULL;
}

void* OBEXSessionSetServerCallback(void)
{
    if (verbose) puts("STUB: OBEXSessionSetServerCallback called");
    return NULL;
}

void* OBEXSetDebugLogVerbosity(void)
{
    if (verbose) puts("STUB: OBEXSetDebugLogVerbosity called");
    return NULL;
}

void* PStringAppend(void)
{
    if (verbose) puts("STUB: PStringAppend called");
    return NULL;
}

void* PStringAppendChar(void)
{
    if (verbose) puts("STUB: PStringAppendChar called");
    return NULL;
}

void* PStringAppendNumber(void)
{
    if (verbose) puts("STUB: PStringAppendNumber called");
    return NULL;
}

void* PStringAppendText(void)
{
    if (verbose) puts("STUB: PStringAppendText called");
    return NULL;
}

void* PStringCopy(void)
{
    if (verbose) puts("STUB: PStringCopy called");
    return NULL;
}

void* PStringToUnicodeSimple(void)
{
    if (verbose) puts("STUB: PStringToUnicodeSimple called");
    return NULL;
}

void* PackData(void)
{
    if (verbose) puts("STUB: PackData called");
    return NULL;
}

void* PackDataList(void)
{
    if (verbose) puts("STUB: PackDataList called");
    return NULL;
}

void* ParseOBEXPacket(void)
{
    if (verbose) puts("STUB: ParseOBEXPacket called");
    return NULL;
}

void* ParseServerConnectResponse(void)
{
    if (verbose) puts("STUB: ParseServerConnectResponse called");
    return NULL;
}

void* PostNotification(void)
{
    if (verbose) puts("STUB: PostNotification called");
    return NULL;
}

void* QueueNonEmptyCallback(void)
{
    if (verbose) puts("STUB: QueueNonEmptyCallback called");
    return NULL;
}

void* RegistryObject_ConnectedCallback(void)
{
    if (verbose) puts("STUB: RegistryObject_ConnectedCallback called");
    return NULL;
}

void* RegistryObject_DisconnectedCallback(void)
{
    if (verbose) puts("STUB: RegistryObject_DisconnectedCallback called");
    return NULL;
}

void* ReturnAllCurrentDevices(void)
{
    if (verbose) puts("STUB: ReturnAllCurrentDevices called");
    return NULL;
}

void* ReturnAllCurrentL2Channels(void)
{
    if (verbose) puts("STUB: ReturnAllCurrentL2Channels called");
    return NULL;
}

void* ReturnAllCurrentRFCOMMChannels(void)
{
    if (verbose) puts("STUB: ReturnAllCurrentRFCOMMChannels called");
    return NULL;
}

void* ServiceInterestCallback(void)
{
    if (verbose) puts("STUB: ServiceInterestCallback called");
    return NULL;
}

void* SetupDevicePublicationNotifications(void)
{
    if (verbose) puts("STUB: SetupDevicePublicationNotifications called");
    return NULL;
}

void* StringByRemovingCharactersInString(void)
{
    if (verbose) puts("STUB: StringByRemovingCharactersInString called");
    return NULL;
}

void* SystemPowerNotificationLib_SleepNotificationCallback(void)
{
    if (verbose) puts("STUB: SystemPowerNotificationLib_SleepNotificationCallback called");
    return NULL;
}

void* TextFindExtension(void)
{
    if (verbose) puts("STUB: TextFindExtension called");
    return NULL;
}

void* UInt16ToBCCMD16(void)
{
    if (verbose) puts("STUB: UInt16ToBCCMD16 called");
    return NULL;
}

void* UInt32ToBCCMD32(void)
{
    if (verbose) puts("STUB: UInt32ToBCCMD32 called");
    return NULL;
}

void* UInt8ToBCCMD16(void)
{
    if (verbose) puts("STUB: UInt8ToBCCMD16 called");
    return NULL;
}

void* UTCDateTimeToAppleSingleSeconds(void)
{
    if (verbose) puts("STUB: UTCDateTimeToAppleSingleSeconds called");
    return NULL;
}

void* UTF8ToUnicode(void)
{
    if (verbose) puts("STUB: UTF8ToUnicode called");
    return NULL;
}

void* UnicodeAppend(void)
{
    if (verbose) puts("STUB: UnicodeAppend called");
    return NULL;
}

void* UnicodeFindFilenameExtension(void)
{
    if (verbose) puts("STUB: UnicodeFindFilenameExtension called");
    return NULL;
}

void* UnicodeIsAlpha(void)
{
    if (verbose) puts("STUB: UnicodeIsAlpha called");
    return NULL;
}

void* UnicodeIsPrint(void)
{
    if (verbose) puts("STUB: UnicodeIsPrint called");
    return NULL;
}

void* UnicodeReverseCharSearch(void)
{
    if (verbose) puts("STUB: UnicodeReverseCharSearch called");
    return NULL;
}

void* UnicodeToUTF8(void)
{
    if (verbose) puts("STUB: UnicodeToUTF8 called");
    return NULL;
}

void* UnpackData(void)
{
    if (verbose) puts("STUB: UnpackData called");
    return NULL;
}

void* UnpackDataList(void)
{
    if (verbose) puts("STUB: UnpackDataList called");
    return NULL;
}

void* UnpackExtendedInquiryResponse(void)
{
    if (verbose) puts("STUB: UnpackExtendedInquiryResponse called");
    return NULL;
}

void* _IOBluetoothRingBufferBytesAvailable(void)
{
    if (verbose) puts("STUB: _IOBluetoothRingBufferBytesAvailable called");
    return NULL;
}

void* _IOBluetoothRingBufferBytesInBuffer(void)
{
    if (verbose) puts("STUB: _IOBluetoothRingBufferBytesInBuffer called");
    return NULL;
}

void* _IOBluetoothRingBufferSequentialBytesAvailable(void)
{
    if (verbose) puts("STUB: _IOBluetoothRingBufferSequentialBytesAvailable called");
    return NULL;
}

void* _IOBluetoothRingBufferSequentialBytesInBuffer(void)
{
    if (verbose) puts("STUB: _IOBluetoothRingBufferSequentialBytesInBuffer called");
    return NULL;
}

void* _recurseElements(void)
{
    if (verbose) puts("STUB: _recurseElements called");
    return NULL;
}

void* btNotificationAddNotificationForClass(void)
{
    if (verbose) puts("STUB: btNotificationAddNotificationForClass called");
    return NULL;
}

void* btNotificationDevicePublished(void)
{
    if (verbose) puts("STUB: btNotificationDevicePublished called");
    return NULL;
}

void* btNotificationDeviceTerminated(void)
{
    if (verbose) puts("STUB: btNotificationDeviceTerminated called");
    return NULL;
}

void* btNotificationL2ChannelPublished(void)
{
    if (verbose) puts("STUB: btNotificationL2ChannelPublished called");
    return NULL;
}

void* btNotificationL2ChannelTerminated(void)
{
    if (verbose) puts("STUB: btNotificationL2ChannelTerminated called");
    return NULL;
}

void* btNotificationRFCOMMChannelPublished(void)
{
    if (verbose) puts("STUB: btNotificationRFCOMMChannelPublished called");
    return NULL;
}

void* btNotificationRFCOMMChannelTerminated(void)
{
    if (verbose) puts("STUB: btNotificationRFCOMMChannelTerminated called");
    return NULL;
}

void* findServiceWithObjectID(void)
{
    if (verbose) puts("STUB: findServiceWithObjectID called");
    return NULL;
}

void* internetSharingIsEnabledForBluetooth(void)
{
    if (verbose) puts("STUB: internetSharingIsEnabledForBluetooth called");
    return NULL;
}

void* memcpySwap(void)
{
    if (verbose) puts("STUB: memcpySwap called");
    return NULL;
}

void* memrchr(void)
{
    if (verbose) puts("STUB: memrchr called");
    return NULL;
}

void* newConnectionShowedUp(void)
{
    if (verbose) puts("STUB: newConnectionShowedUp called");
    return NULL;
}

void* sendDaemonXPCMessageWithCFArrayAndReply(void)
{
    if (verbose) puts("STUB: sendDaemonXPCMessageWithCFArrayAndReply called");
    return NULL;
}

void* sendDaemonXPCMessageWithCFObject(void)
{
    if (verbose) puts("STUB: sendDaemonXPCMessageWithCFObject called");
    return NULL;
}

void* sendDaemonXPCMessageWithNSArray(void)
{
    if (verbose) puts("STUB: sendDaemonXPCMessageWithNSArray called");
    return NULL;
}

void* sendDaemonXPCMessageWithReply(void)
{
    if (verbose) puts("STUB: sendDaemonXPCMessageWithReply called");
    return NULL;
}

void* sendDaemonXPCMessageWithReplySync(void)
{
    if (verbose) puts("STUB: sendDaemonXPCMessageWithReplySync called");
    return NULL;
}

void* sendRawHCIRequest(void)
{
    if (verbose) puts("STUB: sendRawHCIRequest called");
    return NULL;
}

void* servicePublished(void)
{
    if (verbose) puts("STUB: servicePublished called");
    return NULL;
}

void* shouldSubmitToSampling(void)
{
    if (verbose) puts("STUB: shouldSubmitToSampling called");
    return NULL;
}

void* systemContinuityCapable(void)
{
    if (verbose) puts("STUB: systemContinuityCapable called");
    return NULL;
}

void* unpackEIRorAdvertisementData(void)
{
    if (verbose) puts("STUB: unpackEIRorAdvertisementData called");
    return NULL;
}

void* waitForMatchingService(void)
{
    if (verbose) puts("STUB: waitForMatchingService called");
    return NULL;
}
