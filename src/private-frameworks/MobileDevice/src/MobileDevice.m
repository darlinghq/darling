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


#include <MobileDevice/MobileDevice.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AFCConnectionClose(void)
{
    if (verbose) puts("STUB: AFCConnectionClose called");
    return NULL;
}

void* AFCConnectionCopyLastErrorInfo(void)
{
    if (verbose) puts("STUB: AFCConnectionCopyLastErrorInfo called");
    return NULL;
}

void* AFCConnectionCreate(void)
{
    if (verbose) puts("STUB: AFCConnectionCreate called");
    return NULL;
}

void* AFCConnectionGetContext(void)
{
    if (verbose) puts("STUB: AFCConnectionGetContext called");
    return NULL;
}

void* AFCConnectionGetFSBlockSize(void)
{
    if (verbose) puts("STUB: AFCConnectionGetFSBlockSize called");
    return NULL;
}

void* AFCConnectionGetIOTimeout(void)
{
    if (verbose) puts("STUB: AFCConnectionGetIOTimeout called");
    return NULL;
}

void* AFCConnectionGetSecureContext(void)
{
    if (verbose) puts("STUB: AFCConnectionGetSecureContext called");
    return NULL;
}

void* AFCConnectionGetSocketBlockSize(void)
{
    if (verbose) puts("STUB: AFCConnectionGetSocketBlockSize called");
    return NULL;
}

void* AFCConnectionGetStatus(void)
{
    if (verbose) puts("STUB: AFCConnectionGetStatus called");
    return NULL;
}

void* AFCConnectionGetTypeID(void)
{
    if (verbose) puts("STUB: AFCConnectionGetTypeID called");
    return NULL;
}

void* AFCConnectionInvalidate(void)
{
    if (verbose) puts("STUB: AFCConnectionInvalidate called");
    return NULL;
}

void* AFCConnectionIsValid(void)
{
    if (verbose) puts("STUB: AFCConnectionIsValid called");
    return NULL;
}

void* AFCConnectionOpen(void)
{
    if (verbose) puts("STUB: AFCConnectionOpen called");
    return NULL;
}

void* AFCConnectionProcessOperation(void)
{
    if (verbose) puts("STUB: AFCConnectionProcessOperation called");
    return NULL;
}

void* AFCConnectionProcessOperations(void)
{
    if (verbose) puts("STUB: AFCConnectionProcessOperations called");
    return NULL;
}

void* AFCConnectionScheduleWithRunLoop(void)
{
    if (verbose) puts("STUB: AFCConnectionScheduleWithRunLoop called");
    return NULL;
}

void* AFCConnectionSetCallBack(void)
{
    if (verbose) puts("STUB: AFCConnectionSetCallBack called");
    return NULL;
}

void* AFCConnectionSetContext(void)
{
    if (verbose) puts("STUB: AFCConnectionSetContext called");
    return NULL;
}

void* AFCConnectionSetDisposeSecureContextOnInvalidate(void)
{
    if (verbose) puts("STUB: AFCConnectionSetDisposeSecureContextOnInvalidate called");
    return NULL;
}

void* AFCConnectionSetFSBlockSize(void)
{
    if (verbose) puts("STUB: AFCConnectionSetFSBlockSize called");
    return NULL;
}

void* AFCConnectionSetIOTimeout(void)
{
    if (verbose) puts("STUB: AFCConnectionSetIOTimeout called");
    return NULL;
}

void* AFCConnectionSetSecureContext(void)
{
    if (verbose) puts("STUB: AFCConnectionSetSecureContext called");
    return NULL;
}

void* AFCConnectionSetSocketBlockSize(void)
{
    if (verbose) puts("STUB: AFCConnectionSetSocketBlockSize called");
    return NULL;
}

void* AFCConnectionSubmitOperation(void)
{
    if (verbose) puts("STUB: AFCConnectionSubmitOperation called");
    return NULL;
}

void* AFCConnectionUnscheduleFromRunLoop(void)
{
    if (verbose) puts("STUB: AFCConnectionUnscheduleFromRunLoop called");
    return NULL;
}

void* AFCCopyErrorString(void)
{
    if (verbose) puts("STUB: AFCCopyErrorString called");
    return NULL;
}

void* AFCCopyPacketTypeString(void)
{
    if (verbose) puts("STUB: AFCCopyPacketTypeString called");
    return NULL;
}

void* AFCDeviceInfoOpen(void)
{
    if (verbose) puts("STUB: AFCDeviceInfoOpen called");
    return NULL;
}

void* AFCDirectoryClose(void)
{
    if (verbose) puts("STUB: AFCDirectoryClose called");
    return NULL;
}

void* AFCDirectoryCreate(void)
{
    if (verbose) puts("STUB: AFCDirectoryCreate called");
    return NULL;
}

void* AFCDirectoryOpen(void)
{
    if (verbose) puts("STUB: AFCDirectoryOpen called");
    return NULL;
}

void* AFCDirectoryRead(void)
{
    if (verbose) puts("STUB: AFCDirectoryRead called");
    return NULL;
}

void* AFCErrorString(void)
{
    if (verbose) puts("STUB: AFCErrorString called");
    return NULL;
}

void* AFCFileDescriptorCreateCloseOperation(void)
{
    if (verbose) puts("STUB: AFCFileDescriptorCreateCloseOperation called");
    return NULL;
}

void* AFCFileDescriptorCreateGetPositionOperation(void)
{
    if (verbose) puts("STUB: AFCFileDescriptorCreateGetPositionOperation called");
    return NULL;
}

void* AFCFileDescriptorCreateLockOperation(void)
{
    if (verbose) puts("STUB: AFCFileDescriptorCreateLockOperation called");
    return NULL;
}

void* AFCFileDescriptorCreateReadAtPositionOperation(void)
{
    if (verbose) puts("STUB: AFCFileDescriptorCreateReadAtPositionOperation called");
    return NULL;
}

void* AFCFileDescriptorCreateReadOperation(void)
{
    if (verbose) puts("STUB: AFCFileDescriptorCreateReadOperation called");
    return NULL;
}

void* AFCFileDescriptorCreateSetImmutableHintOperation(void)
{
    if (verbose) puts("STUB: AFCFileDescriptorCreateSetImmutableHintOperation called");
    return NULL;
}

void* AFCFileDescriptorCreateSetPositionOperation(void)
{
    if (verbose) puts("STUB: AFCFileDescriptorCreateSetPositionOperation called");
    return NULL;
}

void* AFCFileDescriptorCreateSetSizeOperation(void)
{
    if (verbose) puts("STUB: AFCFileDescriptorCreateSetSizeOperation called");
    return NULL;
}

void* AFCFileDescriptorCreateUnlockOperation(void)
{
    if (verbose) puts("STUB: AFCFileDescriptorCreateUnlockOperation called");
    return NULL;
}

void* AFCFileDescriptorCreateWriteAtPositionOperation(void)
{
    if (verbose) puts("STUB: AFCFileDescriptorCreateWriteAtPositionOperation called");
    return NULL;
}

void* AFCFileDescriptorCreateWriteOperation(void)
{
    if (verbose) puts("STUB: AFCFileDescriptorCreateWriteOperation called");
    return NULL;
}

void* AFCFileDescriptorGetTypeID(void)
{
    if (verbose) puts("STUB: AFCFileDescriptorGetTypeID called");
    return NULL;
}

void* AFCFileDescriptorInvalidate(void)
{
    if (verbose) puts("STUB: AFCFileDescriptorInvalidate called");
    return NULL;
}

void* AFCFileDescriptorIsValid(void)
{
    if (verbose) puts("STUB: AFCFileDescriptorIsValid called");
    return NULL;
}

void* AFCFileInfoOpen(void)
{
    if (verbose) puts("STUB: AFCFileInfoOpen called");
    return NULL;
}

void* AFCFileRefClose(void)
{
    if (verbose) puts("STUB: AFCFileRefClose called");
    return NULL;
}

void* AFCFileRefLock(void)
{
    if (verbose) puts("STUB: AFCFileRefLock called");
    return NULL;
}

void* AFCFileRefOpen(void)
{
    if (verbose) puts("STUB: AFCFileRefOpen called");
    return NULL;
}

void* AFCFileRefRead(void)
{
    if (verbose) puts("STUB: AFCFileRefRead called");
    return NULL;
}

void* AFCFileRefSeek(void)
{
    if (verbose) puts("STUB: AFCFileRefSeek called");
    return NULL;
}

void* AFCFileRefSetFileSize(void)
{
    if (verbose) puts("STUB: AFCFileRefSetFileSize called");
    return NULL;
}

void* AFCFileRefTell(void)
{
    if (verbose) puts("STUB: AFCFileRefTell called");
    return NULL;
}

void* AFCFileRefUnlock(void)
{
    if (verbose) puts("STUB: AFCFileRefUnlock called");
    return NULL;
}

void* AFCFileRefWrite(void)
{
    if (verbose) puts("STUB: AFCFileRefWrite called");
    return NULL;
}

void* AFCGetClientVersionString(void)
{
    if (verbose) puts("STUB: AFCGetClientVersionString called");
    return NULL;
}

void* AFCKeyValueClose(void)
{
    if (verbose) puts("STUB: AFCKeyValueClose called");
    return NULL;
}

void* AFCKeyValueRead(void)
{
    if (verbose) puts("STUB: AFCKeyValueRead called");
    return NULL;
}

void* AFCLinkPath(void)
{
    if (verbose) puts("STUB: AFCLinkPath called");
    return NULL;
}

void* AFCLog(void)
{
    if (verbose) puts("STUB: AFCLog called");
    return NULL;
}

void* AFCOperationCopyPacketData(void)
{
    if (verbose) puts("STUB: AFCOperationCopyPacketData called");
    return NULL;
}

void* AFCOperationCreateGetConnectionInfo(void)
{
    if (verbose) puts("STUB: AFCOperationCreateGetConnectionInfo called");
    return NULL;
}

void* AFCOperationCreateGetDeviceInfo(void)
{
    if (verbose) puts("STUB: AFCOperationCreateGetDeviceInfo called");
    return NULL;
}

void* AFCOperationCreateGetFileHash(void)
{
    if (verbose) puts("STUB: AFCOperationCreateGetFileHash called");
    return NULL;
}

void* AFCOperationCreateGetFileHashWithRange(void)
{
    if (verbose) puts("STUB: AFCOperationCreateGetFileHashWithRange called");
    return NULL;
}

void* AFCOperationCreateGetFileInfo(void)
{
    if (verbose) puts("STUB: AFCOperationCreateGetFileInfo called");
    return NULL;
}

void* AFCOperationCreateGetSizeOfPathContents(void)
{
    if (verbose) puts("STUB: AFCOperationCreateGetSizeOfPathContents called");
    return NULL;
}

void* AFCOperationCreateLinkPath(void)
{
    if (verbose) puts("STUB: AFCOperationCreateLinkPath called");
    return NULL;
}

void* AFCOperationCreateMakeDirectory(void)
{
    if (verbose) puts("STUB: AFCOperationCreateMakeDirectory called");
    return NULL;
}

void* AFCOperationCreateOpenFile(void)
{
    if (verbose) puts("STUB: AFCOperationCreateOpenFile called");
    return NULL;
}

void* AFCOperationCreatePacketHeaderDictionary(void)
{
    if (verbose) puts("STUB: AFCOperationCreatePacketHeaderDictionary called");
    return NULL;
}

void* AFCOperationCreateReadDirectory(void)
{
    if (verbose) puts("STUB: AFCOperationCreateReadDirectory called");
    return NULL;
}

void* AFCOperationCreateRemovePath(void)
{
    if (verbose) puts("STUB: AFCOperationCreateRemovePath called");
    return NULL;
}

void* AFCOperationCreateRemovePathAndContents(void)
{
    if (verbose) puts("STUB: AFCOperationCreateRemovePathAndContents called");
    return NULL;
}

void* AFCOperationCreateRenamePath(void)
{
    if (verbose) puts("STUB: AFCOperationCreateRenamePath called");
    return NULL;
}

void* AFCOperationCreateSetConnectionOptions(void)
{
    if (verbose) puts("STUB: AFCOperationCreateSetConnectionOptions called");
    return NULL;
}

void* AFCOperationCreateSetModTime(void)
{
    if (verbose) puts("STUB: AFCOperationCreateSetModTime called");
    return NULL;
}

void* AFCOperationGetContext(void)
{
    if (verbose) puts("STUB: AFCOperationGetContext called");
    return NULL;
}

void* AFCOperationGetResultObject(void)
{
    if (verbose) puts("STUB: AFCOperationGetResultObject called");
    return NULL;
}

void* AFCOperationGetResultStatus(void)
{
    if (verbose) puts("STUB: AFCOperationGetResultStatus called");
    return NULL;
}

void* AFCOperationGetState(void)
{
    if (verbose) puts("STUB: AFCOperationGetState called");
    return NULL;
}

void* AFCOperationGetTypeID(void)
{
    if (verbose) puts("STUB: AFCOperationGetTypeID called");
    return NULL;
}

void* AFCOperationSetContext(void)
{
    if (verbose) puts("STUB: AFCOperationSetContext called");
    return NULL;
}

void* AFCPlatformInitialize(void)
{
    if (verbose) puts("STUB: AFCPlatformInitialize called");
    return NULL;
}

void* AFCRemovePath(void)
{
    if (verbose) puts("STUB: AFCRemovePath called");
    return NULL;
}

void* AFCRenamePath(void)
{
    if (verbose) puts("STUB: AFCRenamePath called");
    return NULL;
}

void* AFCSetLogLevel(void)
{
    if (verbose) puts("STUB: AFCSetLogLevel called");
    return NULL;
}

void* AMAuthInstallApCopyImageTag(void)
{
    if (verbose) puts("STUB: AMAuthInstallApCopyImageTag called");
    return NULL;
}

void* AMAuthInstallApCreateImageProperties(void)
{
    if (verbose) puts("STUB: AMAuthInstallApCreateImageProperties called");
    return NULL;
}

void* AMAuthInstallApCreatePersonalizedResponse(void)
{
    if (verbose) puts("STUB: AMAuthInstallApCreatePersonalizedResponse called");
    return NULL;
}

void* AMAuthInstallApExtractZippedDeviceMap(void)
{
    if (verbose) puts("STUB: AMAuthInstallApExtractZippedDeviceMap called");
    return NULL;
}

void* AMAuthInstallApImg4CopyPayloadType(void)
{
    if (verbose) puts("STUB: AMAuthInstallApImg4CopyPayloadType called");
    return NULL;
}

void* AMAuthInstallApImg4CreateServerRequestDictionary(void)
{
    if (verbose) puts("STUB: AMAuthInstallApImg4CreateServerRequestDictionary called");
    return NULL;
}

void* AMAuthInstallApImg4CreateStitchTicket(void)
{
    if (verbose) puts("STUB: AMAuthInstallApImg4CreateStitchTicket called");
    return NULL;
}

void* AMAuthInstallApImg4ForceLocalSigning(void)
{
    if (verbose) puts("STUB: AMAuthInstallApImg4ForceLocalSigning called");
    return NULL;
}

void* AMAuthInstallApImg4PersonalizeFile(void)
{
    if (verbose) puts("STUB: AMAuthInstallApImg4PersonalizeFile called");
    return NULL;
}

void* AMAuthInstallApImg4SetSepNonce(void)
{
    if (verbose) puts("STUB: AMAuthInstallApImg4SetSepNonce called");
    return NULL;
}

void* AMAuthInstallApSetNonce(void)
{
    if (verbose) puts("STUB: AMAuthInstallApSetNonce called");
    return NULL;
}

void* AMAuthInstallApSetParameters(void)
{
    if (verbose) puts("STUB: AMAuthInstallApSetParameters called");
    return NULL;
}

void* AMAuthInstallCreate(void)
{
    if (verbose) puts("STUB: AMAuthInstallCreate called");
    return NULL;
}

void* AMAuthInstallGetLocalizedStatusString(void)
{
    if (verbose) puts("STUB: AMAuthInstallGetLocalizedStatusString called");
    return NULL;
}

void* AMAuthInstallLogSetHandler(void)
{
    if (verbose) puts("STUB: AMAuthInstallLogSetHandler called");
    return NULL;
}

void* AMAuthInstallRequestSendSync(void)
{
    if (verbose) puts("STUB: AMAuthInstallRequestSendSync called");
    return NULL;
}

void* AMAuthInstallSetSigningServerURL(void)
{
    if (verbose) puts("STUB: AMAuthInstallSetSigningServerURL called");
    return NULL;
}

void* AMAuthInstallSsoEnable(void)
{
    if (verbose) puts("STUB: AMAuthInstallSsoEnable called");
    return NULL;
}

void* AMAuthInstallSsoInitialize(void)
{
    if (verbose) puts("STUB: AMAuthInstallSsoInitialize called");
    return NULL;
}

void* AMAuthInstallSupportCompareStringToInt32(void)
{
    if (verbose) puts("STUB: AMAuthInstallSupportCompareStringToInt32 called");
    return NULL;
}

void* AMAuthInstallSupportCopyDataFromHexString(void)
{
    if (verbose) puts("STUB: AMAuthInstallSupportCopyDataFromHexString called");
    return NULL;
}

void* AMAuthInstallSupportCopyURLToNewTempDirectory(void)
{
    if (verbose) puts("STUB: AMAuthInstallSupportCopyURLToNewTempDirectory called");
    return NULL;
}

void* AMAuthInstallSupportCreateDataFromFileURL(void)
{
    if (verbose) puts("STUB: AMAuthInstallSupportCreateDataFromFileURL called");
    return NULL;
}

void* AMAuthInstallSupportCreateDictionaryFromFileURL(void)
{
    if (verbose) puts("STUB: AMAuthInstallSupportCreateDictionaryFromFileURL called");
    return NULL;
}

void* AMAuthInstallSupportDictionarySetBoolean(void)
{
    if (verbose) puts("STUB: AMAuthInstallSupportDictionarySetBoolean called");
    return NULL;
}

void* AMAuthInstallSupportDictionarySetInteger32(void)
{
    if (verbose) puts("STUB: AMAuthInstallSupportDictionarySetInteger32 called");
    return NULL;
}

void* AMAuthInstallSupportDictionarySetInteger64(void)
{
    if (verbose) puts("STUB: AMAuthInstallSupportDictionarySetInteger64 called");
    return NULL;
}

void* AMAuthInstallSupportFileURLExists(void)
{
    if (verbose) puts("STUB: AMAuthInstallSupportFileURLExists called");
    return NULL;
}

void* AMAuthInstallSupportGetLibraryVersionString(void)
{
    if (verbose) puts("STUB: AMAuthInstallSupportGetLibraryVersionString called");
    return NULL;
}

void* AMAuthInstallSupportMakeDirectory(void)
{
    if (verbose) puts("STUB: AMAuthInstallSupportMakeDirectory called");
    return NULL;
}

void* AMDCopyArrayOfDevicesMatchingQuery(void)
{
    if (verbose) puts("STUB: AMDCopyArrayOfDevicesMatchingQuery called");
    return NULL;
}

void* AMDCopyAuthenticationIdentityForDevice(void)
{
    if (verbose) puts("STUB: AMDCopyAuthenticationIdentityForDevice called");
    return NULL;
}

void* AMDCopyErrorText(void)
{
    if (verbose) puts("STUB: AMDCopyErrorText called");
    return NULL;
}

void* AMDCopySystemBonjourUniqueID(void)
{
    if (verbose) puts("STUB: AMDCopySystemBonjourUniqueID called");
    return NULL;
}

void* AMDCopyUDIDForPairingIdentity(void)
{
    if (verbose) puts("STUB: AMDCopyUDIDForPairingIdentity called");
    return NULL;
}

void* AMDCreateDeviceList(void)
{
    if (verbose) puts("STUB: AMDCreateDeviceList called");
    return NULL;
}

void* AMDErrorForMobileInstallationCallbackDict(void)
{
    if (verbose) puts("STUB: AMDErrorForMobileInstallationCallbackDict called");
    return NULL;
}

void* AMDErrorString(void)
{
    if (verbose) puts("STUB: AMDErrorString called");
    return NULL;
}

void* AMDFUModeDeviceCopyAPNonce(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceCopyAPNonce called");
    return NULL;
}

void* AMDFUModeDeviceCopyAuthInstallPreflightOptions(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceCopyAuthInstallPreflightOptions called");
    return NULL;
}

void* AMDFUModeDeviceCopyBoardConfig(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceCopyBoardConfig called");
    return NULL;
}

void* AMDFUModeDeviceCopyEcid(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceCopyEcid called");
    return NULL;
}

void* AMDFUModeDeviceCopySEPNonce(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceCopySEPNonce called");
    return NULL;
}

void* AMDFUModeDeviceGetBoardID(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceGetBoardID called");
    return NULL;
}

void* AMDFUModeDeviceGetChipID(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceGetChipID called");
    return NULL;
}

void* AMDFUModeDeviceGetECID(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceGetECID called");
    return NULL;
}

void* AMDFUModeDeviceGetEffectiveProductionMode(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceGetEffectiveProductionMode called");
    return NULL;
}

void* AMDFUModeDeviceGetEffectiveSecurityMode(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceGetEffectiveSecurityMode called");
    return NULL;
}

void* AMDFUModeDeviceGetImage4Aware(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceGetImage4Aware called");
    return NULL;
}

void* AMDFUModeDeviceGetLocationID(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceGetLocationID called");
    return NULL;
}

void* AMDFUModeDeviceGetOperation(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceGetOperation called");
    return NULL;
}

void* AMDFUModeDeviceGetProductID(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceGetProductID called");
    return NULL;
}

void* AMDFUModeDeviceGetProductType(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceGetProductType called");
    return NULL;
}

void* AMDFUModeDeviceGetProductionMode(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceGetProductionMode called");
    return NULL;
}

void* AMDFUModeDeviceGetProgress(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceGetProgress called");
    return NULL;
}

void* AMDFUModeDeviceGetSecurityDomain(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceGetSecurityDomain called");
    return NULL;
}

void* AMDFUModeDeviceGetSecurityEpoch(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceGetSecurityEpoch called");
    return NULL;
}

void* AMDFUModeDeviceGetSecurityMode(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceGetSecurityMode called");
    return NULL;
}

void* AMDFUModeDeviceGetTypeID(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceGetTypeID called");
    return NULL;
}

void* AMDFUModeDeviceIsBootstrapOnly(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceIsBootstrapOnly called");
    return NULL;
}

void* AMDFUModeDeviceRequestAbbreviatedSendSync(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceRequestAbbreviatedSendSync called");
    return NULL;
}

void* AMDFUModeDeviceResetAuthInstallSettings(void)
{
    if (verbose) puts("STUB: AMDFUModeDeviceResetAuthInstallSettings called");
    return NULL;
}

void* AMDGetBundleVersion(void)
{
    if (verbose) puts("STUB: AMDGetBundleVersion called");
    return NULL;
}

void* AMDGetPairingRecordDirectoryPath(void)
{
    if (verbose) puts("STUB: AMDGetPairingRecordDirectoryPath called");
    return NULL;
}

void* AMDListenForNotifications(void)
{
    if (verbose) puts("STUB: AMDListenForNotifications called");
    return NULL;
}

void* AMDObserveNotification(void)
{
    if (verbose) puts("STUB: AMDObserveNotification called");
    return NULL;
}

void* AMDPostNotification(void)
{
    if (verbose) puts("STUB: AMDPostNotification called");
    return NULL;
}

void* AMDSecureListenForNotifications(void)
{
    if (verbose) puts("STUB: AMDSecureListenForNotifications called");
    return NULL;
}

void* AMDSecureListenForNotificationsWithRetainedContext(void)
{
    if (verbose) puts("STUB: AMDSecureListenForNotificationsWithRetainedContext called");
    return NULL;
}

void* AMDSecureObserveNotification(void)
{
    if (verbose) puts("STUB: AMDSecureObserveNotification called");
    return NULL;
}

void* AMDSecurePostNotification(void)
{
    if (verbose) puts("STUB: AMDSecurePostNotification called");
    return NULL;
}

void* AMDSecureShutdownNotificationProxy(void)
{
    if (verbose) puts("STUB: AMDSecureShutdownNotificationProxy called");
    return NULL;
}

void* AMDServiceConnectionBufferedReadSize(void)
{
    if (verbose) puts("STUB: AMDServiceConnectionBufferedReadSize called");
    return NULL;
}

void* AMDServiceConnectionCreate(void)
{
    if (verbose) puts("STUB: AMDServiceConnectionCreate called");
    return NULL;
}

void* AMDServiceConnectionGetSecureIOContext(void)
{
    if (verbose) puts("STUB: AMDServiceConnectionGetSecureIOContext called");
    return NULL;
}

void* AMDServiceConnectionGetSocket(void)
{
    if (verbose) puts("STUB: AMDServiceConnectionGetSocket called");
    return NULL;
}

void* AMDServiceConnectionGetTypeID(void)
{
    if (verbose) puts("STUB: AMDServiceConnectionGetTypeID called");
    return NULL;
}

void* AMDServiceConnectionInvalidate(void)
{
    if (verbose) puts("STUB: AMDServiceConnectionInvalidate called");
    return NULL;
}

void* AMDServiceConnectionReceive(void)
{
    if (verbose) puts("STUB: AMDServiceConnectionReceive called");
    return NULL;
}

void* AMDServiceConnectionReceiveMessage(void)
{
    if (verbose) puts("STUB: AMDServiceConnectionReceiveMessage called");
    return NULL;
}

void* AMDServiceConnectionSend(void)
{
    if (verbose) puts("STUB: AMDServiceConnectionSend called");
    return NULL;
}

void* AMDServiceConnectionSendMessage(void)
{
    if (verbose) puts("STUB: AMDServiceConnectionSendMessage called");
    return NULL;
}

void* AMDServiceConnectionSetDevice(void)
{
    if (verbose) puts("STUB: AMDServiceConnectionSetDevice called");
    return NULL;
}

void* AMDServiceConnectionSetServiceName(void)
{
    if (verbose) puts("STUB: AMDServiceConnectionSetServiceName called");
    return NULL;
}

void* AMDSetLogLevel(void)
{
    if (verbose) puts("STUB: AMDSetLogLevel called");
    return NULL;
}

void* AMDShutdownNotificationProxy(void)
{
    if (verbose) puts("STUB: AMDShutdownNotificationProxy called");
    return NULL;
}

void* AMDeviceActivate(void)
{
    if (verbose) puts("STUB: AMDeviceActivate called");
    return NULL;
}

void* AMDeviceActivateWithOptions(void)
{
    if (verbose) puts("STUB: AMDeviceActivateWithOptions called");
    return NULL;
}

void* AMDeviceArchiveApplication(void)
{
    if (verbose) puts("STUB: AMDeviceArchiveApplication called");
    return NULL;
}

void* AMDeviceCheckCapabilitiesMatch(void)
{
    if (verbose) puts("STUB: AMDeviceCheckCapabilitiesMatch called");
    return NULL;
}

void* AMDeviceConnect(void)
{
    if (verbose) puts("STUB: AMDeviceConnect called");
    return NULL;
}

void* AMDeviceCopyAuthInstallPreflightOptions(void)
{
    if (verbose) puts("STUB: AMDeviceCopyAuthInstallPreflightOptions called");
    return NULL;
}

void* AMDeviceCopyDeviceIdentifier(void)
{
    if (verbose) puts("STUB: AMDeviceCopyDeviceIdentifier called");
    return NULL;
}

void* AMDeviceCopyDeviceLocation(void)
{
    if (verbose) puts("STUB: AMDeviceCopyDeviceLocation called");
    return NULL;
}

void* AMDeviceCopyDeviceUSBSerialNumber(void)
{
    if (verbose) puts("STUB: AMDeviceCopyDeviceUSBSerialNumber called");
    return NULL;
}

void* AMDeviceCopyFDRPreflightOptions(void)
{
    if (verbose) puts("STUB: AMDeviceCopyFDRPreflightOptions called");
    return NULL;
}

void* AMDeviceCopyMultipleValuesWithError(void)
{
    if (verbose) puts("STUB: AMDeviceCopyMultipleValuesWithError called");
    return NULL;
}

void* AMDeviceCopyPairedCompanion(void)
{
    if (verbose) puts("STUB: AMDeviceCopyPairedCompanion called");
    return NULL;
}

void* AMDeviceCopyPairedWatch(void)
{
    if (verbose) puts("STUB: AMDeviceCopyPairedWatch called");
    return NULL;
}

void* AMDeviceCopyPersonalizationNonce(void)
{
    if (verbose) puts("STUB: AMDeviceCopyPersonalizationNonce called");
    return NULL;
}

void* AMDeviceCopyPersonalizationNonceBridgeOS(void)
{
    if (verbose) puts("STUB: AMDeviceCopyPersonalizationNonceBridgeOS called");
    return NULL;
}

void* AMDeviceCopyProvisioningProfiles(void)
{
    if (verbose) puts("STUB: AMDeviceCopyProvisioningProfiles called");
    return NULL;
}

void* AMDeviceCopyValue(void)
{
    if (verbose) puts("STUB: AMDeviceCopyValue called");
    return NULL;
}

void* AMDeviceCopyValueWithError(void)
{
    if (verbose) puts("STUB: AMDeviceCopyValueWithError called");
    return NULL;
}

void* AMDeviceCreate(void)
{
    if (verbose) puts("STUB: AMDeviceCreate called");
    return NULL;
}

void* AMDeviceCreateActivationInfo(void)
{
    if (verbose) puts("STUB: AMDeviceCreateActivationInfo called");
    return NULL;
}

void* AMDeviceCreateActivationInfoWithOptions(void)
{
    if (verbose) puts("STUB: AMDeviceCreateActivationInfoWithOptions called");
    return NULL;
}

void* AMDeviceCreateActivationSessionInfo(void)
{
    if (verbose) puts("STUB: AMDeviceCreateActivationSessionInfo called");
    return NULL;
}

void* AMDeviceCreateCopy(void)
{
    if (verbose) puts("STUB: AMDeviceCreateCopy called");
    return NULL;
}

void* AMDeviceCreateFromProperties(void)
{
    if (verbose) puts("STUB: AMDeviceCreateFromProperties called");
    return NULL;
}

void* AMDeviceCreateHouseArrestService(void)
{
    if (verbose) puts("STUB: AMDeviceCreateHouseArrestService called");
    return NULL;
}

void* AMDeviceCreateSRPConnection(void)
{
    if (verbose) puts("STUB: AMDeviceCreateSRPConnection called");
    return NULL;
}

void* AMDeviceCreateWakeupToken(void)
{
    if (verbose) puts("STUB: AMDeviceCreateWakeupToken called");
    return NULL;
}

void* AMDeviceDeactivate(void)
{
    if (verbose) puts("STUB: AMDeviceDeactivate called");
    return NULL;
}

void* AMDeviceDeleteHostPairingRecordForUDID(void)
{
    if (verbose) puts("STUB: AMDeviceDeleteHostPairingRecordForUDID called");
    return NULL;
}

void* AMDeviceDisconnect(void)
{
    if (verbose) puts("STUB: AMDeviceDisconnect called");
    return NULL;
}

void* AMDeviceEnterRecovery(void)
{
    if (verbose) puts("STUB: AMDeviceEnterRecovery called");
    return NULL;
}

void* AMDeviceExtendedPairWithOptions(void)
{
    if (verbose) puts("STUB: AMDeviceExtendedPairWithOptions called");
    return NULL;
}

void* AMDeviceGetConnectionID(void)
{
    if (verbose) puts("STUB: AMDeviceGetConnectionID called");
    return NULL;
}

void* AMDeviceGetInterfaceSpeed(void)
{
    if (verbose) puts("STUB: AMDeviceGetInterfaceSpeed called");
    return NULL;
}

void* AMDeviceGetInterfaceType(void)
{
    if (verbose) puts("STUB: AMDeviceGetInterfaceType called");
    return NULL;
}

void* AMDeviceGetLocalOrRemoteOffsetToResume(void)
{
    if (verbose) puts("STUB: AMDeviceGetLocalOrRemoteOffsetToResume called");
    return NULL;
}

void* AMDeviceGetName(void)
{
    if (verbose) puts("STUB: AMDeviceGetName called");
    return NULL;
}

void* AMDeviceGetTypeID(void)
{
    if (verbose) puts("STUB: AMDeviceGetTypeID called");
    return NULL;
}

void* AMDeviceGetUserInfo(void)
{
    if (verbose) puts("STUB: AMDeviceGetUserInfo called");
    return NULL;
}

void* AMDeviceGetWirelessBuddyFlags(void)
{
    if (verbose) puts("STUB: AMDeviceGetWirelessBuddyFlags called");
    return NULL;
}

void* AMDeviceImageMounted(void)
{
    if (verbose) puts("STUB: AMDeviceImageMounted called");
    return NULL;
}

void* AMDeviceImageMountedBridgeOS(void)
{
    if (verbose) puts("STUB: AMDeviceImageMountedBridgeOS called");
    return NULL;
}

void* AMDeviceImageMountedMacOS(void)
{
    if (verbose) puts("STUB: AMDeviceImageMountedMacOS called");
    return NULL;
}

void* AMDeviceInstallApplication(void)
{
    if (verbose) puts("STUB: AMDeviceInstallApplication called");
    return NULL;
}

void* AMDeviceInstallProvisioningProfile(void)
{
    if (verbose) puts("STUB: AMDeviceInstallProvisioningProfile called");
    return NULL;
}

void* AMDeviceIsAtLeastVersionOnPlatform(void)
{
    if (verbose) puts("STUB: AMDeviceIsAtLeastVersionOnPlatform called");
    return NULL;
}

void* AMDeviceIsPaired(void)
{
    if (verbose) puts("STUB: AMDeviceIsPaired called");
    return NULL;
}

void* AMDeviceIsValid(void)
{
    if (verbose) puts("STUB: AMDeviceIsValid called");
    return NULL;
}

void* AMDeviceIsWiFiPairableRef(void)
{
    if (verbose) puts("STUB: AMDeviceIsWiFiPairableRef called");
    return NULL;
}

void* AMDeviceLookupApplicationArchives(void)
{
    if (verbose) puts("STUB: AMDeviceLookupApplicationArchives called");
    return NULL;
}

void* AMDeviceLookupApplications(void)
{
    if (verbose) puts("STUB: AMDeviceLookupApplications called");
    return NULL;
}

void* AMDeviceMountImage(void)
{
    if (verbose) puts("STUB: AMDeviceMountImage called");
    return NULL;
}

void* AMDeviceMountImageBridgeOS(void)
{
    if (verbose) puts("STUB: AMDeviceMountImageBridgeOS called");
    return NULL;
}

void* AMDeviceMountImageMacOS(void)
{
    if (verbose) puts("STUB: AMDeviceMountImageMacOS called");
    return NULL;
}

void* AMDeviceMountImageOnHost(void)
{
    if (verbose) puts("STUB: AMDeviceMountImageOnHost called");
    return NULL;
}

void* AMDeviceMountPersonalizedBundle(void)
{
    if (verbose) puts("STUB: AMDeviceMountPersonalizedBundle called");
    return NULL;
}

void* AMDeviceMountPersonalizedBundleBridgeOS(void)
{
    if (verbose) puts("STUB: AMDeviceMountPersonalizedBundleBridgeOS called");
    return NULL;
}

void* AMDeviceNotificationSubscribe(void)
{
    if (verbose) puts("STUB: AMDeviceNotificationSubscribe called");
    return NULL;
}

void* AMDeviceNotificationSubscribeWithOptions(void)
{
    if (verbose) puts("STUB: AMDeviceNotificationSubscribeWithOptions called");
    return NULL;
}

void* AMDeviceNotificationUnsubscribe(void)
{
    if (verbose) puts("STUB: AMDeviceNotificationUnsubscribe called");
    return NULL;
}

void* AMDevicePair(void)
{
    if (verbose) puts("STUB: AMDevicePair called");
    return NULL;
}

void* AMDevicePairWithCallback(void)
{
    if (verbose) puts("STUB: AMDevicePairWithCallback called");
    return NULL;
}

void* AMDevicePairWithOptions(void)
{
    if (verbose) puts("STUB: AMDevicePairWithOptions called");
    return NULL;
}

void* AMDevicePowerAssertionCreate(void)
{
    if (verbose) puts("STUB: AMDevicePowerAssertionCreate called");
    return NULL;
}

void* AMDevicePowerAssertionGetTypeID(void)
{
    if (verbose) puts("STUB: AMDevicePowerAssertionGetTypeID called");
    return NULL;
}

void* AMDevicePreflightOperationCreate(void)
{
    if (verbose) puts("STUB: AMDevicePreflightOperationCreate called");
    return NULL;
}

void* AMDevicePreflightOperationGetRunLoopSource(void)
{
    if (verbose) puts("STUB: AMDevicePreflightOperationGetRunLoopSource called");
    return NULL;
}

void* AMDevicePreflightOperationGetTypeID(void)
{
    if (verbose) puts("STUB: AMDevicePreflightOperationGetTypeID called");
    return NULL;
}

void* AMDevicePreflightOperationInvalidate(void)
{
    if (verbose) puts("STUB: AMDevicePreflightOperationInvalidate called");
    return NULL;
}

void* AMDeviceRegisterForCUNotificationsWithOptions(void)
{
    if (verbose) puts("STUB: AMDeviceRegisterForCUNotificationsWithOptions called");
    return NULL;
}

void* AMDeviceRegisterForCUStateNotificationsWithOptions(void)
{
    if (verbose) puts("STUB: AMDeviceRegisterForCUStateNotificationsWithOptions called");
    return NULL;
}

void* AMDeviceRelayFile(void)
{
    if (verbose) puts("STUB: AMDeviceRelayFile called");
    return NULL;
}

void* AMDeviceRelease(void)
{
    if (verbose) puts("STUB: AMDeviceRelease called");
    return NULL;
}

void* AMDeviceRemoveApplicationArchive(void)
{
    if (verbose) puts("STUB: AMDeviceRemoveApplicationArchive called");
    return NULL;
}

void* AMDeviceRemoveProvisioningProfile(void)
{
    if (verbose) puts("STUB: AMDeviceRemoveProvisioningProfile called");
    return NULL;
}

void* AMDeviceRemoveValue(void)
{
    if (verbose) puts("STUB: AMDeviceRemoveValue called");
    return NULL;
}

void* AMDeviceRequestAbbreviatedSendSync(void)
{
    if (verbose) puts("STUB: AMDeviceRequestAbbreviatedSendSync called");
    return NULL;
}

void* AMDeviceRestoreApplication(void)
{
    if (verbose) puts("STUB: AMDeviceRestoreApplication called");
    return NULL;
}

void* AMDeviceRetain(void)
{
    if (verbose) puts("STUB: AMDeviceRetain called");
    return NULL;
}

void* AMDeviceSecureArchiveApplication(void)
{
    if (verbose) puts("STUB: AMDeviceSecureArchiveApplication called");
    return NULL;
}

void* AMDeviceSecureCheckCapabilitiesMatch(void)
{
    if (verbose) puts("STUB: AMDeviceSecureCheckCapabilitiesMatch called");
    return NULL;
}

void* AMDeviceSecureInstallApplication(void)
{
    if (verbose) puts("STUB: AMDeviceSecureInstallApplication called");
    return NULL;
}

void* AMDeviceSecureInstallApplicationBundle(void)
{
    if (verbose) puts("STUB: AMDeviceSecureInstallApplicationBundle called");
    return NULL;
}

void* AMDeviceSecureRemoveApplicationArchive(void)
{
    if (verbose) puts("STUB: AMDeviceSecureRemoveApplicationArchive called");
    return NULL;
}

void* AMDeviceSecureRestoreApplication(void)
{
    if (verbose) puts("STUB: AMDeviceSecureRestoreApplication called");
    return NULL;
}

void* AMDeviceSecureStartService(void)
{
    if (verbose) puts("STUB: AMDeviceSecureStartService called");
    return NULL;
}

void* AMDeviceSecureTransferPath(void)
{
    if (verbose) puts("STUB: AMDeviceSecureTransferPath called");
    return NULL;
}

void* AMDeviceSecureUninstallApplication(void)
{
    if (verbose) puts("STUB: AMDeviceSecureUninstallApplication called");
    return NULL;
}

void* AMDeviceSecureUpgradeApplication(void)
{
    if (verbose) puts("STUB: AMDeviceSecureUpgradeApplication called");
    return NULL;
}

void* AMDeviceSetUserInfo(void)
{
    if (verbose) puts("STUB: AMDeviceSetUserInfo called");
    return NULL;
}

void* AMDeviceSetValue(void)
{
    if (verbose) puts("STUB: AMDeviceSetValue called");
    return NULL;
}

void* AMDeviceSetWirelessBuddyFlags(void)
{
    if (verbose) puts("STUB: AMDeviceSetWirelessBuddyFlags called");
    return NULL;
}

void* AMDeviceStartHouseArrestService(void)
{
    if (verbose) puts("STUB: AMDeviceStartHouseArrestService called");
    return NULL;
}

void* AMDeviceStartService(void)
{
    if (verbose) puts("STUB: AMDeviceStartService called");
    return NULL;
}

void* AMDeviceStartServiceWithOptions(void)
{
    if (verbose) puts("STUB: AMDeviceStartServiceWithOptions called");
    return NULL;
}

void* AMDeviceStartSession(void)
{
    if (verbose) puts("STUB: AMDeviceStartSession called");
    return NULL;
}

void* AMDeviceStopSession(void)
{
    if (verbose) puts("STUB: AMDeviceStopSession called");
    return NULL;
}

void* AMDeviceTransferApplication(void)
{
    if (verbose) puts("STUB: AMDeviceTransferApplication called");
    return NULL;
}

void* AMDeviceTransferPath(void)
{
    if (verbose) puts("STUB: AMDeviceTransferPath called");
    return NULL;
}

void* AMDeviceUSBDeviceID(void)
{
    if (verbose) puts("STUB: AMDeviceUSBDeviceID called");
    return NULL;
}

void* AMDeviceUSBLocationID(void)
{
    if (verbose) puts("STUB: AMDeviceUSBLocationID called");
    return NULL;
}

void* AMDeviceUSBProductID(void)
{
    if (verbose) puts("STUB: AMDeviceUSBProductID called");
    return NULL;
}

void* AMDeviceUninstallApplication(void)
{
    if (verbose) puts("STUB: AMDeviceUninstallApplication called");
    return NULL;
}

void* AMDeviceUnmountImageOnHost(void)
{
    if (verbose) puts("STUB: AMDeviceUnmountImageOnHost called");
    return NULL;
}

void* AMDeviceUnpair(void)
{
    if (verbose) puts("STUB: AMDeviceUnpair called");
    return NULL;
}

void* AMDeviceUpgradeApplication(void)
{
    if (verbose) puts("STUB: AMDeviceUpgradeApplication called");
    return NULL;
}

void* AMDeviceValidatePairing(void)
{
    if (verbose) puts("STUB: AMDeviceValidatePairing called");
    return NULL;
}

void* AMDeviceVerifySRPConnection(void)
{
    if (verbose) puts("STUB: AMDeviceVerifySRPConnection called");
    return NULL;
}

void* AMDeviceWakeupOperationCreateWithToken(void)
{
    if (verbose) puts("STUB: AMDeviceWakeupOperationCreateWithToken called");
    return NULL;
}

void* AMDeviceWakeupOperationGetTypeID(void)
{
    if (verbose) puts("STUB: AMDeviceWakeupOperationGetTypeID called");
    return NULL;
}

void* AMDeviceWakeupOperationInvalidate(void)
{
    if (verbose) puts("STUB: AMDeviceWakeupOperationInvalidate called");
    return NULL;
}

void* AMDeviceWakeupOperationSchedule(void)
{
    if (verbose) puts("STUB: AMDeviceWakeupOperationSchedule called");
    return NULL;
}

void* AMDeviceWakeupUsingToken(void)
{
    if (verbose) puts("STUB: AMDeviceWakeupUsingToken called");
    return NULL;
}

void* AMRAuthInstallCopyLocalizedStringForServerError(void)
{
    if (verbose) puts("STUB: AMRAuthInstallCopyLocalizedStringForServerError called");
    return NULL;
}

void* AMRAuthInstallCopyPreflightOptions(void)
{
    if (verbose) puts("STUB: AMRAuthInstallCopyPreflightOptions called");
    return NULL;
}

void* AMRAuthInstallCreateOptionsForBootstrapDevice(void)
{
    if (verbose) puts("STUB: AMRAuthInstallCreateOptionsForBootstrapDevice called");
    return NULL;
}

void* AMRAuthInstallFinalize(void)
{
    if (verbose) puts("STUB: AMRAuthInstallFinalize called");
    return NULL;
}

void* AMRAuthInstallPreflight(void)
{
    if (verbose) puts("STUB: AMRAuthInstallPreflight called");
    return NULL;
}

void* AMRLocalizedCopyStringForAMRError(void)
{
    if (verbose) puts("STUB: AMRLocalizedCopyStringForAMRError called");
    return NULL;
}

void* AMRLocalizedCopyStringForAMROperation(void)
{
    if (verbose) puts("STUB: AMRLocalizedCopyStringForAMROperation called");
    return NULL;
}

void* AMRLog(void)
{
    if (verbose) puts("STUB: AMRLog called");
    return NULL;
}

void* AMRLogv(void)
{
    if (verbose) puts("STUB: AMRLogv called");
    return NULL;
}

void* AMRecoveryModeDeviceCopyAPNonce(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceCopyAPNonce called");
    return NULL;
}

void* AMRecoveryModeDeviceCopyAuthInstallPreflightOptions(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceCopyAuthInstallPreflightOptions called");
    return NULL;
}

void* AMRecoveryModeDeviceCopyBoardConfig(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceCopyBoardConfig called");
    return NULL;
}

void* AMRecoveryModeDeviceCopyEcid(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceCopyEcid called");
    return NULL;
}

void* AMRecoveryModeDeviceCopyEnvironmentVariableFromDevice(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceCopyEnvironmentVariableFromDevice called");
    return NULL;
}

void* AMRecoveryModeDeviceCopyIMEI(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceCopyIMEI called");
    return NULL;
}

void* AMRecoveryModeDeviceCopySEPNonce(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceCopySEPNonce called");
    return NULL;
}

void* AMRecoveryModeDeviceCopySerialNumber(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceCopySerialNumber called");
    return NULL;
}

void* AMRecoveryModeDeviceCreateWithIOService(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceCreateWithIOService called");
    return NULL;
}

void* AMRecoveryModeDeviceGetBoardID(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceGetBoardID called");
    return NULL;
}

void* AMRecoveryModeDeviceGetChipID(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceGetChipID called");
    return NULL;
}

void* AMRecoveryModeDeviceGetECID(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceGetECID called");
    return NULL;
}

void* AMRecoveryModeDeviceGetEffectiveProductionMode(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceGetEffectiveProductionMode called");
    return NULL;
}

void* AMRecoveryModeDeviceGetEffectiveSecurityMode(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceGetEffectiveSecurityMode called");
    return NULL;
}

void* AMRecoveryModeDeviceGetLocationID(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceGetLocationID called");
    return NULL;
}

void* AMRecoveryModeDeviceGetProductID(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceGetProductID called");
    return NULL;
}

void* AMRecoveryModeDeviceGetProductType(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceGetProductType called");
    return NULL;
}

void* AMRecoveryModeDeviceGetProductionMode(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceGetProductionMode called");
    return NULL;
}

void* AMRecoveryModeDeviceGetProgress(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceGetProgress called");
    return NULL;
}

void* AMRecoveryModeDeviceGetSecurityEpoch(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceGetSecurityEpoch called");
    return NULL;
}

void* AMRecoveryModeDeviceGetSecurityMode(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceGetSecurityMode called");
    return NULL;
}

void* AMRecoveryModeDeviceGetSupportedImageFormat(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceGetSupportedImageFormat called");
    return NULL;
}

void* AMRecoveryModeDeviceGetTypeID(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceGetTypeID called");
    return NULL;
}

void* AMRecoveryModeDeviceIsBootstrapOnly(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceIsBootstrapOnly called");
    return NULL;
}

void* AMRecoveryModeDeviceReboot(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceReboot called");
    return NULL;
}

void* AMRecoveryModeDeviceRequestAbbreviatedSendSync(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceRequestAbbreviatedSendSync called");
    return NULL;
}

void* AMRecoveryModeDeviceSendBlindCommandToDevice(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceSendBlindCommandToDevice called");
    return NULL;
}

void* AMRecoveryModeDeviceSendCommandToDevice(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceSendCommandToDevice called");
    return NULL;
}

void* AMRecoveryModeDeviceSendFileToDevice(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceSendFileToDevice called");
    return NULL;
}

void* AMRecoveryModeDeviceSetAutoBoot(void)
{
    if (verbose) puts("STUB: AMRecoveryModeDeviceSetAutoBoot called");
    return NULL;
}

void* AMRecoveryModeGetSoftwareBuildVersion(void)
{
    if (verbose) puts("STUB: AMRecoveryModeGetSoftwareBuildVersion called");
    return NULL;
}

void* AMRestorableAttemptMuxRegistration(void)
{
    if (verbose) puts("STUB: AMRestorableAttemptMuxRegistration called");
    return NULL;
}

void* AMRestorableBuildCopyRestoreBundleURL(void)
{
    if (verbose) puts("STUB: AMRestorableBuildCopyRestoreBundleURL called");
    return NULL;
}

void* AMRestorableBuildCopySupportedBoardConfigs(void)
{
    if (verbose) puts("STUB: AMRestorableBuildCopySupportedBoardConfigs called");
    return NULL;
}

void* AMRestorableBuildCopySupportedVariants(void)
{
    if (verbose) puts("STUB: AMRestorableBuildCopySupportedVariants called");
    return NULL;
}

void* AMRestorableBuildCreate(void)
{
    if (verbose) puts("STUB: AMRestorableBuildCreate called");
    return NULL;
}

void* AMRestorableCommitStashBag(void)
{
    if (verbose) puts("STUB: AMRestorableCommitStashBag called");
    return NULL;
}

void* AMRestorableCommitStashBagWithManifest(void)
{
    if (verbose) puts("STUB: AMRestorableCommitStashBagWithManifest called");
    return NULL;
}

void* AMRestorableCommitStashBagWithRestoreOptions(void)
{
    if (verbose) puts("STUB: AMRestorableCommitStashBagWithRestoreOptions called");
    return NULL;
}

void* AMRestorableDeviceCopyAMDevice(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceCopyAMDevice called");
    return NULL;
}

void* AMRestorableDeviceCopyAuthInstallPreflightOptions(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceCopyAuthInstallPreflightOptions called");
    return NULL;
}

void* AMRestorableDeviceCopyBoardConfig(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceCopyBoardConfig called");
    return NULL;
}

void* AMRestorableDeviceCopyDFUModeDevice(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceCopyDFUModeDevice called");
    return NULL;
}

void* AMRestorableDeviceCopyDefaultRestoreOptions(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceCopyDefaultRestoreOptions called");
    return NULL;
}

void* AMRestorableDeviceCopyEnvironmentVariable(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceCopyEnvironmentVariable called");
    return NULL;
}

void* AMRestorableDeviceCopyMDRemoteServiceDevice(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceCopyMDRemoteServiceDevice called");
    return NULL;
}

void* AMRestorableDeviceCopyProductString(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceCopyProductString called");
    return NULL;
}

void* AMRestorableDeviceCopyRecoveryModeDevice(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceCopyRecoveryModeDevice called");
    return NULL;
}

void* AMRestorableDeviceCopyRestoreModeDevice(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceCopyRestoreModeDevice called");
    return NULL;
}

void* AMRestorableDeviceCopyRestoreOptionsFromDocument(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceCopyRestoreOptionsFromDocument called");
    return NULL;
}

void* AMRestorableDeviceCopySerialDevicePath(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceCopySerialDevicePath called");
    return NULL;
}

void* AMRestorableDeviceCopySerialDeviceSWDName(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceCopySerialDeviceSWDName called");
    return NULL;
}

void* AMRestorableDeviceCopySerialNumber(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceCopySerialNumber called");
    return NULL;
}

void* AMRestorableDeviceCopyUserFriendlyName(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceCopyUserFriendlyName called");
    return NULL;
}

void* AMRestorableDeviceCreateFromAMDevice(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceCreateFromAMDevice called");
    return NULL;
}

void* AMRestorableDeviceEnableExtraDFUDevices(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceEnableExtraDFUDevices called");
    return NULL;
}

void* AMRestorableDeviceGetBoardID(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceGetBoardID called");
    return NULL;
}

void* AMRestorableDeviceGetChipID(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceGetChipID called");
    return NULL;
}

void* AMRestorableDeviceGetDFUModeDevice(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceGetDFUModeDevice called");
    return NULL;
}

void* AMRestorableDeviceGetDeviceClass(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceGetDeviceClass called");
    return NULL;
}

void* AMRestorableDeviceGetDeviceMapEntryForBoardAndChipID(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceGetDeviceMapEntryForBoardAndChipID called");
    return NULL;
}

void* AMRestorableDeviceGetECID(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceGetECID called");
    return NULL;
}

void* AMRestorableDeviceGetFusingInfo(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceGetFusingInfo called");
    return NULL;
}

void* AMRestorableDeviceGetLocationID(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceGetLocationID called");
    return NULL;
}

void* AMRestorableDeviceGetProductID(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceGetProductID called");
    return NULL;
}

void* AMRestorableDeviceGetProductType(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceGetProductType called");
    return NULL;
}

void* AMRestorableDeviceGetRecoveryModeDevice(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceGetRecoveryModeDevice called");
    return NULL;
}

void* AMRestorableDeviceGetRestoreOperationsQueue(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceGetRestoreOperationsQueue called");
    return NULL;
}

void* AMRestorableDeviceGetState(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceGetState called");
    return NULL;
}

void* AMRestorableDeviceGetTypeID(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceGetTypeID called");
    return NULL;
}

void* AMRestorableDeviceIsInDeviceClass(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceIsInDeviceClass called");
    return NULL;
}

void* AMRestorableDeviceNeedsUpdatedFramework(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceNeedsUpdatedFramework called");
    return NULL;
}

void* AMRestorableDeviceRegisterForNotifications(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceRegisterForNotifications called");
    return NULL;
}

void* AMRestorableDeviceRegisterForNotificationsForDevices(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceRegisterForNotificationsForDevices called");
    return NULL;
}

void* AMRestorableDeviceRestore(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceRestore called");
    return NULL;
}

void* AMRestorableDeviceRestoreWithError(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceRestoreWithError called");
    return NULL;
}

void* AMRestorableDeviceSendBlindCommand(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceSendBlindCommand called");
    return NULL;
}

void* AMRestorableDeviceSendCommand(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceSendCommand called");
    return NULL;
}

void* AMRestorableDeviceSendFile(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceSendFile called");
    return NULL;
}

void* AMRestorableDeviceSetLogFileURL(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceSetLogFileURL called");
    return NULL;
}

void* AMRestorableDeviceSetProxyCredentialsCallback(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceSetProxyCredentialsCallback called");
    return NULL;
}

void* AMRestorableDeviceTriggerUpdateForBoardAndChipIDWithPrompt(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceTriggerUpdateForBoardAndChipIDWithPrompt called");
    return NULL;
}

void* AMRestorableDeviceTriggerUpdateForDeviceWithPrompt(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceTriggerUpdateForDeviceWithPrompt called");
    return NULL;
}

void* AMRestorableDeviceUnregisterForNotifications(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceUnregisterForNotifications called");
    return NULL;
}

void* AMRestorableDeviceUpdateAvailableForBoardAndChipID(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceUpdateAvailableForBoardAndChipID called");
    return NULL;
}

void* AMRestorableDeviceUpdateAvailableForDevice(void)
{
    if (verbose) puts("STUB: AMRestorableDeviceUpdateAvailableForDevice called");
    return NULL;
}

void* AMRestorableEnableLogStreaming(void)
{
    if (verbose) puts("STUB: AMRestorableEnableLogStreaming called");
    return NULL;
}

void* AMRestorableGetCheckpointData(void)
{
    if (verbose) puts("STUB: AMRestorableGetCheckpointData called");
    return NULL;
}

void* AMRestorableGetIdentifyingErrorCode(void)
{
    if (verbose) puts("STUB: AMRestorableGetIdentifyingErrorCode called");
    return NULL;
}

void* AMRestorableLogToFile(void)
{
    if (verbose) puts("STUB: AMRestorableLogToFile called");
    return NULL;
}

void* AMRestorablePersonalizeCopyManifestTag(void)
{
    if (verbose) puts("STUB: AMRestorablePersonalizeCopyManifestTag called");
    return NULL;
}

void* AMRestorablePersonalizeSendFile(void)
{
    if (verbose) puts("STUB: AMRestorablePersonalizeSendFile called");
    return NULL;
}

void* AMRestorableSetCheckpointData(void)
{
    if (verbose) puts("STUB: AMRestorableSetCheckpointData called");
    return NULL;
}

void* AMRestorableSetGlobalLocationIDFilter(void)
{
    if (verbose) puts("STUB: AMRestorableSetGlobalLocationIDFilter called");
    return NULL;
}

void* AMRestorableSetGlobalLogFileURL(void)
{
    if (verbose) puts("STUB: AMRestorableSetGlobalLogFileURL called");
    return NULL;
}

void* AMRestorableSetLogLevel(void)
{
    if (verbose) puts("STUB: AMRestorableSetLogLevel called");
    return NULL;
}

void* AMRestoreCopyNormalizedBoardConfig(void)
{
    if (verbose) puts("STUB: AMRestoreCopyNormalizedBoardConfig called");
    return NULL;
}

void* AMRestoreCopySystemRestoreImagePath(void)
{
    if (verbose) puts("STUB: AMRestoreCopySystemRestoreImagePath called");
    return NULL;
}

void* AMRestoreCreateBootArgsByAddingArg(void)
{
    if (verbose) puts("STUB: AMRestoreCreateBootArgsByAddingArg called");
    return NULL;
}

void* AMRestoreCreateBootArgsByRemovingArg(void)
{
    if (verbose) puts("STUB: AMRestoreCreateBootArgsByRemovingArg called");
    return NULL;
}

void* AMRestoreCreateDefaultOptions(void)
{
    if (verbose) puts("STUB: AMRestoreCreateDefaultOptions called");
    return NULL;
}

void* AMRestoreCreatePathsForBundle(void)
{
    if (verbose) puts("STUB: AMRestoreCreatePathsForBundle called");
    return NULL;
}

void* AMRestoreDeviceGetFwImages(void)
{
    if (verbose) puts("STUB: AMRestoreDeviceGetFwImages called");
    return NULL;
}

void* AMRestoreDisableFileLogging(void)
{
    if (verbose) puts("STUB: AMRestoreDisableFileLogging called");
    return NULL;
}

void* AMRestoreEnableExtraDFUDevices(void)
{
    if (verbose) puts("STUB: AMRestoreEnableExtraDFUDevices called");
    return NULL;
}

void* AMRestoreEnableFileLogging(void)
{
    if (verbose) puts("STUB: AMRestoreEnableFileLogging called");
    return NULL;
}

void* AMRestoreGetSupportedPayloadVersion(void)
{
    if (verbose) puts("STUB: AMRestoreGetSupportedPayloadVersion called");
    return NULL;
}

void* AMRestoreLogFilePaths(void)
{
    if (verbose) puts("STUB: AMRestoreLogFilePaths called");
    return NULL;
}

void* AMRestoreModeCopyDeviceInfo(void)
{
    if (verbose) puts("STUB: AMRestoreModeCopyDeviceInfo called");
    return NULL;
}

void* AMRestoreModeDeviceCopyBoardConfig(void)
{
    if (verbose) puts("STUB: AMRestoreModeDeviceCopyBoardConfig called");
    return NULL;
}

void* AMRestoreModeDeviceCopyEcid(void)
{
    if (verbose) puts("STUB: AMRestoreModeDeviceCopyEcid called");
    return NULL;
}

void* AMRestoreModeDeviceCopyIMEI(void)
{
    if (verbose) puts("STUB: AMRestoreModeDeviceCopyIMEI called");
    return NULL;
}

void* AMRestoreModeDeviceCopyMarketingPartNumber(void)
{
    if (verbose) puts("STUB: AMRestoreModeDeviceCopyMarketingPartNumber called");
    return NULL;
}

void* AMRestoreModeDeviceCopyRestoreLog(void)
{
    if (verbose) puts("STUB: AMRestoreModeDeviceCopyRestoreLog called");
    return NULL;
}

void* AMRestoreModeDeviceCopySerialNumber(void)
{
    if (verbose) puts("STUB: AMRestoreModeDeviceCopySerialNumber called");
    return NULL;
}

void* AMRestoreModeDeviceCreate(void)
{
    if (verbose) puts("STUB: AMRestoreModeDeviceCreate called");
    return NULL;
}

void* AMRestoreModeDeviceGetDeviceID(void)
{
    if (verbose) puts("STUB: AMRestoreModeDeviceGetDeviceID called");
    return NULL;
}

void* AMRestoreModeDeviceGetLocationID(void)
{
    if (verbose) puts("STUB: AMRestoreModeDeviceGetLocationID called");
    return NULL;
}

void* AMRestoreModeDeviceGetProgress(void)
{
    if (verbose) puts("STUB: AMRestoreModeDeviceGetProgress called");
    return NULL;
}

void* AMRestoreModeDeviceGetTypeID(void)
{
    if (verbose) puts("STUB: AMRestoreModeDeviceGetTypeID called");
    return NULL;
}

void* AMRestoreModeDeviceReboot(void)
{
    if (verbose) puts("STUB: AMRestoreModeDeviceReboot called");
    return NULL;
}

void* AMRestoreModeGetLastFailureLog(void)
{
    if (verbose) puts("STUB: AMRestoreModeGetLastFailureLog called");
    return NULL;
}

void* AMRestorePerformDFURestore(void)
{
    if (verbose) puts("STUB: AMRestorePerformDFURestore called");
    return NULL;
}

void* AMRestorePerformRecoveryModeRestore(void)
{
    if (verbose) puts("STUB: AMRestorePerformRecoveryModeRestore called");
    return NULL;
}

void* AMRestorePerformRestoreModeRestore(void)
{
    if (verbose) puts("STUB: AMRestorePerformRestoreModeRestore called");
    return NULL;
}

void* AMRestorePerformRestoreModeRestoreWithError(void)
{
    if (verbose) puts("STUB: AMRestorePerformRestoreModeRestoreWithError called");
    return NULL;
}

void* AMRestoreRegisterForDeviceNotifications(void)
{
    if (verbose) puts("STUB: AMRestoreRegisterForDeviceNotifications called");
    return NULL;
}

void* AMRestoreScrubLogs(void)
{
    if (verbose) puts("STUB: AMRestoreScrubLogs called");
    return NULL;
}

void* AMRestoreSetLogLevel(void)
{
    if (verbose) puts("STUB: AMRestoreSetLogLevel called");
    return NULL;
}

void* AMRestoreUnregisterForDeviceNotifications(void)
{
    if (verbose) puts("STUB: AMRestoreUnregisterForDeviceNotifications called");
    return NULL;
}

void* AMRestoreUseFactoryLoggingFormat(void)
{
    if (verbose) puts("STUB: AMRestoreUseFactoryLoggingFormat called");
    return NULL;
}

void* AMSArchiveBackup(void)
{
    if (verbose) puts("STUB: AMSArchiveBackup called");
    return NULL;
}

void* AMSBackupWithOptions(void)
{
    if (verbose) puts("STUB: AMSBackupWithOptions called");
    return NULL;
}

void* AMSBeginSync(void)
{
    if (verbose) puts("STUB: AMSBeginSync called");
    return NULL;
}

void* AMSBeginSyncForDataClasses(void)
{
    if (verbose) puts("STUB: AMSBeginSyncForDataClasses called");
    return NULL;
}

void* AMSCancelBackupRestore(void)
{
    if (verbose) puts("STUB: AMSCancelBackupRestore called");
    return NULL;
}

void* AMSCancelCrashReportCopy(void)
{
    if (verbose) puts("STUB: AMSCancelCrashReportCopy called");
    return NULL;
}

void* AMSCancelSync(void)
{
    if (verbose) puts("STUB: AMSCancelSync called");
    return NULL;
}

void* AMSChangeBackupPassword(void)
{
    if (verbose) puts("STUB: AMSChangeBackupPassword called");
    return NULL;
}

void* AMSCleanup(void)
{
    if (verbose) puts("STUB: AMSCleanup called");
    return NULL;
}

void* AMSClearDataClasses(void)
{
    if (verbose) puts("STUB: AMSClearDataClasses called");
    return NULL;
}

void* AMSConnectToCrashReportCopyTarget(void)
{
    if (verbose) puts("STUB: AMSConnectToCrashReportCopyTarget called");
    return NULL;
}

void* AMSCopyApplicationListFromBackup(void)
{
    if (verbose) puts("STUB: AMSCopyApplicationListFromBackup called");
    return NULL;
}

void* AMSCopyCrashReportPath(void)
{
    if (verbose) puts("STUB: AMSCopyCrashReportPath called");
    return NULL;
}

void* AMSCopyCrashReportsFromTarget(void)
{
    if (verbose) puts("STUB: AMSCopyCrashReportsFromTarget called");
    return NULL;
}

void* AMSCopySourcesForRestoreCompatibleWith(void)
{
    if (verbose) puts("STUB: AMSCopySourcesForRestoreCompatibleWith called");
    return NULL;
}

void* AMSDisconnectFromCrashReportCopyTarget(void)
{
    if (verbose) puts("STUB: AMSDisconnectFromCrashReportCopyTarget called");
    return NULL;
}

void* AMSEnableCloudBackup(void)
{
    if (verbose) puts("STUB: AMSEnableCloudBackup called");
    return NULL;
}

void* AMSEnableSyncServices(void)
{
    if (verbose) puts("STUB: AMSEnableSyncServices called");
    return NULL;
}

void* AMSEraseDevice(void)
{
    if (verbose) puts("STUB: AMSEraseDevice called");
    return NULL;
}

void* AMSGetBackupInfo(void)
{
    if (verbose) puts("STUB: AMSGetBackupInfo called");
    return NULL;
}

void* AMSGetBackupPasswordFromKeychainForTarget(void)
{
    if (verbose) puts("STUB: AMSGetBackupPasswordFromKeychainForTarget called");
    return NULL;
}

void* AMSGetCalendarDayLimit(void)
{
    if (verbose) puts("STUB: AMSGetCalendarDayLimit called");
    return NULL;
}

void* AMSGetClientIdentifierAndDisplayNameForTarget(void)
{
    if (verbose) puts("STUB: AMSGetClientIdentifierAndDisplayNameForTarget called");
    return NULL;
}

void* AMSGetCollectionsForDataClassName(void)
{
    if (verbose) puts("STUB: AMSGetCollectionsForDataClassName called");
    return NULL;
}

void* AMSGetCrashReportCopyPreferencesForTarget(void)
{
    if (verbose) puts("STUB: AMSGetCrashReportCopyPreferencesForTarget called");
    return NULL;
}

void* AMSGetDataClassInfoForTarget(void)
{
    if (verbose) puts("STUB: AMSGetDataClassInfoForTarget called");
    return NULL;
}

void* AMSGetLastSyncDateForDataClass(void)
{
    if (verbose) puts("STUB: AMSGetLastSyncDateForDataClass called");
    return NULL;
}

void* AMSGetNewRecordCalendarName(void)
{
    if (verbose) puts("STUB: AMSGetNewRecordCalendarName called");
    return NULL;
}

void* AMSGetNewRecordGroupName(void)
{
    if (verbose) puts("STUB: AMSGetNewRecordGroupName called");
    return NULL;
}

void* AMSGetNumberOfCrashReportsToCopy(void)
{
    if (verbose) puts("STUB: AMSGetNumberOfCrashReportsToCopy called");
    return NULL;
}

void* AMSGetNumberOfCrashReportsToSubmit(void)
{
    if (verbose) puts("STUB: AMSGetNumberOfCrashReportsToSubmit called");
    return NULL;
}

void* AMSGetSourcesForRestore(void)
{
    if (verbose) puts("STUB: AMSGetSourcesForRestore called");
    return NULL;
}

void* AMSGetSupportedDataClassNames(void)
{
    if (verbose) puts("STUB: AMSGetSupportedDataClassNames called");
    return NULL;
}

void* AMSInitialize(void)
{
    if (verbose) puts("STUB: AMSInitialize called");
    return NULL;
}

void* AMSIsSyncServicesEnabled(void)
{
    if (verbose) puts("STUB: AMSIsSyncServicesEnabled called");
    return NULL;
}

void* AMSProcessLinkExitChild(void)
{
    if (verbose) puts("STUB: AMSProcessLinkExitChild called");
    return NULL;
}

void* AMSProcessLinkInitializeChild(void)
{
    if (verbose) puts("STUB: AMSProcessLinkInitializeChild called");
    return NULL;
}

void* AMSProcessLinkInvalidate(void)
{
    if (verbose) puts("STUB: AMSProcessLinkInvalidate called");
    return NULL;
}

void* AMSProcessLinkReadMessageFromChild(void)
{
    if (verbose) puts("STUB: AMSProcessLinkReadMessageFromChild called");
    return NULL;
}

void* AMSProcessLinkRelease(void)
{
    if (verbose) puts("STUB: AMSProcessLinkRelease called");
    return NULL;
}

void* AMSProcessLinkRetain(void)
{
    if (verbose) puts("STUB: AMSProcessLinkRetain called");
    return NULL;
}

void* AMSProcessLinkRunChild(void)
{
    if (verbose) puts("STUB: AMSProcessLinkRunChild called");
    return NULL;
}

void* AMSProcessLinkSendMessage(void)
{
    if (verbose) puts("STUB: AMSProcessLinkSendMessage called");
    return NULL;
}

void* AMSProcessLinkSendMessageAsync(void)
{
    if (verbose) puts("STUB: AMSProcessLinkSendMessageAsync called");
    return NULL;
}

void* AMSProcessLinkSendMessageLock(void)
{
    if (verbose) puts("STUB: AMSProcessLinkSendMessageLock called");
    return NULL;
}

void* AMSProcessLinkSendMessageToParent(void)
{
    if (verbose) puts("STUB: AMSProcessLinkSendMessageToParent called");
    return NULL;
}

void* AMSRegisterClientWithTargetIdentifierAndDisplayName(void)
{
    if (verbose) puts("STUB: AMSRegisterClientWithTargetIdentifierAndDisplayName called");
    return NULL;
}

void* AMSResetSyncData(void)
{
    if (verbose) puts("STUB: AMSResetSyncData called");
    return NULL;
}

void* AMSRestoreWithApplications(void)
{
    if (verbose) puts("STUB: AMSRestoreWithApplications called");
    return NULL;
}

void* AMSSetBackupPasswordInKeychain(void)
{
    if (verbose) puts("STUB: AMSSetBackupPasswordInKeychain called");
    return NULL;
}

void* AMSSetCalendarDayLimit(void)
{
    if (verbose) puts("STUB: AMSSetCalendarDayLimit called");
    return NULL;
}

void* AMSSetCrashReportCopyPreferencesForTarget(void)
{
    if (verbose) puts("STUB: AMSSetCrashReportCopyPreferencesForTarget called");
    return NULL;
}

void* AMSSetDataClassInfoForTarget(void)
{
    if (verbose) puts("STUB: AMSSetDataClassInfoForTarget called");
    return NULL;
}

void* AMSSetFilteredCollectionNamesForDataClassName(void)
{
    if (verbose) puts("STUB: AMSSetFilteredCollectionNamesForDataClassName called");
    return NULL;
}

void* AMSSetNewRecordCalendarName(void)
{
    if (verbose) puts("STUB: AMSSetNewRecordCalendarName called");
    return NULL;
}

void* AMSSetNewRecordGroupName(void)
{
    if (verbose) puts("STUB: AMSSetNewRecordGroupName called");
    return NULL;
}

void* AMSSubmitCrashReportsFromTarget(void)
{
    if (verbose) puts("STUB: AMSSubmitCrashReportsFromTarget called");
    return NULL;
}

void* AMSSubmitRestoreLogFromPath(void)
{
    if (verbose) puts("STUB: AMSSubmitRestoreLogFromPath called");
    return NULL;
}

void* AMSUnregisterTarget(void)
{
    if (verbose) puts("STUB: AMSUnregisterTarget called");
    return NULL;
}

void* AMSupportCreateDataFromFileURL(void)
{
    if (verbose) puts("STUB: AMSupportCreateDataFromFileURL called");
    return NULL;
}

void* AMSupportCreateDigest(void)
{
    if (verbose) puts("STUB: AMSupportCreateDigest called");
    return NULL;
}

void* AMSupportCreateErrorInternal(void)
{
    if (verbose) puts("STUB: AMSupportCreateErrorInternal called");
    return NULL;
}

void* AMSupportCreateURLFromString(void)
{
    if (verbose) puts("STUB: AMSupportCreateURLFromString called");
    return NULL;
}

void* AMSupportHttpCreateDataFromURL(void)
{
    if (verbose) puts("STUB: AMSupportHttpCreateDataFromURL called");
    return NULL;
}

void* AMSupportSafeFree(void)
{
    if (verbose) puts("STUB: AMSupportSafeFree called");
    return NULL;
}

void* AMSupportSafeRelease(void)
{
    if (verbose) puts("STUB: AMSupportSafeRelease called");
    return NULL;
}

void* AMSupportWriteDataToFileURL(void)
{
    if (verbose) puts("STUB: AMSupportWriteDataToFileURL called");
    return NULL;
}

void* ASRServerHandleConnection(void)
{
    if (verbose) puts("STUB: ASRServerHandleConnection called");
    return NULL;
}

void* ASRServerHandleSlurp(void)
{
    if (verbose) puts("STUB: ASRServerHandleSlurp called");
    return NULL;
}

void* DERDecodeItemPartialBuffer(void)
{
    if (verbose) puts("STUB: DERDecodeItemPartialBuffer called");
    return NULL;
}

void* DERParseInteger(void)
{
    if (verbose) puts("STUB: DERParseInteger called");
    return NULL;
}

void* MDRemoteServiceDeviceGetPropertyDictionary(void)
{
    if (verbose) puts("STUB: MDRemoteServiceDeviceGetPropertyDictionary called");
    return NULL;
}

void* MDRemoteServiceDeviceGetTypeID(void)
{
    if (verbose) puts("STUB: MDRemoteServiceDeviceGetTypeID called");
    return NULL;
}

void* MISCopyErrorStringForErrorCode(void)
{
    if (verbose) puts("STUB: MISCopyErrorStringForErrorCode called");
    return NULL;
}

void* MISProfileCopyPayload(void)
{
    if (verbose) puts("STUB: MISProfileCopyPayload called");
    return NULL;
}

void* MISProfileCopySignerSubjectSummary(void)
{
    if (verbose) puts("STUB: MISProfileCopySignerSubjectSummary called");
    return NULL;
}

void* MISProfileCreate(void)
{
    if (verbose) puts("STUB: MISProfileCreate called");
    return NULL;
}

void* MISProfileCreateDataRepresentation(void)
{
    if (verbose) puts("STUB: MISProfileCreateDataRepresentation called");
    return NULL;
}

void* MISProfileCreateForProvisioning(void)
{
    if (verbose) puts("STUB: MISProfileCreateForProvisioning called");
    return NULL;
}

void* MISProfileCreateMutableCopy(void)
{
    if (verbose) puts("STUB: MISProfileCreateMutableCopy called");
    return NULL;
}

void* MISProfileCreateWithData(void)
{
    if (verbose) puts("STUB: MISProfileCreateWithData called");
    return NULL;
}

void* MISProfileCreateWithFile(void)
{
    if (verbose) puts("STUB: MISProfileCreateWithFile called");
    return NULL;
}

void* MISProfileGetTypeID(void)
{
    if (verbose) puts("STUB: MISProfileGetTypeID called");
    return NULL;
}

void* MISProfileGetValue(void)
{
    if (verbose) puts("STUB: MISProfileGetValue called");
    return NULL;
}

void* MISProfileIsMutable(void)
{
    if (verbose) puts("STUB: MISProfileIsMutable called");
    return NULL;
}

void* MISProfileRemoveValue(void)
{
    if (verbose) puts("STUB: MISProfileRemoveValue called");
    return NULL;
}

void* MISProfileSetPayload(void)
{
    if (verbose) puts("STUB: MISProfileSetPayload called");
    return NULL;
}

void* MISProfileSetValue(void)
{
    if (verbose) puts("STUB: MISProfileSetValue called");
    return NULL;
}

void* MISProfileSignWithKeyAndCertificates(void)
{
    if (verbose) puts("STUB: MISProfileSignWithKeyAndCertificates called");
    return NULL;
}

void* MISProfileSignWithRSACallBack(void)
{
    if (verbose) puts("STUB: MISProfileSignWithRSACallBack called");
    return NULL;
}

void* MISProfileValidateSignature(void)
{
    if (verbose) puts("STUB: MISProfileValidateSignature called");
    return NULL;
}

void* MISProfileValidateSignatureWithAnchors(void)
{
    if (verbose) puts("STUB: MISProfileValidateSignatureWithAnchors called");
    return NULL;
}

void* MISProfileValidateSignatureWithAnchorsAndPolicy(void)
{
    if (verbose) puts("STUB: MISProfileValidateSignatureWithAnchorsAndPolicy called");
    return NULL;
}

void* MISProfileWriteToFile(void)
{
    if (verbose) puts("STUB: MISProfileWriteToFile called");
    return NULL;
}

void* MISProvisioningProfileAddDeveloperCertificate(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileAddDeveloperCertificate called");
    return NULL;
}

void* MISProvisioningProfileAddEntitlement(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileAddEntitlement called");
    return NULL;
}

void* MISProvisioningProfileAddProvisionedDevice(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileAddProvisionedDevice called");
    return NULL;
}

void* MISProvisioningProfileCheckValidity(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileCheckValidity called");
    return NULL;
}

void* MISProvisioningProfileCopyApplicationIdentifierPrefixes(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileCopyApplicationIdentifierPrefixes called");
    return NULL;
}

void* MISProvisioningProfileCreateMutableCopy(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileCreateMutableCopy called");
    return NULL;
}

void* MISProvisioningProfileGetCreationDate(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileGetCreationDate called");
    return NULL;
}

void* MISProvisioningProfileGetDeveloperCertificates(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileGetDeveloperCertificates called");
    return NULL;
}

void* MISProvisioningProfileGetEntitlements(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileGetEntitlements called");
    return NULL;
}

void* MISProvisioningProfileGetExpirationDate(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileGetExpirationDate called");
    return NULL;
}

void* MISProvisioningProfileGetName(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileGetName called");
    return NULL;
}

void* MISProvisioningProfileGetProvisionedDevices(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileGetProvisionedDevices called");
    return NULL;
}

void* MISProvisioningProfileGetTeamIdentifier(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileGetTeamIdentifier called");
    return NULL;
}

void* MISProvisioningProfileGetUUID(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileGetUUID called");
    return NULL;
}

void* MISProvisioningProfileGetVersion(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileGetVersion called");
    return NULL;
}

void* MISProvisioningProfileGrantsEntitlement(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileGrantsEntitlement called");
    return NULL;
}

void* MISProvisioningProfileIncludesDevice(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileIncludesDevice called");
    return NULL;
}

void* MISProvisioningProfileIncludesPlatform(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileIncludesPlatform called");
    return NULL;
}

void* MISProvisioningProfileIsAppleInternalProfile(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileIsAppleInternalProfile called");
    return NULL;
}

void* MISProvisioningProfileIsForLocalProvisioning(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileIsForLocalProvisioning called");
    return NULL;
}

void* MISProvisioningProfileProvisionsAllDevices(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileProvisionsAllDevices called");
    return NULL;
}

void* MISProvisioningProfileSetApplicationIdentifierPrefixes(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileSetApplicationIdentifierPrefixes called");
    return NULL;
}

void* MISProvisioningProfileSetDeveloperCertificates(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileSetDeveloperCertificates called");
    return NULL;
}

void* MISProvisioningProfileSetName(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileSetName called");
    return NULL;
}

void* MISProvisioningProfileSetProvisionsAllDevices(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileSetProvisionsAllDevices called");
    return NULL;
}

void* MISProvisioningProfileSetTimeToLive(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileSetTimeToLive called");
    return NULL;
}

void* MISProvisioningProfileValidateSignature(void)
{
    if (verbose) puts("STUB: MISProvisioningProfileValidateSignature called");
    return NULL;
}

void* ProcessLinkSetupParent(void)
{
    if (verbose) puts("STUB: ProcessLinkSetupParent called");
    return NULL;
}

void* RCGetSystemPartitionExtrasSizeForDevice(void)
{
    if (verbose) puts("STUB: RCGetSystemPartitionExtrasSizeForDevice called");
    return NULL;
}

void* RPCreateSocksProxyForDeviceRef(void)
{
    if (verbose) puts("STUB: RPCreateSocksProxyForDeviceRef called");
    return NULL;
}

void* RPSocksProxyInvalidate(void)
{
    if (verbose) puts("STUB: RPSocksProxyInvalidate called");
    return NULL;
}

void* RPSocksProxySetDoneCallback(void)
{
    if (verbose) puts("STUB: RPSocksProxySetDoneCallback called");
    return NULL;
}

void* RPSocksProxyStart(void)
{
    if (verbose) puts("STUB: RPSocksProxyStart called");
    return NULL;
}

void* RestoreCheckpointCreateCheckpointFingerprint(void)
{
    if (verbose) puts("STUB: RestoreCheckpointCreateCheckpointFingerprint called");
    return NULL;
}

void* RestoreCheckpointDataCreate(void)
{
    if (verbose) puts("STUB: RestoreCheckpointDataCreate called");
    return NULL;
}

void* RestoreCheckpointGetCheckpointStatements(void)
{
    if (verbose) puts("STUB: RestoreCheckpointGetCheckpointStatements called");
    return NULL;
}

void* RestoreCheckpointGetCheckpointWarnings(void)
{
    if (verbose) puts("STUB: RestoreCheckpointGetCheckpointWarnings called");
    return NULL;
}

void* RestoreCheckpointGetLastCheckpointError(void)
{
    if (verbose) puts("STUB: RestoreCheckpointGetLastCheckpointError called");
    return NULL;
}

void* RestoreCheckpointGetLastCheckpointID(void)
{
    if (verbose) puts("STUB: RestoreCheckpointGetLastCheckpointID called");
    return NULL;
}

void* RestoreCheckpointGetLastCheckpointResult(void)
{
    if (verbose) puts("STUB: RestoreCheckpointGetLastCheckpointResult called");
    return NULL;
}

void* SZArchiverConvertZipArchiveToStreamable(void)
{
    if (verbose) puts("STUB: SZArchiverConvertZipArchiveToStreamable called");
    return NULL;
}

void* SZArchiverCreateStreamableZip(void)
{
    if (verbose) puts("STUB: SZArchiverCreateStreamableZip called");
    return NULL;
}

void* SZArchiverExtractFile(void)
{
    if (verbose) puts("STUB: SZArchiverExtractFile called");
    return NULL;
}

void* SZConduitSendPath(void)
{
    if (verbose) puts("STUB: SZConduitSendPath called");
    return NULL;
}

void* SZConduitSendPathWithPreflight(void)
{
    if (verbose) puts("STUB: SZConduitSendPathWithPreflight called");
    return NULL;
}

void* USBMuxConnectByPort(void)
{
    if (verbose) puts("STUB: USBMuxConnectByPort called");
    return NULL;
}

void* USBMuxCopyDeviceArray(void)
{
    if (verbose) puts("STUB: USBMuxCopyDeviceArray called");
    return NULL;
}

void* USBMuxListenForDevices(void)
{
    if (verbose) puts("STUB: USBMuxListenForDevices called");
    return NULL;
}

void* USBMuxListenerClose(void)
{
    if (verbose) puts("STUB: USBMuxListenerClose called");
    return NULL;
}

void* USBMuxListenerCreate(void)
{
    if (verbose) puts("STUB: USBMuxListenerCreate called");
    return NULL;
}

void* USBMuxListenerCreateFiltered(void)
{
    if (verbose) puts("STUB: USBMuxListenerCreateFiltered called");
    return NULL;
}

void* USBMuxListenerGetRunLoopSource(void)
{
    if (verbose) puts("STUB: USBMuxListenerGetRunLoopSource called");
    return NULL;
}

void* USBMuxListenerSetDebug(void)
{
    if (verbose) puts("STUB: USBMuxListenerSetDebug called");
    return NULL;
}

void* _AMRestorableDeviceCopyBoardConfigNoLocking(void)
{
    if (verbose) puts("STUB: _AMRestorableDeviceCopyBoardConfigNoLocking called");
    return NULL;
}

void* _AMRestorableDeviceGetECIDNoLocking(void)
{
    if (verbose) puts("STUB: _AMRestorableDeviceGetECIDNoLocking called");
    return NULL;
}

void* _AMRestorableDeviceGetLocationIDNoLocking(void)
{
    if (verbose) puts("STUB: _AMRestorableDeviceGetLocationIDNoLocking called");
    return NULL;
}

void* _AMRestorableDeviceGetProxy(void)
{
    if (verbose) puts("STUB: _AMRestorableDeviceGetProxy called");
    return NULL;
}

void* _AMRestorableDeviceSendBlindCommandNoLocking(void)
{
    if (verbose) puts("STUB: _AMRestorableDeviceSendBlindCommandNoLocking called");
    return NULL;
}

void* _AMRestorableDeviceSendCommandNoLocking(void)
{
    if (verbose) puts("STUB: _AMRestorableDeviceSendCommandNoLocking called");
    return NULL;
}

void* _AMRestorableDeviceSendFileNoLocking(void)
{
    if (verbose) puts("STUB: _AMRestorableDeviceSendFileNoLocking called");
    return NULL;
}

void* _AMRestorableGetEventQueue(void)
{
    if (verbose) puts("STUB: _AMRestorableGetEventQueue called");
    return NULL;
}

void* _AMRestorableGetStateForString(void)
{
    if (verbose) puts("STUB: _AMRestorableGetStateForString called");
    return NULL;
}

void* _AMRestorableGetStringForState(void)
{
    if (verbose) puts("STUB: _AMRestorableGetStringForState called");
    return NULL;
}

void* _AMSGetErrorReasonForErrorCode(void)
{
    if (verbose) puts("STUB: _AMSGetErrorReasonForErrorCode called");
    return NULL;
}

void* _CreateBonjourDeviceDescriptions(void)
{
    if (verbose) puts("STUB: _CreateBonjourDeviceDescriptions called");
    return NULL;
}

void* _FinalizeMobileDevice(void)
{
    if (verbose) puts("STUB: _FinalizeMobileDevice called");
    return NULL;
}

void* _InitializeMobileDevice(void)
{
    if (verbose) puts("STUB: _InitializeMobileDevice called");
    return NULL;
}

void* _ensureDirectoryExists(void)
{
    if (verbose) puts("STUB: _ensureDirectoryExists called");
    return NULL;
}

void* _logData(void)
{
    if (verbose) puts("STUB: _logData called");
    return NULL;
}

void* _readDictionary(void)
{
    if (verbose) puts("STUB: _readDictionary called");
    return NULL;
}

void* _runBackupTool(void)
{
    if (verbose) puts("STUB: _runBackupTool called");
    return NULL;
}

void* _sendMessage(void)
{
    if (verbose) puts("STUB: _sendMessage called");
    return NULL;
}

void* _writeDictionary(void)
{
    if (verbose) puts("STUB: _writeDictionary called");
    return NULL;
}
