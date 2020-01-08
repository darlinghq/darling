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


#ifndef _CoreMediaIO_H_
#define _CoreMediaIO_H_

#import <Foundation/Foundation.h>


void* CMIOAddAtExitListener(void);
void* CMIODeviceProcessAVCCommand(void);
void* CMIODeviceProcessRS422Command(void);
void* CMIODeviceStartStream(void);
void* CMIODeviceStopStream(void);
void* CMIOFileWritingControlTokenAllVolumesAreOutOfDiskSpace(void);
void* CMIOFileWritingControlTokenBufferIsIncompatibleWithCurrentFile(void);
void* CMIOFileWritingControlTokenCreate(void);
void* CMIOFileWritingControlTokenCreateOutputPathToCapturePathDictionary(void);
void* CMIOFileWritingControlTokenCurrentVolumeIsOutOfDiskSpace(void);
void* CMIOFileWritingControlTokenGetCompletedFileSizeEstimate(void);
void* CMIOFileWritingControlTokenGetDiscontinuityFlags(void);
void* CMIOFileWritingControlTokenGetFileDuration(void);
void* CMIOFileWritingControlTokenGetFileSize(void);
void* CMIOFileWritingControlTokenGetFileWriterOptionsDictionary(void);
void* CMIOFileWritingControlTokenGetSMPTETime(void);
void* CMIOFileWritingControlTokenGetSampleBuffer(void);
void* CMIOFileWritingControlTokenMaximumFileSizeHasBeenReached(void);
void* CMIOFileWritingControlTokenMaximumRecordDurationHasBeenReached(void);
void* CMIOFileWritingControlTokenPauseWriting(void);
void* CMIOFileWritingControlTokenRelease(void);
void* CMIOFileWritingControlTokenResumeWriting(void);
void* CMIOFileWritingControlTokenStartWriting(void);
void* CMIOFileWritingControlTokenStopWriting(void);
void* CMIOFileWritingControlTokenTriggerFileWritingControlCallback(void);
void* CMIOFileWritingControlTokenUnitCanPauseAndResumeWriting(void);
void* CMIOFileWritingControlTokenUnitCanStartWriting(void);
void* CMIOFileWritingControlTokenUnitIsPaused(void);
void* CMIOFileWritingControlTokenUnitIsWriting(void);
void* CMIOFormatDescriptionGetManufacturerCode(void);
void* CMIOFormatDescriptionGetOnlyHasIFrames(void);
void* CMIOFormatDescriptionSignifiesDiscontinuity(void);
void* CMIOGetUnitRegistry(void);
void* CMIOGlobalDTraceFireProbe(void);
void* CMIOGlobalDTraceProbeIsEnabled(void);
void* CMIOGraphAddPropertyListener(void);
void* CMIOGraphAddRenderNotify(void);
void* CMIOGraphClearConnections(void);
void* CMIOGraphConnectNodeInput(void);
void* CMIOGraphCountNodeConnections(void);
void* CMIOGraphCreate(void);
void* CMIOGraphCreateAndConfigureForUnits(void);
void* CMIOGraphCreateNode(void);
void* CMIOGraphDictionaryRepresentation(void);
void* CMIOGraphDisconnectNodeInput(void);
void* CMIOGraphGetConnectionInfo(void);
void* CMIOGraphGetIndNode(void);
void* CMIOGraphGetNodeByFunctionalDesignationAndIndex(void);
void* CMIOGraphGetNodeConnections(void);
void* CMIOGraphGetNodeCount(void);
void* CMIOGraphGetNodeInfo(void);
void* CMIOGraphGetNotificationCenter(void);
void* CMIOGraphGetNumberOfConnections(void);
void* CMIOGraphGetPropertiesDictionary(void);
void* CMIOGraphGetProperty(void);
void* CMIOGraphGetPropertyInfo(void);
void* CMIOGraphGetTypeID(void);
void* CMIOGraphInitialize(void);
void* CMIOGraphPause(void);
void* CMIOGraphRelease(void);
void* CMIOGraphRemoveNode(void);
void* CMIOGraphRemovePropertyListener(void);
void* CMIOGraphRemoveRenderNotify(void);
void* CMIOGraphResume(void);
void* CMIOGraphResyncPreview(void);
void* CMIOGraphRetain(void);
void* CMIOGraphSetProperties(void);
void* CMIOGraphSetProperty(void);
void* CMIOGraphStart(void);
void* CMIOGraphStop(void);
void* CMIOGraphUninitialize(void);
void* CMIOGraphUpdate(void);
void* CMIOHardwareBeMaster(void);
void* CMIOHardwareUnload(void);
void* CMIOMetadataCopyAsDictionary(void);
void* CMIOMetadataCopyCommonAsDictionary(void);
void* CMIOObjectAddPropertyListener(void);
void* CMIOObjectAddPropertyListenerBlock(void);
void* CMIOObjectCreate(void);
void* CMIOObjectGetPropertyData(void);
void* CMIOObjectGetPropertyDataSize(void);
void* CMIOObjectHasProperty(void);
void* CMIOObjectIsPropertySettable(void);
void* CMIOObjectPropertiesChanged(void);
void* CMIOObjectRemovePropertyListener(void);
void* CMIOObjectRemovePropertyListenerBlock(void);
void* CMIOObjectSetPropertyData(void);
void* CMIOObjectShow(void);
void* CMIOObjectsPublishedAndDied(void);
void* CMIOOutputCoordinatorCheckOutputTimebaseRate(void);
void* CMIOOutputCoordinatorCreate(void);
void* CMIOOutputCoordinatorDetach(void);
void* CMIOOutputCoordinatorGetCoordinatedOutputTimebase(void);
void* CMIOOutputCoordinatorGetEarliestVideoPresentationTime(void);
void* CMIOOutputCoordinatorGetMasterOutputDeviceLatency(void);
void* CMIOOutputCoordinatorGetNumberOfAudioOutputUnitsBeingCoordinated(void);
void* CMIOOutputCoordinatorGetNumberOfVideoOutputUnitsBeingCoordinated(void);
void* CMIOOutputCoordinatorGetOutputUnitForSynchronizerUnit(void);
void* CMIOOutputCoordinatorGetState(void);
void* CMIOOutputCoordinatorGetTypeID(void);
void* CMIOOutputCoordinatorIdle(void);
void* CMIOOutputCoordinatorNoteAudioOutputPrimed(void);
void* CMIOOutputCoordinatorNoteMasterOutputDeviceLatency(void);
void* CMIOOutputCoordinatorNoteVideoOutputPrimed(void);
void* CMIOOutputCoordinatorNoteVideoOutputSynchronizerPrimed(void);
void* CMIOOutputCoordinatorRelease(void);
void* CMIOOutputCoordinatorReset(void);
void* CMIOOutputCoordinatorRetain(void);
void* CMIORemoveAtExitListener(void);
void* CMIOSampleBufferCopyNonRequiredAttachments(void);
void* CMIOSampleBufferCopySampleAttachments(void);
void* CMIOSampleBufferCreate(void);
void* CMIOSampleBufferCreateForImageBuffer(void);
void* CMIOSampleBufferCreateNoDataMarker(void);
void* CMIOSampleBufferGetDiscontinuityFlags(void);
void* CMIOSampleBufferGetSequenceNumber(void);
void* CMIOSampleBufferSetDiscontinuityFlags(void);
void* CMIOSampleBufferSetSequenceNumber(void);
void* CMIOStreamClockConvertHostTimeToDeviceTime(void);
void* CMIOStreamClockCreate(void);
void* CMIOStreamClockInvalidate(void);
void* CMIOStreamClockPostTimingEvent(void);
void* CMIOStreamCopyBufferQueue(void);
void* CMIOStreamDeckCueTo(void);
void* CMIOStreamDeckJog(void);
void* CMIOStreamDeckPlay(void);
void* CMIOStreamDeckStop(void);
void* CMIOUnitCreateFromDescription(void);
void* CMIOUnitGetClassID(void);
void* CMIOUnitGetTypeID(void);
void* CMIOUnitRelease(void);
void* CMIOUnitRetain(void);
void* CMIOUnitUtilityCreateAudioCompressionOptionsDictionary(void);
void* CMIOUnitUtilityCreateAudioCompressionOptionsDictionary2(void);
void* CMIOUnitUtilityGetAudioCompressionOptionsDictionaryFields(void);
void* CMIOUnitUtilityGetAudioCompressionOptionsDictionaryFields2(void);

#endif
