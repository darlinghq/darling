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


#include <DiscRecording/DiscRecording.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* DRAudioTrackCreate(void)
{
    if (verbose) puts("STUB: DRAudioTrackCreate called");
    return NULL;
}

void* DRAudioTrackCreateWithURL(void)
{
    if (verbose) puts("STUB: DRAudioTrackCreateWithURL called");
    return NULL;
}

void* DRBurnAbort(void)
{
    if (verbose) puts("STUB: DRBurnAbort called");
    return NULL;
}

void* DRBurnCopyStatus(void)
{
    if (verbose) puts("STUB: DRBurnCopyStatus called");
    return NULL;
}

void* DRBurnCreate(void)
{
    if (verbose) puts("STUB: DRBurnCreate called");
    return NULL;
}

void* DRBurnGetDevice(void)
{
    if (verbose) puts("STUB: DRBurnGetDevice called");
    return NULL;
}

void* DRBurnGetProperties(void)
{
    if (verbose) puts("STUB: DRBurnGetProperties called");
    return NULL;
}

void* DRBurnGetTypeID(void)
{
    if (verbose) puts("STUB: DRBurnGetTypeID called");
    return NULL;
}

void* DRBurnSetProperties(void)
{
    if (verbose) puts("STUB: DRBurnSetProperties called");
    return NULL;
}

void* DRBurnWriteLayout(void)
{
    if (verbose) puts("STUB: DRBurnWriteLayout called");
    return NULL;
}

void* DRCDTextBlockCreate(void)
{
    if (verbose) puts("STUB: DRCDTextBlockCreate called");
    return NULL;
}

void* DRCDTextBlockCreateArrayFromPackList(void)
{
    if (verbose) puts("STUB: DRCDTextBlockCreateArrayFromPackList called");
    return NULL;
}

void* DRCDTextBlockFlatten(void)
{
    if (verbose) puts("STUB: DRCDTextBlockFlatten called");
    return NULL;
}

void* DRCDTextBlockGetProperties(void)
{
    if (verbose) puts("STUB: DRCDTextBlockGetProperties called");
    return NULL;
}

void* DRCDTextBlockGetTrackDictionaries(void)
{
    if (verbose) puts("STUB: DRCDTextBlockGetTrackDictionaries called");
    return NULL;
}

void* DRCDTextBlockGetTypeID(void)
{
    if (verbose) puts("STUB: DRCDTextBlockGetTypeID called");
    return NULL;
}

void* DRCDTextBlockGetValue(void)
{
    if (verbose) puts("STUB: DRCDTextBlockGetValue called");
    return NULL;
}

void* DRCDTextBlockSetProperties(void)
{
    if (verbose) puts("STUB: DRCDTextBlockSetProperties called");
    return NULL;
}

void* DRCDTextBlockSetTrackDictionaries(void)
{
    if (verbose) puts("STUB: DRCDTextBlockSetTrackDictionaries called");
    return NULL;
}

void* DRCDTextBlockSetValue(void)
{
    if (verbose) puts("STUB: DRCDTextBlockSetValue called");
    return NULL;
}

void* DRCallbackDeviceCreate(void)
{
    if (verbose) puts("STUB: DRCallbackDeviceCreate called");
    return NULL;
}

void* DRCopyDeviceArray(void)
{
    if (verbose) puts("STUB: DRCopyDeviceArray called");
    return NULL;
}

void* DRCopyLocalizedStringForAdditionalSense(void)
{
    if (verbose) puts("STUB: DRCopyLocalizedStringForAdditionalSense called");
    return NULL;
}

void* DRCopyLocalizedStringForDiscRecordingError(void)
{
    if (verbose) puts("STUB: DRCopyLocalizedStringForDiscRecordingError called");
    return NULL;
}

void* DRCopyLocalizedStringForExceptionInfo(void)
{
    if (verbose) puts("STUB: DRCopyLocalizedStringForExceptionInfo called");
    return NULL;
}

void* DRCopyLocalizedStringForSenseCode(void)
{
    if (verbose) puts("STUB: DRCopyLocalizedStringForSenseCode called");
    return NULL;
}

void* DRCopyLocalizedStringForValue(void)
{
    if (verbose) puts("STUB: DRCopyLocalizedStringForValue called");
    return NULL;
}

void* DRDeviceAcquireExclusiveAccess(void)
{
    if (verbose) puts("STUB: DRDeviceAcquireExclusiveAccess called");
    return NULL;
}

void* DRDeviceAcquireMediaReservation(void)
{
    if (verbose) puts("STUB: DRDeviceAcquireMediaReservation called");
    return NULL;
}

void* DRDeviceCloseTray(void)
{
    if (verbose) puts("STUB: DRDeviceCloseTray called");
    return NULL;
}

void* DRDeviceCopyCDText(void)
{
    if (verbose) puts("STUB: DRDeviceCopyCDText called");
    return NULL;
}

void* DRDeviceCopyDeviceForBSDName(void)
{
    if (verbose) puts("STUB: DRDeviceCopyDeviceForBSDName called");
    return NULL;
}

void* DRDeviceCopyDeviceForIORegistryEntryPath(void)
{
    if (verbose) puts("STUB: DRDeviceCopyDeviceForIORegistryEntryPath called");
    return NULL;
}

void* DRDeviceCopyInfo(void)
{
    if (verbose) puts("STUB: DRDeviceCopyInfo called");
    return NULL;
}

void* DRDeviceCopyMediaID(void)
{
    if (verbose) puts("STUB: DRDeviceCopyMediaID called");
    return NULL;
}

void* DRDeviceCopyStatus(void)
{
    if (verbose) puts("STUB: DRDeviceCopyStatus called");
    return NULL;
}

void* DRDeviceEjectMedia(void)
{
    if (verbose) puts("STUB: DRDeviceEjectMedia called");
    return NULL;
}

void* DRDeviceGetTypeID(void)
{
    if (verbose) puts("STUB: DRDeviceGetTypeID called");
    return NULL;
}

void* DRDeviceIsValid(void)
{
    if (verbose) puts("STUB: DRDeviceIsValid called");
    return NULL;
}

void* DRDeviceKPSForXFactor(void)
{
    if (verbose) puts("STUB: DRDeviceKPSForXFactor called");
    return NULL;
}

void* DRDeviceOpenTray(void)
{
    if (verbose) puts("STUB: DRDeviceOpenTray called");
    return NULL;
}

void* DRDeviceQueryProperty(void)
{
    if (verbose) puts("STUB: DRDeviceQueryProperty called");
    return NULL;
}

void* DRDeviceReleaseExclusiveAccess(void)
{
    if (verbose) puts("STUB: DRDeviceReleaseExclusiveAccess called");
    return NULL;
}

void* DRDeviceReleaseMediaReservation(void)
{
    if (verbose) puts("STUB: DRDeviceReleaseMediaReservation called");
    return NULL;
}

void* DRDeviceXFactorForKPS(void)
{
    if (verbose) puts("STUB: DRDeviceXFactorForKPS called");
    return NULL;
}

void* DREraseCopyStatus(void)
{
    if (verbose) puts("STUB: DREraseCopyStatus called");
    return NULL;
}

void* DREraseCreate(void)
{
    if (verbose) puts("STUB: DREraseCreate called");
    return NULL;
}

void* DREraseGetDevice(void)
{
    if (verbose) puts("STUB: DREraseGetDevice called");
    return NULL;
}

void* DREraseGetProperties(void)
{
    if (verbose) puts("STUB: DREraseGetProperties called");
    return NULL;
}

void* DREraseGetTypeID(void)
{
    if (verbose) puts("STUB: DREraseGetTypeID called");
    return NULL;
}

void* DREraseSetProperties(void)
{
    if (verbose) puts("STUB: DREraseSetProperties called");
    return NULL;
}

void* DREraseStart(void)
{
    if (verbose) puts("STUB: DREraseStart called");
    return NULL;
}

void* DRFSObjectCopyBaseName(void)
{
    if (verbose) puts("STUB: DRFSObjectCopyBaseName called");
    return NULL;
}

void* DRFSObjectCopyFilesystemProperties(void)
{
    if (verbose) puts("STUB: DRFSObjectCopyFilesystemProperties called");
    return NULL;
}

void* DRFSObjectCopyFilesystemProperty(void)
{
    if (verbose) puts("STUB: DRFSObjectCopyFilesystemProperty called");
    return NULL;
}

void* DRFSObjectCopyMangledName(void)
{
    if (verbose) puts("STUB: DRFSObjectCopyMangledName called");
    return NULL;
}

void* DRFSObjectCopyMangledNames(void)
{
    if (verbose) puts("STUB: DRFSObjectCopyMangledNames called");
    return NULL;
}

void* DRFSObjectCopyRealURL(void)
{
    if (verbose) puts("STUB: DRFSObjectCopyRealURL called");
    return NULL;
}

void* DRFSObjectCopySpecificName(void)
{
    if (verbose) puts("STUB: DRFSObjectCopySpecificName called");
    return NULL;
}

void* DRFSObjectCopySpecificNames(void)
{
    if (verbose) puts("STUB: DRFSObjectCopySpecificNames called");
    return NULL;
}

void* DRFSObjectGetFilesystemMask(void)
{
    if (verbose) puts("STUB: DRFSObjectGetFilesystemMask called");
    return NULL;
}

void* DRFSObjectGetParent(void)
{
    if (verbose) puts("STUB: DRFSObjectGetParent called");
    return NULL;
}

void* DRFSObjectGetRealFSRef(void)
{
    if (verbose) puts("STUB: DRFSObjectGetRealFSRef called");
    return NULL;
}

void* DRFSObjectIsVirtual(void)
{
    if (verbose) puts("STUB: DRFSObjectIsVirtual called");
    return NULL;
}

void* DRFSObjectSetBaseName(void)
{
    if (verbose) puts("STUB: DRFSObjectSetBaseName called");
    return NULL;
}

void* DRFSObjectSetFilesystemMask(void)
{
    if (verbose) puts("STUB: DRFSObjectSetFilesystemMask called");
    return NULL;
}

void* DRFSObjectSetFilesystemProperties(void)
{
    if (verbose) puts("STUB: DRFSObjectSetFilesystemProperties called");
    return NULL;
}

void* DRFSObjectSetFilesystemProperty(void)
{
    if (verbose) puts("STUB: DRFSObjectSetFilesystemProperty called");
    return NULL;
}

void* DRFSObjectSetSpecificName(void)
{
    if (verbose) puts("STUB: DRFSObjectSetSpecificName called");
    return NULL;
}

void* DRFSObjectSetSpecificNames(void)
{
    if (verbose) puts("STUB: DRFSObjectSetSpecificNames called");
    return NULL;
}

void* DRFileCreateReal(void)
{
    if (verbose) puts("STUB: DRFileCreateReal called");
    return NULL;
}

void* DRFileCreateRealWithURL(void)
{
    if (verbose) puts("STUB: DRFileCreateRealWithURL called");
    return NULL;
}

void* DRFileCreateVirtualLink(void)
{
    if (verbose) puts("STUB: DRFileCreateVirtualLink called");
    return NULL;
}

void* DRFileCreateVirtualWithCallback(void)
{
    if (verbose) puts("STUB: DRFileCreateVirtualWithCallback called");
    return NULL;
}

void* DRFileCreateVirtualWithData(void)
{
    if (verbose) puts("STUB: DRFileCreateVirtualWithData called");
    return NULL;
}

void* DRFileGetTypeID(void)
{
    if (verbose) puts("STUB: DRFileGetTypeID called");
    return NULL;
}

void* DRFilesystemTrackCreate(void)
{
    if (verbose) puts("STUB: DRFilesystemTrackCreate called");
    return NULL;
}

void* DRFilesystemTrackEstimateOverhead(void)
{
    if (verbose) puts("STUB: DRFilesystemTrackEstimateOverhead called");
    return NULL;
}

void* DRFolderAddChild(void)
{
    if (verbose) puts("STUB: DRFolderAddChild called");
    return NULL;
}

void* DRFolderConvertRealToVirtual(void)
{
    if (verbose) puts("STUB: DRFolderConvertRealToVirtual called");
    return NULL;
}

void* DRFolderCopyChildren(void)
{
    if (verbose) puts("STUB: DRFolderCopyChildren called");
    return NULL;
}

void* DRFolderCountChildren(void)
{
    if (verbose) puts("STUB: DRFolderCountChildren called");
    return NULL;
}

void* DRFolderCreateReal(void)
{
    if (verbose) puts("STUB: DRFolderCreateReal called");
    return NULL;
}

void* DRFolderCreateRealWithURL(void)
{
    if (verbose) puts("STUB: DRFolderCreateRealWithURL called");
    return NULL;
}

void* DRFolderCreateVirtual(void)
{
    if (verbose) puts("STUB: DRFolderCreateVirtual called");
    return NULL;
}

void* DRFolderGetTypeID(void)
{
    if (verbose) puts("STUB: DRFolderGetTypeID called");
    return NULL;
}

void* DRFolderRemoveChild(void)
{
    if (verbose) puts("STUB: DRFolderRemoveChild called");
    return NULL;
}

void* DRGetRefCon(void)
{
    if (verbose) puts("STUB: DRGetRefCon called");
    return NULL;
}

void* DRGetVersion(void)
{
    if (verbose) puts("STUB: DRGetVersion called");
    return NULL;
}

void* DRNotificationCenterAddObserver(void)
{
    if (verbose) puts("STUB: DRNotificationCenterAddObserver called");
    return NULL;
}

void* DRNotificationCenterCreate(void)
{
    if (verbose) puts("STUB: DRNotificationCenterCreate called");
    return NULL;
}

void* DRNotificationCenterCreateRunLoopSource(void)
{
    if (verbose) puts("STUB: DRNotificationCenterCreateRunLoopSource called");
    return NULL;
}

void* DRNotificationCenterGetTypeID(void)
{
    if (verbose) puts("STUB: DRNotificationCenterGetTypeID called");
    return NULL;
}

void* DRNotificationCenterRemoveObserver(void)
{
    if (verbose) puts("STUB: DRNotificationCenterRemoveObserver called");
    return NULL;
}

void* DRPrepareCopyStatus(void)
{
    if (verbose) puts("STUB: DRPrepareCopyStatus called");
    return NULL;
}

void* DRPrepareCreate(void)
{
    if (verbose) puts("STUB: DRPrepareCreate called");
    return NULL;
}

void* DRPrepareGetDevice(void)
{
    if (verbose) puts("STUB: DRPrepareGetDevice called");
    return NULL;
}

void* DRPrepareGetTypeID(void)
{
    if (verbose) puts("STUB: DRPrepareGetTypeID called");
    return NULL;
}

void* DRPrepareStart(void)
{
    if (verbose) puts("STUB: DRPrepareStart called");
    return NULL;
}

void* DRSessionCreate(void)
{
    if (verbose) puts("STUB: DRSessionCreate called");
    return NULL;
}

void* DRSessionGetProperties(void)
{
    if (verbose) puts("STUB: DRSessionGetProperties called");
    return NULL;
}

void* DRSessionGetTracks(void)
{
    if (verbose) puts("STUB: DRSessionGetTracks called");
    return NULL;
}

void* DRSessionGetTypeID(void)
{
    if (verbose) puts("STUB: DRSessionGetTypeID called");
    return NULL;
}

void* DRSessionSetProperties(void)
{
    if (verbose) puts("STUB: DRSessionSetProperties called");
    return NULL;
}

void* DRSessionSetTracks(void)
{
    if (verbose) puts("STUB: DRSessionSetTracks called");
    return NULL;
}

void* DRSetRefCon(void)
{
    if (verbose) puts("STUB: DRSetRefCon called");
    return NULL;
}

void* DRTaskCopyInfo(void)
{
    if (verbose) puts("STUB: DRTaskCopyInfo called");
    return NULL;
}

void* DRTaskGetTypeID(void)
{
    if (verbose) puts("STUB: DRTaskGetTypeID called");
    return NULL;
}

void* DRTaskIsValid(void)
{
    if (verbose) puts("STUB: DRTaskIsValid called");
    return NULL;
}

void* DRTrackCreate(void)
{
    if (verbose) puts("STUB: DRTrackCreate called");
    return NULL;
}

void* DRTrackEstimateLength(void)
{
    if (verbose) puts("STUB: DRTrackEstimateLength called");
    return NULL;
}

void* DRTrackGetProperties(void)
{
    if (verbose) puts("STUB: DRTrackGetProperties called");
    return NULL;
}

void* DRTrackGetTypeID(void)
{
    if (verbose) puts("STUB: DRTrackGetTypeID called");
    return NULL;
}

void* DRTrackSetProperties(void)
{
    if (verbose) puts("STUB: DRTrackSetProperties called");
    return NULL;
}

void* DRTrackSpeedTest(void)
{
    if (verbose) puts("STUB: DRTrackSpeedTest called");
    return NULL;
}

void* _DRBurnConsumerThread(void)
{
    if (verbose) puts("STUB: _DRBurnConsumerThread called");
    return NULL;
}

void* _DRBurnProducerThread(void)
{
    if (verbose) puts("STUB: _DRBurnProducerThread called");
    return NULL;
}

void* _DRBurnThread(void)
{
    if (verbose) puts("STUB: _DRBurnThread called");
    return NULL;
}

void* _DRDeviceBlank(void)
{
    if (verbose) puts("STUB: _DRDeviceBlank called");
    return NULL;
}

void* _DRDeviceBlankProgress(void)
{
    if (verbose) puts("STUB: _DRDeviceBlankProgress called");
    return NULL;
}

void* _DRDeviceCloseFormat(void)
{
    if (verbose) puts("STUB: _DRDeviceCloseFormat called");
    return NULL;
}

void* _DRDeviceCloseSession(void)
{
    if (verbose) puts("STUB: _DRDeviceCloseSession called");
    return NULL;
}

void* _DRDeviceCloseTrack(void)
{
    if (verbose) puts("STUB: _DRDeviceCloseTrack called");
    return NULL;
}

void* _DRDeviceCopyCachedValue(void)
{
    if (verbose) puts("STUB: _DRDeviceCopyCachedValue called");
    return NULL;
}

void* _DRDeviceCopyInfoForProfile(void)
{
    if (verbose) puts("STUB: _DRDeviceCopyInfoForProfile called");
    return NULL;
}

void* _DRDeviceCreate(void)
{
    if (verbose) puts("STUB: _DRDeviceCreate called");
    return NULL;
}

void* _DRDeviceCreateSenseString(void)
{
    if (verbose) puts("STUB: _DRDeviceCreateSenseString called");
    return NULL;
}

void* _DRDeviceDriveReady(void)
{
    if (verbose) puts("STUB: _DRDeviceDriveReady called");
    return NULL;
}

void* _DRDeviceFormatUnitMode1(void)
{
    if (verbose) puts("STUB: _DRDeviceFormatUnitMode1 called");
    return NULL;
}

void* _DRDeviceGetCDCapabilitiesPage(void)
{
    if (verbose) puts("STUB: _DRDeviceGetCDCapabilitiesPage called");
    return NULL;
}

void* _DRDeviceGetCDSpeed(void)
{
    if (verbose) puts("STUB: _DRDeviceGetCDSpeed called");
    return NULL;
}

void* _DRDeviceGetConfiguration(void)
{
    if (verbose) puts("STUB: _DRDeviceGetConfiguration called");
    return NULL;
}

void* _DRDeviceGetDeviceSpeed(void)
{
    if (verbose) puts("STUB: _DRDeviceGetDeviceSpeed called");
    return NULL;
}

void* _DRDeviceGetFeature(void)
{
    if (verbose) puts("STUB: _DRDeviceGetFeature called");
    return NULL;
}

void* _DRDeviceGetPerformance(void)
{
    if (verbose) puts("STUB: _DRDeviceGetPerformance called");
    return NULL;
}

void* _DRDeviceGetPerformanceNominalInfo(void)
{
    if (verbose) puts("STUB: _DRDeviceGetPerformanceNominalInfo called");
    return NULL;
}

void* _DRDeviceGetWriteParamsPage(void)
{
    if (verbose) puts("STUB: _DRDeviceGetWriteParamsPage called");
    return NULL;
}

void* _DRDeviceInquiry(void)
{
    if (verbose) puts("STUB: _DRDeviceInquiry called");
    return NULL;
}

void* _DRDeviceLockDoor(void)
{
    if (verbose) puts("STUB: _DRDeviceLockDoor called");
    return NULL;
}

void* _DRDeviceMediaEject(void)
{
    if (verbose) puts("STUB: _DRDeviceMediaEject called");
    return NULL;
}

void* _DRDeviceModeSelect10(void)
{
    if (verbose) puts("STUB: _DRDeviceModeSelect10 called");
    return NULL;
}

void* _DRDeviceModeSense10(void)
{
    if (verbose) puts("STUB: _DRDeviceModeSense10 called");
    return NULL;
}

void* _DRDeviceQueryDriveFeatures(void)
{
    if (verbose) puts("STUB: _DRDeviceQueryDriveFeatures called");
    return NULL;
}

void* _DRDeviceQuerySupportLevel(void)
{
    if (verbose) puts("STUB: _DRDeviceQuerySupportLevel called");
    return NULL;
}

void* _DRDeviceQuiesce(void)
{
    if (verbose) puts("STUB: _DRDeviceQuiesce called");
    return NULL;
}

void* _DRDeviceReadBlocks(void)
{
    if (verbose) puts("STUB: _DRDeviceReadBlocks called");
    return NULL;
}

void* _DRDeviceReadBufferCapacity(void)
{
    if (verbose) puts("STUB: _DRDeviceReadBufferCapacity called");
    return NULL;
}

void* _DRDeviceReadCD(void)
{
    if (verbose) puts("STUB: _DRDeviceReadCD called");
    return NULL;
}

void* _DRDeviceReadCD2(void)
{
    if (verbose) puts("STUB: _DRDeviceReadCD2 called");
    return NULL;
}

void* _DRDeviceReadCDText(void)
{
    if (verbose) puts("STUB: _DRDeviceReadCDText called");
    return NULL;
}

void* _DRDeviceReadCapacity(void)
{
    if (verbose) puts("STUB: _DRDeviceReadCapacity called");
    return NULL;
}

void* _DRDeviceReadDVDADIPDescriptor(void)
{
    if (verbose) puts("STUB: _DRDeviceReadDVDADIPDescriptor called");
    return NULL;
}

void* _DRDeviceReadDVDJumpIntervalSize(void)
{
    if (verbose) puts("STUB: _DRDeviceReadDVDJumpIntervalSize called");
    return NULL;
}

void* _DRDeviceReadDVDLayer0DataZoneCapacity(void)
{
    if (verbose) puts("STUB: _DRDeviceReadDVDLayer0DataZoneCapacity called");
    return NULL;
}

void* _DRDeviceReadDVDLayerDescriptor(void)
{
    if (verbose) puts("STUB: _DRDeviceReadDVDLayerDescriptor called");
    return NULL;
}

void* _DRDeviceReadDVDLayerDescriptor2(void)
{
    if (verbose) puts("STUB: _DRDeviceReadDVDLayerDescriptor2 called");
    return NULL;
}

void* _DRDeviceReadDVDManualLayerJump(void)
{
    if (verbose) puts("STUB: _DRDeviceReadDVDManualLayerJump called");
    return NULL;
}

void* _DRDeviceReadDVDShiftedMiddleArea(void)
{
    if (verbose) puts("STUB: _DRDeviceReadDVDShiftedMiddleArea called");
    return NULL;
}

void* _DRDeviceReadDVDStructure(void)
{
    if (verbose) puts("STUB: _DRDeviceReadDVDStructure called");
    return NULL;
}

void* _DRDeviceReadDVDWriteProtect(void)
{
    if (verbose) puts("STUB: _DRDeviceReadDVDWriteProtect called");
    return NULL;
}

void* _DRDeviceReadDiscInfo(void)
{
    if (verbose) puts("STUB: _DRDeviceReadDiscInfo called");
    return NULL;
}

void* _DRDeviceReadDiscInfoType(void)
{
    if (verbose) puts("STUB: _DRDeviceReadDiscInfoType called");
    return NULL;
}

void* _DRDeviceReadDiscStructure(void)
{
    if (verbose) puts("STUB: _DRDeviceReadDiscStructure called");
    return NULL;
}

void* _DRDeviceReadFormatCapacities(void)
{
    if (verbose) puts("STUB: _DRDeviceReadFormatCapacities called");
    return NULL;
}

void* _DRDeviceReadSubchannel(void)
{
    if (verbose) puts("STUB: _DRDeviceReadSubchannel called");
    return NULL;
}

void* _DRDeviceReadTOC(void)
{
    if (verbose) puts("STUB: _DRDeviceReadTOC called");
    return NULL;
}

void* _DRDeviceReadTOC2(void)
{
    if (verbose) puts("STUB: _DRDeviceReadTOC2 called");
    return NULL;
}

void* _DRDeviceReadTrackInfo(void)
{
    if (verbose) puts("STUB: _DRDeviceReadTrackInfo called");
    return NULL;
}

void* _DRDeviceRemoveCachedValue(void)
{
    if (verbose) puts("STUB: _DRDeviceRemoveCachedValue called");
    return NULL;
}

void* _DRDeviceRequestSense(void)
{
    if (verbose) puts("STUB: _DRDeviceRequestSense called");
    return NULL;
}

void* _DRDeviceReserveTrack(void)
{
    if (verbose) puts("STUB: _DRDeviceReserveTrack called");
    return NULL;
}

void* _DRDeviceReserveTrack2(void)
{
    if (verbose) puts("STUB: _DRDeviceReserveTrack2 called");
    return NULL;
}

void* _DRDeviceSendCueSheet(void)
{
    if (verbose) puts("STUB: _DRDeviceSendCueSheet called");
    return NULL;
}

void* _DRDeviceSendDVDCPMInfo(void)
{
    if (verbose) puts("STUB: _DRDeviceSendDVDCPMInfo called");
    return NULL;
}

void* _DRDeviceSendDVDJumpIntervalSize(void)
{
    if (verbose) puts("STUB: _DRDeviceSendDVDJumpIntervalSize called");
    return NULL;
}

void* _DRDeviceSendDVDLayer0DataZoneCapacity(void)
{
    if (verbose) puts("STUB: _DRDeviceSendDVDLayer0DataZoneCapacity called");
    return NULL;
}

void* _DRDeviceSendDVDManualLayerJump(void)
{
    if (verbose) puts("STUB: _DRDeviceSendDVDManualLayerJump called");
    return NULL;
}

void* _DRDeviceSendDVDShiftedMiddleArea(void)
{
    if (verbose) puts("STUB: _DRDeviceSendDVDShiftedMiddleArea called");
    return NULL;
}

void* _DRDeviceSendDVDTimestamp(void)
{
    if (verbose) puts("STUB: _DRDeviceSendDVDTimestamp called");
    return NULL;
}

void* _DRDeviceSetCDSpeed(void)
{
    if (verbose) puts("STUB: _DRDeviceSetCDSpeed called");
    return NULL;
}

void* _DRDeviceSetCachedValue(void)
{
    if (verbose) puts("STUB: _DRDeviceSetCachedValue called");
    return NULL;
}

void* _DRDeviceSetDeviceSpeed(void)
{
    if (verbose) puts("STUB: _DRDeviceSetDeviceSpeed called");
    return NULL;
}

void* _DRDeviceSetStreamingPerformance(void)
{
    if (verbose) puts("STUB: _DRDeviceSetStreamingPerformance called");
    return NULL;
}

void* _DRDeviceSetWriteParamsPage(void)
{
    if (verbose) puts("STUB: _DRDeviceSetWriteParamsPage called");
    return NULL;
}

void* _DRDeviceStartStopUnit(void)
{
    if (verbose) puts("STUB: _DRDeviceStartStopUnit called");
    return NULL;
}

void* _DRDeviceSynchronizeCache(void)
{
    if (verbose) puts("STUB: _DRDeviceSynchronizeCache called");
    return NULL;
}

void* _DRDeviceUnlockDoor(void)
{
    if (verbose) puts("STUB: _DRDeviceUnlockDoor called");
    return NULL;
}

void* _DRDeviceUnquiesce(void)
{
    if (verbose) puts("STUB: _DRDeviceUnquiesce called");
    return NULL;
}

void* _DRDeviceUpdateThread(void)
{
    if (verbose) puts("STUB: _DRDeviceUpdateThread called");
    return NULL;
}

void* _DRDeviceVerify(void)
{
    if (verbose) puts("STUB: _DRDeviceVerify called");
    return NULL;
}

void* _DRDeviceWaitAndCloseSession(void)
{
    if (verbose) puts("STUB: _DRDeviceWaitAndCloseSession called");
    return NULL;
}

void* _DRDeviceWaitAndSynchronizeCache(void)
{
    if (verbose) puts("STUB: _DRDeviceWaitAndSynchronizeCache called");
    return NULL;
}

void* _DRDeviceWaitForBurnCompletion(void)
{
    if (verbose) puts("STUB: _DRDeviceWaitForBurnCompletion called");
    return NULL;
}

void* _DRDeviceWriteAndVerify(void)
{
    if (verbose) puts("STUB: _DRDeviceWriteAndVerify called");
    return NULL;
}

void* _DRDeviceWriteBlocks(void)
{
    if (verbose) puts("STUB: _DRDeviceWriteBlocks called");
    return NULL;
}

void* _DRDeviceWriteBufferZone1(void)
{
    if (verbose) puts("STUB: _DRDeviceWriteBufferZone1 called");
    return NULL;
}

void* _DREnableToolMode(void)
{
    if (verbose) puts("STUB: _DREnableToolMode called");
    return NULL;
}

void* _DREraseThread(void)
{
    if (verbose) puts("STUB: _DREraseThread called");
    return NULL;
}

void* _DRIsRemoteLoggingAvailable(void)
{
    if (verbose) puts("STUB: _DRIsRemoteLoggingAvailable called");
    return NULL;
}

void* _DRNotificationCenterAddObserver(void)
{
    if (verbose) puts("STUB: _DRNotificationCenterAddObserver called");
    return NULL;
}

void* _DRNotificationCenterCreateDistributed(void)
{
    if (verbose) puts("STUB: _DRNotificationCenterCreateDistributed called");
    return NULL;
}

void* _DRNotificationCenterGetObserverClientData(void)
{
    if (verbose) puts("STUB: _DRNotificationCenterGetObserverClientData called");
    return NULL;
}

void* _DRNotificationPost(void)
{
    if (verbose) puts("STUB: _DRNotificationPost called");
    return NULL;
}

void* _DRNotificationPostCoalescable(void)
{
    if (verbose) puts("STUB: _DRNotificationPostCoalescable called");
    return NULL;
}

void* _DRPrepareThread(void)
{
    if (verbose) puts("STUB: _DRPrepareThread called");
    return NULL;
}

void* _DRRemoteLogWithDevice(void)
{
    if (verbose) puts("STUB: _DRRemoteLogWithDevice called");
    return NULL;
}

void* _DRRemoteLogWithPath(void)
{
    if (verbose) puts("STUB: _DRRemoteLogWithPath called");
    return NULL;
}

void* _DRToolModeEnabled(void)
{
    if (verbose) puts("STUB: _DRToolModeEnabled called");
    return NULL;
}

void* _DRTrackSpeedTestThread(void)
{
    if (verbose) puts("STUB: _DRTrackSpeedTestThread called");
    return NULL;
}

void* _DRWorkLoopThread(void)
{
    if (verbose) puts("STUB: _DRWorkLoopThread called");
    return NULL;
}
