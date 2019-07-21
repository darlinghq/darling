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


#include <CoreMediaIO/CoreMediaIO.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CMIOAddAtExitListener(void)
{
    if (verbose) puts("STUB: CMIOAddAtExitListener called");
    return NULL;
}

void* CMIODeviceProcessAVCCommand(void)
{
    if (verbose) puts("STUB: CMIODeviceProcessAVCCommand called");
    return NULL;
}

void* CMIODeviceProcessRS422Command(void)
{
    if (verbose) puts("STUB: CMIODeviceProcessRS422Command called");
    return NULL;
}

void* CMIODeviceStartStream(void)
{
    if (verbose) puts("STUB: CMIODeviceStartStream called");
    return NULL;
}

void* CMIODeviceStopStream(void)
{
    if (verbose) puts("STUB: CMIODeviceStopStream called");
    return NULL;
}

void* CMIOFileWritingControlTokenAllVolumesAreOutOfDiskSpace(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenAllVolumesAreOutOfDiskSpace called");
    return NULL;
}

void* CMIOFileWritingControlTokenBufferIsIncompatibleWithCurrentFile(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenBufferIsIncompatibleWithCurrentFile called");
    return NULL;
}

void* CMIOFileWritingControlTokenCreate(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenCreate called");
    return NULL;
}

void* CMIOFileWritingControlTokenCreateOutputPathToCapturePathDictionary(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenCreateOutputPathToCapturePathDictionary called");
    return NULL;
}

void* CMIOFileWritingControlTokenCurrentVolumeIsOutOfDiskSpace(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenCurrentVolumeIsOutOfDiskSpace called");
    return NULL;
}

void* CMIOFileWritingControlTokenGetCompletedFileSizeEstimate(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenGetCompletedFileSizeEstimate called");
    return NULL;
}

void* CMIOFileWritingControlTokenGetDiscontinuityFlags(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenGetDiscontinuityFlags called");
    return NULL;
}

void* CMIOFileWritingControlTokenGetFileDuration(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenGetFileDuration called");
    return NULL;
}

void* CMIOFileWritingControlTokenGetFileSize(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenGetFileSize called");
    return NULL;
}

void* CMIOFileWritingControlTokenGetFileWriterOptionsDictionary(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenGetFileWriterOptionsDictionary called");
    return NULL;
}

void* CMIOFileWritingControlTokenGetSMPTETime(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenGetSMPTETime called");
    return NULL;
}

void* CMIOFileWritingControlTokenGetSampleBuffer(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenGetSampleBuffer called");
    return NULL;
}

void* CMIOFileWritingControlTokenMaximumFileSizeHasBeenReached(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenMaximumFileSizeHasBeenReached called");
    return NULL;
}

void* CMIOFileWritingControlTokenMaximumRecordDurationHasBeenReached(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenMaximumRecordDurationHasBeenReached called");
    return NULL;
}

void* CMIOFileWritingControlTokenPauseWriting(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenPauseWriting called");
    return NULL;
}

void* CMIOFileWritingControlTokenRelease(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenRelease called");
    return NULL;
}

void* CMIOFileWritingControlTokenResumeWriting(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenResumeWriting called");
    return NULL;
}

void* CMIOFileWritingControlTokenStartWriting(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenStartWriting called");
    return NULL;
}

void* CMIOFileWritingControlTokenStopWriting(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenStopWriting called");
    return NULL;
}

void* CMIOFileWritingControlTokenTriggerFileWritingControlCallback(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenTriggerFileWritingControlCallback called");
    return NULL;
}

void* CMIOFileWritingControlTokenUnitCanPauseAndResumeWriting(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenUnitCanPauseAndResumeWriting called");
    return NULL;
}

void* CMIOFileWritingControlTokenUnitCanStartWriting(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenUnitCanStartWriting called");
    return NULL;
}

void* CMIOFileWritingControlTokenUnitIsPaused(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenUnitIsPaused called");
    return NULL;
}

void* CMIOFileWritingControlTokenUnitIsWriting(void)
{
    if (verbose) puts("STUB: CMIOFileWritingControlTokenUnitIsWriting called");
    return NULL;
}

void* CMIOFormatDescriptionGetManufacturerCode(void)
{
    if (verbose) puts("STUB: CMIOFormatDescriptionGetManufacturerCode called");
    return NULL;
}

void* CMIOFormatDescriptionGetOnlyHasIFrames(void)
{
    if (verbose) puts("STUB: CMIOFormatDescriptionGetOnlyHasIFrames called");
    return NULL;
}

void* CMIOFormatDescriptionSignifiesDiscontinuity(void)
{
    if (verbose) puts("STUB: CMIOFormatDescriptionSignifiesDiscontinuity called");
    return NULL;
}

void* CMIOGetUnitRegistry(void)
{
    if (verbose) puts("STUB: CMIOGetUnitRegistry called");
    return NULL;
}

void* CMIOGlobalDTraceFireProbe(void)
{
    if (verbose) puts("STUB: CMIOGlobalDTraceFireProbe called");
    return NULL;
}

void* CMIOGlobalDTraceProbeIsEnabled(void)
{
    if (verbose) puts("STUB: CMIOGlobalDTraceProbeIsEnabled called");
    return NULL;
}

void* CMIOGraphAddPropertyListener(void)
{
    if (verbose) puts("STUB: CMIOGraphAddPropertyListener called");
    return NULL;
}

void* CMIOGraphAddRenderNotify(void)
{
    if (verbose) puts("STUB: CMIOGraphAddRenderNotify called");
    return NULL;
}

void* CMIOGraphClearConnections(void)
{
    if (verbose) puts("STUB: CMIOGraphClearConnections called");
    return NULL;
}

void* CMIOGraphConnectNodeInput(void)
{
    if (verbose) puts("STUB: CMIOGraphConnectNodeInput called");
    return NULL;
}

void* CMIOGraphCountNodeConnections(void)
{
    if (verbose) puts("STUB: CMIOGraphCountNodeConnections called");
    return NULL;
}

void* CMIOGraphCreate(void)
{
    if (verbose) puts("STUB: CMIOGraphCreate called");
    return NULL;
}

void* CMIOGraphCreateAndConfigureForUnits(void)
{
    if (verbose) puts("STUB: CMIOGraphCreateAndConfigureForUnits called");
    return NULL;
}

void* CMIOGraphCreateNode(void)
{
    if (verbose) puts("STUB: CMIOGraphCreateNode called");
    return NULL;
}

void* CMIOGraphDictionaryRepresentation(void)
{
    if (verbose) puts("STUB: CMIOGraphDictionaryRepresentation called");
    return NULL;
}

void* CMIOGraphDisconnectNodeInput(void)
{
    if (verbose) puts("STUB: CMIOGraphDisconnectNodeInput called");
    return NULL;
}

void* CMIOGraphGetConnectionInfo(void)
{
    if (verbose) puts("STUB: CMIOGraphGetConnectionInfo called");
    return NULL;
}

void* CMIOGraphGetIndNode(void)
{
    if (verbose) puts("STUB: CMIOGraphGetIndNode called");
    return NULL;
}

void* CMIOGraphGetNodeByFunctionalDesignationAndIndex(void)
{
    if (verbose) puts("STUB: CMIOGraphGetNodeByFunctionalDesignationAndIndex called");
    return NULL;
}

void* CMIOGraphGetNodeConnections(void)
{
    if (verbose) puts("STUB: CMIOGraphGetNodeConnections called");
    return NULL;
}

void* CMIOGraphGetNodeCount(void)
{
    if (verbose) puts("STUB: CMIOGraphGetNodeCount called");
    return NULL;
}

void* CMIOGraphGetNodeInfo(void)
{
    if (verbose) puts("STUB: CMIOGraphGetNodeInfo called");
    return NULL;
}

void* CMIOGraphGetNotificationCenter(void)
{
    if (verbose) puts("STUB: CMIOGraphGetNotificationCenter called");
    return NULL;
}

void* CMIOGraphGetNumberOfConnections(void)
{
    if (verbose) puts("STUB: CMIOGraphGetNumberOfConnections called");
    return NULL;
}

void* CMIOGraphGetPropertiesDictionary(void)
{
    if (verbose) puts("STUB: CMIOGraphGetPropertiesDictionary called");
    return NULL;
}

void* CMIOGraphGetProperty(void)
{
    if (verbose) puts("STUB: CMIOGraphGetProperty called");
    return NULL;
}

void* CMIOGraphGetPropertyInfo(void)
{
    if (verbose) puts("STUB: CMIOGraphGetPropertyInfo called");
    return NULL;
}

void* CMIOGraphGetTypeID(void)
{
    if (verbose) puts("STUB: CMIOGraphGetTypeID called");
    return NULL;
}

void* CMIOGraphInitialize(void)
{
    if (verbose) puts("STUB: CMIOGraphInitialize called");
    return NULL;
}

void* CMIOGraphPause(void)
{
    if (verbose) puts("STUB: CMIOGraphPause called");
    return NULL;
}

void* CMIOGraphRelease(void)
{
    if (verbose) puts("STUB: CMIOGraphRelease called");
    return NULL;
}

void* CMIOGraphRemoveNode(void)
{
    if (verbose) puts("STUB: CMIOGraphRemoveNode called");
    return NULL;
}

void* CMIOGraphRemovePropertyListener(void)
{
    if (verbose) puts("STUB: CMIOGraphRemovePropertyListener called");
    return NULL;
}

void* CMIOGraphRemoveRenderNotify(void)
{
    if (verbose) puts("STUB: CMIOGraphRemoveRenderNotify called");
    return NULL;
}

void* CMIOGraphResume(void)
{
    if (verbose) puts("STUB: CMIOGraphResume called");
    return NULL;
}

void* CMIOGraphResyncPreview(void)
{
    if (verbose) puts("STUB: CMIOGraphResyncPreview called");
    return NULL;
}

void* CMIOGraphRetain(void)
{
    if (verbose) puts("STUB: CMIOGraphRetain called");
    return NULL;
}

void* CMIOGraphSetProperties(void)
{
    if (verbose) puts("STUB: CMIOGraphSetProperties called");
    return NULL;
}

void* CMIOGraphSetProperty(void)
{
    if (verbose) puts("STUB: CMIOGraphSetProperty called");
    return NULL;
}

void* CMIOGraphStart(void)
{
    if (verbose) puts("STUB: CMIOGraphStart called");
    return NULL;
}

void* CMIOGraphStop(void)
{
    if (verbose) puts("STUB: CMIOGraphStop called");
    return NULL;
}

void* CMIOGraphUninitialize(void)
{
    if (verbose) puts("STUB: CMIOGraphUninitialize called");
    return NULL;
}

void* CMIOGraphUpdate(void)
{
    if (verbose) puts("STUB: CMIOGraphUpdate called");
    return NULL;
}

void* CMIOHardwareBeMaster(void)
{
    if (verbose) puts("STUB: CMIOHardwareBeMaster called");
    return NULL;
}

void* CMIOHardwareUnload(void)
{
    if (verbose) puts("STUB: CMIOHardwareUnload called");
    return NULL;
}

void* CMIOMetadataCopyAsDictionary(void)
{
    if (verbose) puts("STUB: CMIOMetadataCopyAsDictionary called");
    return NULL;
}

void* CMIOMetadataCopyCommonAsDictionary(void)
{
    if (verbose) puts("STUB: CMIOMetadataCopyCommonAsDictionary called");
    return NULL;
}

void* CMIOObjectAddPropertyListener(void)
{
    if (verbose) puts("STUB: CMIOObjectAddPropertyListener called");
    return NULL;
}

void* CMIOObjectAddPropertyListenerBlock(void)
{
    if (verbose) puts("STUB: CMIOObjectAddPropertyListenerBlock called");
    return NULL;
}

void* CMIOObjectCreate(void)
{
    if (verbose) puts("STUB: CMIOObjectCreate called");
    return NULL;
}

void* CMIOObjectGetPropertyData(void)
{
    if (verbose) puts("STUB: CMIOObjectGetPropertyData called");
    return NULL;
}

void* CMIOObjectGetPropertyDataSize(void)
{
    if (verbose) puts("STUB: CMIOObjectGetPropertyDataSize called");
    return NULL;
}

void* CMIOObjectHasProperty(void)
{
    if (verbose) puts("STUB: CMIOObjectHasProperty called");
    return NULL;
}

void* CMIOObjectIsPropertySettable(void)
{
    if (verbose) puts("STUB: CMIOObjectIsPropertySettable called");
    return NULL;
}

void* CMIOObjectPropertiesChanged(void)
{
    if (verbose) puts("STUB: CMIOObjectPropertiesChanged called");
    return NULL;
}

void* CMIOObjectRemovePropertyListener(void)
{
    if (verbose) puts("STUB: CMIOObjectRemovePropertyListener called");
    return NULL;
}

void* CMIOObjectRemovePropertyListenerBlock(void)
{
    if (verbose) puts("STUB: CMIOObjectRemovePropertyListenerBlock called");
    return NULL;
}

void* CMIOObjectSetPropertyData(void)
{
    if (verbose) puts("STUB: CMIOObjectSetPropertyData called");
    return NULL;
}

void* CMIOObjectShow(void)
{
    if (verbose) puts("STUB: CMIOObjectShow called");
    return NULL;
}

void* CMIOObjectsPublishedAndDied(void)
{
    if (verbose) puts("STUB: CMIOObjectsPublishedAndDied called");
    return NULL;
}

void* CMIOOutputCoordinatorCheckOutputTimebaseRate(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorCheckOutputTimebaseRate called");
    return NULL;
}

void* CMIOOutputCoordinatorCreate(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorCreate called");
    return NULL;
}

void* CMIOOutputCoordinatorDetach(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorDetach called");
    return NULL;
}

void* CMIOOutputCoordinatorGetCoordinatedOutputTimebase(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorGetCoordinatedOutputTimebase called");
    return NULL;
}

void* CMIOOutputCoordinatorGetEarliestVideoPresentationTime(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorGetEarliestVideoPresentationTime called");
    return NULL;
}

void* CMIOOutputCoordinatorGetMasterOutputDeviceLatency(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorGetMasterOutputDeviceLatency called");
    return NULL;
}

void* CMIOOutputCoordinatorGetNumberOfAudioOutputUnitsBeingCoordinated(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorGetNumberOfAudioOutputUnitsBeingCoordinated called");
    return NULL;
}

void* CMIOOutputCoordinatorGetNumberOfVideoOutputUnitsBeingCoordinated(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorGetNumberOfVideoOutputUnitsBeingCoordinated called");
    return NULL;
}

void* CMIOOutputCoordinatorGetOutputUnitForSynchronizerUnit(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorGetOutputUnitForSynchronizerUnit called");
    return NULL;
}

void* CMIOOutputCoordinatorGetState(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorGetState called");
    return NULL;
}

void* CMIOOutputCoordinatorGetTypeID(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorGetTypeID called");
    return NULL;
}

void* CMIOOutputCoordinatorIdle(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorIdle called");
    return NULL;
}

void* CMIOOutputCoordinatorNoteAudioOutputPrimed(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorNoteAudioOutputPrimed called");
    return NULL;
}

void* CMIOOutputCoordinatorNoteMasterOutputDeviceLatency(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorNoteMasterOutputDeviceLatency called");
    return NULL;
}

void* CMIOOutputCoordinatorNoteVideoOutputPrimed(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorNoteVideoOutputPrimed called");
    return NULL;
}

void* CMIOOutputCoordinatorNoteVideoOutputSynchronizerPrimed(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorNoteVideoOutputSynchronizerPrimed called");
    return NULL;
}

void* CMIOOutputCoordinatorRelease(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorRelease called");
    return NULL;
}

void* CMIOOutputCoordinatorReset(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorReset called");
    return NULL;
}

void* CMIOOutputCoordinatorRetain(void)
{
    if (verbose) puts("STUB: CMIOOutputCoordinatorRetain called");
    return NULL;
}

void* CMIORemoveAtExitListener(void)
{
    if (verbose) puts("STUB: CMIORemoveAtExitListener called");
    return NULL;
}

void* CMIOSampleBufferCopyNonRequiredAttachments(void)
{
    if (verbose) puts("STUB: CMIOSampleBufferCopyNonRequiredAttachments called");
    return NULL;
}

void* CMIOSampleBufferCopySampleAttachments(void)
{
    if (verbose) puts("STUB: CMIOSampleBufferCopySampleAttachments called");
    return NULL;
}

void* CMIOSampleBufferCreate(void)
{
    if (verbose) puts("STUB: CMIOSampleBufferCreate called");
    return NULL;
}

void* CMIOSampleBufferCreateForImageBuffer(void)
{
    if (verbose) puts("STUB: CMIOSampleBufferCreateForImageBuffer called");
    return NULL;
}

void* CMIOSampleBufferCreateNoDataMarker(void)
{
    if (verbose) puts("STUB: CMIOSampleBufferCreateNoDataMarker called");
    return NULL;
}

void* CMIOSampleBufferGetDiscontinuityFlags(void)
{
    if (verbose) puts("STUB: CMIOSampleBufferGetDiscontinuityFlags called");
    return NULL;
}

void* CMIOSampleBufferGetSequenceNumber(void)
{
    if (verbose) puts("STUB: CMIOSampleBufferGetSequenceNumber called");
    return NULL;
}

void* CMIOSampleBufferSetDiscontinuityFlags(void)
{
    if (verbose) puts("STUB: CMIOSampleBufferSetDiscontinuityFlags called");
    return NULL;
}

void* CMIOSampleBufferSetSequenceNumber(void)
{
    if (verbose) puts("STUB: CMIOSampleBufferSetSequenceNumber called");
    return NULL;
}

void* CMIOStreamClockConvertHostTimeToDeviceTime(void)
{
    if (verbose) puts("STUB: CMIOStreamClockConvertHostTimeToDeviceTime called");
    return NULL;
}

void* CMIOStreamClockCreate(void)
{
    if (verbose) puts("STUB: CMIOStreamClockCreate called");
    return NULL;
}

void* CMIOStreamClockInvalidate(void)
{
    if (verbose) puts("STUB: CMIOStreamClockInvalidate called");
    return NULL;
}

void* CMIOStreamClockPostTimingEvent(void)
{
    if (verbose) puts("STUB: CMIOStreamClockPostTimingEvent called");
    return NULL;
}

void* CMIOStreamCopyBufferQueue(void)
{
    if (verbose) puts("STUB: CMIOStreamCopyBufferQueue called");
    return NULL;
}

void* CMIOStreamDeckCueTo(void)
{
    if (verbose) puts("STUB: CMIOStreamDeckCueTo called");
    return NULL;
}

void* CMIOStreamDeckJog(void)
{
    if (verbose) puts("STUB: CMIOStreamDeckJog called");
    return NULL;
}

void* CMIOStreamDeckPlay(void)
{
    if (verbose) puts("STUB: CMIOStreamDeckPlay called");
    return NULL;
}

void* CMIOStreamDeckStop(void)
{
    if (verbose) puts("STUB: CMIOStreamDeckStop called");
    return NULL;
}

void* CMIOUnitCreateFromDescription(void)
{
    if (verbose) puts("STUB: CMIOUnitCreateFromDescription called");
    return NULL;
}

void* CMIOUnitGetClassID(void)
{
    if (verbose) puts("STUB: CMIOUnitGetClassID called");
    return NULL;
}

void* CMIOUnitGetTypeID(void)
{
    if (verbose) puts("STUB: CMIOUnitGetTypeID called");
    return NULL;
}

void* CMIOUnitRelease(void)
{
    if (verbose) puts("STUB: CMIOUnitRelease called");
    return NULL;
}

void* CMIOUnitRetain(void)
{
    if (verbose) puts("STUB: CMIOUnitRetain called");
    return NULL;
}

void* CMIOUnitUtilityCreateAudioCompressionOptionsDictionary(void)
{
    if (verbose) puts("STUB: CMIOUnitUtilityCreateAudioCompressionOptionsDictionary called");
    return NULL;
}

void* CMIOUnitUtilityCreateAudioCompressionOptionsDictionary2(void)
{
    if (verbose) puts("STUB: CMIOUnitUtilityCreateAudioCompressionOptionsDictionary2 called");
    return NULL;
}

void* CMIOUnitUtilityGetAudioCompressionOptionsDictionaryFields(void)
{
    if (verbose) puts("STUB: CMIOUnitUtilityGetAudioCompressionOptionsDictionaryFields called");
    return NULL;
}

void* CMIOUnitUtilityGetAudioCompressionOptionsDictionaryFields2(void)
{
    if (verbose) puts("STUB: CMIOUnitUtilityGetAudioCompressionOptionsDictionaryFields2 called");
    return NULL;
}
