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


#include <CoreMedia/CoreMedia.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdlib.h>
#include <stdio.h>

const CFStringRef kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms = CFSTR("SampleDescriptionExtensionAtoms");
const CFStringRef kCMSampleAttachmentKey_NotSync = CFSTR("NotSync");

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AudioToolbox_AudioConverterDispose(void) {
    if (verbose) puts("STUB: AudioToolbox_AudioConverterDispose called");
    return NULL;
}

void* AudioToolbox_AudioConverterGetProperty(void) {
    if (verbose) puts("STUB: AudioToolbox_AudioConverterGetProperty called");
    return NULL;
}

void* AudioToolbox_AudioConverterNew(void) {
    if (verbose) puts("STUB: AudioToolbox_AudioConverterNew called");
    return NULL;
}

void* AudioToolbox_AudioConverterSetProperty(void) {
    if (verbose) puts("STUB: AudioToolbox_AudioConverterSetProperty called");
    return NULL;
}

void* AudioToolbox_AudioFormatGetProperty(void) {
    if (verbose) puts("STUB: AudioToolbox_AudioFormatGetProperty called");
    return NULL;
}

void* AudioToolbox_AudioFormatGetPropertyInfo(void) {
    if (verbose) puts("STUB: AudioToolbox_AudioFormatGetPropertyInfo called");
    return NULL;
}

void* CM8021ASClockAddAWDLPort(void) {
    if (verbose) puts("STUB: CM8021ASClockAddAWDLPort called");
    return NULL;
}

void* CM8021ASClockAddIPv4Port(void) {
    if (verbose) puts("STUB: CM8021ASClockAddIPv4Port called");
    return NULL;
}

void* CM8021ASClockAddIPv6Port(void) {
    if (verbose) puts("STUB: CM8021ASClockAddIPv6Port called");
    return NULL;
}

void* CM8021ASClockCopySystemClockIdentifier(void) {
    if (verbose) puts("STUB: CM8021ASClockCopySystemClockIdentifier called");
    return NULL;
}

void* CM8021ASClockCreate(void) {
    if (verbose) puts("STUB: CM8021ASClockCreate called");
    return NULL;
}

void* CM8021ASClockGetClockTimeForHostTime(void) {
    if (verbose) puts("STUB: CM8021ASClockGetClockTimeForHostTime called");
    return NULL;
}

void* CM8021ASClockGetHostTimeForClockTime(void) {
    if (verbose) puts("STUB: CM8021ASClockGetHostTimeForClockTime called");
    return NULL;
}

void* CM8021ASClockGetTypeID(void) {
    if (verbose) puts("STUB: CM8021ASClockGetTypeID called");
    return NULL;
}

void* CM8021ASClockIsLocked(void) {
    if (verbose) puts("STUB: CM8021ASClockIsLocked called");
    return NULL;
}

void* CM8021ASClockRemoveAWDLPort(void) {
    if (verbose) puts("STUB: CM8021ASClockRemoveAWDLPort called");
    return NULL;
}

void* CM8021ASClockRemoveIPv4Port(void) {
    if (verbose) puts("STUB: CM8021ASClockRemoveIPv4Port called");
    return NULL;
}

void* CM8021ASClockRemoveIPv6Port(void) {
    if (verbose) puts("STUB: CM8021ASClockRemoveIPv6Port called");
    return NULL;
}

void* CM8021ASClockSetAllPortRemoteSyncMessageIntervals(void) {
    if (verbose) puts("STUB: CM8021ASClockSetAllPortRemoteSyncMessageIntervals called");
    return NULL;
}

void* CMAudioDeviceClockCreate(void) {
    if (verbose) puts("STUB: CMAudioDeviceClockCreate called");
    return NULL;
}

void* CMAudioDeviceClockCreateFromAudioDeviceID(void) {
    if (verbose) puts("STUB: CMAudioDeviceClockCreateFromAudioDeviceID called");
    return NULL;
}

void* CMAudioDeviceClockGetAudioDevice(void) {
    if (verbose) puts("STUB: CMAudioDeviceClockGetAudioDevice called");
    return NULL;
}

void* CMAudioDeviceClockSetAudioDeviceID(void) {
    if (verbose) puts("STUB: CMAudioDeviceClockSetAudioDeviceID called");
    return NULL;
}

void* CMAudioDeviceClockSetAudioDeviceUID(void) {
    if (verbose) puts("STUB: CMAudioDeviceClockSetAudioDeviceUID called");
    return NULL;
}

void* CMAudioFormatDescriptionCopyAsBigEndianSoundDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: CMAudioFormatDescriptionCopyAsBigEndianSoundDescriptionBlockBuffer called");
    return NULL;
}

void* CMAudioFormatDescriptionCreate(void) {
    if (verbose) puts("STUB: CMAudioFormatDescriptionCreate called");
    return NULL;
}

void* CMAudioFormatDescriptionCreateFromBigEndianSoundDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: CMAudioFormatDescriptionCreateFromBigEndianSoundDescriptionBlockBuffer called");
    return NULL;
}

void* CMAudioFormatDescriptionCreateFromBigEndianSoundDescriptionData(void) {
    if (verbose) puts("STUB: CMAudioFormatDescriptionCreateFromBigEndianSoundDescriptionData called");
    return NULL;
}

void* CMAudioFormatDescriptionCreateSummary(void) {
    if (verbose) puts("STUB: CMAudioFormatDescriptionCreateSummary called");
    return NULL;
}

void* CMAudioFormatDescriptionEqual(void) {
    if (verbose) puts("STUB: CMAudioFormatDescriptionEqual called");
    return NULL;
}

void* CMAudioFormatDescriptionGetChannelCount(void) {
    if (verbose) puts("STUB: CMAudioFormatDescriptionGetChannelCount called");
    return NULL;
}

void* CMAudioFormatDescriptionGetChannelLayout(void) {
    if (verbose) puts("STUB: CMAudioFormatDescriptionGetChannelLayout called");
    return NULL;
}

void* CMAudioFormatDescriptionGetFormatList(void) {
    if (verbose) puts("STUB: CMAudioFormatDescriptionGetFormatList called");
    return NULL;
}

void* CMAudioFormatDescriptionGetMagicCookie(void) {
    if (verbose) puts("STUB: CMAudioFormatDescriptionGetMagicCookie called");
    return NULL;
}

void* CMAudioFormatDescriptionGetMostCompatibleFormat(void) {
    if (verbose) puts("STUB: CMAudioFormatDescriptionGetMostCompatibleFormat called");
    return NULL;
}

void* CMAudioFormatDescriptionGetRichestDecodableFormat(void) {
    if (verbose) puts("STUB: CMAudioFormatDescriptionGetRichestDecodableFormat called");
    return NULL;
}

void* CMAudioFormatDescriptionGetStreamBasicDescription(void) {
    if (verbose) puts("STUB: CMAudioFormatDescriptionGetStreamBasicDescription called");
    return NULL;
}

void* CMAudioSampleBufferCreateReadyWithPacketDescriptions(void) {
    if (verbose) puts("STUB: CMAudioSampleBufferCreateReadyWithPacketDescriptions called");
    return NULL;
}

void* CMAudioSampleBufferCreateWithPacketDescriptions(void) {
    if (verbose) puts("STUB: CMAudioSampleBufferCreateWithPacketDescriptions called");
    return NULL;
}

void* CMBaseClassGetCFTypeID(void) {
    if (verbose) puts("STUB: CMBaseClassGetCFTypeID called");
    return NULL;
}

void* CMBaseGetClassID(void) {
    if (verbose) puts("STUB: CMBaseGetClassID called");
    return NULL;
}

void* CMBaseObjectGetClassID(void) {
    if (verbose) puts("STUB: CMBaseObjectGetClassID called");
    return NULL;
}

void* CMBaseObjectGetDerivedStorage(void) {
    if (verbose) puts("STUB: CMBaseObjectGetDerivedStorage called");
    return NULL;
}

void* CMBaseObjectGetProtocolVTable(void) {
    if (verbose) puts("STUB: CMBaseObjectGetProtocolVTable called");
    return NULL;
}

void* CMBaseObjectGetVTable(void) {
    if (verbose) puts("STUB: CMBaseObjectGetVTable called");
    return NULL;
}

void* CMBaseObjectImplementsProtocol(void) {
    if (verbose) puts("STUB: CMBaseObjectImplementsProtocol called");
    return NULL;
}

void* CMBaseObjectIsMemberOfClass(void) {
    if (verbose) puts("STUB: CMBaseObjectIsMemberOfClass called");
    return NULL;
}

void* CMBaseProtocolCopyDebugDescription(void) {
    if (verbose) puts("STUB: CMBaseProtocolCopyDebugDescription called");
    return NULL;
}

void* CMBlockBufferAccessDataBytes(void) {
    if (verbose) puts("STUB: CMBlockBufferAccessDataBytes called");
    return NULL;
}

void* CMBlockBufferAppendBufferReference(void) {
    if (verbose) puts("STUB: CMBlockBufferAppendBufferReference called");
    return NULL;
}

void* CMBlockBufferAppendMemoryBlock(void) {
    if (verbose) puts("STUB: CMBlockBufferAppendMemoryBlock called");
    return NULL;
}

void* CMBlockBufferAssureBlockMemory(void) {
    if (verbose) puts("STUB: CMBlockBufferAssureBlockMemory called");
    return NULL;
}

void* CMBlockBufferCopyDataBytes(void) {
    if (verbose) puts("STUB: CMBlockBufferCopyDataBytes called");
    return NULL;
}

void* CMBlockBufferCreateContiguous(void) {
    if (verbose) puts("STUB: CMBlockBufferCreateContiguous called");
    return NULL;
}

void* CMBlockBufferCreateEmpty(void) {
    if (verbose) puts("STUB: CMBlockBufferCreateEmpty called");
    return NULL;
}

void* CMBlockBufferCreateWithBufferReference(void) {
    if (verbose) puts("STUB: CMBlockBufferCreateWithBufferReference called");
    return NULL;
}

void* CMBlockBufferCreateWithMemoryBlock(void) {
    if (verbose) puts("STUB: CMBlockBufferCreateWithMemoryBlock called");
    return NULL;
}

void* CMBlockBufferFillDataBytes(void) {
    if (verbose) puts("STUB: CMBlockBufferFillDataBytes called");
    return NULL;
}

void* CMBlockBufferGetDataLength(void) {
    if (verbose) puts("STUB: CMBlockBufferGetDataLength called");
    return NULL;
}

void* CMBlockBufferGetDataPointer(void) {
    if (verbose) puts("STUB: CMBlockBufferGetDataPointer called");
    return NULL;
}

void* CMBlockBufferGetTypeID(void) {
    if (verbose) puts("STUB: CMBlockBufferGetTypeID called");
    return NULL;
}

void* CMBlockBufferIsEmpty(void) {
    if (verbose) puts("STUB: CMBlockBufferIsEmpty called");
    return NULL;
}

void* CMBlockBufferIsRangeContiguous(void) {
    if (verbose) puts("STUB: CMBlockBufferIsRangeContiguous called");
    return NULL;
}

void* CMBlockBufferReplaceDataBytes(void) {
    if (verbose) puts("STUB: CMBlockBufferReplaceDataBytes called");
    return NULL;
}

void* CMBufferQueueCallForEachBuffer(void) {
    if (verbose) puts("STUB: CMBufferQueueCallForEachBuffer called");
    return NULL;
}

void* CMBufferQueueContainsEndOfData(void) {
    if (verbose) puts("STUB: CMBufferQueueContainsEndOfData called");
    return NULL;
}

void* CMBufferQueueCreate(void) {
    if (verbose) puts("STUB: CMBufferQueueCreate called");
    return NULL;
}

void* CMBufferQueueDequeueAndRetain(void) {
    if (verbose) puts("STUB: CMBufferQueueDequeueAndRetain called");
    return NULL;
}

void* CMBufferQueueDequeueIfDataReadyAndRetain(void) {
    if (verbose) puts("STUB: CMBufferQueueDequeueIfDataReadyAndRetain called");
    return NULL;
}

void* CMBufferQueueEnqueue(void) {
    if (verbose) puts("STUB: CMBufferQueueEnqueue called");
    return NULL;
}

void* CMBufferQueueGetBufferCount(void) {
    if (verbose) puts("STUB: CMBufferQueueGetBufferCount called");
    return NULL;
}

void* CMBufferQueueGetCallbacksForSampleBuffersSortedByOutputPTS(void) {
    if (verbose) puts("STUB: CMBufferQueueGetCallbacksForSampleBuffersSortedByOutputPTS called");
    return NULL;
}

void* CMBufferQueueGetCallbacksForUnsortedSampleBuffers(void) {
    if (verbose) puts("STUB: CMBufferQueueGetCallbacksForUnsortedSampleBuffers called");
    return NULL;
}

void* CMBufferQueueGetDuration(void) {
    if (verbose) puts("STUB: CMBufferQueueGetDuration called");
    return NULL;
}

void* CMBufferQueueGetEndPresentationTimeStamp(void) {
    if (verbose) puts("STUB: CMBufferQueueGetEndPresentationTimeStamp called");
    return NULL;
}

void* CMBufferQueueGetFirstDecodeTimeStamp(void) {
    if (verbose) puts("STUB: CMBufferQueueGetFirstDecodeTimeStamp called");
    return NULL;
}

void* CMBufferQueueGetFirstPresentationTimeStamp(void) {
    if (verbose) puts("STUB: CMBufferQueueGetFirstPresentationTimeStamp called");
    return NULL;
}

void* CMBufferQueueGetHead(void) {
    if (verbose) puts("STUB: CMBufferQueueGetHead called");
    return NULL;
}

void* CMBufferQueueGetMaxPresentationTimeStamp(void) {
    if (verbose) puts("STUB: CMBufferQueueGetMaxPresentationTimeStamp called");
    return NULL;
}

void* CMBufferQueueGetMinDecodeTimeStamp(void) {
    if (verbose) puts("STUB: CMBufferQueueGetMinDecodeTimeStamp called");
    return NULL;
}

void* CMBufferQueueGetMinPresentationTimeStamp(void) {
    if (verbose) puts("STUB: CMBufferQueueGetMinPresentationTimeStamp called");
    return NULL;
}

void* CMBufferQueueGetTotalSize(void) {
    if (verbose) puts("STUB: CMBufferQueueGetTotalSize called");
    return NULL;
}

void* CMBufferQueueGetTypeID(void) {
    if (verbose) puts("STUB: CMBufferQueueGetTypeID called");
    return NULL;
}

void* CMBufferQueueInstallTrigger(void) {
    if (verbose) puts("STUB: CMBufferQueueInstallTrigger called");
    return NULL;
}

void* CMBufferQueueInstallTriggerWithIntegerThreshold(void) {
    if (verbose) puts("STUB: CMBufferQueueInstallTriggerWithIntegerThreshold called");
    return NULL;
}

void* CMBufferQueueIsAtEndOfData(void) {
    if (verbose) puts("STUB: CMBufferQueueIsAtEndOfData called");
    return NULL;
}

void* CMBufferQueueIsEmpty(void) {
    if (verbose) puts("STUB: CMBufferQueueIsEmpty called");
    return NULL;
}

void* CMBufferQueueMarkEndOfData(void) {
    if (verbose) puts("STUB: CMBufferQueueMarkEndOfData called");
    return NULL;
}

void* CMBufferQueueRemoveTrigger(void) {
    if (verbose) puts("STUB: CMBufferQueueRemoveTrigger called");
    return NULL;
}

void* CMBufferQueueReset(void) {
    if (verbose) puts("STUB: CMBufferQueueReset called");
    return NULL;
}

void* CMBufferQueueResetWithCallback(void) {
    if (verbose) puts("STUB: CMBufferQueueResetWithCallback called");
    return NULL;
}

void* CMBufferQueueSetValidationCallback(void) {
    if (verbose) puts("STUB: CMBufferQueueSetValidationCallback called");
    return NULL;
}

void* CMBufferQueueTestTrigger(void) {
    if (verbose) puts("STUB: CMBufferQueueTestTrigger called");
    return NULL;
}

void* CMByteStreamAppend(void) {
    if (verbose) puts("STUB: CMByteStreamAppend called");
    return NULL;
}

void* CMByteStreamAppendBlockBuffer(void) {
    if (verbose) puts("STUB: CMByteStreamAppendBlockBuffer called");
    return NULL;
}

void* CMByteStreamBaseGetTypeID(void) {
    if (verbose) puts("STUB: CMByteStreamBaseGetTypeID called");
    return NULL;
}

void* CMByteStreamCreateCompositeForRead(void) {
    if (verbose) puts("STUB: CMByteStreamCreateCompositeForRead called");
    return NULL;
}

void* CMByteStreamCreateForBlockBuffer(void) {
    if (verbose) puts("STUB: CMByteStreamCreateForBlockBuffer called");
    return NULL;
}

void* CMByteStreamCreateForFileURL(void) {
    if (verbose) puts("STUB: CMByteStreamCreateForFileURL called");
    return NULL;
}

void* CMByteStreamCreateWritableForBlockBuffer(void) {
    if (verbose) puts("STUB: CMByteStreamCreateWritableForBlockBuffer called");
    return NULL;
}

void* CMByteStreamGetCMBaseObject(void) {
    if (verbose) puts("STUB: CMByteStreamGetCMBaseObject called");
    return NULL;
}

void* CMByteStreamGetClassID(void) {
    if (verbose) puts("STUB: CMByteStreamGetClassID called");
    return NULL;
}

void* CMByteStreamWriteBlockBuffer(void) {
    if (verbose) puts("STUB: CMByteStreamWriteBlockBuffer called");
    return NULL;
}

void* CMClockConvertHostTimeToSystemUnits(void) {
    if (verbose) puts("STUB: CMClockConvertHostTimeToSystemUnits called");
    return NULL;
}

void* CMClockGetAnchorTime(void) {
    if (verbose) puts("STUB: CMClockGetAnchorTime called");
    return NULL;
}

void* CMClockGetHostTimeClock(void) {
    if (verbose) puts("STUB: CMClockGetHostTimeClock called");
    return NULL;
}

void* CMClockGetTime(void) {
    if (verbose) puts("STUB: CMClockGetTime called");
    return NULL;
}

void* CMClockGetTypeID(void) {
    if (verbose) puts("STUB: CMClockGetTypeID called");
    return NULL;
}

void* CMClockInvalidate(void) {
    if (verbose) puts("STUB: CMClockInvalidate called");
    return NULL;
}

void* CMClockMakeHostTimeFromSystemUnits(void) {
    if (verbose) puts("STUB: CMClockMakeHostTimeFromSystemUnits called");
    return NULL;
}

void* CMClockMightDrift(void) {
    if (verbose) puts("STUB: CMClockMightDrift called");
    return NULL;
}

void* CMClosedCaptionFormatDescriptionCopyAsBigEndianClosedCaptionDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: CMClosedCaptionFormatDescriptionCopyAsBigEndianClosedCaptionDescriptionBlockBuffer called");
    return NULL;
}

void* CMClosedCaptionFormatDescriptionCreateFromBigEndianClosedCaptionDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: CMClosedCaptionFormatDescriptionCreateFromBigEndianClosedCaptionDescriptionBlockBuffer called");
    return NULL;
}

void* CMClosedCaptionFormatDescriptionCreateFromBigEndianClosedCaptionDescriptionData(void) {
    if (verbose) puts("STUB: CMClosedCaptionFormatDescriptionCreateFromBigEndianClosedCaptionDescriptionData called");
    return NULL;
}

void* CMCopyDictionaryOfAttachments(void) {
    if (verbose) puts("STUB: CMCopyDictionaryOfAttachments called");
    return NULL;
}

void* CMCreateContiguousBlockBufferFromStream(void) {
    if (verbose) puts("STUB: CMCreateContiguousBlockBufferFromStream called");
    return NULL;
}

void* CMDerivedObjectCreate(void) {
    if (verbose) puts("STUB: CMDerivedObjectCreate called");
    return NULL;
}

void* CMDoesBigEndianSoundDescriptionRequireLegacyCBRSampleTableLayout(void) {
    if (verbose) puts("STUB: CMDoesBigEndianSoundDescriptionRequireLegacyCBRSampleTableLayout called");
    return NULL;
}

void* CMFormatDescriptionCreate(void) {
    if (verbose) puts("STUB: CMFormatDescriptionCreate called");
    return NULL;
}

void* CMFormatDescriptionEqual(void) {
    if (verbose) puts("STUB: CMFormatDescriptionEqual called");
    return NULL;
}

void* CMFormatDescriptionEqualIgnoringExtensionKeys(void) {
    if (verbose) puts("STUB: CMFormatDescriptionEqualIgnoringExtensionKeys called");
    return NULL;
}

void* CMFormatDescriptionGetExtension(void) {
    if (verbose) puts("STUB: CMFormatDescriptionGetExtension called");
    return NULL;
}

void* CMFormatDescriptionGetExtensions(void) {
    if (verbose) puts("STUB: CMFormatDescriptionGetExtensions called");
    return NULL;
}

void* CMFormatDescriptionGetMediaSubType(void) {
    if (verbose) puts("STUB: CMFormatDescriptionGetMediaSubType called");
    return NULL;
}

void* CMFormatDescriptionGetMediaType(void) {
    if (verbose) puts("STUB: CMFormatDescriptionGetMediaType called");
    return NULL;
}

void* CMFormatDescriptionGetTypeID(void) {
    if (verbose) puts("STUB: CMFormatDescriptionGetTypeID called");
    return NULL;
}

void* CMFormatDescriptionGetWidestColorPropertiesFromFormatDescriptions(void) {
    if (verbose) puts("STUB: CMFormatDescriptionGetWidestColorPropertiesFromFormatDescriptions called");
    return NULL;
}

void* CMGetAttachment(void) {
    if (verbose) puts("STUB: CMGetAttachment called");
    return NULL;
}

void* CMHapticFormatDescriptionCopyAsBigEndianHapticDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: CMHapticFormatDescriptionCopyAsBigEndianHapticDescriptionBlockBuffer called");
    return NULL;
}

void* CMHapticFormatDescriptionCreateFromBigEndianHapticDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: CMHapticFormatDescriptionCreateFromBigEndianHapticDescriptionBlockBuffer called");
    return NULL;
}

void* CMHapticFormatDescriptionCreateFromBigEndianHapticDescriptionData(void) {
    if (verbose) puts("STUB: CMHapticFormatDescriptionCreateFromBigEndianHapticDescriptionData called");
    return NULL;
}

void* CMMemoryPoolCreate(void) {
    if (verbose) puts("STUB: CMMemoryPoolCreate called");
    return NULL;
}

void* CMMemoryPoolFlush(void) {
    if (verbose) puts("STUB: CMMemoryPoolFlush called");
    return NULL;
}

void* CMMemoryPoolGetAllocator(void) {
    if (verbose) puts("STUB: CMMemoryPoolGetAllocator called");
    return NULL;
}

void* CMMemoryPoolGetTypeID(void) {
    if (verbose) puts("STUB: CMMemoryPoolGetTypeID called");
    return NULL;
}

void* CMMemoryPoolInvalidate(void) {
    if (verbose) puts("STUB: CMMemoryPoolInvalidate called");
    return NULL;
}

void* CMMetadataCreateIdentifierForKeyAndKeySpace(void) {
    if (verbose) puts("STUB: CMMetadataCreateIdentifierForKeyAndKeySpace called");
    return NULL;
}

void* CMMetadataCreateKeyFromIdentifier(void) {
    if (verbose) puts("STUB: CMMetadataCreateKeyFromIdentifier called");
    return NULL;
}

void* CMMetadataCreateKeyFromIdentifierAsCFData(void) {
    if (verbose) puts("STUB: CMMetadataCreateKeyFromIdentifierAsCFData called");
    return NULL;
}

void* CMMetadataCreateKeySpaceFromIdentifier(void) {
    if (verbose) puts("STUB: CMMetadataCreateKeySpaceFromIdentifier called");
    return NULL;
}

void* CMMetadataDataTypeRegistryDataTypeConformsToDataType(void) {
    if (verbose) puts("STUB: CMMetadataDataTypeRegistryDataTypeConformsToDataType called");
    return NULL;
}

void* CMMetadataDataTypeRegistryDataTypeIsBaseDataType(void) {
    if (verbose) puts("STUB: CMMetadataDataTypeRegistryDataTypeIsBaseDataType called");
    return NULL;
}

void* CMMetadataDataTypeRegistryDataTypeIsRegistered(void) {
    if (verbose) puts("STUB: CMMetadataDataTypeRegistryDataTypeIsRegistered called");
    return NULL;
}

void* CMMetadataDataTypeRegistryGetBaseDataTypeForConformingDataType(void) {
    if (verbose) puts("STUB: CMMetadataDataTypeRegistryGetBaseDataTypeForConformingDataType called");
    return NULL;
}

void* CMMetadataDataTypeRegistryGetBaseDataTypes(void) {
    if (verbose) puts("STUB: CMMetadataDataTypeRegistryGetBaseDataTypes called");
    return NULL;
}

void* CMMetadataDataTypeRegistryGetConformingDataTypes(void) {
    if (verbose) puts("STUB: CMMetadataDataTypeRegistryGetConformingDataTypes called");
    return NULL;
}

void* CMMetadataDataTypeRegistryGetDataTypeDescription(void) {
    if (verbose) puts("STUB: CMMetadataDataTypeRegistryGetDataTypeDescription called");
    return NULL;
}

void* CMMetadataDataTypeRegistryRegisterDataType(void) {
    if (verbose) puts("STUB: CMMetadataDataTypeRegistryRegisterDataType called");
    return NULL;
}

void* CMMetadataFormatDescriptionCopyAsBigEndianMetadataDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: CMMetadataFormatDescriptionCopyAsBigEndianMetadataDescriptionBlockBuffer called");
    return NULL;
}

void* CMMetadataFormatDescriptionCreateByMergingMetadataFormatDescriptions(void) {
    if (verbose) puts("STUB: CMMetadataFormatDescriptionCreateByMergingMetadataFormatDescriptions called");
    return NULL;
}

void* CMMetadataFormatDescriptionCreateFromBigEndianMetadataDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: CMMetadataFormatDescriptionCreateFromBigEndianMetadataDescriptionBlockBuffer called");
    return NULL;
}

void* CMMetadataFormatDescriptionCreateFromBigEndianMetadataDescriptionData(void) {
    if (verbose) puts("STUB: CMMetadataFormatDescriptionCreateFromBigEndianMetadataDescriptionData called");
    return NULL;
}

void* CMMetadataFormatDescriptionCreateWithKeys(void) {
    if (verbose) puts("STUB: CMMetadataFormatDescriptionCreateWithKeys called");
    return NULL;
}

void* CMMetadataFormatDescriptionCreateWithMetadataFormatDescriptionAndMetadataSpecifications(void) {
    if (verbose) puts("STUB: CMMetadataFormatDescriptionCreateWithMetadataFormatDescriptionAndMetadataSpecifications called");
    return NULL;
}

void* CMMetadataFormatDescriptionCreateWithMetadataSpecifications(void) {
    if (verbose) puts("STUB: CMMetadataFormatDescriptionCreateWithMetadataSpecifications called");
    return NULL;
}

void* CMMetadataFormatDescriptionGetIdentifiers(void) {
    if (verbose) puts("STUB: CMMetadataFormatDescriptionGetIdentifiers called");
    return NULL;
}

void* CMMetadataFormatDescriptionGetKeyWithLocalID(void) {
    if (verbose) puts("STUB: CMMetadataFormatDescriptionGetKeyWithLocalID called");
    return NULL;
}

void* CMMuxedFormatDescriptionCreate(void) {
    if (verbose) puts("STUB: CMMuxedFormatDescriptionCreate called");
    return NULL;
}

void* CMNotificationCenterAddListener(void) {
    if (verbose) puts("STUB: CMNotificationCenterAddListener called");
    return NULL;
}

void* CMNotificationCenterBarrier(void) {
    if (verbose) puts("STUB: CMNotificationCenterBarrier called");
    return NULL;
}

void* CMNotificationCenterCreate(void) {
    if (verbose) puts("STUB: CMNotificationCenterCreate called");
    return NULL;
}

void* CMNotificationCenterGetDefaultLocalCenter(void) {
    if (verbose) puts("STUB: CMNotificationCenterGetDefaultLocalCenter called");
    return NULL;
}

void* CMNotificationCenterGetTypeID(void) {
    if (verbose) puts("STUB: CMNotificationCenterGetTypeID called");
    return NULL;
}

void* CMNotificationCenterPostNotification(void) {
    if (verbose) puts("STUB: CMNotificationCenterPostNotification called");
    return NULL;
}

void* CMNotificationCenterRegisterForBarrierSupport(void) {
    if (verbose) puts("STUB: CMNotificationCenterRegisterForBarrierSupport called");
    return NULL;
}

void* CMNotificationCenterRemoveListener(void) {
    if (verbose) puts("STUB: CMNotificationCenterRemoveListener called");
    return NULL;
}

void* CMNotificationCenterUnregisterForBarrierSupport(void) {
    if (verbose) puts("STUB: CMNotificationCenterUnregisterForBarrierSupport called");
    return NULL;
}

void* CMPropagateAttachments(void) {
    if (verbose) puts("STUB: CMPropagateAttachments called");
    return NULL;
}

void* CMRemoveAllAttachments(void) {
    if (verbose) puts("STUB: CMRemoveAllAttachments called");
    return NULL;
}

void* CMRemoveAttachment(void) {
    if (verbose) puts("STUB: CMRemoveAttachment called");
    return NULL;
}

void* CMSampleBufferCallBlockForEachSample(void) {
    if (verbose) puts("STUB: CMSampleBufferCallBlockForEachSample called");
    return NULL;
}

void* CMSampleBufferCallForEachSample(void) {
    if (verbose) puts("STUB: CMSampleBufferCallForEachSample called");
    return NULL;
}

void* CMSampleBufferCopyPCMDataIntoAudioBufferList(void) {
    if (verbose) puts("STUB: CMSampleBufferCopyPCMDataIntoAudioBufferList called");
    return NULL;
}

void* CMSampleBufferCopySampleBufferForRange(void) {
    if (verbose) puts("STUB: CMSampleBufferCopySampleBufferForRange called");
    return NULL;
}

void* CMSampleBufferCreate(void) {
    if (verbose) puts("STUB: CMSampleBufferCreate called");
    return NULL;
}

void* CMSampleBufferCreateCopy(void) {
    if (verbose) puts("STUB: CMSampleBufferCreateCopy called");
    return NULL;
}

void* CMSampleBufferCreateCopyWithNewTiming(void) {
    if (verbose) puts("STUB: CMSampleBufferCreateCopyWithNewTiming called");
    return NULL;
}

void* CMSampleBufferCreateForImageBuffer(void) {
    if (verbose) puts("STUB: CMSampleBufferCreateForImageBuffer called");
    return NULL;
}

void* CMSampleBufferCreateReady(void) {
    if (verbose) puts("STUB: CMSampleBufferCreateReady called");
    return NULL;
}

void* CMSampleBufferCreateReadyWithImageBuffer(void) {
    if (verbose) puts("STUB: CMSampleBufferCreateReadyWithImageBuffer called");
    return NULL;
}

void* CMSampleBufferDataIsReady(void) {
    if (verbose) puts("STUB: CMSampleBufferDataIsReady called");
    return NULL;
}

void* CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(void) {
    if (verbose) puts("STUB: CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer called");
    return NULL;
}

void* CMSampleBufferGetAudioStreamPacketDescriptions(void) {
    if (verbose) puts("STUB: CMSampleBufferGetAudioStreamPacketDescriptions called");
    return NULL;
}

void* CMSampleBufferGetAudioStreamPacketDescriptionsPtr(void) {
    if (verbose) puts("STUB: CMSampleBufferGetAudioStreamPacketDescriptionsPtr called");
    return NULL;
}

void* CMSampleBufferGetDataBuffer(void) {
    if (verbose) puts("STUB: CMSampleBufferGetDataBuffer called");
    return NULL;
}

void* CMSampleBufferGetDecodeTimeStamp(void) {
    if (verbose) puts("STUB: CMSampleBufferGetDecodeTimeStamp called");
    return NULL;
}

void* CMSampleBufferGetDuration(void) {
    if (verbose) puts("STUB: CMSampleBufferGetDuration called");
    return NULL;
}

void* CMSampleBufferGetFormatDescription(void) {
    if (verbose) puts("STUB: CMSampleBufferGetFormatDescription called");
    return NULL;
}

void* CMSampleBufferGetImageBuffer(void) {
    if (verbose) puts("STUB: CMSampleBufferGetImageBuffer called");
    return NULL;
}

void* CMSampleBufferGetNumSamples(void) {
    if (verbose) puts("STUB: CMSampleBufferGetNumSamples called");
    return NULL;
}

void* CMSampleBufferGetOutputDecodeTimeStamp(void) {
    if (verbose) puts("STUB: CMSampleBufferGetOutputDecodeTimeStamp called");
    return NULL;
}

void* CMSampleBufferGetOutputDuration(void) {
    if (verbose) puts("STUB: CMSampleBufferGetOutputDuration called");
    return NULL;
}

void* CMSampleBufferGetOutputPresentationTimeStamp(void) {
    if (verbose) puts("STUB: CMSampleBufferGetOutputPresentationTimeStamp called");
    return NULL;
}

void* CMSampleBufferGetOutputSampleTimingInfoArray(void) {
    if (verbose) puts("STUB: CMSampleBufferGetOutputSampleTimingInfoArray called");
    return NULL;
}

void* CMSampleBufferGetPresentationTimeStamp(void) {
    if (verbose) puts("STUB: CMSampleBufferGetPresentationTimeStamp called");
    return NULL;
}

void* CMSampleBufferGetSampleAttachmentsArray(void) {
    if (verbose) puts("STUB: CMSampleBufferGetSampleAttachmentsArray called");
    return NULL;
}

void* CMSampleBufferGetSampleSize(void) {
    if (verbose) puts("STUB: CMSampleBufferGetSampleSize called");
    return NULL;
}

void* CMSampleBufferGetSampleSizeArray(void) {
    if (verbose) puts("STUB: CMSampleBufferGetSampleSizeArray called");
    return NULL;
}

void* CMSampleBufferGetSampleTimingInfo(void) {
    if (verbose) puts("STUB: CMSampleBufferGetSampleTimingInfo called");
    return NULL;
}

void* CMSampleBufferGetSampleTimingInfoArray(void) {
    if (verbose) puts("STUB: CMSampleBufferGetSampleTimingInfoArray called");
    return NULL;
}

void* CMSampleBufferGetTotalSampleSize(void) {
    if (verbose) puts("STUB: CMSampleBufferGetTotalSampleSize called");
    return NULL;
}

void* CMSampleBufferGetTypeID(void) {
    if (verbose) puts("STUB: CMSampleBufferGetTypeID called");
    return NULL;
}

void* CMSampleBufferHasDataFailed(void) {
    if (verbose) puts("STUB: CMSampleBufferHasDataFailed called");
    return NULL;
}

void* CMSampleBufferInvalidate(void) {
    if (verbose) puts("STUB: CMSampleBufferInvalidate called");
    return NULL;
}

void* CMSampleBufferIsValid(void) {
    if (verbose) puts("STUB: CMSampleBufferIsValid called");
    return NULL;
}

void* CMSampleBufferMakeDataReady(void) {
    if (verbose) puts("STUB: CMSampleBufferMakeDataReady called");
    return NULL;
}

void* CMSampleBufferSetDataBuffer(void) {
    if (verbose) puts("STUB: CMSampleBufferSetDataBuffer called");
    return NULL;
}

void* CMSampleBufferSetDataBufferFromAudioBufferList(void) {
    if (verbose) puts("STUB: CMSampleBufferSetDataBufferFromAudioBufferList called");
    return NULL;
}

void* CMSampleBufferSetDataFailed(void) {
    if (verbose) puts("STUB: CMSampleBufferSetDataFailed called");
    return NULL;
}

void* CMSampleBufferSetDataReady(void) {
    if (verbose) puts("STUB: CMSampleBufferSetDataReady called");
    return NULL;
}

void* CMSampleBufferSetInvalidateCallback(void) {
    if (verbose) puts("STUB: CMSampleBufferSetInvalidateCallback called");
    return NULL;
}

void* CMSampleBufferSetInvalidateHandler(void) {
    if (verbose) puts("STUB: CMSampleBufferSetInvalidateHandler called");
    return NULL;
}

void* CMSampleBufferSetOutputPresentationTimeStamp(void) {
    if (verbose) puts("STUB: CMSampleBufferSetOutputPresentationTimeStamp called");
    return NULL;
}

void* CMSampleBufferTrackDataReadiness(void) {
    if (verbose) puts("STUB: CMSampleBufferTrackDataReadiness called");
    return NULL;
}

void* CMSetAttachment(void) {
    if (verbose) puts("STUB: CMSetAttachment called");
    return NULL;
}

void* CMSetAttachments(void) {
    if (verbose) puts("STUB: CMSetAttachments called");
    return NULL;
}

void* CMSimpleQueueCreate(void) {
    if (verbose) puts("STUB: CMSimpleQueueCreate called");
    return NULL;
}

void* CMSimpleQueueDequeue(void) {
    if (verbose) puts("STUB: CMSimpleQueueDequeue called");
    return NULL;
}

void* CMSimpleQueueEnqueue(void) {
    if (verbose) puts("STUB: CMSimpleQueueEnqueue called");
    return NULL;
}

void* CMSimpleQueueGetCapacity(void) {
    if (verbose) puts("STUB: CMSimpleQueueGetCapacity called");
    return NULL;
}

void* CMSimpleQueueGetCount(void) {
    if (verbose) puts("STUB: CMSimpleQueueGetCount called");
    return NULL;
}

void* CMSimpleQueueGetHead(void) {
    if (verbose) puts("STUB: CMSimpleQueueGetHead called");
    return NULL;
}

void* CMSimpleQueueGetTypeID(void) {
    if (verbose) puts("STUB: CMSimpleQueueGetTypeID called");
    return NULL;
}

void* CMSimpleQueueReset(void) {
    if (verbose) puts("STUB: CMSimpleQueueReset called");
    return NULL;
}

void* CMSwapBigEndianClosedCaptionDescriptionToHost(void) {
    if (verbose) puts("STUB: CMSwapBigEndianClosedCaptionDescriptionToHost called");
    return NULL;
}

void* CMSwapBigEndianHapticDescriptionToHost(void) {
    if (verbose) puts("STUB: CMSwapBigEndianHapticDescriptionToHost called");
    return NULL;
}

void* CMSwapBigEndianImageDescriptionToHost(void) {
    if (verbose) puts("STUB: CMSwapBigEndianImageDescriptionToHost called");
    return NULL;
}

void* CMSwapBigEndianMetadataDescriptionToHost(void) {
    if (verbose) puts("STUB: CMSwapBigEndianMetadataDescriptionToHost called");
    return NULL;
}

void* CMSwapBigEndianSoundDescriptionToHost(void) {
    if (verbose) puts("STUB: CMSwapBigEndianSoundDescriptionToHost called");
    return NULL;
}

void* CMSwapBigEndianTextDescriptionToHost(void) {
    if (verbose) puts("STUB: CMSwapBigEndianTextDescriptionToHost called");
    return NULL;
}

void* CMSwapBigEndianTimeCodeDescriptionToHost(void) {
    if (verbose) puts("STUB: CMSwapBigEndianTimeCodeDescriptionToHost called");
    return NULL;
}

void* CMSwapHostEndianClosedCaptionDescriptionToBig(void) {
    if (verbose) puts("STUB: CMSwapHostEndianClosedCaptionDescriptionToBig called");
    return NULL;
}

void* CMSwapHostEndianHapticDescriptionToBig(void) {
    if (verbose) puts("STUB: CMSwapHostEndianHapticDescriptionToBig called");
    return NULL;
}

void* CMSwapHostEndianImageDescriptionToBig(void) {
    if (verbose) puts("STUB: CMSwapHostEndianImageDescriptionToBig called");
    return NULL;
}

void* CMSwapHostEndianMetadataDescriptionToBig(void) {
    if (verbose) puts("STUB: CMSwapHostEndianMetadataDescriptionToBig called");
    return NULL;
}

void* CMSwapHostEndianSoundDescriptionToBig(void) {
    if (verbose) puts("STUB: CMSwapHostEndianSoundDescriptionToBig called");
    return NULL;
}

void* CMSwapHostEndianTextDescriptionToBig(void) {
    if (verbose) puts("STUB: CMSwapHostEndianTextDescriptionToBig called");
    return NULL;
}

void* CMSwapHostEndianTimeCodeDescriptionToBig(void) {
    if (verbose) puts("STUB: CMSwapHostEndianTimeCodeDescriptionToBig called");
    return NULL;
}

void* CMSyncConvertTime(void) {
    if (verbose) puts("STUB: CMSyncConvertTime called");
    return NULL;
}

void* CMSyncGetRelativeRate(void) {
    if (verbose) puts("STUB: CMSyncGetRelativeRate called");
    return NULL;
}

void* CMSyncGetRelativeRateAndAnchorTime(void) {
    if (verbose) puts("STUB: CMSyncGetRelativeRateAndAnchorTime called");
    return NULL;
}

void* CMSyncGetTime(void) {
    if (verbose) puts("STUB: CMSyncGetTime called");
    return NULL;
}

void* CMSyncMightDrift(void) {
    if (verbose) puts("STUB: CMSyncMightDrift called");
    return NULL;
}

void* CMTextFormatDescriptionCopyAsBigEndianTextDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: CMTextFormatDescriptionCopyAsBigEndianTextDescriptionBlockBuffer called");
    return NULL;
}

void* CMTextFormatDescriptionCreateFromBigEndianTextDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: CMTextFormatDescriptionCreateFromBigEndianTextDescriptionBlockBuffer called");
    return NULL;
}

void* CMTextFormatDescriptionCreateFromBigEndianTextDescriptionData(void) {
    if (verbose) puts("STUB: CMTextFormatDescriptionCreateFromBigEndianTextDescriptionData called");
    return NULL;
}

void* CMTextFormatDescriptionGetDefaultStyle(void) {
    if (verbose) puts("STUB: CMTextFormatDescriptionGetDefaultStyle called");
    return NULL;
}

void* CMTextFormatDescriptionGetDefaultTextBox(void) {
    if (verbose) puts("STUB: CMTextFormatDescriptionGetDefaultTextBox called");
    return NULL;
}

void* CMTextFormatDescriptionGetDisplayFlags(void) {
    if (verbose) puts("STUB: CMTextFormatDescriptionGetDisplayFlags called");
    return NULL;
}

void* CMTextFormatDescriptionGetFontName(void) {
    if (verbose) puts("STUB: CMTextFormatDescriptionGetFontName called");
    return NULL;
}

void* CMTextFormatDescriptionGetJustification(void) {
    if (verbose) puts("STUB: CMTextFormatDescriptionGetJustification called");
    return NULL;
}

void* CMTimeAbsoluteValue(void) {
    if (verbose) puts("STUB: CMTimeAbsoluteValue called");
    return NULL;
}

void* CMTimeAdd(void) {
    if (verbose) puts("STUB: CMTimeAdd called");
    return NULL;
}

void* CMTimeClampToRange(void) {
    if (verbose) puts("STUB: CMTimeClampToRange called");
    return NULL;
}

void* CMTimeCodeFormatDescriptionCopyAsBigEndianTimeCodeDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: CMTimeCodeFormatDescriptionCopyAsBigEndianTimeCodeDescriptionBlockBuffer called");
    return NULL;
}

void* CMTimeCodeFormatDescriptionCreate(void) {
    if (verbose) puts("STUB: CMTimeCodeFormatDescriptionCreate called");
    return NULL;
}

void* CMTimeCodeFormatDescriptionCreateFromBigEndianTimeCodeDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: CMTimeCodeFormatDescriptionCreateFromBigEndianTimeCodeDescriptionBlockBuffer called");
    return NULL;
}

void* CMTimeCodeFormatDescriptionCreateFromBigEndianTimeCodeDescriptionData(void) {
    if (verbose) puts("STUB: CMTimeCodeFormatDescriptionCreateFromBigEndianTimeCodeDescriptionData called");
    return NULL;
}

void* CMTimeCodeFormatDescriptionGetFrameDuration(void) {
    if (verbose) puts("STUB: CMTimeCodeFormatDescriptionGetFrameDuration called");
    return NULL;
}

void* CMTimeCodeFormatDescriptionGetFrameQuanta(void) {
    if (verbose) puts("STUB: CMTimeCodeFormatDescriptionGetFrameQuanta called");
    return NULL;
}

void* CMTimeCodeFormatDescriptionGetTimeCodeFlags(void) {
    if (verbose) puts("STUB: CMTimeCodeFormatDescriptionGetTimeCodeFlags called");
    return NULL;
}

void* CMTimeCompare(void) {
    if (verbose) puts("STUB: CMTimeCompare called");
    return NULL;
}

void* CMTimeConvertScale(void) {
    if (verbose) puts("STUB: CMTimeConvertScale called");
    return NULL;
}

void* CMTimeCopyAsCVBufferTimeDictionary(void) {
    if (verbose) puts("STUB: CMTimeCopyAsCVBufferTimeDictionary called");
    return NULL;
}

void* CMTimeCopyAsDictionary(void) {
    if (verbose) puts("STUB: CMTimeCopyAsDictionary called");
    return NULL;
}

void* CMTimeCopyDescription(void) {
    if (verbose) puts("STUB: CMTimeCopyDescription called");
    return NULL;
}

void* CMTimeGetSeconds(void) {
    if (verbose) puts("STUB: CMTimeGetSeconds called");
    return NULL;
}

void* CMTimeMake(void) {
    if (verbose) puts("STUB: CMTimeMake called");
    return NULL;
}

void* CMTimeMakeFromCVBufferTimeDictionary(void) {
    if (verbose) puts("STUB: CMTimeMakeFromCVBufferTimeDictionary called");
    return NULL;
}

void* CMTimeMakeFromDictionary(void) {
    if (verbose) puts("STUB: CMTimeMakeFromDictionary called");
    return NULL;
}

void* CMTimeMakeWithEpoch(void) {
    if (verbose) puts("STUB: CMTimeMakeWithEpoch called");
    return NULL;
}

void* CMTimeMakeWithSeconds(void) {
    if (verbose) puts("STUB: CMTimeMakeWithSeconds called");
    return NULL;
}

void* CMTimeMapDurationFromRangeToRange(void) {
    if (verbose) puts("STUB: CMTimeMapDurationFromRangeToRange called");
    return NULL;
}

void* CMTimeMapTimeFromRangeToRange(void) {
    if (verbose) puts("STUB: CMTimeMapTimeFromRangeToRange called");
    return NULL;
}

void* CMTimeMappingCopyAsDictionary(void) {
    if (verbose) puts("STUB: CMTimeMappingCopyAsDictionary called");
    return NULL;
}

void* CMTimeMappingCopyDescription(void) {
    if (verbose) puts("STUB: CMTimeMappingCopyDescription called");
    return NULL;
}

void* CMTimeMappingMake(void) {
    if (verbose) puts("STUB: CMTimeMappingMake called");
    return NULL;
}

void* CMTimeMappingMakeEmpty(void) {
    if (verbose) puts("STUB: CMTimeMappingMakeEmpty called");
    return NULL;
}

void* CMTimeMappingMakeFromDictionary(void) {
    if (verbose) puts("STUB: CMTimeMappingMakeFromDictionary called");
    return NULL;
}

void* CMTimeMappingShow(void) {
    if (verbose) puts("STUB: CMTimeMappingShow called");
    return NULL;
}

void* CMTimeMaximum(void) {
    if (verbose) puts("STUB: CMTimeMaximum called");
    return NULL;
}

void* CMTimeMinimum(void) {
    if (verbose) puts("STUB: CMTimeMinimum called");
    return NULL;
}

void* CMTimeMultiply(void) {
    if (verbose) puts("STUB: CMTimeMultiply called");
    return NULL;
}

void* CMTimeMultiplyByFloat64(void) {
    if (verbose) puts("STUB: CMTimeMultiplyByFloat64 called");
    return NULL;
}

void* CMTimeMultiplyByRatio(void) {
    if (verbose) puts("STUB: CMTimeMultiplyByRatio called");
    return NULL;
}

void* CMTimeRangeContainsTime(void) {
    if (verbose) puts("STUB: CMTimeRangeContainsTime called");
    return NULL;
}

void* CMTimeRangeContainsTimeRange(void) {
    if (verbose) puts("STUB: CMTimeRangeContainsTimeRange called");
    return NULL;
}

void* CMTimeRangeCopyAsDictionary(void) {
    if (verbose) puts("STUB: CMTimeRangeCopyAsDictionary called");
    return NULL;
}

void* CMTimeRangeCopyDescription(void) {
    if (verbose) puts("STUB: CMTimeRangeCopyDescription called");
    return NULL;
}

void* CMTimeRangeEqual(void) {
    if (verbose) puts("STUB: CMTimeRangeEqual called");
    return NULL;
}

void* CMTimeRangeFromTimeToTime(void) {
    if (verbose) puts("STUB: CMTimeRangeFromTimeToTime called");
    return NULL;
}

void* CMTimeRangeGetEnd(void) {
    if (verbose) puts("STUB: CMTimeRangeGetEnd called");
    return NULL;
}

void* CMTimeRangeGetIntersection(void) {
    if (verbose) puts("STUB: CMTimeRangeGetIntersection called");
    return NULL;
}

void* CMTimeRangeGetUnion(void) {
    if (verbose) puts("STUB: CMTimeRangeGetUnion called");
    return NULL;
}

void* CMTimeRangeMake(void) {
    if (verbose) puts("STUB: CMTimeRangeMake called");
    return NULL;
}

void* CMTimeRangeMakeFromDictionary(void) {
    if (verbose) puts("STUB: CMTimeRangeMakeFromDictionary called");
    return NULL;
}

void* CMTimeRangeShow(void) {
    if (verbose) puts("STUB: CMTimeRangeShow called");
    return NULL;
}

void* CMTimeShow(void) {
    if (verbose) puts("STUB: CMTimeShow called");
    return NULL;
}

void* CMTimeSubtract(void) {
    if (verbose) puts("STUB: CMTimeSubtract called");
    return NULL;
}

void* CMTimeSyncClockCreateForSystemDomainClockIdentifier(void) {
    if (verbose) puts("STUB: CMTimeSyncClockCreateForSystemDomainClockIdentifier called");
    return NULL;
}

void* CMTimebaseAddTimer(void) {
    if (verbose) puts("STUB: CMTimebaseAddTimer called");
    return NULL;
}

void* CMTimebaseAddTimerDispatchSource(void) {
    if (verbose) puts("STUB: CMTimebaseAddTimerDispatchSource called");
    return NULL;
}

void* CMTimebaseCopyMaster(void) {
    if (verbose) puts("STUB: CMTimebaseCopyMaster called");
    return NULL;
}

void* CMTimebaseCopyMasterClock(void) {
    if (verbose) puts("STUB: CMTimebaseCopyMasterClock called");
    return NULL;
}

void* CMTimebaseCopyMasterTimebase(void) {
    if (verbose) puts("STUB: CMTimebaseCopyMasterTimebase called");
    return NULL;
}

void* CMTimebaseCopyUltimateMasterClock(void) {
    if (verbose) puts("STUB: CMTimebaseCopyUltimateMasterClock called");
    return NULL;
}

void* CMTimebaseCreateReadOnlyTimebase(void) {
    if (verbose) puts("STUB: CMTimebaseCreateReadOnlyTimebase called");
    return NULL;
}

void* CMTimebaseCreateWithMasterClock(void) {
    if (verbose) puts("STUB: CMTimebaseCreateWithMasterClock called");
    return NULL;
}

void* CMTimebaseCreateWithMasterTimebase(void) {
    if (verbose) puts("STUB: CMTimebaseCreateWithMasterTimebase called");
    return NULL;
}

void* CMTimebaseGetEffectiveRate(void) {
    if (verbose) puts("STUB: CMTimebaseGetEffectiveRate called");
    return NULL;
}

void* CMTimebaseGetMaster(void) {
    if (verbose) puts("STUB: CMTimebaseGetMaster called");
    return NULL;
}

void* CMTimebaseGetMasterClock(void) {
    if (verbose) puts("STUB: CMTimebaseGetMasterClock called");
    return NULL;
}

void* CMTimebaseGetMasterTimebase(void) {
    if (verbose) puts("STUB: CMTimebaseGetMasterTimebase called");
    return NULL;
}

void* CMTimebaseGetRate(void) {
    if (verbose) puts("STUB: CMTimebaseGetRate called");
    return NULL;
}

void* CMTimebaseGetTime(void) {
    if (verbose) puts("STUB: CMTimebaseGetTime called");
    return NULL;
}

void* CMTimebaseGetTimeAndRate(void) {
    if (verbose) puts("STUB: CMTimebaseGetTimeAndRate called");
    return NULL;
}

void* CMTimebaseGetTimeClampedAboveAnchorTime(void) {
    if (verbose) puts("STUB: CMTimebaseGetTimeClampedAboveAnchorTime called");
    return NULL;
}

void* CMTimebaseGetTimeWithTimeScale(void) {
    if (verbose) puts("STUB: CMTimebaseGetTimeWithTimeScale called");
    return NULL;
}

void* CMTimebaseGetTypeID(void) {
    if (verbose) puts("STUB: CMTimebaseGetTypeID called");
    return NULL;
}

void* CMTimebaseGetUltimateMasterClock(void) {
    if (verbose) puts("STUB: CMTimebaseGetUltimateMasterClock called");
    return NULL;
}

void* CMTimebaseNotificationBarrier(void) {
    if (verbose) puts("STUB: CMTimebaseNotificationBarrier called");
    return NULL;
}

void* CMTimebaseRemoveTimer(void) {
    if (verbose) puts("STUB: CMTimebaseRemoveTimer called");
    return NULL;
}

void* CMTimebaseRemoveTimerDispatchSource(void) {
    if (verbose) puts("STUB: CMTimebaseRemoveTimerDispatchSource called");
    return NULL;
}

void* CMTimebaseSetAnchorTime(void) {
    if (verbose) puts("STUB: CMTimebaseSetAnchorTime called");
    return NULL;
}

void* CMTimebaseSetMasterClock(void) {
    if (verbose) puts("STUB: CMTimebaseSetMasterClock called");
    return NULL;
}

void* CMTimebaseSetMasterTimebase(void) {
    if (verbose) puts("STUB: CMTimebaseSetMasterTimebase called");
    return NULL;
}

void* CMTimebaseSetRate(void) {
    if (verbose) puts("STUB: CMTimebaseSetRate called");
    return NULL;
}

void* CMTimebaseSetRateAndAnchorTime(void) {
    if (verbose) puts("STUB: CMTimebaseSetRateAndAnchorTime called");
    return NULL;
}

void* CMTimebaseSetRateAndAnchorTimeWithFlags(void) {
    if (verbose) puts("STUB: CMTimebaseSetRateAndAnchorTimeWithFlags called");
    return NULL;
}

void* CMTimebaseSetTime(void) {
    if (verbose) puts("STUB: CMTimebaseSetTime called");
    return NULL;
}

void* CMTimebaseSetTimerDispatchSourceNextFireTime(void) {
    if (verbose) puts("STUB: CMTimebaseSetTimerDispatchSourceNextFireTime called");
    return NULL;
}

void* CMTimebaseSetTimerDispatchSourceToFireImmediately(void) {
    if (verbose) puts("STUB: CMTimebaseSetTimerDispatchSourceToFireImmediately called");
    return NULL;
}

void* CMTimebaseSetTimerNextFireTime(void) {
    if (verbose) puts("STUB: CMTimebaseSetTimerNextFireTime called");
    return NULL;
}

void* CMTimebaseSetTimerToFireImmediately(void) {
    if (verbose) puts("STUB: CMTimebaseSetTimerToFireImmediately called");
    return NULL;
}

void* CMVideoFormatDescriptionCopyAsBigEndianImageDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: CMVideoFormatDescriptionCopyAsBigEndianImageDescriptionBlockBuffer called");
    return NULL;
}

void* CMVideoFormatDescriptionCreate(void) {
    if (verbose) puts("STUB: CMVideoFormatDescriptionCreate called");
    return NULL;
}

void* CMVideoFormatDescriptionCreateForImageBuffer(void) {
    if (verbose) puts("STUB: CMVideoFormatDescriptionCreateForImageBuffer called");
    return NULL;
}

void* CMVideoFormatDescriptionCreateFromBigEndianImageDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: CMVideoFormatDescriptionCreateFromBigEndianImageDescriptionBlockBuffer called");
    return NULL;
}

void* CMVideoFormatDescriptionCreateFromBigEndianImageDescriptionData(void) {
    if (verbose) puts("STUB: CMVideoFormatDescriptionCreateFromBigEndianImageDescriptionData called");
    return NULL;
}

void* CMVideoFormatDescriptionCreateFromH264ParameterSets(void) {
    if (verbose) puts("STUB: CMVideoFormatDescriptionCreateFromH264ParameterSets called");
    return NULL;
}

void* CMVideoFormatDescriptionCreateFromHEVCParameterSets(void) {
    if (verbose) puts("STUB: CMVideoFormatDescriptionCreateFromHEVCParameterSets called");
    return NULL;
}

void* CMVideoFormatDescriptionGetCleanAperture(void) {
    if (verbose) puts("STUB: CMVideoFormatDescriptionGetCleanAperture called");
    return NULL;
}

void* CMVideoFormatDescriptionGetDimensions(void) {
    if (verbose) puts("STUB: CMVideoFormatDescriptionGetDimensions called");
    return NULL;
}

void* CMVideoFormatDescriptionGetExtensionKeysCommonWithImageBuffers(void) {
    if (verbose) puts("STUB: CMVideoFormatDescriptionGetExtensionKeysCommonWithImageBuffers called");
    return NULL;
}

void* CMVideoFormatDescriptionGetH264ParameterSetAtIndex(void) {
    if (verbose) puts("STUB: CMVideoFormatDescriptionGetH264ParameterSetAtIndex called");
    return NULL;
}

void* CMVideoFormatDescriptionGetHEVCParameterSetAtIndex(void) {
    if (verbose) puts("STUB: CMVideoFormatDescriptionGetHEVCParameterSetAtIndex called");
    return NULL;
}

void* CMVideoFormatDescriptionGetPresentationDimensions(void) {
    if (verbose) puts("STUB: CMVideoFormatDescriptionGetPresentationDimensions called");
    return NULL;
}

void* CMVideoFormatDescriptionGetVideoDynamicRange(void) {
    if (verbose) puts("STUB: CMVideoFormatDescriptionGetVideoDynamicRange called");
    return NULL;
}

void* CMVideoFormatDescriptionMatchesImageBuffer(void) {
    if (verbose) puts("STUB: CMVideoFormatDescriptionMatchesImageBuffer called");
    return NULL;
}

void* ColorsWheelContext_RotateRad(void) {
    if (verbose) puts("STUB: ColorsWheelContext_RotateRad called");
    return NULL;
}

void* ColorsWheelContext_computeColorComponent(void) {
    if (verbose) puts("STUB: ColorsWheelContext_computeColorComponent called");
    return NULL;
}

void* ColorsWheelContext_computeColorComponentWheel(void) {
    if (verbose) puts("STUB: ColorsWheelContext_computeColorComponentWheel called");
    return NULL;
}

void* ColorsWheelContext_countColorsTransitions(void) {
    if (verbose) puts("STUB: ColorsWheelContext_countColorsTransitions called");
    return NULL;
}

void* ColorsWheelContext_create(void) {
    if (verbose) puts("STUB: ColorsWheelContext_create called");
    return NULL;
}

void* ColorsWheelContext_destroy(void) {
    if (verbose) puts("STUB: ColorsWheelContext_destroy called");
    return NULL;
}

void* ColorsWheelContext_drawColorsWheel(void) {
    if (verbose) puts("STUB: ColorsWheelContext_drawColorsWheel called");
    return NULL;
}

void* ColorsWheelContext_drawMatrix_f16(void) {
    if (verbose) puts("STUB: ColorsWheelContext_drawMatrix_f16 called");
    return NULL;
}

void* ColorsWheelContext_drawMatrix_f32(void) {
    if (verbose) puts("STUB: ColorsWheelContext_drawMatrix_f32 called");
    return NULL;
}

void* ColorsWheelContext_drawMatrix_u8(void) {
    if (verbose) puts("STUB: ColorsWheelContext_drawMatrix_u8 called");
    return NULL;
}

void* ColorsWheelContext_firstTransitionIndex(void) {
    if (verbose) puts("STUB: ColorsWheelContext_firstTransitionIndex called");
    return NULL;
}

void* ColorsWheelContext_isTransitionInverted(void) {
    if (verbose) puts("STUB: ColorsWheelContext_isTransitionInverted called");
    return NULL;
}

void* FigAtomStreamAdvanceToNextAtom(void) {
    if (verbose) puts("STUB: FigAtomStreamAdvanceToNextAtom called");
    return NULL;
}

void* FigAtomStreamAdvanceToNextAtomWithType(void) {
    if (verbose) puts("STUB: FigAtomStreamAdvanceToNextAtomWithType called");
    return NULL;
}

void* FigAtomStreamAdvanceViaAtomPath(void) {
    if (verbose) puts("STUB: FigAtomStreamAdvanceViaAtomPath called");
    return NULL;
}

void* FigAtomStreamGetCurrentAtomDataGlobalOffset(void) {
    if (verbose) puts("STUB: FigAtomStreamGetCurrentAtomDataGlobalOffset called");
    return NULL;
}

void* FigAtomStreamGetCurrentAtomGlobalOffset(void) {
    if (verbose) puts("STUB: FigAtomStreamGetCurrentAtomGlobalOffset called");
    return NULL;
}

void* FigAtomStreamGetCurrentAtomHeaderSize(void) {
    if (verbose) puts("STUB: FigAtomStreamGetCurrentAtomHeaderSize called");
    return NULL;
}

void* FigAtomStreamGetCurrentAtomOffsetInParent(void) {
    if (verbose) puts("STUB: FigAtomStreamGetCurrentAtomOffsetInParent called");
    return NULL;
}

void* FigAtomStreamGetCurrentAtomTypeAndDataLength(void) {
    if (verbose) puts("STUB: FigAtomStreamGetCurrentAtomTypeAndDataLength called");
    return NULL;
}

void* FigAtomStreamGetCurrentAtomUUIDType(void) {
    if (verbose) puts("STUB: FigAtomStreamGetCurrentAtomUUIDType called");
    return NULL;
}

void* FigAtomStreamGetCurrentAtomVersionAndFlags(void) {
    if (verbose) puts("STUB: FigAtomStreamGetCurrentAtomVersionAndFlags called");
    return NULL;
}

void* FigAtomStreamInitWithBBuf(void) {
    if (verbose) puts("STUB: FigAtomStreamInitWithBBuf called");
    return NULL;
}

void* FigAtomStreamInitWithByteStream(void) {
    if (verbose) puts("STUB: FigAtomStreamInitWithByteStream called");
    return NULL;
}

void* FigAtomStreamInitWithByteStreamWithSize(void) {
    if (verbose) puts("STUB: FigAtomStreamInitWithByteStreamWithSize called");
    return NULL;
}

void* FigAtomStreamInitWithMemoryBlock(void) {
    if (verbose) puts("STUB: FigAtomStreamInitWithMemoryBlock called");
    return NULL;
}

void* FigAtomStreamInitWithParent(void) {
    if (verbose) puts("STUB: FigAtomStreamInitWithParent called");
    return NULL;
}

void* FigAtomStreamPositionAtFirstAtomWithType(void) {
    if (verbose) puts("STUB: FigAtomStreamPositionAtFirstAtomWithType called");
    return NULL;
}

void* FigAtomStreamPositionViaAtomPath(void) {
    if (verbose) puts("STUB: FigAtomStreamPositionViaAtomPath called");
    return NULL;
}

void* FigAtomStreamReadCurrentAtomAndCreateBBuf(void) {
    if (verbose) puts("STUB: FigAtomStreamReadCurrentAtomAndCreateBBuf called");
    return NULL;
}

void* FigAtomStreamReadCurrentAtomData(void) {
    if (verbose) puts("STUB: FigAtomStreamReadCurrentAtomData called");
    return NULL;
}

void* FigAtomStreamReadCurrentAtomDataAndCreateBBuf(void) {
    if (verbose) puts("STUB: FigAtomStreamReadCurrentAtomDataAndCreateBBuf called");
    return NULL;
}

void* FigAtomUtilityGetAtomTypeAndLengthInBBuf(void) {
    if (verbose) puts("STUB: FigAtomUtilityGetAtomTypeAndLengthInBBuf called");
    return NULL;
}

void* FigAtomUtilityGetAtomTypeAndLengthInByteStream(void) {
    if (verbose) puts("STUB: FigAtomUtilityGetAtomTypeAndLengthInByteStream called");
    return NULL;
}

void* FigAtomUtilityGetAtomTypeAndLengthInByteStreamWithSize(void) {
    if (verbose) puts("STUB: FigAtomUtilityGetAtomTypeAndLengthInByteStreamWithSize called");
    return NULL;
}

void* FigAtomWriterAppendBlockBufferData(void) {
    if (verbose) puts("STUB: FigAtomWriterAppendBlockBufferData called");
    return NULL;
}

void* FigAtomWriterAppendData(void) {
    if (verbose) puts("STUB: FigAtomWriterAppendData called");
    return NULL;
}

void* FigAtomWriterAppendVersionAndFlags(void) {
    if (verbose) puts("STUB: FigAtomWriterAppendVersionAndFlags called");
    return NULL;
}

void* FigAtomWriterBeginAtom(void) {
    if (verbose) puts("STUB: FigAtomWriterBeginAtom called");
    return NULL;
}

void* FigAtomWriterBeginUUIDAtom(void) {
    if (verbose) puts("STUB: FigAtomWriterBeginUUIDAtom called");
    return NULL;
}

void* FigAtomWriterEndAtom(void) {
    if (verbose) puts("STUB: FigAtomWriterEndAtom called");
    return NULL;
}

void* FigAtomWriterGetAtomDataSize(void) {
    if (verbose) puts("STUB: FigAtomWriterGetAtomDataSize called");
    return NULL;
}

void* FigAtomWriterGetAtomGlobalDataOffset(void) {
    if (verbose) puts("STUB: FigAtomWriterGetAtomGlobalDataOffset called");
    return NULL;
}

void* FigAtomWriterGetAtomSize(void) {
    if (verbose) puts("STUB: FigAtomWriterGetAtomSize called");
    return NULL;
}

void* FigAtomWriterInitWithBlockBuffer(void) {
    if (verbose) puts("STUB: FigAtomWriterInitWithBlockBuffer called");
    return NULL;
}

void* FigAtomWriterInitWithByteStream(void) {
    if (verbose) puts("STUB: FigAtomWriterInitWithByteStream called");
    return NULL;
}

void* FigAtomWriterInitWithParent(void) {
    if (verbose) puts("STUB: FigAtomWriterInitWithParent called");
    return NULL;
}

void* FigAtomicAdd32(void) {
    if (verbose) puts("STUB: FigAtomicAdd32 called");
    return NULL;
}

void* FigAtomicBitAnd32(void) {
    if (verbose) puts("STUB: FigAtomicBitAnd32 called");
    return NULL;
}

void* FigAtomicBitAnd32Orig(void) {
    if (verbose) puts("STUB: FigAtomicBitAnd32Orig called");
    return NULL;
}

void* FigAtomicBitOr32(void) {
    if (verbose) puts("STUB: FigAtomicBitOr32 called");
    return NULL;
}

void* FigAtomicBitOr32Orig(void) {
    if (verbose) puts("STUB: FigAtomicBitOr32Orig called");
    return NULL;
}

void* FigAtomicBitXor32(void) {
    if (verbose) puts("STUB: FigAtomicBitXor32 called");
    return NULL;
}

void* FigAtomicBitXor32Orig(void) {
    if (verbose) puts("STUB: FigAtomicBitXor32Orig called");
    return NULL;
}

void* FigAtomicCompareAndSwap32(void) {
    if (verbose) puts("STUB: FigAtomicCompareAndSwap32 called");
    return NULL;
}

void* FigAtomicCompareAndSwapPtr(void) {
    if (verbose) puts("STUB: FigAtomicCompareAndSwapPtr called");
    return NULL;
}

void* FigAtomicDecrement32(void) {
    if (verbose) puts("STUB: FigAtomicDecrement32 called");
    return NULL;
}

void* FigAtomicIncrement32(void) {
    if (verbose) puts("STUB: FigAtomicIncrement32 called");
    return NULL;
}

void* FigAudioDeviceClockCreate(void) {
    if (verbose) puts("STUB: FigAudioDeviceClockCreate called");
    return NULL;
}

void* FigAudioDeviceClockCreateFromAudioDeviceID(void) {
    if (verbose) puts("STUB: FigAudioDeviceClockCreateFromAudioDeviceID called");
    return NULL;
}

void* FigAudioDeviceClockGetAudioDevice(void) {
    if (verbose) puts("STUB: FigAudioDeviceClockGetAudioDevice called");
    return NULL;
}

void* FigAudioDeviceClockSetAudioDeviceID(void) {
    if (verbose) puts("STUB: FigAudioDeviceClockSetAudioDeviceID called");
    return NULL;
}

void* FigAudioDeviceClockSetAudioDeviceUID(void) {
    if (verbose) puts("STUB: FigAudioDeviceClockSetAudioDeviceUID called");
    return NULL;
}

void* FigAudioFormatDescriptionCopyAsBigEndianSoundDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: FigAudioFormatDescriptionCopyAsBigEndianSoundDescriptionBlockBuffer called");
    return NULL;
}

void* FigAudioFormatDescriptionCreateFromBigEndianSoundDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: FigAudioFormatDescriptionCreateFromBigEndianSoundDescriptionBlockBuffer called");
    return NULL;
}

void* FigAudioFormatDescriptionCreateFromBigEndianSoundDescriptionData(void) {
    if (verbose) puts("STUB: FigAudioFormatDescriptionCreateFromBigEndianSoundDescriptionData called");
    return NULL;
}

void* FigAudioFormatDescriptionGetMostCompatibleFormatAndChannelLayout(void) {
    if (verbose) puts("STUB: FigAudioFormatDescriptionGetMostCompatibleFormatAndChannelLayout called");
    return NULL;
}

void* FigAudioFormatDescriptionGetRichestDecodableFormatAndChannelLayout(void) {
    if (verbose) puts("STUB: FigAudioFormatDescriptionGetRichestDecodableFormatAndChannelLayout called");
    return NULL;
}

void* FigBBMessageCommit(void) {
    if (verbose) puts("STUB: FigBBMessageCommit called");
    return NULL;
}

void* FigBBMessageCopyBlockBuffer(void) {
    if (verbose) puts("STUB: FigBBMessageCopyBlockBuffer called");
    return NULL;
}

void* FigBBMessageCopyCFArray(void) {
    if (verbose) puts("STUB: FigBBMessageCopyCFArray called");
    return NULL;
}

void* FigBBMessageCopyCFData(void) {
    if (verbose) puts("STUB: FigBBMessageCopyCFData called");
    return NULL;
}

void* FigBBMessageCopyCFDictionary(void) {
    if (verbose) puts("STUB: FigBBMessageCopyCFDictionary called");
    return NULL;
}

void* FigBBMessageCopyCFString(void) {
    if (verbose) puts("STUB: FigBBMessageCopyCFString called");
    return NULL;
}

void* FigBBMessageCreate(void) {
    if (verbose) puts("STUB: FigBBMessageCreate called");
    return NULL;
}

void* FigBBMessageGetCString(void) {
    if (verbose) puts("STUB: FigBBMessageGetCString called");
    return NULL;
}

void* FigBBMessageGetDataPtr(void) {
    if (verbose) puts("STUB: FigBBMessageGetDataPtr called");
    return NULL;
}

void* FigBBMessageGetFloat32(void) {
    if (verbose) puts("STUB: FigBBMessageGetFloat32 called");
    return NULL;
}

void* FigBBMessageGetInt64(void) {
    if (verbose) puts("STUB: FigBBMessageGetInt64 called");
    return NULL;
}

void* FigBBMessageSetBlockBuffer(void) {
    if (verbose) puts("STUB: FigBBMessageSetBlockBuffer called");
    return NULL;
}

void* FigBBMessageSetCFArray(void) {
    if (verbose) puts("STUB: FigBBMessageSetCFArray called");
    return NULL;
}

void* FigBBMessageSetCFData(void) {
    if (verbose) puts("STUB: FigBBMessageSetCFData called");
    return NULL;
}

void* FigBBMessageSetCFDictionary(void) {
    if (verbose) puts("STUB: FigBBMessageSetCFDictionary called");
    return NULL;
}

void* FigBBMessageSetCFString(void) {
    if (verbose) puts("STUB: FigBBMessageSetCFString called");
    return NULL;
}

void* FigBBMessageSetCString(void) {
    if (verbose) puts("STUB: FigBBMessageSetCString called");
    return NULL;
}

void* FigBBMessageSetDataPtr(void) {
    if (verbose) puts("STUB: FigBBMessageSetDataPtr called");
    return NULL;
}

void* FigBBMessageSetFloat32(void) {
    if (verbose) puts("STUB: FigBBMessageSetFloat32 called");
    return NULL;
}

void* FigBBMessageSetInt64(void) {
    if (verbose) puts("STUB: FigBBMessageSetInt64 called");
    return NULL;
}

void* FigBaseClassGetCFTypeID(void) {
    if (verbose) puts("STUB: FigBaseClassGetCFTypeID called");
    return NULL;
}

void* FigBaseClassRegisterClass(void) {
    if (verbose) puts("STUB: FigBaseClassRegisterClass called");
    return NULL;
}

void* FigBaseGetClassID(void) {
    if (verbose) puts("STUB: FigBaseGetClassID called");
    return NULL;
}

void* FigBaseObjectCopyDebugDescription(void) {
    if (verbose) puts("STUB: FigBaseObjectCopyDebugDescription called");
    return NULL;
}

void* FigBaseObjectEqual(void) {
    if (verbose) puts("STUB: FigBaseObjectEqual called");
    return NULL;
}

void* FigBaseObjectFinalize(void) {
    if (verbose) puts("STUB: FigBaseObjectFinalize called");
    return NULL;
}

void* FigBaseObjectGetClassID(void) {
    if (verbose) puts("STUB: FigBaseObjectGetClassID called");
    return NULL;
}

void* FigBaseObjectGetDerivedStorage(void) {
    if (verbose) puts("STUB: FigBaseObjectGetDerivedStorage called");
    return NULL;
}

void* FigBaseObjectGetVTable(void) {
    if (verbose) puts("STUB: FigBaseObjectGetVTable called");
    return NULL;
}

void* FigBaseObjectInit(void) {
    if (verbose) puts("STUB: FigBaseObjectInit called");
    return NULL;
}

void* FigBaseObjectIsMemberOfClass(void) {
    if (verbose) puts("STUB: FigBaseObjectIsMemberOfClass called");
    return NULL;
}

void* FigBaseObjectStandardNotificationBarrier(void) {
    if (verbose) puts("STUB: FigBaseObjectStandardNotificationBarrier called");
    return NULL;
}

void* FigBaseProtocolRegisterProtocol(void) {
    if (verbose) puts("STUB: FigBaseProtocolRegisterProtocol called");
    return NULL;
}

void* FigBlockBufferCompare(void) {
    if (verbose) puts("STUB: FigBlockBufferCompare called");
    return NULL;
}

void* FigBlockBufferGetDataPointerAndOrigin(void) {
    if (verbose) puts("STUB: FigBlockBufferGetDataPointerAndOrigin called");
    return NULL;
}

void* FigBlockBufferHexDump(void) {
    if (verbose) puts("STUB: FigBlockBufferHexDump called");
    return NULL;
}

void* FigBlockBufferHexDumpWithEmitter(void) {
    if (verbose) puts("STUB: FigBlockBufferHexDumpWithEmitter called");
    return NULL;
}

void* FigBlockBufferRelease(void) {
    if (verbose) puts("STUB: FigBlockBufferRelease called");
    return NULL;
}

void* FigBlockBufferRetain(void) {
    if (verbose) puts("STUB: FigBlockBufferRetain called");
    return NULL;
}

void* FigBoxedMetadataAppendCFTypedValue(void) {
    if (verbose) puts("STUB: FigBoxedMetadataAppendCFTypedValue called");
    return NULL;
}

void* FigBoxedMetadataAppendCGAffineTransform(void) {
    if (verbose) puts("STUB: FigBoxedMetadataAppendCGAffineTransform called");
    return NULL;
}

void* FigBoxedMetadataAppendCGPoint(void) {
    if (verbose) puts("STUB: FigBoxedMetadataAppendCGPoint called");
    return NULL;
}

void* FigBoxedMetadataAppendCGRect(void) {
    if (verbose) puts("STUB: FigBoxedMetadataAppendCGRect called");
    return NULL;
}

void* FigBoxedMetadataAppendCGSize(void) {
    if (verbose) puts("STUB: FigBoxedMetadataAppendCGSize called");
    return NULL;
}

void* FigBoxedMetadataAppendFloat32(void) {
    if (verbose) puts("STUB: FigBoxedMetadataAppendFloat32 called");
    return NULL;
}

void* FigBoxedMetadataAppendFloat64(void) {
    if (verbose) puts("STUB: FigBoxedMetadataAppendFloat64 called");
    return NULL;
}

void* FigBoxedMetadataAppendSInt16(void) {
    if (verbose) puts("STUB: FigBoxedMetadataAppendSInt16 called");
    return NULL;
}

void* FigBoxedMetadataAppendSInt32(void) {
    if (verbose) puts("STUB: FigBoxedMetadataAppendSInt32 called");
    return NULL;
}

void* FigBoxedMetadataAppendSInt64(void) {
    if (verbose) puts("STUB: FigBoxedMetadataAppendSInt64 called");
    return NULL;
}

void* FigBoxedMetadataAppendSInt8(void) {
    if (verbose) puts("STUB: FigBoxedMetadataAppendSInt8 called");
    return NULL;
}

void* FigBoxedMetadataAppendUInt16(void) {
    if (verbose) puts("STUB: FigBoxedMetadataAppendUInt16 called");
    return NULL;
}

void* FigBoxedMetadataAppendUInt32(void) {
    if (verbose) puts("STUB: FigBoxedMetadataAppendUInt32 called");
    return NULL;
}

void* FigBoxedMetadataAppendUInt64(void) {
    if (verbose) puts("STUB: FigBoxedMetadataAppendUInt64 called");
    return NULL;
}

void* FigBoxedMetadataAppendUInt8(void) {
    if (verbose) puts("STUB: FigBoxedMetadataAppendUInt8 called");
    return NULL;
}

void* FigBoxedMetadataAppendValue(void) {
    if (verbose) puts("STUB: FigBoxedMetadataAppendValue called");
    return NULL;
}

void* FigBoxedMetadataCopyValueOfItemAtIndexAsCFType(void) {
    if (verbose) puts("STUB: FigBoxedMetadataCopyValueOfItemAtIndexAsCFType called");
    return NULL;
}

void* FigBoxedMetadataCreateBlockBufferForIndex(void) {
    if (verbose) puts("STUB: FigBoxedMetadataCreateBlockBufferForIndex called");
    return NULL;
}

void* FigBoxedMetadataCreateForConstruction(void) {
    if (verbose) puts("STUB: FigBoxedMetadataCreateForConstruction called");
    return NULL;
}

void* FigBoxedMetadataCreateFromBlockBuffer(void) {
    if (verbose) puts("STUB: FigBoxedMetadataCreateFromBlockBuffer called");
    return NULL;
}

void* FigBoxedMetadataEndConstruction(void) {
    if (verbose) puts("STUB: FigBoxedMetadataEndConstruction called");
    return NULL;
}

void* FigBoxedMetadataGetBlockBuffer(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetBlockBuffer called");
    return NULL;
}

void* FigBoxedMetadataGetFormatDescription(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetFormatDescription called");
    return NULL;
}

void* FigBoxedMetadataGetLocalIDOfItemAtIndex(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetLocalIDOfItemAtIndex called");
    return NULL;
}

void* FigBoxedMetadataGetNumberOfItems(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetNumberOfItems called");
    return NULL;
}

void* FigBoxedMetadataGetSizeOfItemAtIndex(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetSizeOfItemAtIndex called");
    return NULL;
}

void* FigBoxedMetadataGetTypeID(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetTypeID called");
    return NULL;
}

void* FigBoxedMetadataGetValueOfItemAtIndex(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetValueOfItemAtIndex called");
    return NULL;
}

void* FigBoxedMetadataGetValueOfItemAtIndexAsCGAffineTransform(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetValueOfItemAtIndexAsCGAffineTransform called");
    return NULL;
}

void* FigBoxedMetadataGetValueOfItemAtIndexAsCGPoint(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetValueOfItemAtIndexAsCGPoint called");
    return NULL;
}

void* FigBoxedMetadataGetValueOfItemAtIndexAsCGRect(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetValueOfItemAtIndexAsCGRect called");
    return NULL;
}

void* FigBoxedMetadataGetValueOfItemAtIndexAsCGSize(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetValueOfItemAtIndexAsCGSize called");
    return NULL;
}

void* FigBoxedMetadataGetValueOfItemAtIndexAsFloat32(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetValueOfItemAtIndexAsFloat32 called");
    return NULL;
}

void* FigBoxedMetadataGetValueOfItemAtIndexAsFloat64(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetValueOfItemAtIndexAsFloat64 called");
    return NULL;
}

void* FigBoxedMetadataGetValueOfItemAtIndexAsSInt16(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetValueOfItemAtIndexAsSInt16 called");
    return NULL;
}

void* FigBoxedMetadataGetValueOfItemAtIndexAsSInt32(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetValueOfItemAtIndexAsSInt32 called");
    return NULL;
}

void* FigBoxedMetadataGetValueOfItemAtIndexAsSInt64(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetValueOfItemAtIndexAsSInt64 called");
    return NULL;
}

void* FigBoxedMetadataGetValueOfItemAtIndexAsSInt8(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetValueOfItemAtIndexAsSInt8 called");
    return NULL;
}

void* FigBoxedMetadataGetValueOfItemAtIndexAsUInt16(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetValueOfItemAtIndexAsUInt16 called");
    return NULL;
}

void* FigBoxedMetadataGetValueOfItemAtIndexAsUInt32(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetValueOfItemAtIndexAsUInt32 called");
    return NULL;
}

void* FigBoxedMetadataGetValueOfItemAtIndexAsUInt64(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetValueOfItemAtIndexAsUInt64 called");
    return NULL;
}

void* FigBoxedMetadataGetValueOfItemAtIndexAsUInt8(void) {
    if (verbose) puts("STUB: FigBoxedMetadataGetValueOfItemAtIndexAsUInt8 called");
    return NULL;
}

void* FigBoxedMetadataIsUnderConstruction(void) {
    if (verbose) puts("STUB: FigBoxedMetadataIsUnderConstruction called");
    return NULL;
}

void* FigBoxedMetadataRelease(void) {
    if (verbose) puts("STUB: FigBoxedMetadataRelease called");
    return NULL;
}

void* FigBoxedMetadataRetain(void) {
    if (verbose) puts("STUB: FigBoxedMetadataRetain called");
    return NULL;
}

void* FigBufferQueueDequeueIfMatchingAndRetain(void) {
    if (verbose) puts("STUB: FigBufferQueueDequeueIfMatchingAndRetain called");
    return NULL;
}

void* FigBufferQueueInstallTriggerWithTimeAndIntegerThresholds(void) {
    if (verbose) puts("STUB: FigBufferQueueInstallTriggerWithTimeAndIntegerThresholds called");
    return NULL;
}

void* FigBufferQueueRelease(void) {
    if (verbose) puts("STUB: FigBufferQueueRelease called");
    return NULL;
}

void* FigBufferQueueRetain(void) {
    if (verbose) puts("STUB: FigBufferQueueRetain called");
    return NULL;
}

void* FigByteStreamCopyMD5Digest(void) {
    if (verbose) puts("STUB: FigByteStreamCopyMD5Digest called");
    return NULL;
}

void* FigByteStreamCopySHA1Digest(void) {
    if (verbose) puts("STUB: FigByteStreamCopySHA1Digest called");
    return NULL;
}

void* FigByteStreamCreateForBlockBuffer(void) {
    if (verbose) puts("STUB: FigByteStreamCreateForBlockBuffer called");
    return NULL;
}

void* FigByteStreamCreateForFile(void) {
    if (verbose) puts("STUB: FigByteStreamCreateForFile called");
    return NULL;
}

void* FigByteStreamCreateForFileURL(void) {
    if (verbose) puts("STUB: FigByteStreamCreateForFileURL called");
    return NULL;
}

void* FigByteStreamCreateFromOpenFile(void) {
    if (verbose) puts("STUB: FigByteStreamCreateFromOpenFile called");
    return NULL;
}

void* FigByteStreamCreateWithCFData(void) {
    if (verbose) puts("STUB: FigByteStreamCreateWithCFData called");
    return NULL;
}

void* FigByteStreamCreateWithIOSurface(void) {
    if (verbose) puts("STUB: FigByteStreamCreateWithIOSurface called");
    return NULL;
}

void* FigByteStreamCreateWithWriteCallback(void) {
    if (verbose) puts("STUB: FigByteStreamCreateWithWriteCallback called");
    return NULL;
}

void* FigByteStreamCreateWritableWithCFMutableData(void) {
    if (verbose) puts("STUB: FigByteStreamCreateWritableWithCFMutableData called");
    return NULL;
}

void* FigByteStreamCreateWritableWithIOSurface(void) {
    if (verbose) puts("STUB: FigByteStreamCreateWritableWithIOSurface called");
    return NULL;
}

void* FigByteStreamProviderCreateForBaseURLWithSecurityOptions(void) {
    if (verbose) puts("STUB: FigByteStreamProviderCreateForBaseURLWithSecurityOptions called");
    return NULL;
}

void* FigByteStreamProviderGetClassID(void) {
    if (verbose) puts("STUB: FigByteStreamProviderGetClassID called");
    return NULL;
}

void* FigByteStreamProviderGetFigBaseObject(void) {
    if (verbose) puts("STUB: FigByteStreamProviderGetFigBaseObject called");
    return NULL;
}

void* FigByteStreamProviderGetTypeID(void) {
    if (verbose) puts("STUB: FigByteStreamProviderGetTypeID called");
    return NULL;
}

void* FigByteStreamStatsCreateStatsDictionary(void) {
    if (verbose) puts("STUB: FigByteStreamStatsCreateStatsDictionary called");
    return NULL;
}

void* FigByteStreamStatsLogOneRead(void) {
    if (verbose) puts("STUB: FigByteStreamStatsLogOneRead called");
    return NULL;
}

void* FigByteStreamStatsSetupWorker(void) {
    if (verbose) puts("STUB: FigByteStreamStatsSetupWorker called");
    return NULL;
}

void* FigByteStreamStatsTeardownWorker(void) {
    if (verbose) puts("STUB: FigByteStreamStatsTeardownWorker called");
    return NULL;
}

void* FigCFArrayAppendCFIndex(void) {
    if (verbose) puts("STUB: FigCFArrayAppendCFIndex called");
    return NULL;
}

void* FigCFArrayAppendCGFloat(void) {
    if (verbose) puts("STUB: FigCFArrayAppendCGFloat called");
    return NULL;
}

void* FigCFArrayAppendCGRect(void) {
    if (verbose) puts("STUB: FigCFArrayAppendCGRect called");
    return NULL;
}

void* FigCFArrayAppendCGSize(void) {
    if (verbose) puts("STUB: FigCFArrayAppendCGSize called");
    return NULL;
}

void* FigCFArrayAppendCMTime(void) {
    if (verbose) puts("STUB: FigCFArrayAppendCMTime called");
    return NULL;
}

void* FigCFArrayAppendDouble(void) {
    if (verbose) puts("STUB: FigCFArrayAppendDouble called");
    return NULL;
}

void* FigCFArrayAppendFloat(void) {
    if (verbose) puts("STUB: FigCFArrayAppendFloat called");
    return NULL;
}

void* FigCFArrayAppendFloat32(void) {
    if (verbose) puts("STUB: FigCFArrayAppendFloat32 called");
    return NULL;
}

void* FigCFArrayAppendInt(void) {
    if (verbose) puts("STUB: FigCFArrayAppendInt called");
    return NULL;
}

void* FigCFArrayAppendInt16(void) {
    if (verbose) puts("STUB: FigCFArrayAppendInt16 called");
    return NULL;
}

void* FigCFArrayAppendInt32(void) {
    if (verbose) puts("STUB: FigCFArrayAppendInt32 called");
    return NULL;
}

void* FigCFArrayAppendInt64(void) {
    if (verbose) puts("STUB: FigCFArrayAppendInt64 called");
    return NULL;
}

void* FigCFArrayApplyFunction(void) {
    if (verbose) puts("STUB: FigCFArrayApplyFunction called");
    return NULL;
}

void* FigCFArrayContainsValue(void) {
    if (verbose) puts("STUB: FigCFArrayContainsValue called");
    return NULL;
}

void* FigCFArrayCreateConcatenationOfTwoArrays(void) {
    if (verbose) puts("STUB: FigCFArrayCreateConcatenationOfTwoArrays called");
    return NULL;
}

void* FigCFArrayCreateMutableCopy(void) {
    if (verbose) puts("STUB: FigCFArrayCreateMutableCopy called");
    return NULL;
}

void* FigCFArrayForEach2(void) {
    if (verbose) puts("STUB: FigCFArrayForEach2 called");
    return NULL;
}

void* FigCFArrayGetCFIndexAtIndex(void) {
    if (verbose) puts("STUB: FigCFArrayGetCFIndexAtIndex called");
    return NULL;
}

void* FigCFArrayGetCGFloatAtIndex(void) {
    if (verbose) puts("STUB: FigCFArrayGetCGFloatAtIndex called");
    return NULL;
}

void* FigCFArrayGetCGRectAtIndex(void) {
    if (verbose) puts("STUB: FigCFArrayGetCGRectAtIndex called");
    return NULL;
}

void* FigCFArrayGetCGSizeAtIndex(void) {
    if (verbose) puts("STUB: FigCFArrayGetCGSizeAtIndex called");
    return NULL;
}

void* FigCFArrayGetCMTimeAtIndex(void) {
    if (verbose) puts("STUB: FigCFArrayGetCMTimeAtIndex called");
    return NULL;
}

void* FigCFArrayGetCount(void) {
    if (verbose) puts("STUB: FigCFArrayGetCount called");
    return NULL;
}

void* FigCFArrayGetDoubleAtIndex(void) {
    if (verbose) puts("STUB: FigCFArrayGetDoubleAtIndex called");
    return NULL;
}

void* FigCFArrayGetFloat32AtIndex(void) {
    if (verbose) puts("STUB: FigCFArrayGetFloat32AtIndex called");
    return NULL;
}

void* FigCFArrayGetFloatAtIndex(void) {
    if (verbose) puts("STUB: FigCFArrayGetFloatAtIndex called");
    return NULL;
}

void* FigCFArrayGetInt16AtIndex(void) {
    if (verbose) puts("STUB: FigCFArrayGetInt16AtIndex called");
    return NULL;
}

void* FigCFArrayGetInt32AtIndex(void) {
    if (verbose) puts("STUB: FigCFArrayGetInt32AtIndex called");
    return NULL;
}

void* FigCFArrayGetInt64AtIndex(void) {
    if (verbose) puts("STUB: FigCFArrayGetInt64AtIndex called");
    return NULL;
}

void* FigCFArrayGetIntAtIndex(void) {
    if (verbose) puts("STUB: FigCFArrayGetIntAtIndex called");
    return NULL;
}

void* FigCFAttributedStringSetDouble(void) {
    if (verbose) puts("STUB: FigCFAttributedStringSetDouble called");
    return NULL;
}

void* FigCFBundleIDResourceSubdirExists(void) {
    if (verbose) puts("STUB: FigCFBundleIDResourceSubdirExists called");
    return NULL;
}

void* FigCFCopyCompactDescription(void) {
    if (verbose) puts("STUB: FigCFCopyCompactDescription called");
    return NULL;
}

void* FigCFCreateCombinedDictionary(void) {
    if (verbose) puts("STUB: FigCFCreateCombinedDictionary called");
    return NULL;
}

void* FigCFCreateExtendedDictionary(void) {
    if (verbose) puts("STUB: FigCFCreateExtendedDictionary called");
    return NULL;
}

void* FigCFCreatePropertyListFromBundleIdentifier(void) {
    if (verbose) puts("STUB: FigCFCreatePropertyListFromBundleIdentifier called");
    return NULL;
}

void* FigCFCreatePropertyListFromBundleIdentifierOnPlatform(void) {
    if (verbose) puts("STUB: FigCFCreatePropertyListFromBundleIdentifierOnPlatform called");
    return NULL;
}

void* FigCFDateCreateFromString(void) {
    if (verbose) puts("STUB: FigCFDateCreateFromString called");
    return NULL;
}

void* FigCFDictionaryAddEntriesToDictionaryWithRecursion(void) {
    if (verbose) puts("STUB: FigCFDictionaryAddEntriesToDictionaryWithRecursion called");
    return NULL;
}

void* FigCFDictionaryCopyArrayOfKeys(void) {
    if (verbose) puts("STUB: FigCFDictionaryCopyArrayOfKeys called");
    return NULL;
}

void* FigCFDictionaryCopyArrayOfValues(void) {
    if (verbose) puts("STUB: FigCFDictionaryCopyArrayOfValues called");
    return NULL;
}

void* FigCFDictionaryCreateMappedArray(void) {
    if (verbose) puts("STUB: FigCFDictionaryCreateMappedArray called");
    return NULL;
}

void* FigCFDictionaryCreateMutableCopy(void) {
    if (verbose) puts("STUB: FigCFDictionaryCreateMutableCopy called");
    return NULL;
}

void* FigCFDictionaryGetBooleanIfPresent(void) {
    if (verbose) puts("STUB: FigCFDictionaryGetBooleanIfPresent called");
    return NULL;
}

void* FigCFDictionaryGetCFIndexIfPresent(void) {
    if (verbose) puts("STUB: FigCFDictionaryGetCFIndexIfPresent called");
    return NULL;
}

void* FigCFDictionaryGetCGFloatIfPresent(void) {
    if (verbose) puts("STUB: FigCFDictionaryGetCGFloatIfPresent called");
    return NULL;
}

void* FigCFDictionaryGetCGRectIfPresent(void) {
    if (verbose) puts("STUB: FigCFDictionaryGetCGRectIfPresent called");
    return NULL;
}

void* FigCFDictionaryGetCGSizeIfPresent(void) {
    if (verbose) puts("STUB: FigCFDictionaryGetCGSizeIfPresent called");
    return NULL;
}

void* FigCFDictionaryGetCMTimeIfPresent(void) {
    if (verbose) puts("STUB: FigCFDictionaryGetCMTimeIfPresent called");
    return NULL;
}

void* FigCFDictionaryGetCount(void) {
    if (verbose) puts("STUB: FigCFDictionaryGetCount called");
    return NULL;
}

void* FigCFDictionaryGetDoubleIfPresent(void) {
    if (verbose) puts("STUB: FigCFDictionaryGetDoubleIfPresent called");
    return NULL;
}

void* FigCFDictionaryGetFloat32IfPresent(void) {
    if (verbose) puts("STUB: FigCFDictionaryGetFloat32IfPresent called");
    return NULL;
}

void* FigCFDictionaryGetFloatIfPresent(void) {
    if (verbose) puts("STUB: FigCFDictionaryGetFloatIfPresent called");
    return NULL;
}

void* FigCFDictionaryGetInt16IfPresent(void) {
    if (verbose) puts("STUB: FigCFDictionaryGetInt16IfPresent called");
    return NULL;
}

void* FigCFDictionaryGetInt32IfPresent(void) {
    if (verbose) puts("STUB: FigCFDictionaryGetInt32IfPresent called");
    return NULL;
}

void* FigCFDictionaryGetInt64IfPresent(void) {
    if (verbose) puts("STUB: FigCFDictionaryGetInt64IfPresent called");
    return NULL;
}

void* FigCFDictionaryGetIntIfPresent(void) {
    if (verbose) puts("STUB: FigCFDictionaryGetIntIfPresent called");
    return NULL;
}

void* FigCFDictionaryGetValueForCaseInsensitiveKeyIfPresent(void) {
    if (verbose) puts("STUB: FigCFDictionaryGetValueForCaseInsensitiveKeyIfPresent called");
    return NULL;
}

void* FigCFDictionarySetBase64Data(void) {
    if (verbose) puts("STUB: FigCFDictionarySetBase64Data called");
    return NULL;
}

void* FigCFDictionarySetCFIndex(void) {
    if (verbose) puts("STUB: FigCFDictionarySetCFIndex called");
    return NULL;
}

void* FigCFDictionarySetCGFloat(void) {
    if (verbose) puts("STUB: FigCFDictionarySetCGFloat called");
    return NULL;
}

void* FigCFDictionarySetCGRect(void) {
    if (verbose) puts("STUB: FigCFDictionarySetCGRect called");
    return NULL;
}

void* FigCFDictionarySetCGSize(void) {
    if (verbose) puts("STUB: FigCFDictionarySetCGSize called");
    return NULL;
}

void* FigCFDictionarySetCMTime(void) {
    if (verbose) puts("STUB: FigCFDictionarySetCMTime called");
    return NULL;
}

void* FigCFDictionarySetDouble(void) {
    if (verbose) puts("STUB: FigCFDictionarySetDouble called");
    return NULL;
}

void* FigCFDictionarySetFloat(void) {
    if (verbose) puts("STUB: FigCFDictionarySetFloat called");
    return NULL;
}

void* FigCFDictionarySetFloat32(void) {
    if (verbose) puts("STUB: FigCFDictionarySetFloat32 called");
    return NULL;
}

void* FigCFDictionarySetInt(void) {
    if (verbose) puts("STUB: FigCFDictionarySetInt called");
    return NULL;
}

void* FigCFDictionarySetInt16(void) {
    if (verbose) puts("STUB: FigCFDictionarySetInt16 called");
    return NULL;
}

void* FigCFDictionarySetInt32(void) {
    if (verbose) puts("STUB: FigCFDictionarySetInt32 called");
    return NULL;
}

void* FigCFDictionarySetInt64(void) {
    if (verbose) puts("STUB: FigCFDictionarySetInt64 called");
    return NULL;
}

void* FigCFDictionarySetValue(void) {
    if (verbose) puts("STUB: FigCFDictionarySetValue called");
    return NULL;
}

void* FigCFDictionarySetValueFromKeyInDict(void) {
    if (verbose) puts("STUB: FigCFDictionarySetValueFromKeyInDict called");
    return NULL;
}

void* FigCFEqual(void) {
    if (verbose) puts("STUB: FigCFEqual called");
    return NULL;
}

void* FigCFGetLocalTimeString(void) {
    if (verbose) puts("STUB: FigCFGetLocalTimeString called");
    return NULL;
}

void* FigCFIOKitObjectCreate(void) {
    if (verbose) puts("STUB: FigCFIOKitObjectCreate called");
    return NULL;
}

void* FigCFIOKitObjectGetIOKitObject(void) {
    if (verbose) puts("STUB: FigCFIOKitObjectGetIOKitObject called");
    return NULL;
}

void* FigCFIOKitObjectGetTypeID(void) {
    if (verbose) puts("STUB: FigCFIOKitObjectGetTypeID called");
    return NULL;
}

void* FigCFLocaleCreateWithMovieLangCode(void) {
    if (verbose) puts("STUB: FigCFLocaleCreateWithMovieLangCode called");
    return NULL;
}

void* FigCFNumberCreateFloat32(void) {
    if (verbose) puts("STUB: FigCFNumberCreateFloat32 called");
    return NULL;
}

void* FigCFNumberCreateFloat64(void) {
    if (verbose) puts("STUB: FigCFNumberCreateFloat64 called");
    return NULL;
}

void* FigCFNumberCreateSInt16(void) {
    if (verbose) puts("STUB: FigCFNumberCreateSInt16 called");
    return NULL;
}

void* FigCFNumberCreateSInt32(void) {
    if (verbose) puts("STUB: FigCFNumberCreateSInt32 called");
    return NULL;
}

void* FigCFNumberCreateSInt64(void) {
    if (verbose) puts("STUB: FigCFNumberCreateSInt64 called");
    return NULL;
}

void* FigCFNumberCreateSInt8(void) {
    if (verbose) puts("STUB: FigCFNumberCreateSInt8 called");
    return NULL;
}

void* FigCFNumberCreateUInt16(void) {
    if (verbose) puts("STUB: FigCFNumberCreateUInt16 called");
    return NULL;
}

void* FigCFNumberCreateUInt32(void) {
    if (verbose) puts("STUB: FigCFNumberCreateUInt32 called");
    return NULL;
}

void* FigCFNumberCreateUInt64(void) {
    if (verbose) puts("STUB: FigCFNumberCreateUInt64 called");
    return NULL;
}

void* FigCFNumberCreateUInt8(void) {
    if (verbose) puts("STUB: FigCFNumberCreateUInt8 called");
    return NULL;
}

void* FigCFParseISO8601Date(void) {
    if (verbose) puts("STUB: FigCFParseISO8601Date called");
    return NULL;
}

void* FigCFPropertyListCreateFromFigFileFork(void) {
    if (verbose) puts("STUB: FigCFPropertyListCreateFromFigFileFork called");
    return NULL;
}

void* FigCFPropertyListWriteToFigFileFork(void) {
    if (verbose) puts("STUB: FigCFPropertyListWriteToFigFileFork called");
    return NULL;
}

void* FigCFRangeEqual(void) {
    if (verbose) puts("STUB: FigCFRangeEqual called");
    return NULL;
}

void* FigCFRangeGetLimit(void) {
    if (verbose) puts("STUB: FigCFRangeGetLimit called");
    return NULL;
}

void* FigCFStringCreateWithBytesAndMovieLangCode(void) {
    if (verbose) puts("STUB: FigCFStringCreateWithBytesAndMovieLangCode called");
    return NULL;
}

void* FigCFStringCreateWithBytesWithUnknownEncoding(void) {
    if (verbose) puts("STUB: FigCFStringCreateWithBytesWithUnknownEncoding called");
    return NULL;
}

void* FigCFStringCreateWithJSONArray(void) {
    if (verbose) puts("STUB: FigCFStringCreateWithJSONArray called");
    return NULL;
}

void* FigCFStringCreateWithJSONDictionary(void) {
    if (verbose) puts("STUB: FigCFStringCreateWithJSONDictionary called");
    return NULL;
}

void* FigCFStringFind(void) {
    if (verbose) puts("STUB: FigCFStringFind called");
    return NULL;
}

void* FigCFStringGetCStringPtrAndBufferToFree(void) {
    if (verbose) puts("STUB: FigCFStringGetCStringPtrAndBufferToFree called");
    return NULL;
}

void* FigCFStringGetCStringPtrMaybeUsingPreallocatedBuffer(void) {
    if (verbose) puts("STUB: FigCFStringGetCStringPtrMaybeUsingPreallocatedBuffer called");
    return NULL;
}

void* FigCFStringGetOSTypeValue(void) {
    if (verbose) puts("STUB: FigCFStringGetOSTypeValue called");
    return NULL;
}

void* FigCFURLCopyCanonicalPath(void) {
    if (verbose) puts("STUB: FigCFURLCopyCanonicalPath called");
    return NULL;
}

void* FigCFURLCreateCacheKey(void) {
    if (verbose) puts("STUB: FigCFURLCreateCacheKey called");
    return NULL;
}

void* FigCFURLCreateDirectory(void) {
    if (verbose) puts("STUB: FigCFURLCreateDirectory called");
    return NULL;
}

void* FigCFURLCreateRelativeURLWithURLAndBaseURL(void) {
    if (verbose) puts("STUB: FigCFURLCreateRelativeURLWithURLAndBaseURL called");
    return NULL;
}

void* FigCFURLDoesDirectoryExist(void) {
    if (verbose) puts("STUB: FigCFURLDoesDirectoryExist called");
    return NULL;
}

void* FigCFURLGetFileStats(void) {
    if (verbose) puts("STUB: FigCFURLGetFileStats called");
    return NULL;
}

void* FigCFURLIsLocalResource(void) {
    if (verbose) puts("STUB: FigCFURLIsLocalResource called");
    return NULL;
}

void* FigCFWeakReferenceDestroy(void) {
    if (verbose) puts("STUB: FigCFWeakReferenceDestroy called");
    return NULL;
}

void* FigCFWeakReferenceHolderCopyReferencedObject(void) {
    if (verbose) puts("STUB: FigCFWeakReferenceHolderCopyReferencedObject called");
    return NULL;
}

void* FigCFWeakReferenceHolderCreateWithReferencedObject(void) {
    if (verbose) puts("STUB: FigCFWeakReferenceHolderCreateWithReferencedObject called");
    return NULL;
}

void* FigCFWeakReferenceHolderGetTypeID(void) {
    if (verbose) puts("STUB: FigCFWeakReferenceHolderGetTypeID called");
    return NULL;
}

void* FigCFWeakReferenceInit(void) {
    if (verbose) puts("STUB: FigCFWeakReferenceInit called");
    return NULL;
}

void* FigCFWeakReferenceLoadAndRetain(void) {
    if (verbose) puts("STUB: FigCFWeakReferenceLoadAndRetain called");
    return NULL;
}

void* FigCFWeakReferenceStore(void) {
    if (verbose) puts("STUB: FigCFWeakReferenceStore called");
    return NULL;
}

void* FigCFWeakReferenceTableAddValueAndGetKey(void) {
    if (verbose) puts("STUB: FigCFWeakReferenceTableAddValueAndGetKey called");
    return NULL;
}

void* FigCFWeakReferenceTableAddValueAssociatedWithKey(void) {
    if (verbose) puts("STUB: FigCFWeakReferenceTableAddValueAssociatedWithKey called");
    return NULL;
}

void* FigCFWeakReferenceTableApplyFunction(void) {
    if (verbose) puts("STUB: FigCFWeakReferenceTableApplyFunction called");
    return NULL;
}

void* FigCFWeakReferenceTableCopyValue(void) {
    if (verbose) puts("STUB: FigCFWeakReferenceTableCopyValue called");
    return NULL;
}

void* FigCFWeakReferenceTableCreate(void) {
    if (verbose) puts("STUB: FigCFWeakReferenceTableCreate called");
    return NULL;
}

void* FigCFWeakReferenceTableGetKey(void) {
    if (verbose) puts("STUB: FigCFWeakReferenceTableGetKey called");
    return NULL;
}

void* FigCFWeakReferenceTableGetTypeID(void) {
    if (verbose) puts("STUB: FigCFWeakReferenceTableGetTypeID called");
    return NULL;
}

void* FigCFWeakReferenceTableRemoveValue(void) {
    if (verbose) puts("STUB: FigCFWeakReferenceTableRemoveValue called");
    return NULL;
}

void* FigCPECryptorGetClassID(void) {
    if (verbose) puts("STUB: FigCPECryptorGetClassID called");
    return NULL;
}

void* FigCPECryptorGetNotificationCenter(void) {
    if (verbose) puts("STUB: FigCPECryptorGetNotificationCenter called");
    return NULL;
}

void* FigCPECryptorGetTypeID(void) {
    if (verbose) puts("STUB: FigCPECryptorGetTypeID called");
    return NULL;
}

void* FigCaptionDataCopySubrange(void) {
    if (verbose) puts("STUB: FigCaptionDataCopySubrange called");
    return NULL;
}

void* FigCaptionDataCreate(void) {
    if (verbose) puts("STUB: FigCaptionDataCreate called");
    return NULL;
}

void* FigCaptionDataCreateMutable(void) {
    if (verbose) puts("STUB: FigCaptionDataCreateMutable called");
    return NULL;
}

void* FigCaptionDataCreateMutableCopy(void) {
    if (verbose) puts("STUB: FigCaptionDataCreateMutableCopy called");
    return NULL;
}

void* FigCaptionDataGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigCaptionDataGetCMBaseObject called");
    return NULL;
}

void* FigCaptionDataGetClassID(void) {
    if (verbose) puts("STUB: FigCaptionDataGetClassID called");
    return NULL;
}

void* FigCaptionDataGetTypeID(void) {
    if (verbose) puts("STUB: FigCaptionDataGetTypeID called");
    return NULL;
}

void* FigCaptionDynamicStyleAddKeyFrameValue(void) {
    if (verbose) puts("STUB: FigCaptionDynamicStyleAddKeyFrameValue called");
    return NULL;
}

void* FigCaptionDynamicStyleCopyKeyFrameValueAtIndex(void) {
    if (verbose) puts("STUB: FigCaptionDynamicStyleCopyKeyFrameValueAtIndex called");
    return NULL;
}

void* FigCaptionDynamicStyleCreate(void) {
    if (verbose) puts("STUB: FigCaptionDynamicStyleCreate called");
    return NULL;
}

void* FigCaptionDynamicStyleCreateMutable(void) {
    if (verbose) puts("STUB: FigCaptionDynamicStyleCreateMutable called");
    return NULL;
}

void* FigCaptionDynamicStyleGetInitialValue(void) {
    if (verbose) puts("STUB: FigCaptionDynamicStyleGetInitialValue called");
    return NULL;
}

void* FigCaptionDynamicStyleGetKeyFrameCount(void) {
    if (verbose) puts("STUB: FigCaptionDynamicStyleGetKeyFrameCount called");
    return NULL;
}

void* FigCaptionDynamicStyleGetTypeID(void) {
    if (verbose) puts("STUB: FigCaptionDynamicStyleGetTypeID called");
    return NULL;
}

void* FigCaptionFontFamilyNameListAppend(void) {
    if (verbose) puts("STUB: FigCaptionFontFamilyNameListAppend called");
    return NULL;
}

void* FigCaptionFontFamilyNameListCopyFamilyKindAndNameAtIndex(void) {
    if (verbose) puts("STUB: FigCaptionFontFamilyNameListCopyFamilyKindAndNameAtIndex called");
    return NULL;
}

void* FigCaptionFontFamilyNameListCreateMutable(void) {
    if (verbose) puts("STUB: FigCaptionFontFamilyNameListCreateMutable called");
    return NULL;
}

void* FigCaptionFontFamilyNameListCreateMutableCopy(void) {
    if (verbose) puts("STUB: FigCaptionFontFamilyNameListCreateMutableCopy called");
    return NULL;
}

void* FigCaptionFontFamilyNameListGetCount(void) {
    if (verbose) puts("STUB: FigCaptionFontFamilyNameListGetCount called");
    return NULL;
}

void* FigCaptionFontFamilyNameListGetTypeID(void) {
    if (verbose) puts("STUB: FigCaptionFontFamilyNameListGetTypeID called");
    return NULL;
}

void* FigCaptionGeometryCopyCellBasedDimensionAsDictionary(void) {
    if (verbose) puts("STUB: FigCaptionGeometryCopyCellBasedDimensionAsDictionary called");
    return NULL;
}

void* FigCaptionGeometryCopyCellBasedPointAsDictionary(void) {
    if (verbose) puts("STUB: FigCaptionGeometryCopyCellBasedPointAsDictionary called");
    return NULL;
}

void* FigCaptionGeometryCopyCellBasedSizeAsDictionary(void) {
    if (verbose) puts("STUB: FigCaptionGeometryCopyCellBasedSizeAsDictionary called");
    return NULL;
}

void* FigCaptionGeometryGetCellBasedDimensionFromDictionary(void) {
    if (verbose) puts("STUB: FigCaptionGeometryGetCellBasedDimensionFromDictionary called");
    return NULL;
}

void* FigCaptionGeometryGetCellBasedPointFromDictionary(void) {
    if (verbose) puts("STUB: FigCaptionGeometryGetCellBasedPointFromDictionary called");
    return NULL;
}

void* FigCaptionGeometryGetCellBasedSizeFromDictionary(void) {
    if (verbose) puts("STUB: FigCaptionGeometryGetCellBasedSizeFromDictionary called");
    return NULL;
}

void* FigCaptionGroupCreate(void) {
    if (verbose) puts("STUB: FigCaptionGroupCreate called");
    return NULL;
}

void* FigCaptionGroupCreateEmpty(void) {
    if (verbose) puts("STUB: FigCaptionGroupCreateEmpty called");
    return NULL;
}

void* FigCaptionGroupFormatDescriptionCreate(void) {
    if (verbose) puts("STUB: FigCaptionGroupFormatDescriptionCreate called");
    return NULL;
}

void* FigCaptionGroupGetCaptionData(void) {
    if (verbose) puts("STUB: FigCaptionGroupGetCaptionData called");
    return NULL;
}

void* FigCaptionGroupGetDurationAfterSlice(void) {
    if (verbose) puts("STUB: FigCaptionGroupGetDurationAfterSlice called");
    return NULL;
}

void* FigCaptionGroupGetDurationBeforeSlice(void) {
    if (verbose) puts("STUB: FigCaptionGroupGetDurationBeforeSlice called");
    return NULL;
}

void* FigCaptionGroupGetSliceCount(void) {
    if (verbose) puts("STUB: FigCaptionGroupGetSliceCount called");
    return NULL;
}

void* FigCaptionGroupGetTypeID(void) {
    if (verbose) puts("STUB: FigCaptionGroupGetTypeID called");
    return NULL;
}

void* FigCaptionRegionCreate(void) {
    if (verbose) puts("STUB: FigCaptionRegionCreate called");
    return NULL;
}

void* FigCaptionRegionCreateMutable(void) {
    if (verbose) puts("STUB: FigCaptionRegionCreateMutable called");
    return NULL;
}

void* FigCaptionRegionCreateMutableCopy(void) {
    if (verbose) puts("STUB: FigCaptionRegionCreateMutableCopy called");
    return NULL;
}

void* FigCaptionRegionGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigCaptionRegionGetCMBaseObject called");
    return NULL;
}

void* FigCaptionRegionGetClassID(void) {
    if (verbose) puts("STUB: FigCaptionRegionGetClassID called");
    return NULL;
}

void* FigCaptionRegionGetTypeID(void) {
    if (verbose) puts("STUB: FigCaptionRegionGetTypeID called");
    return NULL;
}

void* FigCaptionTextOutlineCreate(void) {
    if (verbose) puts("STUB: FigCaptionTextOutlineCreate called");
    return NULL;
}

void* FigCaptionTextOutlineGetColor(void) {
    if (verbose) puts("STUB: FigCaptionTextOutlineGetColor called");
    return NULL;
}

void* FigCaptionTextOutlineGetThickness(void) {
    if (verbose) puts("STUB: FigCaptionTextOutlineGetThickness called");
    return NULL;
}

void* FigCaptionTextOutlineGetTypeID(void) {
    if (verbose) puts("STUB: FigCaptionTextOutlineGetTypeID called");
    return NULL;
}

void* FigCheckIsCurrentDispatchQueue(void) {
    if (verbose) puts("STUB: FigCheckIsCurrentDispatchQueue called");
    return NULL;
}

void* FigCheckIsNotCurrentDispatchQueue(void) {
    if (verbose) puts("STUB: FigCheckIsNotCurrentDispatchQueue called");
    return NULL;
}

void* FigClockConvertHostTimeToSystemUnits(void) {
    if (verbose) puts("STUB: FigClockConvertHostTimeToSystemUnits called");
    return NULL;
}

void* FigClockGetAnchorTime(void) {
    if (verbose) puts("STUB: FigClockGetAnchorTime called");
    return NULL;
}

void* FigClockGetHostTimeClock(void) {
    if (verbose) puts("STUB: FigClockGetHostTimeClock called");
    return NULL;
}

void* FigClockGetTime(void) {
    if (verbose) puts("STUB: FigClockGetTime called");
    return NULL;
}

void* FigClockGetTypeID(void) {
    if (verbose) puts("STUB: FigClockGetTypeID called");
    return NULL;
}

void* FigClockInvalidate(void) {
    if (verbose) puts("STUB: FigClockInvalidate called");
    return NULL;
}

void* FigClockMakeHostTimeFromSystemUnits(void) {
    if (verbose) puts("STUB: FigClockMakeHostTimeFromSystemUnits called");
    return NULL;
}

void* FigClockMightDrift(void) {
    if (verbose) puts("STUB: FigClockMightDrift called");
    return NULL;
}

void* FigClockRelease(void) {
    if (verbose) puts("STUB: FigClockRelease called");
    return NULL;
}

void* FigClockRetain(void) {
    if (verbose) puts("STUB: FigClockRetain called");
    return NULL;
}

void* FigCompositeReadByteStreamCreate(void) {
    if (verbose) puts("STUB: FigCompositeReadByteStreamCreate called");
    return NULL;
}

void* FigConcurrentAccessDetectorBeginReading(void) {
    if (verbose) puts("STUB: FigConcurrentAccessDetectorBeginReading called");
    return NULL;
}

void* FigConcurrentAccessDetectorBeginWriting(void) {
    if (verbose) puts("STUB: FigConcurrentAccessDetectorBeginWriting called");
    return NULL;
}

void* FigConcurrentAccessDetectorCreate(void) {
    if (verbose) puts("STUB: FigConcurrentAccessDetectorCreate called");
    return NULL;
}

void* FigConcurrentAccessDetectorDestroy(void) {
    if (verbose) puts("STUB: FigConcurrentAccessDetectorDestroy called");
    return NULL;
}

void* FigConcurrentAccessDetectorEndReading(void) {
    if (verbose) puts("STUB: FigConcurrentAccessDetectorEndReading called");
    return NULL;
}

void* FigConcurrentAccessDetectorEndWriting(void) {
    if (verbose) puts("STUB: FigConcurrentAccessDetectorEndWriting called");
    return NULL;
}

void* FigConditionVariableBroadcast(void) {
    if (verbose) puts("STUB: FigConditionVariableBroadcast called");
    return NULL;
}

void* FigConditionVariableCreate(void) {
    if (verbose) puts("STUB: FigConditionVariableCreate called");
    return NULL;
}

void* FigConditionVariableDestroy(void) {
    if (verbose) puts("STUB: FigConditionVariableDestroy called");
    return NULL;
}

void* FigConditionVariableWaitRelative(void) {
    if (verbose) puts("STUB: FigConditionVariableWaitRelative called");
    return NULL;
}

void* FigCopyBacktrace(void) {
    if (verbose) puts("STUB: FigCopyBacktrace called");
    return NULL;
}

void* FigCopyCFPreferenceAppValueOrOptionCFTypeWithDefault(void) {
    if (verbose) puts("STUB: FigCopyCFPreferenceAppValueOrOptionCFTypeWithDefault called");
    return NULL;
}

void* FigCopyRankedLanguagesAccordingToPreferredLanguages(void) {
    if (verbose) puts("STUB: FigCopyRankedLanguagesAccordingToPreferredLanguages called");
    return NULL;
}

void* FigCreateBase32EncodedStringFromBytes(void) {
    if (verbose) puts("STUB: FigCreateBase32EncodedStringFromBytes called");
    return NULL;
}

void* FigCreateBase64EncodedStringFromBytes(void) {
    if (verbose) puts("STUB: FigCreateBase64EncodedStringFromBytes called");
    return NULL;
}

void* FigCreateBlockBufferCopyingMemoryBlock(void) {
    if (verbose) puts("STUB: FigCreateBlockBufferCopyingMemoryBlock called");
    return NULL;
}

void* FigCreateBlockBufferWithCFDataNoCopy(void) {
    if (verbose) puts("STUB: FigCreateBlockBufferWithCFDataNoCopy called");
    return NULL;
}

void* FigCreateBlockBufferWithCFString(void) {
    if (verbose) puts("STUB: FigCreateBlockBufferWithCFString called");
    return NULL;
}

void* FigCreateBytesFromCFString(void) {
    if (verbose) puts("STUB: FigCreateBytesFromCFString called");
    return NULL;
}

void* FigCreateBytesFromCFStringAndMovieLangCode(void) {
    if (verbose) puts("STUB: FigCreateBytesFromCFStringAndMovieLangCode called");
    return NULL;
}

void* FigCreateCFArrayFromColorValues(void) {
    if (verbose) puts("STUB: FigCreateCFArrayFromColorValues called");
    return NULL;
}

void* FigCreateCFDataFromCFPropertyList(void) {
    if (verbose) puts("STUB: FigCreateCFDataFromCFPropertyList called");
    return NULL;
}

void* FigCreateCFDataWithBlockBufferNoCopy(void) {
    if (verbose) puts("STUB: FigCreateCFDataWithBlockBufferNoCopy called");
    return NULL;
}

void* FigCreateCFPropertyListFromData(void) {
    if (verbose) puts("STUB: FigCreateCFPropertyListFromData called");
    return NULL;
}

void* FigCreateCFURLFromAliasRecordBytes(void) {
    if (verbose) puts("STUB: FigCreateCFURLFromAliasRecordBytes called");
    return NULL;
}

void* FigCustomURLHandlerGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigCustomURLHandlerGetCMBaseObject called");
    return NULL;
}

void* FigCustomURLHandlerGetClassID(void) {
    if (verbose) puts("STUB: FigCustomURLHandlerGetClassID called");
    return NULL;
}

void* FigCustomURLHandlerGetTypeID(void) {
    if (verbose) puts("STUB: FigCustomURLHandlerGetTypeID called");
    return NULL;
}

void* FigCustomURLLoaderCreate(void) {
    if (verbose) puts("STUB: FigCustomURLLoaderCreate called");
    return NULL;
}

void* FigCustomURLLoaderGetCFTypeID(void) {
    if (verbose) puts("STUB: FigCustomURLLoaderGetCFTypeID called");
    return NULL;
}

void* FigCustomURLLoaderGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigCustomURLLoaderGetCMBaseObject called");
    return NULL;
}

void* FigCustomURLLoaderGetClassID(void) {
    if (verbose) puts("STUB: FigCustomURLLoaderGetClassID called");
    return NULL;
}

void* FigCustomURLProviderGetCFTypeID(void) {
    if (verbose) puts("STUB: FigCustomURLProviderGetCFTypeID called");
    return NULL;
}

void* FigCustomURLProviderGetClassID(void) {
    if (verbose) puts("STUB: FigCustomURLProviderGetClassID called");
    return NULL;
}

void* FigCustomURLProviderGetFigBaseObject(void) {
    if (verbose) puts("STUB: FigCustomURLProviderGetFigBaseObject called");
    return NULL;
}

void* FigCustomURLProviderNewRequestID(void) {
    if (verbose) puts("STUB: FigCustomURLProviderNewRequestID called");
    return NULL;
}

void* FigCustomURLProviderRelease(void) {
    if (verbose) puts("STUB: FigCustomURLProviderRelease called");
    return NULL;
}

void* FigCustomURLProviderRetain(void) {
    if (verbose) puts("STUB: FigCustomURLProviderRetain called");
    return NULL;
}

void* FigCustomURLRequestInfoCopyAcceptEncoding(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoCopyAcceptEncoding called");
    return NULL;
}

void* FigCustomURLRequestInfoCopyAllowedUniversalTypeIdentifiers(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoCopyAllowedUniversalTypeIdentifiers called");
    return NULL;
}

void* FigCustomURLRequestInfoCopyCryptor(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoCopyCryptor called");
    return NULL;
}

void* FigCustomURLRequestInfoCopyHTTPHeaders(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoCopyHTTPHeaders called");
    return NULL;
}

void* FigCustomURLRequestInfoCopyNSURLAuthenticationChallenge(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoCopyNSURLAuthenticationChallenge called");
    return NULL;
}

void* FigCustomURLRequestInfoCopyURL(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoCopyURL called");
    return NULL;
}

void* FigCustomURLRequestInfoCopyUserAgent(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoCopyUserAgent called");
    return NULL;
}

void* FigCustomURLRequestInfoCreateMutable(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoCreateMutable called");
    return NULL;
}

void* FigCustomURLRequestInfoCreateMutableCopy(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoCreateMutableCopy called");
    return NULL;
}

void* FigCustomURLRequestInfoGetByteRange(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoGetByteRange called");
    return NULL;
}

void* FigCustomURLRequestInfoGetIsRenewalRequest(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoGetIsRenewalRequest called");
    return NULL;
}

void* FigCustomURLRequestInfoSetAcceptEncoding(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoSetAcceptEncoding called");
    return NULL;
}

void* FigCustomURLRequestInfoSetAllowedUniversalTypeIdentifiers(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoSetAllowedUniversalTypeIdentifiers called");
    return NULL;
}

void* FigCustomURLRequestInfoSetByteRange(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoSetByteRange called");
    return NULL;
}

void* FigCustomURLRequestInfoSetCryptor(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoSetCryptor called");
    return NULL;
}

void* FigCustomURLRequestInfoSetHTTPHeaders(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoSetHTTPHeaders called");
    return NULL;
}

void* FigCustomURLRequestInfoSetIsRenewalRequest(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoSetIsRenewalRequest called");
    return NULL;
}

void* FigCustomURLRequestInfoSetNSURLAuthenticationChallenge(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoSetNSURLAuthenticationChallenge called");
    return NULL;
}

void* FigCustomURLRequestInfoSetUserAgent(void) {
    if (verbose) puts("STUB: FigCustomURLRequestInfoSetUserAgent called");
    return NULL;
}

void* FigCustomURLResponseInfoCopyContentRenewalDate(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoCopyContentRenewalDate called");
    return NULL;
}

void* FigCustomURLResponseInfoCopyFinalURL(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoCopyFinalURL called");
    return NULL;
}

void* FigCustomURLResponseInfoCopyHTTPHeaders(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoCopyHTTPHeaders called");
    return NULL;
}

void* FigCustomURLResponseInfoCopyNSURLCredential(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoCopyNSURLCredential called");
    return NULL;
}

void* FigCustomURLResponseInfoCopyRedirectLocation(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoCopyRedirectLocation called");
    return NULL;
}

void* FigCustomURLResponseInfoCopyRequestInfo(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoCopyRequestInfo called");
    return NULL;
}

void* FigCustomURLResponseInfoCopyUniversalTypeIdentifier(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoCopyUniversalTypeIdentifier called");
    return NULL;
}

void* FigCustomURLResponseInfoCreate(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoCreate called");
    return NULL;
}

void* FigCustomURLResponseInfoCreateMutable(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoCreateMutable called");
    return NULL;
}

void* FigCustomURLResponseInfoCreateMutableCopy(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoCreateMutableCopy called");
    return NULL;
}

void* FigCustomURLResponseInfoGetContentLength(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoGetContentLength called");
    return NULL;
}

void* FigCustomURLResponseInfoGetHTTPStatusCode(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoGetHTTPStatusCode called");
    return NULL;
}

void* FigCustomURLResponseInfoGetIsByteRangeAccessSupported(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoGetIsByteRangeAccessSupported called");
    return NULL;
}

void* FigCustomURLResponseInfoGetIsCachingDisabled(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoGetIsCachingDisabled called");
    return NULL;
}

void* FigCustomURLResponseInfoGetNSURLSessionAuthChallengeDisposition(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoGetNSURLSessionAuthChallengeDisposition called");
    return NULL;
}

void* FigCustomURLResponseInfoGetResourceSize(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoGetResourceSize called");
    return NULL;
}

void* FigCustomURLResponseInfoSetAllValuesFromResponseInfo(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoSetAllValuesFromResponseInfo called");
    return NULL;
}

void* FigCustomURLResponseInfoSetContentLength(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoSetContentLength called");
    return NULL;
}

void* FigCustomURLResponseInfoSetContentRenewalDate(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoSetContentRenewalDate called");
    return NULL;
}

void* FigCustomURLResponseInfoSetFinalURL(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoSetFinalURL called");
    return NULL;
}

void* FigCustomURLResponseInfoSetHTTPHeaders(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoSetHTTPHeaders called");
    return NULL;
}

void* FigCustomURLResponseInfoSetHTTPStatusCode(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoSetHTTPStatusCode called");
    return NULL;
}

void* FigCustomURLResponseInfoSetIsByteRangeAccessSupported(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoSetIsByteRangeAccessSupported called");
    return NULL;
}

void* FigCustomURLResponseInfoSetIsCachingDisabled(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoSetIsCachingDisabled called");
    return NULL;
}

void* FigCustomURLResponseInfoSetNSURLCredential(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoSetNSURLCredential called");
    return NULL;
}

void* FigCustomURLResponseInfoSetNSURLSessionAuthChallengeDisposition(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoSetNSURLSessionAuthChallengeDisposition called");
    return NULL;
}

void* FigCustomURLResponseInfoSetRedirectLocation(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoSetRedirectLocation called");
    return NULL;
}

void* FigCustomURLResponseInfoSetResourceSize(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoSetResourceSize called");
    return NULL;
}

void* FigCustomURLResponseInfoSetUniversalTypeIdentifier(void) {
    if (verbose) puts("STUB: FigCustomURLResponseInfoSetUniversalTypeIdentifier called");
    return NULL;
}

void* FigDebugAssert(void) {
    if (verbose) puts("STUB: FigDebugAssert called");
    return NULL;
}

void* FigDebugAssert2(void) {
    if (verbose) puts("STUB: FigDebugAssert2 called");
    return NULL;
}

void* FigDebugAssertSyslog(void) {
    if (verbose) puts("STUB: FigDebugAssertSyslog called");
    return NULL;
}

void* FigDebugAssertSyslog2(void) {
    if (verbose) puts("STUB: FigDebugAssertSyslog2 called");
    return NULL;
}

void* FigDebugGetAdoptedVoucherProcessIDAndUUIDString(void) {
    if (verbose) puts("STUB: FigDebugGetAdoptedVoucherProcessIDAndUUIDString called");
    return NULL;
}

void* FigDebugSettingGetKeyAsBoolean(void) {
    if (verbose) puts("STUB: FigDebugSettingGetKeyAsBoolean called");
    return NULL;
}

void* FigDebugSettingGetKeyAsInteger(void) {
    if (verbose) puts("STUB: FigDebugSettingGetKeyAsInteger called");
    return NULL;
}

void* FigDeferNotificationToDispatchQueue(void) {
    if (verbose) puts("STUB: FigDeferNotificationToDispatchQueue called");
    return NULL;
}

void* FigDeferNotificationToRunLoop(void) {
    if (verbose) puts("STUB: FigDeferNotificationToRunLoop called");
    return NULL;
}

void* FigDepthBlurEffectRenderingParametersV1FromCFData(void) {
    if (verbose) puts("STUB: FigDepthBlurEffectRenderingParametersV1FromCFData called");
    return NULL;
}

void* FigDepthBytesPerPixelForDepthFormat(void) {
    if (verbose) puts("STUB: FigDepthBytesPerPixelForDepthFormat called");
    return NULL;
}

void* FigDepthConvertBuffer(void) {
    if (verbose) puts("STUB: FigDepthConvertBuffer called");
    return NULL;
}

void* FigDepthConvertRotationAndMirroringToExifOrientation(void) {
    if (verbose) puts("STUB: FigDepthConvertRotationAndMirroringToExifOrientation called");
    return NULL;
}

void* FigDepthConvertToRGBA(void) {
    if (verbose) puts("STUB: FigDepthConvertToRGBA called");
    return NULL;
}

void* FigDepthDataGetCurrentMajorVersion(void) {
    if (verbose) puts("STUB: FigDepthDataGetCurrentMajorVersion called");
    return NULL;
}

void* FigDepthFormatIs16Bit(void) {
    if (verbose) puts("STUB: FigDepthFormatIs16Bit called");
    return NULL;
}

void* FigDepthFormatIs32Bit(void) {
    if (verbose) puts("STUB: FigDepthFormatIs32Bit called");
    return NULL;
}

void* FigDepthFormatIsDepth(void) {
    if (verbose) puts("STUB: FigDepthFormatIsDepth called");
    return NULL;
}

void* FigDepthFormatIsDisparity(void) {
    if (verbose) puts("STUB: FigDepthFormatIsDisparity called");
    return NULL;
}

void* FigDepthFormatIsFixed16(void) {
    if (verbose) puts("STUB: FigDepthFormatIsFixed16 called");
    return NULL;
}

void* FigDepthRotateBuffer(void) {
    if (verbose) puts("STUB: FigDepthRotateBuffer called");
    return NULL;
}

void* FigDepthRotateCalibrationData(void) {
    if (verbose) puts("STUB: FigDepthRotateCalibrationData called");
    return NULL;
}

void* FigDepthRotateMetadataDictionary(void) {
    if (verbose) puts("STUB: FigDepthRotateMetadataDictionary called");
    return NULL;
}

void* FigDepthScaleBuffer(void) {
    if (verbose) puts("STUB: FigDepthScaleBuffer called");
    return NULL;
}

void* FigDerivedClockCreateWithGetTimeCallback(void) {
    if (verbose) puts("STUB: FigDerivedClockCreateWithGetTimeCallback called");
    return NULL;
}

void* FigDerivedClockDiscontinuityOccurred(void) {
    if (verbose) puts("STUB: FigDerivedClockDiscontinuityOccurred called");
    return NULL;
}

void* FigDerivedClockGetDerivedStorage(void) {
    if (verbose) puts("STUB: FigDerivedClockGetDerivedStorage called");
    return NULL;
}

void* FigDerivedClockVerifyCallbacksMatch(void) {
    if (verbose) puts("STUB: FigDerivedClockVerifyCallbacksMatch called");
    return NULL;
}

void* FigDerivedFormatDescriptionCreate(void) {
    if (verbose) puts("STUB: FigDerivedFormatDescriptionCreate called");
    return NULL;
}

void* FigDerivedFormatDescriptionGetDerivedStorage(void) {
    if (verbose) puts("STUB: FigDerivedFormatDescriptionGetDerivedStorage called");
    return NULL;
}

void* FigDerivedFormatDescriptionRegister(void) {
    if (verbose) puts("STUB: FigDerivedFormatDescriptionRegister called");
    return NULL;
}

void* FigDerivedObjectCreate(void) {
    if (verbose) puts("STUB: FigDerivedObjectCreate called");
    return NULL;
}

void* FigDigestMD5AppendData(void) {
    if (verbose) puts("STUB: FigDigestMD5AppendData called");
    return NULL;
}

void* FigDigestMD5Create(void) {
    if (verbose) puts("STUB: FigDigestMD5Create called");
    return NULL;
}

void* FigDigestMD5Destroy(void) {
    if (verbose) puts("STUB: FigDigestMD5Destroy called");
    return NULL;
}

void* FigDigestMD5GetDigest(void) {
    if (verbose) puts("STUB: FigDigestMD5GetDigest called");
    return NULL;
}

void* FigDigestMD5Reset(void) {
    if (verbose) puts("STUB: FigDigestMD5Reset called");
    return NULL;
}

void* FigDigestSHA1AppendData(void) {
    if (verbose) puts("STUB: FigDigestSHA1AppendData called");
    return NULL;
}

void* FigDigestSHA1Create(void) {
    if (verbose) puts("STUB: FigDigestSHA1Create called");
    return NULL;
}

void* FigDigestSHA1Destroy(void) {
    if (verbose) puts("STUB: FigDigestSHA1Destroy called");
    return NULL;
}

void* FigDigestSHA1GetDigest(void) {
    if (verbose) puts("STUB: FigDigestSHA1GetDigest called");
    return NULL;
}

void* FigDigestSHA1Reset(void) {
    if (verbose) puts("STUB: FigDigestSHA1Reset called");
    return NULL;
}

void* FigDispatchAsyncAndWait(void) {
    if (verbose) puts("STUB: FigDispatchAsyncAndWait called");
    return NULL;
}

void* FigDispatchAsyncAndWaitWithTimeout_f(void) {
    if (verbose) puts("STUB: FigDispatchAsyncAndWaitWithTimeout_f called");
    return NULL;
}

void* FigDispatchAsyncAndWait_f(void) {
    if (verbose) puts("STUB: FigDispatchAsyncAndWait_f called");
    return NULL;
}

void* FigDispatchAsyncPostNotification(void) {
    if (verbose) puts("STUB: FigDispatchAsyncPostNotification called");
    return NULL;
}

void* FigDispatchAsyncSetPropertyImplementation(void) {
    if (verbose) puts("STUB: FigDispatchAsyncSetPropertyImplementation called");
    return NULL;
}

void* FigDispatchCreateDispatchSourceBlock(void) {
    if (verbose) puts("STUB: FigDispatchCreateDispatchSourceBlock called");
    return NULL;
}

void* FigDispatchOnceInitWithinObject(void) {
    if (verbose) puts("STUB: FigDispatchOnceInitWithinObject called");
    return NULL;
}

void* FigDispatchQueueCreateWithPriority(void) {
    if (verbose) puts("STUB: FigDispatchQueueCreateWithPriority called");
    return NULL;
}

void* FigDispatchQueueCreateWithPriorityAndClientPID(void) {
    if (verbose) puts("STUB: FigDispatchQueueCreateWithPriorityAndClientPID called");
    return NULL;
}

void* FigDispatchQueueSetPriorityAndClientPID(void) {
    if (verbose) puts("STUB: FigDispatchQueueSetPriorityAndClientPID called");
    return NULL;
}

void* FigDispatchSyncCopyPropertyImplementation(void) {
    if (verbose) puts("STUB: FigDispatchSyncCopyPropertyImplementation called");
    return NULL;
}

void* FigDoesBigEndianSoundDescriptionRequireSampleTableLie(void) {
    if (verbose) puts("STUB: FigDoesBigEndianSoundDescriptionRequireSampleTableLie called");
    return NULL;
}

void* FigEndianClosedCaptionDescriptionData_BtoN(void) {
    if (verbose) puts("STUB: FigEndianClosedCaptionDescriptionData_BtoN called");
    return NULL;
}

void* FigEndianClosedCaptionDescriptionData_NtoB(void) {
    if (verbose) puts("STUB: FigEndianClosedCaptionDescriptionData_NtoB called");
    return NULL;
}

void* FigEndianImageDescriptionData_BtoN(void) {
    if (verbose) puts("STUB: FigEndianImageDescriptionData_BtoN called");
    return NULL;
}

void* FigEndianImageDescriptionData_NtoB(void) {
    if (verbose) puts("STUB: FigEndianImageDescriptionData_NtoB called");
    return NULL;
}

void* FigEndianMetadataDescriptionData_BtoN(void) {
    if (verbose) puts("STUB: FigEndianMetadataDescriptionData_BtoN called");
    return NULL;
}

void* FigEndianMetadataDescriptionData_NtoB(void) {
    if (verbose) puts("STUB: FigEndianMetadataDescriptionData_NtoB called");
    return NULL;
}

void* FigEndianSoundDescriptionData_BtoN(void) {
    if (verbose) puts("STUB: FigEndianSoundDescriptionData_BtoN called");
    return NULL;
}

void* FigEndianSoundDescriptionData_NtoB(void) {
    if (verbose) puts("STUB: FigEndianSoundDescriptionData_NtoB called");
    return NULL;
}

void* FigEndianTextDescriptionData_BtoN(void) {
    if (verbose) puts("STUB: FigEndianTextDescriptionData_BtoN called");
    return NULL;
}

void* FigEndianTextDescriptionData_NtoB(void) {
    if (verbose) puts("STUB: FigEndianTextDescriptionData_NtoB called");
    return NULL;
}

void* FigEndianTimeCodeDescriptionData_BtoN(void) {
    if (verbose) puts("STUB: FigEndianTimeCodeDescriptionData_BtoN called");
    return NULL;
}

void* FigEndianTimeCodeDescriptionData_NtoB(void) {
    if (verbose) puts("STUB: FigEndianTimeCodeDescriptionData_NtoB called");
    return NULL;
}

void* FigEndpointActivate(void) {
    if (verbose) puts("STUB: FigEndpointActivate called");
    return NULL;
}

void* FigEndpointActivateSync(void) {
    if (verbose) puts("STUB: FigEndpointActivateSync called");
    return NULL;
}

void* FigEndpointAggregateGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigEndpointAggregateGetCMBaseObject called");
    return NULL;
}

void* FigEndpointAggregateGetClassID(void) {
    if (verbose) puts("STUB: FigEndpointAggregateGetClassID called");
    return NULL;
}

void* FigEndpointAggregateGetFigEndpoint(void) {
    if (verbose) puts("STUB: FigEndpointAggregateGetFigEndpoint called");
    return NULL;
}

void* FigEndpointAudioGroupXPCRemoteCreate(void) {
    if (verbose) puts("STUB: FigEndpointAudioGroupXPCRemoteCreate called");
    return NULL;
}

void* FigEndpointAudioSourceGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigEndpointAudioSourceGetCMBaseObject called");
    return NULL;
}

void* FigEndpointAudioSourceGetClassID(void) {
    if (verbose) puts("STUB: FigEndpointAudioSourceGetClassID called");
    return NULL;
}

void* FigEndpointAudioSourceGetTypeID(void) {
    if (verbose) puts("STUB: FigEndpointAudioSourceGetTypeID called");
    return NULL;
}

void* FigEndpointCopyActiveCarPlayEndpoint(void) {
    if (verbose) puts("STUB: FigEndpointCopyActiveCarPlayEndpoint called");
    return NULL;
}

void* FigEndpointCopyActiveEndpointOfType(void) {
    if (verbose) puts("STUB: FigEndpointCopyActiveEndpointOfType called");
    return NULL;
}

void* FigEndpointCopyFirstStreamOfType(void) {
    if (verbose) puts("STUB: FigEndpointCopyFirstStreamOfType called");
    return NULL;
}

void* FigEndpointCopyStreamsForTypeAndSubType(void) {
    if (verbose) puts("STUB: FigEndpointCopyStreamsForTypeAndSubType called");
    return NULL;
}

void* FigEndpointDeactivate(void) {
    if (verbose) puts("STUB: FigEndpointDeactivate called");
    return NULL;
}

void* FigEndpointEnsureAuthorizedSync(void) {
    if (verbose) puts("STUB: FigEndpointEnsureAuthorizedSync called");
    return NULL;
}

void* FigEndpointExtendedGetClassID(void) {
    if (verbose) puts("STUB: FigEndpointExtendedGetClassID called");
    return NULL;
}

void* FigEndpointExtendedGetFigEndpoint(void) {
    if (verbose) puts("STUB: FigEndpointExtendedGetFigEndpoint called");
    return NULL;
}

void* FigEndpointGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigEndpointGetCMBaseObject called");
    return NULL;
}

void* FigEndpointGetClassID(void) {
    if (verbose) puts("STUB: FigEndpointGetClassID called");
    return NULL;
}

void* FigEndpointGetSupportedFeatures(void) {
    if (verbose) puts("STUB: FigEndpointGetSupportedFeatures called");
    return NULL;
}

void* FigEndpointGetTypeID(void) {
    if (verbose) puts("STUB: FigEndpointGetTypeID called");
    return NULL;
}

void* FigEndpointGroupStartServer(void) {
    if (verbose) puts("STUB: FigEndpointGroupStartServer called");
    return NULL;
}

void* FigEndpointGroupXPCRemoteGetObjectID(void) {
    if (verbose) puts("STUB: FigEndpointGroupXPCRemoteGetObjectID called");
    return NULL;
}

void* FigEndpointIsCarPlayActive(void) {
    if (verbose) puts("STUB: FigEndpointIsCarPlayActive called");
    return NULL;
}

void* FigEndpointManagerCopyActivatedEndpoints(void) {
    if (verbose) puts("STUB: FigEndpointManagerCopyActivatedEndpoints called");
    return NULL;
}

void* FigEndpointManagerCopyAirPlayManager(void) {
    if (verbose) puts("STUB: FigEndpointManagerCopyAirPlayManager called");
    return NULL;
}

void* FigEndpointManagerCopyEndpointForID(void) {
    if (verbose) puts("STUB: FigEndpointManagerCopyEndpointForID called");
    return NULL;
}

void* FigEndpointManagerCopyEndpointForName(void) {
    if (verbose) puts("STUB: FigEndpointManagerCopyEndpointForName called");
    return NULL;
}

void* FigEndpointManagerCopyEndpointsForType(void) {
    if (verbose) puts("STUB: FigEndpointManagerCopyEndpointsForType called");
    return NULL;
}

void* FigEndpointManagerGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigEndpointManagerGetCMBaseObject called");
    return NULL;
}

void* FigEndpointManagerGetClassID(void) {
    if (verbose) puts("STUB: FigEndpointManagerGetClassID called");
    return NULL;
}

void* FigEndpointManagerGetTypeID(void) {
    if (verbose) puts("STUB: FigEndpointManagerGetTypeID called");
    return NULL;
}

void* FigEndpointManagerRemoteCopyAirPlayManager(void) {
    if (verbose) puts("STUB: FigEndpointManagerRemoteCopyAirPlayManager called");
    return NULL;
}

void* FigEndpointManagerStartServer(void) {
    if (verbose) puts("STUB: FigEndpointManagerStartServer called");
    return NULL;
}

void* FigEndpointMessengerGetClassID(void) {
    if (verbose) puts("STUB: FigEndpointMessengerGetClassID called");
    return NULL;
}

void* FigEndpointMessengerGetTypeID(void) {
    if (verbose) puts("STUB: FigEndpointMessengerGetTypeID called");
    return NULL;
}

void* FigEndpointMessengerStartServer(void) {
    if (verbose) puts("STUB: FigEndpointMessengerStartServer called");
    return NULL;
}

void* FigEndpointMessengerXPCRemoteCreateWithObjectID(void) {
    if (verbose) puts("STUB: FigEndpointMessengerXPCRemoteCreateWithObjectID called");
    return NULL;
}

void* FigEndpointPickerCreate(void) {
    if (verbose) puts("STUB: FigEndpointPickerCreate called");
    return NULL;
}

void* FigEndpointPickerGetClassID(void) {
    if (verbose) puts("STUB: FigEndpointPickerGetClassID called");
    return NULL;
}

void* FigEndpointPickerGetTypeID(void) {
    if (verbose) puts("STUB: FigEndpointPickerGetTypeID called");
    return NULL;
}

void* FigEndpointPickerManagerGetSharedManager(void) {
    if (verbose) puts("STUB: FigEndpointPickerManagerGetSharedManager called");
    return NULL;
}

void* FigEndpointPickerManagerRegisterEndpointManager(void) {
    if (verbose) puts("STUB: FigEndpointPickerManagerRegisterEndpointManager called");
    return NULL;
}

void* FigEndpointPickerManagerStart(void) {
    if (verbose) puts("STUB: FigEndpointPickerManagerStart called");
    return NULL;
}

void* FigEndpointPickerStartServer(void) {
    if (verbose) puts("STUB: FigEndpointPickerStartServer called");
    return NULL;
}

void* FigEndpointPickerXPCRemoteCreate(void) {
    if (verbose) puts("STUB: FigEndpointPickerXPCRemoteCreate called");
    return NULL;
}

void* FigEndpointPlaybackSessionGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigEndpointPlaybackSessionGetCMBaseObject called");
    return NULL;
}

void* FigEndpointPlaybackSessionGetClassID(void) {
    if (verbose) puts("STUB: FigEndpointPlaybackSessionGetClassID called");
    return NULL;
}

void* FigEndpointPlaybackSessionGetTypeID(void) {
    if (verbose) puts("STUB: FigEndpointPlaybackSessionGetTypeID called");
    return NULL;
}

void* FigEndpointPlaybackSessionStartServer(void) {
    if (verbose) puts("STUB: FigEndpointPlaybackSessionStartServer called");
    return NULL;
}

void* FigEndpointRemoteControlGroupGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigEndpointRemoteControlGroupGetCMBaseObject called");
    return NULL;
}

void* FigEndpointRemoteControlGroupGetClassID(void) {
    if (verbose) puts("STUB: FigEndpointRemoteControlGroupGetClassID called");
    return NULL;
}

void* FigEndpointRemoteControlSessionGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigEndpointRemoteControlSessionGetCMBaseObject called");
    return NULL;
}

void* FigEndpointRemoteControlSessionGetClassID(void) {
    if (verbose) puts("STUB: FigEndpointRemoteControlSessionGetClassID called");
    return NULL;
}

void* FigEndpointRemoteControlSessionGetTypeID(void) {
    if (verbose) puts("STUB: FigEndpointRemoteControlSessionGetTypeID called");
    return NULL;
}

void* FigEndpointRemoteControlSessionStartServer(void) {
    if (verbose) puts("STUB: FigEndpointRemoteControlSessionStartServer called");
    return NULL;
}

void* FigEndpointStartServer(void) {
    if (verbose) puts("STUB: FigEndpointStartServer called");
    return NULL;
}

void* FigEndpointStreamAirPlayGetClassID(void) {
    if (verbose) puts("STUB: FigEndpointStreamAirPlayGetClassID called");
    return NULL;
}

void* FigEndpointStreamAirPlayGetFigEndpointStream(void) {
    if (verbose) puts("STUB: FigEndpointStreamAirPlayGetFigEndpointStream called");
    return NULL;
}

void* FigEndpointStreamGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigEndpointStreamGetCMBaseObject called");
    return NULL;
}

void* FigEndpointStreamGetClassID(void) {
    if (verbose) puts("STUB: FigEndpointStreamGetClassID called");
    return NULL;
}

void* FigEndpointStreamGetTypeID(void) {
    if (verbose) puts("STUB: FigEndpointStreamGetTypeID called");
    return NULL;
}

void* FigEndpointStreamResume(void) {
    if (verbose) puts("STUB: FigEndpointStreamResume called");
    return NULL;
}

void* FigEndpointStreamResumeSync(void) {
    if (verbose) puts("STUB: FigEndpointStreamResumeSync called");
    return NULL;
}

void* FigEndpointStreamStartServer(void) {
    if (verbose) puts("STUB: FigEndpointStreamStartServer called");
    return NULL;
}

void* FigEndpointStreamSuspend(void) {
    if (verbose) puts("STUB: FigEndpointStreamSuspend called");
    return NULL;
}

void* FigEndpointStreamSuspendSync(void) {
    if (verbose) puts("STUB: FigEndpointStreamSuspendSync called");
    return NULL;
}

void* FigEndpointUIAgentCreate(void) {
    if (verbose) puts("STUB: FigEndpointUIAgentCreate called");
    return NULL;
}

void* FigEndpointUIAgentGetClassID(void) {
    if (verbose) puts("STUB: FigEndpointUIAgentGetClassID called");
    return NULL;
}

void* FigEndpointUIAgentGetTypeID(void) {
    if (verbose) puts("STUB: FigEndpointUIAgentGetTypeID called");
    return NULL;
}

void* FigEndpointUIAgentStartServer(void) {
    if (verbose) puts("STUB: FigEndpointUIAgentStartServer called");
    return NULL;
}

void* FigEndpointUIAgentXPCRemoteCreate(void) {
    if (verbose) puts("STUB: FigEndpointUIAgentXPCRemoteCreate called");
    return NULL;
}

void* FigEndpointUtility_EndpointPredicate_ContainsID(void) {
    if (verbose) puts("STUB: FigEndpointUtility_EndpointPredicate_ContainsID called");
    return NULL;
}

void* FigEndpointUtility_EndpointPredicate_EqualsAudioStreamID(void) {
    if (verbose) puts("STUB: FigEndpointUtility_EndpointPredicate_EqualsAudioStreamID called");
    return NULL;
}

void* FigEndpointXPCRemoteGetObjectID(void) {
    if (verbose) puts("STUB: FigEndpointXPCRemoteGetObjectID called");
    return NULL;
}

void* FigEventCreate(void) {
    if (verbose) puts("STUB: FigEventCreate called");
    return NULL;
}

void* FigEventDestroy(void) {
    if (verbose) puts("STUB: FigEventDestroy called");
    return NULL;
}

void* FigEventReset(void) {
    if (verbose) puts("STUB: FigEventReset called");
    return NULL;
}

void* FigEventSet(void) {
    if (verbose) puts("STUB: FigEventSet called");
    return NULL;
}

void* FigEventWaitRelative(void) {
    if (verbose) puts("STUB: FigEventWaitRelative called");
    return NULL;
}

void* FigFileDeleteDirectory(void) {
    if (verbose) puts("STUB: FigFileDeleteDirectory called");
    return NULL;
}

void* FigFileDeleteFile(void) {
    if (verbose) puts("STUB: FigFileDeleteFile called");
    return NULL;
}

void* FigFileDoesFileExist(void) {
    if (verbose) puts("STUB: FigFileDoesFileExist called");
    return NULL;
}

void* FigFileForkClose(void) {
    if (verbose) puts("STUB: FigFileForkClose called");
    return NULL;
}

void* FigFileForkGetLengthAtOffset(void) {
    if (verbose) puts("STUB: FigFileForkGetLengthAtOffset called");
    return NULL;
}

void* FigFileForkOpenMainByCFURL(void) {
    if (verbose) puts("STUB: FigFileForkOpenMainByCFURL called");
    return NULL;
}

void* FigFileForkOpenMainByPath(void) {
    if (verbose) puts("STUB: FigFileForkOpenMainByPath called");
    return NULL;
}

void* FigFileForkRead(void) {
    if (verbose) puts("STUB: FigFileForkRead called");
    return NULL;
}

void* FigFileForkReadWithIOPolicy(void) {
    if (verbose) puts("STUB: FigFileForkReadWithIOPolicy called");
    return NULL;
}

void* FigFileForkSynchronize(void) {
    if (verbose) puts("STUB: FigFileForkSynchronize called");
    return NULL;
}

void* FigFileForkTruncate(void) {
    if (verbose) puts("STUB: FigFileForkTruncate called");
    return NULL;
}

void* FigFileForkWrite(void) {
    if (verbose) puts("STUB: FigFileForkWrite called");
    return NULL;
}

void* FigFileForkWriteWithIOPolicy(void) {
    if (verbose) puts("STUB: FigFileForkWriteWithIOPolicy called");
    return NULL;
}

void* FigFileGetDarwinFileDesc(void) {
    if (verbose) puts("STUB: FigFileGetDarwinFileDesc called");
    return NULL;
}

void* FigFileGetDarwinPath(void) {
    if (verbose) puts("STUB: FigFileGetDarwinPath called");
    return NULL;
}

void* FigFileGetFileInfo(void) {
    if (verbose) puts("STUB: FigFileGetFileInfo called");
    return NULL;
}

void* FigFileGetStorageMediumForCFURL(void) {
    if (verbose) puts("STUB: FigFileGetStorageMediumForCFURL called");
    return NULL;
}

void* FigFileInfoEqual(void) {
    if (verbose) puts("STUB: FigFileInfoEqual called");
    return NULL;
}

void* FigFilePathCreateFromChild(void) {
    if (verbose) puts("STUB: FigFilePathCreateFromChild called");
    return NULL;
}

void* FigFilePathCreateFromDarwinPath(void) {
    if (verbose) puts("STUB: FigFilePathCreateFromDarwinPath called");
    return NULL;
}

void* FigFilePathCreateFromNativePath(void) {
    if (verbose) puts("STUB: FigFilePathCreateFromNativePath called");
    return NULL;
}

void* FigFilePathCreateFromParent(void) {
    if (verbose) puts("STUB: FigFilePathCreateFromParent called");
    return NULL;
}

void* FigFilePathGetTypeID(void) {
    if (verbose) puts("STUB: FigFilePathGetTypeID called");
    return NULL;
}

void* FigFileReadURLAndCreateCFData(void) {
    if (verbose) puts("STUB: FigFileReadURLAndCreateCFData called");
    return NULL;
}

void* FigFileWrapDarwinFileDesc(void) {
    if (verbose) puts("STUB: FigFileWrapDarwinFileDesc called");
    return NULL;
}

void* FigFloat16ConvertToFloat32(void) {
    if (verbose) puts("STUB: FigFloat16ConvertToFloat32 called");
    return NULL;
}

void* FigFloat32ConvertToFloat16(void) {
    if (verbose) puts("STUB: FigFloat32ConvertToFloat16 called");
    return NULL;
}

void* FigFormatDescriptionCopyAsBigEndianClosedCaptionDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: FigFormatDescriptionCopyAsBigEndianClosedCaptionDescriptionBlockBuffer called");
    return NULL;
}

void* FigFormatDescriptionCopyAsBigEndianMetadataDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: FigFormatDescriptionCopyAsBigEndianMetadataDescriptionBlockBuffer called");
    return NULL;
}

void* FigFormatDescriptionCopyAsBigEndianTextDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: FigFormatDescriptionCopyAsBigEndianTextDescriptionBlockBuffer called");
    return NULL;
}

void* FigFormatDescriptionCreateFromBigEndianClosedCaptionDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: FigFormatDescriptionCreateFromBigEndianClosedCaptionDescriptionBlockBuffer called");
    return NULL;
}

void* FigFormatDescriptionCreateFromBigEndianClosedCaptionDescriptionData(void) {
    if (verbose) puts("STUB: FigFormatDescriptionCreateFromBigEndianClosedCaptionDescriptionData called");
    return NULL;
}

void* FigFormatDescriptionCreateFromBigEndianMetadataDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: FigFormatDescriptionCreateFromBigEndianMetadataDescriptionBlockBuffer called");
    return NULL;
}

void* FigFormatDescriptionCreateFromBigEndianMetadataDescriptionData(void) {
    if (verbose) puts("STUB: FigFormatDescriptionCreateFromBigEndianMetadataDescriptionData called");
    return NULL;
}

void* FigFormatDescriptionCreateFromBigEndianTextDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: FigFormatDescriptionCreateFromBigEndianTextDescriptionBlockBuffer called");
    return NULL;
}

void* FigFormatDescriptionCreateFromBigEndianTextDescriptionData(void) {
    if (verbose) puts("STUB: FigFormatDescriptionCreateFromBigEndianTextDescriptionData called");
    return NULL;
}

void* FigFormatDescriptionExtensionsDictionariesEqualIgnoringKeys(void) {
    if (verbose) puts("STUB: FigFormatDescriptionExtensionsDictionariesEqualIgnoringKeys called");
    return NULL;
}

void* FigFormatDescriptionRelease(void) {
    if (verbose) puts("STUB: FigFormatDescriptionRelease called");
    return NULL;
}

void* FigFormatDescriptionRetain(void) {
    if (verbose) puts("STUB: FigFormatDescriptionRetain called");
    return NULL;
}

void* FigFoundationCreateHardLink(void) {
    if (verbose) puts("STUB: FigFoundationCreateHardLink called");
    return NULL;
}

void* FigGeometryAspectRatioCopyAsDictionary(void) {
    if (verbose) puts("STUB: FigGeometryAspectRatioCopyAsDictionary called");
    return NULL;
}

void* FigGeometryAspectRatioCopyDescription(void) {
    if (verbose) puts("STUB: FigGeometryAspectRatioCopyDescription called");
    return NULL;
}

void* FigGeometryAspectRatioEqualToAspectRatio(void) {
    if (verbose) puts("STUB: FigGeometryAspectRatioEqualToAspectRatio called");
    return NULL;
}

void* FigGeometryAspectRatioHash(void) {
    if (verbose) puts("STUB: FigGeometryAspectRatioHash called");
    return NULL;
}

void* FigGeometryAspectRatioIsValid(void) {
    if (verbose) puts("STUB: FigGeometryAspectRatioIsValid called");
    return NULL;
}

void* FigGeometryAspectRatioMake(void) {
    if (verbose) puts("STUB: FigGeometryAspectRatioMake called");
    return NULL;
}

void* FigGeometryAspectRatioMakeFromDictionary(void) {
    if (verbose) puts("STUB: FigGeometryAspectRatioMakeFromDictionary called");
    return NULL;
}

void* FigGeometryCoordinateGravityTypeCopyAsCFType(void) {
    if (verbose) puts("STUB: FigGeometryCoordinateGravityTypeCopyAsCFType called");
    return NULL;
}

void* FigGeometryCoordinateGravityTypeCopyDescription(void) {
    if (verbose) puts("STUB: FigGeometryCoordinateGravityTypeCopyDescription called");
    return NULL;
}

void* FigGeometryCoordinateGravityTypeMakeFromCFType(void) {
    if (verbose) puts("STUB: FigGeometryCoordinateGravityTypeMakeFromCFType called");
    return NULL;
}

void* FigGeometryCoordinateSpaceCopyAsDictionary(void) {
    if (verbose) puts("STUB: FigGeometryCoordinateSpaceCopyAsDictionary called");
    return NULL;
}

void* FigGeometryCoordinateSpaceCreate(void) {
    if (verbose) puts("STUB: FigGeometryCoordinateSpaceCreate called");
    return NULL;
}

void* FigGeometryCoordinateSpaceGetTypeID(void) {
    if (verbose) puts("STUB: FigGeometryCoordinateSpaceGetTypeID called");
    return NULL;
}

void* FigGeometryCoordinateSpaceMakeFromDictionary(void) {
    if (verbose) puts("STUB: FigGeometryCoordinateSpaceMakeFromDictionary called");
    return NULL;
}

void* FigGeometryDimensionCopyAsDictionary(void) {
    if (verbose) puts("STUB: FigGeometryDimensionCopyAsDictionary called");
    return NULL;
}

void* FigGeometryDimensionCopyDescription(void) {
    if (verbose) puts("STUB: FigGeometryDimensionCopyDescription called");
    return NULL;
}

void* FigGeometryDimensionEqualToDimension(void) {
    if (verbose) puts("STUB: FigGeometryDimensionEqualToDimension called");
    return NULL;
}

void* FigGeometryDimensionGetCGFloat(void) {
    if (verbose) puts("STUB: FigGeometryDimensionGetCGFloat called");
    return NULL;
}

void* FigGeometryDimensionHash(void) {
    if (verbose) puts("STUB: FigGeometryDimensionHash called");
    return NULL;
}

void* FigGeometryDimensionMake(void) {
    if (verbose) puts("STUB: FigGeometryDimensionMake called");
    return NULL;
}

void* FigGeometryDimensionMakeFromDictionary(void) {
    if (verbose) puts("STUB: FigGeometryDimensionMakeFromDictionary called");
    return NULL;
}

void* FigGeometryDimensionMakeFromDimensionAndReferenceUnits(void) {
    if (verbose) puts("STUB: FigGeometryDimensionMakeFromDimensionAndReferenceUnits called");
    return NULL;
}

void* FigGeometryDimensionMakeZeroedDimension(void) {
    if (verbose) puts("STUB: FigGeometryDimensionMakeZeroedDimension called");
    return NULL;
}

void* FigGeometryMappingConvertDimensionToDimension(void) {
    if (verbose) puts("STUB: FigGeometryMappingConvertDimensionToDimension called");
    return NULL;
}

void* FigGeometryMappingConvertPointToPoint(void) {
    if (verbose) puts("STUB: FigGeometryMappingConvertPointToPoint called");
    return NULL;
}

void* FigGeometryMappingConvertRectToRect(void) {
    if (verbose) puts("STUB: FigGeometryMappingConvertRectToRect called");
    return NULL;
}

void* FigGeometryMappingConvertSizeToSize(void) {
    if (verbose) puts("STUB: FigGeometryMappingConvertSizeToSize called");
    return NULL;
}

void* FigGeometryMappingConvertVectorToVector(void) {
    if (verbose) puts("STUB: FigGeometryMappingConvertVectorToVector called");
    return NULL;
}

void* FigGeometryMappingCreate(void) {
    if (verbose) puts("STUB: FigGeometryMappingCreate called");
    return NULL;
}

void* FigGeometryMappingCreateCEA608Mappings(void) {
    if (verbose) puts("STUB: FigGeometryMappingCreateCEA608Mappings called");
    return NULL;
}

void* FigGeometryMappingCreateInverseMapping(void) {
    if (verbose) puts("STUB: FigGeometryMappingCreateInverseMapping called");
    return NULL;
}

void* FigGeometryMappingGetDestinationUnits(void) {
    if (verbose) puts("STUB: FigGeometryMappingGetDestinationUnits called");
    return NULL;
}

void* FigGeometryMappingGetSourceUnits(void) {
    if (verbose) puts("STUB: FigGeometryMappingGetSourceUnits called");
    return NULL;
}

void* FigGeometryMappingGetTypeID(void) {
    if (verbose) puts("STUB: FigGeometryMappingGetTypeID called");
    return NULL;
}

void* FigGeometryMarginsCopyAsDictionary(void) {
    if (verbose) puts("STUB: FigGeometryMarginsCopyAsDictionary called");
    return NULL;
}

void* FigGeometryMarginsCopyDescription(void) {
    if (verbose) puts("STUB: FigGeometryMarginsCopyDescription called");
    return NULL;
}

void* FigGeometryMarginsEqualToMargins(void) {
    if (verbose) puts("STUB: FigGeometryMarginsEqualToMargins called");
    return NULL;
}

void* FigGeometryMarginsHash(void) {
    if (verbose) puts("STUB: FigGeometryMarginsHash called");
    return NULL;
}

void* FigGeometryMarginsMake(void) {
    if (verbose) puts("STUB: FigGeometryMarginsMake called");
    return NULL;
}

void* FigGeometryMarginsMakeFromDictionary(void) {
    if (verbose) puts("STUB: FigGeometryMarginsMakeFromDictionary called");
    return NULL;
}

void* FigGeometryPointCopyAsDictionary(void) {
    if (verbose) puts("STUB: FigGeometryPointCopyAsDictionary called");
    return NULL;
}

void* FigGeometryPointCopyDescription(void) {
    if (verbose) puts("STUB: FigGeometryPointCopyDescription called");
    return NULL;
}

void* FigGeometryPointEqualToPoint(void) {
    if (verbose) puts("STUB: FigGeometryPointEqualToPoint called");
    return NULL;
}

void* FigGeometryPointGetCGPoint(void) {
    if (verbose) puts("STUB: FigGeometryPointGetCGPoint called");
    return NULL;
}

void* FigGeometryPointHash(void) {
    if (verbose) puts("STUB: FigGeometryPointHash called");
    return NULL;
}

void* FigGeometryPointMake(void) {
    if (verbose) puts("STUB: FigGeometryPointMake called");
    return NULL;
}

void* FigGeometryPointMakeFromCGPointAndReferenceUnits(void) {
    if (verbose) puts("STUB: FigGeometryPointMakeFromCGPointAndReferenceUnits called");
    return NULL;
}

void* FigGeometryPointMakeFromDictionary(void) {
    if (verbose) puts("STUB: FigGeometryPointMakeFromDictionary called");
    return NULL;
}

void* FigGeometryPointMakeFromPointAndReferenceUnits(void) {
    if (verbose) puts("STUB: FigGeometryPointMakeFromPointAndReferenceUnits called");
    return NULL;
}

void* FigGeometryPointMakeZeroedPoint(void) {
    if (verbose) puts("STUB: FigGeometryPointMakeZeroedPoint called");
    return NULL;
}

void* FigGeometryRectCopyAsDictionary(void) {
    if (verbose) puts("STUB: FigGeometryRectCopyAsDictionary called");
    return NULL;
}

void* FigGeometryRectCopyDescription(void) {
    if (verbose) puts("STUB: FigGeometryRectCopyDescription called");
    return NULL;
}

void* FigGeometryRectEqualToRect(void) {
    if (verbose) puts("STUB: FigGeometryRectEqualToRect called");
    return NULL;
}

void* FigGeometryRectGetCGRect(void) {
    if (verbose) puts("STUB: FigGeometryRectGetCGRect called");
    return NULL;
}

void* FigGeometryRectHash(void) {
    if (verbose) puts("STUB: FigGeometryRectHash called");
    return NULL;
}

void* FigGeometryRectMake(void) {
    if (verbose) puts("STUB: FigGeometryRectMake called");
    return NULL;
}

void* FigGeometryRectMakeFromCGRectAndReferenceUnits(void) {
    if (verbose) puts("STUB: FigGeometryRectMakeFromCGRectAndReferenceUnits called");
    return NULL;
}

void* FigGeometryRectMakeFromDictionary(void) {
    if (verbose) puts("STUB: FigGeometryRectMakeFromDictionary called");
    return NULL;
}

void* FigGeometryRectMakeFromRectAndReferenceUnits(void) {
    if (verbose) puts("STUB: FigGeometryRectMakeFromRectAndReferenceUnits called");
    return NULL;
}

void* FigGeometryRectMakeZeroedRect(void) {
    if (verbose) puts("STUB: FigGeometryRectMakeZeroedRect called");
    return NULL;
}

void* FigGeometrySizeCopyAsDictionary(void) {
    if (verbose) puts("STUB: FigGeometrySizeCopyAsDictionary called");
    return NULL;
}

void* FigGeometrySizeCopyDescription(void) {
    if (verbose) puts("STUB: FigGeometrySizeCopyDescription called");
    return NULL;
}

void* FigGeometrySizeEqualToSize(void) {
    if (verbose) puts("STUB: FigGeometrySizeEqualToSize called");
    return NULL;
}

void* FigGeometrySizeGetCGSize(void) {
    if (verbose) puts("STUB: FigGeometrySizeGetCGSize called");
    return NULL;
}

void* FigGeometrySizeHash(void) {
    if (verbose) puts("STUB: FigGeometrySizeHash called");
    return NULL;
}

void* FigGeometrySizeMake(void) {
    if (verbose) puts("STUB: FigGeometrySizeMake called");
    return NULL;
}

void* FigGeometrySizeMakeFromCGSizeAndReferenceUnits(void) {
    if (verbose) puts("STUB: FigGeometrySizeMakeFromCGSizeAndReferenceUnits called");
    return NULL;
}

void* FigGeometrySizeMakeFromDictionary(void) {
    if (verbose) puts("STUB: FigGeometrySizeMakeFromDictionary called");
    return NULL;
}

void* FigGeometrySizeMakeFromSizeAndReferenceUnits(void) {
    if (verbose) puts("STUB: FigGeometrySizeMakeFromSizeAndReferenceUnits called");
    return NULL;
}

void* FigGeometrySizeMakeZeroedSize(void) {
    if (verbose) puts("STUB: FigGeometrySizeMakeZeroedSize called");
    return NULL;
}

void* FigGeometryVectorCopyAsDictionary(void) {
    if (verbose) puts("STUB: FigGeometryVectorCopyAsDictionary called");
    return NULL;
}

void* FigGeometryVectorCopyDescription(void) {
    if (verbose) puts("STUB: FigGeometryVectorCopyDescription called");
    return NULL;
}

void* FigGeometryVectorEqualToVector(void) {
    if (verbose) puts("STUB: FigGeometryVectorEqualToVector called");
    return NULL;
}

void* FigGeometryVectorGetCGVector(void) {
    if (verbose) puts("STUB: FigGeometryVectorGetCGVector called");
    return NULL;
}

void* FigGeometryVectorHash(void) {
    if (verbose) puts("STUB: FigGeometryVectorHash called");
    return NULL;
}

void* FigGeometryVectorMake(void) {
    if (verbose) puts("STUB: FigGeometryVectorMake called");
    return NULL;
}

void* FigGeometryVectorMakeFromCGVectorAndReferenceUnits(void) {
    if (verbose) puts("STUB: FigGeometryVectorMakeFromCGVectorAndReferenceUnits called");
    return NULL;
}

void* FigGeometryVectorMakeFromDictionary(void) {
    if (verbose) puts("STUB: FigGeometryVectorMakeFromDictionary called");
    return NULL;
}

void* FigGeometryVectorMakeFromVectorAndReferenceUnits(void) {
    if (verbose) puts("STUB: FigGeometryVectorMakeFromVectorAndReferenceUnits called");
    return NULL;
}

void* FigGeometryVectorMakeZeroedVector(void) {
    if (verbose) puts("STUB: FigGeometryVectorMakeZeroedVector called");
    return NULL;
}

void* FigGetBaseByteStreamProvider(void) {
    if (verbose) puts("STUB: FigGetBaseByteStreamProvider called");
    return NULL;
}

void* FigGetCFPreferenceDoubleWithDefault(void) {
    if (verbose) puts("STUB: FigGetCFPreferenceDoubleWithDefault called");
    return NULL;
}

void* FigGetCFPreferenceNumberWithDefault(void) {
    if (verbose) puts("STUB: FigGetCFPreferenceNumberWithDefault called");
    return NULL;
}

void* FigGetCFPreferenceOrOptionDoubleWithDefault(void) {
    if (verbose) puts("STUB: FigGetCFPreferenceOrOptionDoubleWithDefault called");
    return NULL;
}

void* FigGetCFPreferenceOrOptionNumberWithDefault(void) {
    if (verbose) puts("STUB: FigGetCFPreferenceOrOptionNumberWithDefault called");
    return NULL;
}

void* FigGetCFStringForOSTypeValue(void) {
    if (verbose) puts("STUB: FigGetCFStringForOSTypeValue called");
    return NULL;
}

void* FigGetDefaultManagedFilePool(void) {
    if (verbose) puts("STUB: FigGetDefaultManagedFilePool called");
    return NULL;
}

void* FigGetDoubleFromCFTypeWithDefault(void) {
    if (verbose) puts("STUB: FigGetDoubleFromCFTypeWithDefault called");
    return NULL;
}

void* FigGetErrorCount(void) {
    if (verbose) puts("STUB: FigGetErrorCount called");
    return NULL;
}

void* FigGetGlobalCFAllocatorFigMalloc(void) {
    if (verbose) puts("STUB: FigGetGlobalCFAllocatorFigMalloc called");
    return NULL;
}

void* FigGetHostTimeToNanosecondsScale(void) {
    if (verbose) puts("STUB: FigGetHostTimeToNanosecondsScale called");
    return NULL;
}

void* FigGetNumberFromCFTypeWithDefault(void) {
    if (verbose) puts("STUB: FigGetNumberFromCFTypeWithDefault called");
    return NULL;
}

void* FigGetTimeOfDayClock(void) {
    if (verbose) puts("STUB: FigGetTimeOfDayClock called");
    return NULL;
}

void* FigGetUpTime(void) {
    if (verbose) puts("STUB: FigGetUpTime called");
    return NULL;
}

void* FigGetUpTimeNanoseconds(void) {
    if (verbose) puts("STUB: FigGetUpTimeNanoseconds called");
    return NULL;
}

void* FigGzipCompressMemory(void) {
    if (verbose) puts("STUB: FigGzipCompressMemory called");
    return NULL;
}

void* FigGzipCreate(void) {
    if (verbose) puts("STUB: FigGzipCreate called");
    return NULL;
}

void* FigGzipDecompressMemory(void) {
    if (verbose) puts("STUB: FigGzipDecompressMemory called");
    return NULL;
}

void* FigGzipDispose(void) {
    if (verbose) puts("STUB: FigGzipDispose called");
    return NULL;
}

void* FigGzipReset(void) {
    if (verbose) puts("STUB: FigGzipReset called");
    return NULL;
}

void* FigH264Bridge_CopyDataWithEmulationPreventionBytesRemoved(void) {
    if (verbose) puts("STUB: FigH264Bridge_CopyDataWithEmulationPreventionBytesRemoved called");
    return NULL;
}

void* FigH264Bridge_CreateHLSfMP4ParsingInfoFromAVCCData(void) {
    if (verbose) puts("STUB: FigH264Bridge_CreateHLSfMP4ParsingInfoFromAVCCData called");
    return NULL;
}

void* FigH264Bridge_GetH264ParameterSetAtIndex(void) {
    if (verbose) puts("STUB: FigH264Bridge_GetH264ParameterSetAtIndex called");
    return NULL;
}

void* FigH264Bridge_GetInterlaceTypeFromSlice(void) {
    if (verbose) puts("STUB: FigH264Bridge_GetInterlaceTypeFromSlice called");
    return NULL;
}

void* FigH264Bridge_GetNALUnitHeaderLengthFromAVCC(void) {
    if (verbose) puts("STUB: FigH264Bridge_GetNALUnitHeaderLengthFromAVCC called");
    return NULL;
}

void* FigH264Bridge_GetPPSFromAVCC(void) {
    if (verbose) puts("STUB: FigH264Bridge_GetPPSFromAVCC called");
    return NULL;
}

void* FigH264Bridge_GetPPSSequenceID(void) {
    if (verbose) puts("STUB: FigH264Bridge_GetPPSSequenceID called");
    return NULL;
}

void* FigH264Bridge_GetPPSSequenceIDFromSlice(void) {
    if (verbose) puts("STUB: FigH264Bridge_GetPPSSequenceIDFromSlice called");
    return NULL;
}

void* FigH264Bridge_GetSPSAndPPSCountFromAVCC(void) {
    if (verbose) puts("STUB: FigH264Bridge_GetSPSAndPPSCountFromAVCC called");
    return NULL;
}

void* FigH264Bridge_GetSPSChromaFormatAndBitDepths(void) {
    if (verbose) puts("STUB: FigH264Bridge_GetSPSChromaFormatAndBitDepths called");
    return NULL;
}

void* FigH264Bridge_GetSPSFromAVCC(void) {
    if (verbose) puts("STUB: FigH264Bridge_GetSPSFromAVCC called");
    return NULL;
}

void* FigH264Bridge_GetSPSIsInterlaced(void) {
    if (verbose) puts("STUB: FigH264Bridge_GetSPSIsInterlaced called");
    return NULL;
}

void* FigH264Bridge_GetSPSSequenceID(void) {
    if (verbose) puts("STUB: FigH264Bridge_GetSPSSequenceID called");
    return NULL;
}

void* FigH264Bridge_GetSPSWidthAndHeight(void) {
    if (verbose) puts("STUB: FigH264Bridge_GetSPSWidthAndHeight called");
    return NULL;
}

void* FigH264Bridge_GetSPS_VUI_Framerate(void) {
    if (verbose) puts("STUB: FigH264Bridge_GetSPS_VUI_Framerate called");
    return NULL;
}

void* FigH264Bridge_GetSliceInformationFromSampleBuffer(void) {
    if (verbose) puts("STUB: FigH264Bridge_GetSliceInformationFromSampleBuffer called");
    return NULL;
}

void* FigH264Bridge_HLSfMP4ParsingInfoDestroy(void) {
    if (verbose) puts("STUB: FigH264Bridge_HLSfMP4ParsingInfoDestroy called");
    return NULL;
}

void* FigH264Bridge_LocateSliceHeaderForHLSfMP4EncryptableNAL(void) {
    if (verbose) puts("STUB: FigH264Bridge_LocateSliceHeaderForHLSfMP4EncryptableNAL called");
    return NULL;
}

void* FigH264Bridge_MeasureSliceHeader(void) {
    if (verbose) puts("STUB: FigH264Bridge_MeasureSliceHeader called");
    return NULL;
}

void* FigHALAudioConfigChangeCloneRecord(void) {
    if (verbose) puts("STUB: FigHALAudioConfigChangeCloneRecord called");
    return NULL;
}

void* FigHALAudioConfigChangeCreateRecord(void) {
    if (verbose) puts("STUB: FigHALAudioConfigChangeCreateRecord called");
    return NULL;
}

void* FigHALAudioConfigChangeDisposeRecord(void) {
    if (verbose) puts("STUB: FigHALAudioConfigChangeDisposeRecord called");
    return NULL;
}

void* FigHALAudioConfigChangeSendRequest(void) {
    if (verbose) puts("STUB: FigHALAudioConfigChangeSendRequest called");
    return NULL;
}

void* FigHALAudioConfigChangeSendRequestWithCustomChangeRecord(void) {
    if (verbose) puts("STUB: FigHALAudioConfigChangeSendRequestWithCustomChangeRecord called");
    return NULL;
}

void* FigHALAudioCreateStandardPluginObject(void) {
    if (verbose) puts("STUB: FigHALAudioCreateStandardPluginObject called");
    return NULL;
}

void* FigHALAudioCreateStandardPluginObjectWithSpecialInstantiator(void) {
    if (verbose) puts("STUB: FigHALAudioCreateStandardPluginObjectWithSpecialInstantiator called");
    return NULL;
}

void* FigHALAudioDeviceGetClassID(void) {
    if (verbose) puts("STUB: FigHALAudioDeviceGetClassID called");
    return NULL;
}

void* FigHALAudioObjectGetClassID(void) {
    if (verbose) puts("STUB: FigHALAudioObjectGetClassID called");
    return NULL;
}

void* FigHALAudioObjectMapperAddMapping(void) {
    if (verbose) puts("STUB: FigHALAudioObjectMapperAddMapping called");
    return NULL;
}

void* FigHALAudioObjectMapperAddPluginObjectMapping(void) {
    if (verbose) puts("STUB: FigHALAudioObjectMapperAddPluginObjectMapping called");
    return NULL;
}

void* FigHALAudioObjectMapperCopyObjectForID(void) {
    if (verbose) puts("STUB: FigHALAudioObjectMapperCopyObjectForID called");
    return NULL;
}

void* FigHALAudioObjectMapperCreate(void) {
    if (verbose) puts("STUB: FigHALAudioObjectMapperCreate called");
    return NULL;
}

void* FigHALAudioObjectMapperDispose(void) {
    if (verbose) puts("STUB: FigHALAudioObjectMapperDispose called");
    return NULL;
}

void* FigHALAudioObjectMapperRemoveMapping(void) {
    if (verbose) puts("STUB: FigHALAudioObjectMapperRemoveMapping called");
    return NULL;
}

void* FigHALAudioPluginGetClassID(void) {
    if (verbose) puts("STUB: FigHALAudioPluginGetClassID called");
    return NULL;
}

void* FigHALAudioPropertySendChanges(void) {
    if (verbose) puts("STUB: FigHALAudioPropertySendChanges called");
    return NULL;
}

void* FigHALAudioPropertySendOneChange(void) {
    if (verbose) puts("STUB: FigHALAudioPropertySendOneChange called");
    return NULL;
}

void* FigHALAudioPropertySendOneChangeAsync(void) {
    if (verbose) puts("STUB: FigHALAudioPropertySendOneChangeAsync called");
    return NULL;
}

void* FigHALAudioStreamGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigHALAudioStreamGetCMBaseObject called");
    return NULL;
}

void* FigHALAudioStreamGetClassID(void) {
    if (verbose) puts("STUB: FigHALAudioStreamGetClassID called");
    return NULL;
}

void* FigHALPluginDriverGetInterface(void) {
    if (verbose) puts("STUB: FigHALPluginDriverGetInterface called");
    return NULL;
}

void* FigHEVCBridge_CopyHEVCSEIPayloadData(void) {
    if (verbose) puts("STUB: FigHEVCBridge_CopyHEVCSEIPayloadData called");
    return NULL;
}

void* FigHEVCBridge_CreateHLSfMP4ParsingInfoFromHVCCData(void) {
    if (verbose) puts("STUB: FigHEVCBridge_CreateHLSfMP4ParsingInfoFromHVCCData called");
    return NULL;
}

void* FigHEVCBridge_CreateHVCCFromHEVCParameterSets(void) {
    if (verbose) puts("STUB: FigHEVCBridge_CreateHVCCFromHEVCParameterSets called");
    return NULL;
}

void* FigHEVCBridge_CreateUpdatedHVCCWithAdditionalHEVCParameterSets(void) {
    if (verbose) puts("STUB: FigHEVCBridge_CreateUpdatedHVCCWithAdditionalHEVCParameterSets called");
    return NULL;
}

void* FigHEVCBridge_GetHEVCParameterSetAtIndex(void) {
    if (verbose) puts("STUB: FigHEVCBridge_GetHEVCParameterSetAtIndex called");
    return NULL;
}

void* FigHEVCBridge_GetNALUnitHeader(void) {
    if (verbose) puts("STUB: FigHEVCBridge_GetNALUnitHeader called");
    return NULL;
}

void* FigHEVCBridge_GetNALUnitHeaderLengthFromHVCC(void) {
    if (verbose) puts("STUB: FigHEVCBridge_GetNALUnitHeaderLengthFromHVCC called");
    return NULL;
}

void* FigHEVCBridge_GetSPSBitDepths(void) {
    if (verbose) puts("STUB: FigHEVCBridge_GetSPSBitDepths called");
    return NULL;
}

void* FigHEVCBridge_GetSPSChromaFormatAndBitDepths(void) {
    if (verbose) puts("STUB: FigHEVCBridge_GetSPSChromaFormatAndBitDepths called");
    return NULL;
}

void* FigHEVCBridge_GetSPSIsInterlaced(void) {
    if (verbose) puts("STUB: FigHEVCBridge_GetSPSIsInterlaced called");
    return NULL;
}

void* FigHEVCBridge_GetSPSIsScalingListPresent(void) {
    if (verbose) puts("STUB: FigHEVCBridge_GetSPSIsScalingListPresent called");
    return NULL;
}

void* FigHEVCBridge_GetSPSProfileTierLevel(void) {
    if (verbose) puts("STUB: FigHEVCBridge_GetSPSProfileTierLevel called");
    return NULL;
}

void* FigHEVCBridge_HLSfMP4ParsingInfoDestroy(void) {
    if (verbose) puts("STUB: FigHEVCBridge_HLSfMP4ParsingInfoDestroy called");
    return NULL;
}

void* FigHEVCBridge_LocateSliceHeaderForHLSfMP4EncryptableNAL(void) {
    if (verbose) puts("STUB: FigHEVCBridge_LocateSliceHeaderForHLSfMP4EncryptableNAL called");
    return NULL;
}

void* FigHEVCBridge_MeasureSliceHeader(void) {
    if (verbose) puts("STUB: FigHEVCBridge_MeasureSliceHeader called");
    return NULL;
}

void* FigHostTimeToNanoseconds(void) {
    if (verbose) puts("STUB: FigHostTimeToNanoseconds called");
    return NULL;
}

void* FigInt64MulDiv(void) {
    if (verbose) puts("STUB: FigInt64MulDiv called");
    return NULL;
}

void* FigIsItOKToLogURLs(void) {
    if (verbose) puts("STUB: FigIsItOKToLogURLs called");
    return NULL;
}

void* FigIsReferenceAllowedBySecurityPolicy(void) {
    if (verbose) puts("STUB: FigIsReferenceAllowedBySecurityPolicy called");
    return NULL;
}

void* FigJSONObjectCreateWithData(void) {
    if (verbose) puts("STUB: FigJSONObjectCreateWithData called");
    return NULL;
}

void* FigKTraceInit(void) {
    if (verbose) puts("STUB: FigKTraceInit called");
    return NULL;
}

void* FigLivePhotoMetadataComputeDeserializationSize(void) {
    if (verbose) puts("STUB: FigLivePhotoMetadataComputeDeserializationSize called");
    return NULL;
}

void* FigLivePhotoMetadataComputeSerializationSize(void) {
    if (verbose) puts("STUB: FigLivePhotoMetadataComputeSerializationSize called");
    return NULL;
}

void* FigLivePhotoMetadataDeserializeIntoBuffer(void) {
    if (verbose) puts("STUB: FigLivePhotoMetadataDeserializeIntoBuffer called");
    return NULL;
}

void* FigLogBacktrace(void) {
    if (verbose) puts("STUB: FigLogBacktrace called");
    return NULL;
}

void* FigLogBacktraceFrames(void) {
    if (verbose) puts("STUB: FigLogBacktraceFrames called");
    return NULL;
}

void* FigMachPortReleaseReceiveRight_(void) {
    if (verbose) puts("STUB: FigMachPortReleaseReceiveRight_ called");
    return NULL;
}

void* FigMachPortReleaseSendRight_(void) {
    if (verbose) puts("STUB: FigMachPortReleaseSendRight_ called");
    return NULL;
}

void* FigMakeIOVecArrayWithBlockBuffer(void) {
    if (verbose) puts("STUB: FigMakeIOVecArrayWithBlockBuffer called");
    return NULL;
}

void* FigManagedFilePoolCreate(void) {
    if (verbose) puts("STUB: FigManagedFilePoolCreate called");
    return NULL;
}

void* FigManagedFilePoolCreateByteStreamForFile(void) {
    if (verbose) puts("STUB: FigManagedFilePoolCreateByteStreamForFile called");
    return NULL;
}

void* FigManagedFilePoolGetTypeID(void) {
    if (verbose) puts("STUB: FigManagedFilePoolGetTypeID called");
    return NULL;
}

void* FigMemoryBarrier(void) {
    if (verbose) puts("STUB: FigMemoryBarrier called");
    return NULL;
}

void* FigMetadataAppendMEBXKeyTableBox(void) {
    if (verbose) puts("STUB: FigMetadataAppendMEBXKeyTableBox called");
    return NULL;
}

void* FigMetadataAppendMEBXLocalIDDependencyListBoxes(void) {
    if (verbose) puts("STUB: FigMetadataAppendMEBXLocalIDDependencyListBoxes called");
    return NULL;
}

void* FigMetadataAppendMEBXPrimaryLocalIDsBox(void) {
    if (verbose) puts("STUB: FigMetadataAppendMEBXPrimaryLocalIDsBox called");
    return NULL;
}

void* FigMetadataCreateIdentifierForKeyAndOSTypeKeySpace(void) {
    if (verbose) puts("STUB: FigMetadataCreateIdentifierForKeyAndOSTypeKeySpace called");
    return NULL;
}

void* FigMetadataFormatDescriptionCreateWithExtensions(void) {
    if (verbose) puts("STUB: FigMetadataFormatDescriptionCreateWithExtensions called");
    return NULL;
}

void* FigMetadataFormatDescriptionCreateWithKeysAndDependencies(void) {
    if (verbose) puts("STUB: FigMetadataFormatDescriptionCreateWithKeysAndDependencies called");
    return NULL;
}

void* FigMetadataFormatDescriptionGetConformingDataTypesForLocalID(void) {
    if (verbose) puts("STUB: FigMetadataFormatDescriptionGetConformingDataTypesForLocalID called");
    return NULL;
}

void* FigMetadataFormatDescriptionGetDataTypeForLocalID(void) {
    if (verbose) puts("STUB: FigMetadataFormatDescriptionGetDataTypeForLocalID called");
    return NULL;
}

void* FigMetadataFormatDescriptionGetFirstLocalIDForIdentifier(void) {
    if (verbose) puts("STUB: FigMetadataFormatDescriptionGetFirstLocalIDForIdentifier called");
    return NULL;
}

void* FigMetadataFormatDescriptionGetIdentifierForLocalID(void) {
    if (verbose) puts("STUB: FigMetadataFormatDescriptionGetIdentifierForLocalID called");
    return NULL;
}

void* FigMetadataFormatDescriptionGetLanguageTagForLocalID(void) {
    if (verbose) puts("STUB: FigMetadataFormatDescriptionGetLanguageTagForLocalID called");
    return NULL;
}

void* FigMetadataFormatDescriptionGetLocalIDForMetadataIdentifyingFactors(void) {
    if (verbose) puts("STUB: FigMetadataFormatDescriptionGetLocalIDForMetadataIdentifyingFactors called");
    return NULL;
}

void* FigMetadataFormatDescriptionGetQuickTimeWellKnownTypeForLocalID(void) {
    if (verbose) puts("STUB: FigMetadataFormatDescriptionGetQuickTimeWellKnownTypeForLocalID called");
    return NULL;
}

void* FigMetadataFormatDescriptionGetSetupDataForLocalID(void) {
    if (verbose) puts("STUB: FigMetadataFormatDescriptionGetSetupDataForLocalID called");
    return NULL;
}

void* FigMetadataFormatDescriptionQuickTimeWellKnownTypeIsRecognized(void) {
    if (verbose) puts("STUB: FigMetadataFormatDescriptionQuickTimeWellKnownTypeIsRecognized called");
    return NULL;
}

void* FigMetadataGetKeySpaceAsOSTypeFromIdentifier(void) {
    if (verbose) puts("STUB: FigMetadataGetKeySpaceAsOSTypeFromIdentifier called");
    return NULL;
}

void* FigMetadataKeyGetDatatypeNamespaceUInt32(void) {
    if (verbose) puts("STUB: FigMetadataKeyGetDatatypeNamespaceUInt32 called");
    return NULL;
}

void* FigMetadataKeyGetDatatypeUInt32(void) {
    if (verbose) puts("STUB: FigMetadataKeyGetDatatypeUInt32 called");
    return NULL;
}

void* FigMetadataKeyGetKeyNamespaceUInt32(void) {
    if (verbose) puts("STUB: FigMetadataKeyGetKeyNamespaceUInt32 called");
    return NULL;
}

void* FigMetadataKeyGetLocalIDUInt32(void) {
    if (verbose) puts("STUB: FigMetadataKeyGetLocalIDUInt32 called");
    return NULL;
}

void* FigMetadataKeyGetUInt32(void) {
    if (verbose) puts("STUB: FigMetadataKeyGetUInt32 called");
    return NULL;
}

void* FigMetadataUnboxMEBXKeyTableBox(void) {
    if (verbose) puts("STUB: FigMetadataUnboxMEBXKeyTableBox called");
    return NULL;
}

void* FigMetadataUnboxMEBXLocalIDDependencyListBoxes(void) {
    if (verbose) puts("STUB: FigMetadataUnboxMEBXLocalIDDependencyListBoxes called");
    return NULL;
}

void* FigMetadataUnboxMEBXPrimaryLocalIDsBox(void) {
    if (verbose) puts("STUB: FigMetadataUnboxMEBXPrimaryLocalIDsBox called");
    return NULL;
}

void* FigNanosecondsToHostTime(void) {
    if (verbose) puts("STUB: FigNanosecondsToHostTime called");
    return NULL;
}

void* FigNote_ChangeValue(void) {
    if (verbose) puts("STUB: FigNote_ChangeValue called");
    return NULL;
}

void* FigNote_CopyAndClearNoteBBuf(void) {
    if (verbose) puts("STUB: FigNote_CopyAndClearNoteBBuf called");
    return NULL;
}

void* FigNote_CopyKeys(void) {
    if (verbose) puts("STUB: FigNote_CopyKeys called");
    return NULL;
}

void* FigNote_FlushRunningLogAndCopyURLContainingLogs(void) {
    if (verbose) puts("STUB: FigNote_FlushRunningLogAndCopyURLContainingLogs called");
    return NULL;
}

void* FigNote_GetLogBuffer(void) {
    if (verbose) puts("STUB: FigNote_GetLogBuffer called");
    return NULL;
}

void* FigNote_GetValue(void) {
    if (verbose) puts("STUB: FigNote_GetValue called");
    return NULL;
}

void* FigNote_OneTimeInitializationForMediaServerd(void) {
    if (verbose) puts("STUB: FigNote_OneTimeInitializationForMediaServerd called");
    return NULL;
}

void* FigNote_RefreshControlPreferences(void) {
    if (verbose) puts("STUB: FigNote_RefreshControlPreferences called");
    return NULL;
}

void* FigNote_SetPreferences(void) {
    if (verbose) puts("STUB: FigNote_SetPreferences called");
    return NULL;
}

void* FigNotificationCenterAddListeners(void) {
    if (verbose) puts("STUB: FigNotificationCenterAddListeners called");
    return NULL;
}

void* FigNotificationCenterAddWeakListener(void) {
    if (verbose) puts("STUB: FigNotificationCenterAddWeakListener called");
    return NULL;
}

void* FigNotificationCenterAddWeakListeners(void) {
    if (verbose) puts("STUB: FigNotificationCenterAddWeakListeners called");
    return NULL;
}

void* FigNotificationCenterRemoveListeners(void) {
    if (verbose) puts("STUB: FigNotificationCenterRemoveListeners called");
    return NULL;
}

void* FigNotificationCenterRemoveWeakListener(void) {
    if (verbose) puts("STUB: FigNotificationCenterRemoveWeakListener called");
    return NULL;
}

void* FigNotificationCenterRemoveWeakListeners(void) {
    if (verbose) puts("STUB: FigNotificationCenterRemoveWeakListeners called");
    return NULL;
}

void* FigOSTransactionCreate(void) {
    if (verbose) puts("STUB: FigOSTransactionCreate called");
    return NULL;
}

void* FigPlaybackLogPowerEvent(void) {
    if (verbose) puts("STUB: FigPlaybackLogPowerEvent called");
    return NULL;
}

void* FigReadAheadByteStreamCreateForFile(void) {
    if (verbose) puts("STUB: FigReadAheadByteStreamCreateForFile called");
    return NULL;
}

void* FigReadAheadByteStreamCreateForFileURL(void) {
    if (verbose) puts("STUB: FigReadAheadByteStreamCreateForFileURL called");
    return NULL;
}

void* FigReadOnlyTimebaseSetTargetTimebase(void) {
    if (verbose) puts("STUB: FigReadOnlyTimebaseSetTargetTimebase called");
    return NULL;
}

void* FigReadWriteLockCreate(void) {
    if (verbose) puts("STUB: FigReadWriteLockCreate called");
    return NULL;
}

void* FigReadWriteLockDestroy(void) {
    if (verbose) puts("STUB: FigReadWriteLockDestroy called");
    return NULL;
}

void* FigReadWriteLockLockForRead(void) {
    if (verbose) puts("STUB: FigReadWriteLockLockForRead called");
    return NULL;
}

void* FigReadWriteLockLockForWrite(void) {
    if (verbose) puts("STUB: FigReadWriteLockLockForWrite called");
    return NULL;
}

void* FigReadWriteLockUnlockForRead(void) {
    if (verbose) puts("STUB: FigReadWriteLockUnlockForRead called");
    return NULL;
}

void* FigReadWriteLockUnlockForWrite(void) {
    if (verbose) puts("STUB: FigReadWriteLockUnlockForWrite called");
    return NULL;
}

void* FigReadWriteQueue_Create(void) {
    if (verbose) puts("STUB: FigReadWriteQueue_Create called");
    return NULL;
}

void* FigReadWriteQueue_Read(void) {
    if (verbose) puts("STUB: FigReadWriteQueue_Read called");
    return NULL;
}

void* FigReadWriteQueue_Write(void) {
    if (verbose) puts("STUB: FigReadWriteQueue_Write called");
    return NULL;
}

void* FigReadWriteQueue_Write_Async(void) {
    if (verbose) puts("STUB: FigReadWriteQueue_Write_Async called");
    return NULL;
}

void* FigReentrantMutexCheckIsLockedOnThisThread(void) {
    if (verbose) puts("STUB: FigReentrantMutexCheckIsLockedOnThisThread called");
    return NULL;
}

void* FigReentrantMutexCheckIsNotLockedOnThisThread(void) {
    if (verbose) puts("STUB: FigReentrantMutexCheckIsNotLockedOnThisThread called");
    return NULL;
}

void* FigReentrantMutexCreate(void) {
    if (verbose) puts("STUB: FigReentrantMutexCreate called");
    return NULL;
}

void* FigReentrantMutexDestroy(void) {
    if (verbose) puts("STUB: FigReentrantMutexDestroy called");
    return NULL;
}

void* FigReentrantMutexLock(void) {
    if (verbose) puts("STUB: FigReentrantMutexLock called");
    return NULL;
}

void* FigReentrantMutexTryLock(void) {
    if (verbose) puts("STUB: FigReentrantMutexTryLock called");
    return NULL;
}

void* FigReentrantMutexUnlock(void) {
    if (verbose) puts("STUB: FigReentrantMutexUnlock called");
    return NULL;
}

void* FigRegistryAddItem(void) {
    if (verbose) puts("STUB: FigRegistryAddItem called");
    return NULL;
}

void* FigRegistryAddSearchPath(void) {
    if (verbose) puts("STUB: FigRegistryAddSearchPath called");
    return NULL;
}

void* FigRegistryAddSearchPathWithOptions(void) {
    if (verbose) puts("STUB: FigRegistryAddSearchPathWithOptions called");
    return NULL;
}

void* FigRegistryCopyFilteredItemList(void) {
    if (verbose) puts("STUB: FigRegistryCopyFilteredItemList called");
    return NULL;
}

void* FigRegistryCopyItemList(void) {
    if (verbose) puts("STUB: FigRegistryCopyItemList called");
    return NULL;
}

void* FigRegistryCreate(void) {
    if (verbose) puts("STUB: FigRegistryCreate called");
    return NULL;
}

void* FigRegistryGetTypeID(void) {
    if (verbose) puts("STUB: FigRegistryGetTypeID called");
    return NULL;
}

void* FigRegistryItemCopyBundle(void) {
    if (verbose) puts("STUB: FigRegistryItemCopyBundle called");
    return NULL;
}

void* FigRegistryItemCopyDescription(void) {
    if (verbose) puts("STUB: FigRegistryItemCopyDescription called");
    return NULL;
}

void* FigRegistryItemCopyMatchingInfo(void) {
    if (verbose) puts("STUB: FigRegistryItemCopyMatchingInfo called");
    return NULL;
}

void* FigRegistryItemGetFactory(void) {
    if (verbose) puts("STUB: FigRegistryItemGetFactory called");
    return NULL;
}

void* FigRegistryItemGetTypeID(void) {
    if (verbose) puts("STUB: FigRegistryItemGetTypeID called");
    return NULL;
}

void* FigRegistryRescan(void) {
    if (verbose) puts("STUB: FigRegistryRescan called");
    return NULL;
}

void* FigRegistrySetIORegistryEntryArray(void) {
    if (verbose) puts("STUB: FigRegistrySetIORegistryEntryArray called");
    return NULL;
}

void* FigRemote_CreateCFTypeFromSerializedAtomDataBlockBuffer(void) {
    if (verbose) puts("STUB: FigRemote_CreateCFTypeFromSerializedAtomDataBlockBuffer called");
    return NULL;
}

void* FigRemote_CreateFormatDescriptionArrayFromSerializedAtomData(void) {
    if (verbose) puts("STUB: FigRemote_CreateFormatDescriptionArrayFromSerializedAtomData called");
    return NULL;
}

void* FigRemote_CreateFormatDescriptionFromSerializedAtomDataBlockBuffer(void) {
    if (verbose) puts("STUB: FigRemote_CreateFormatDescriptionFromSerializedAtomDataBlockBuffer called");
    return NULL;
}

void* FigRemote_CreateKeyValuePairFromSerializedAtomDataBlockBuffer(void) {
    if (verbose) puts("STUB: FigRemote_CreateKeyValuePairFromSerializedAtomDataBlockBuffer called");
    return NULL;
}

void* FigRemote_CreatePixelBufferFromSerializedAtomDataBlockBuffer(void) {
    if (verbose) puts("STUB: FigRemote_CreatePixelBufferFromSerializedAtomDataBlockBuffer called");
    return NULL;
}

void* FigRemote_CreateSampleBufferFromSerializedAtomDataBlockBuffer(void) {
    if (verbose) puts("STUB: FigRemote_CreateSampleBufferFromSerializedAtomDataBlockBuffer called");
    return NULL;
}

void* FigRemote_CreateSerializedAtomDataBlockBufferForCFType(void) {
    if (verbose) puts("STUB: FigRemote_CreateSerializedAtomDataBlockBufferForCFType called");
    return NULL;
}

void* FigRemote_CreateSerializedAtomDataBlockBufferForFormatDescription(void) {
    if (verbose) puts("STUB: FigRemote_CreateSerializedAtomDataBlockBufferForFormatDescription called");
    return NULL;
}

void* FigRemote_CreateSerializedAtomDataBlockBufferForKeyValuePair(void) {
    if (verbose) puts("STUB: FigRemote_CreateSerializedAtomDataBlockBufferForKeyValuePair called");
    return NULL;
}

void* FigRemote_CreateSerializedAtomDataBlockBufferForPixelBuffer(void) {
    if (verbose) puts("STUB: FigRemote_CreateSerializedAtomDataBlockBufferForPixelBuffer called");
    return NULL;
}

void* FigRemote_CreateSerializedAtomDataBlockBufferForSampleBuffer(void) {
    if (verbose) puts("STUB: FigRemote_CreateSerializedAtomDataBlockBufferForSampleBuffer called");
    return NULL;
}

void* FigRemote_CreateSerializedAtomDataForFormatDescriptions(void) {
    if (verbose) puts("STUB: FigRemote_CreateSerializedAtomDataForFormatDescriptions called");
    return NULL;
}

void* FigReplaceStringWithURL(void) {
    if (verbose) puts("STUB: FigReplaceStringWithURL called");
    return NULL;
}

void* FigReplaceURLWithString(void) {
    if (verbose) puts("STUB: FigReplaceURLWithString called");
    return NULL;
}

void* FigRetainProxyConditionVariableWaitRelative(void) {
    if (verbose) puts("STUB: FigRetainProxyConditionVariableWaitRelative called");
    return NULL;
}

void* FigRetainProxyCreate(void) {
    if (verbose) puts("STUB: FigRetainProxyCreate called");
    return NULL;
}

void* FigRetainProxyGetLockCount(void) {
    if (verbose) puts("STUB: FigRetainProxyGetLockCount called");
    return NULL;
}

void* FigRetainProxyGetOwner(void) {
    if (verbose) puts("STUB: FigRetainProxyGetOwner called");
    return NULL;
}

void* FigRetainProxyGetTypeID(void) {
    if (verbose) puts("STUB: FigRetainProxyGetTypeID called");
    return NULL;
}

void* FigRetainProxyInvalidate(void) {
    if (verbose) puts("STUB: FigRetainProxyInvalidate called");
    return NULL;
}

void* FigRetainProxyIsInvalidated(void) {
    if (verbose) puts("STUB: FigRetainProxyIsInvalidated called");
    return NULL;
}

void* FigRetainProxyLockMutex(void) {
    if (verbose) puts("STUB: FigRetainProxyLockMutex called");
    return NULL;
}

void* FigRetainProxyRelease(void) {
    if (verbose) puts("STUB: FigRetainProxyRelease called");
    return NULL;
}

void* FigRetainProxyReleaseOwner(void) {
    if (verbose) puts("STUB: FigRetainProxyReleaseOwner called");
    return NULL;
}

void* FigRetainProxyRelock(void) {
    if (verbose) puts("STUB: FigRetainProxyRelock called");
    return NULL;
}

void* FigRetainProxyRetain(void) {
    if (verbose) puts("STUB: FigRetainProxyRetain called");
    return NULL;
}

void* FigRetainProxyRetainOwner(void) {
    if (verbose) puts("STUB: FigRetainProxyRetainOwner called");
    return NULL;
}

void* FigRetainProxyTryLockMutex(void) {
    if (verbose) puts("STUB: FigRetainProxyTryLockMutex called");
    return NULL;
}

void* FigRetainProxyUnlockAll(void) {
    if (verbose) puts("STUB: FigRetainProxyUnlockAll called");
    return NULL;
}

void* FigRetainProxyUnlockMutex(void) {
    if (verbose) puts("STUB: FigRetainProxyUnlockMutex called");
    return NULL;
}

void* FigRingBufferAppendValueAndEvictBuffer(void) {
    if (verbose) puts("STUB: FigRingBufferAppendValueAndEvictBuffer called");
    return NULL;
}

void* FigRingBufferCreate(void) {
    if (verbose) puts("STUB: FigRingBufferCreate called");
    return NULL;
}

void* FigRingBufferFlush(void) {
    if (verbose) puts("STUB: FigRingBufferFlush called");
    return NULL;
}

void* FigRingBufferGetCount(void) {
    if (verbose) puts("STUB: FigRingBufferGetCount called");
    return NULL;
}

void* FigRingBufferGetEntries(void) {
    if (verbose) puts("STUB: FigRingBufferGetEntries called");
    return NULL;
}

void* FigRingBufferGetSize(void) {
    if (verbose) puts("STUB: FigRingBufferGetSize called");
    return NULL;
}

void* FigRingBufferRelease(void) {
    if (verbose) puts("STUB: FigRingBufferRelease called");
    return NULL;
}

void* FigRouteDiscovererCreate(void) {
    if (verbose) puts("STUB: FigRouteDiscovererCreate called");
    return NULL;
}

void* FigRouteDiscovererGetClassID(void) {
    if (verbose) puts("STUB: FigRouteDiscovererGetClassID called");
    return NULL;
}

void* FigRouteDiscovererGetTypeID(void) {
    if (verbose) puts("STUB: FigRouteDiscovererGetTypeID called");
    return NULL;
}

void* FigRouteDiscovererStartServer(void) {
    if (verbose) puts("STUB: FigRouteDiscovererStartServer called");
    return NULL;
}

void* FigRouteDiscovererXPCRemoteCreate(void) {
    if (verbose) puts("STUB: FigRouteDiscovererXPCRemoteCreate called");
    return NULL;
}

void* FigRoutingContextCopyContextForUUID(void) {
    if (verbose) puts("STUB: FigRoutingContextCopyContextForUUID called");
    return NULL;
}

void* FigRoutingContextCopySystemAudioContext(void) {
    if (verbose) puts("STUB: FigRoutingContextCopySystemAudioContext called");
    return NULL;
}

void* FigRoutingContextCopySystemMirroringContext(void) {
    if (verbose) puts("STUB: FigRoutingContextCopySystemMirroringContext called");
    return NULL;
}

void* FigRoutingContextCopySystemMusicContext(void) {
    if (verbose) puts("STUB: FigRoutingContextCopySystemMusicContext called");
    return NULL;
}

void* FigRoutingContextCopySystemRemotePoolContext(void) {
    if (verbose) puts("STUB: FigRoutingContextCopySystemRemotePoolContext called");
    return NULL;
}

void* FigRoutingContextCreateAudioContext(void) {
    if (verbose) puts("STUB: FigRoutingContextCreateAudioContext called");
    return NULL;
}

void* FigRoutingContextCreateControlChannelOnlyContext(void) {
    if (verbose) puts("STUB: FigRoutingContextCreateControlChannelOnlyContext called");
    return NULL;
}

void* FigRoutingContextCreatePerAppSecondDisplayContext(void) {
    if (verbose) puts("STUB: FigRoutingContextCreatePerAppSecondDisplayContext called");
    return NULL;
}

void* FigRoutingContextCreateRemoteMusicControllerContext(void) {
    if (verbose) puts("STUB: FigRoutingContextCreateRemoteMusicControllerContext called");
    return NULL;
}

void* FigRoutingContextCreateVideoContext(void) {
    if (verbose) puts("STUB: FigRoutingContextCreateVideoContext called");
    return NULL;
}

void* FigRoutingContextGetClassID(void) {
    if (verbose) puts("STUB: FigRoutingContextGetClassID called");
    return NULL;
}

void* FigRoutingContextGetTypeID(void) {
    if (verbose) puts("STUB: FigRoutingContextGetTypeID called");
    return NULL;
}

void* FigRoutingContextRemoteCopyContextForUUID(void) {
    if (verbose) puts("STUB: FigRoutingContextRemoteCopyContextForUUID called");
    return NULL;
}

void* FigRoutingContextRemoteCopySystemAudioContext(void) {
    if (verbose) puts("STUB: FigRoutingContextRemoteCopySystemAudioContext called");
    return NULL;
}

void* FigRoutingContextRemoteCopySystemMirroringContext(void) {
    if (verbose) puts("STUB: FigRoutingContextRemoteCopySystemMirroringContext called");
    return NULL;
}

void* FigRoutingContextRemoteCopySystemMusicContext(void) {
    if (verbose) puts("STUB: FigRoutingContextRemoteCopySystemMusicContext called");
    return NULL;
}

void* FigRoutingContextRemoteCopySystemRemotePoolContext(void) {
    if (verbose) puts("STUB: FigRoutingContextRemoteCopySystemRemotePoolContext called");
    return NULL;
}

void* FigRoutingContextRemoteCreateAudioContext(void) {
    if (verbose) puts("STUB: FigRoutingContextRemoteCreateAudioContext called");
    return NULL;
}

void* FigRoutingContextRemoteCreateControlChannelOnlyContext(void) {
    if (verbose) puts("STUB: FigRoutingContextRemoteCreateControlChannelOnlyContext called");
    return NULL;
}

void* FigRoutingContextRemoteCreatePerAppSecondDisplayContext(void) {
    if (verbose) puts("STUB: FigRoutingContextRemoteCreatePerAppSecondDisplayContext called");
    return NULL;
}

void* FigRoutingContextRemoteCreateRemoteMusicControllerContext(void) {
    if (verbose) puts("STUB: FigRoutingContextRemoteCreateRemoteMusicControllerContext called");
    return NULL;
}

void* FigRoutingContextRemoteCreateVideoContext(void) {
    if (verbose) puts("STUB: FigRoutingContextRemoteCreateVideoContext called");
    return NULL;
}

void* FigRoutingContextStartServer(void) {
    if (verbose) puts("STUB: FigRoutingContextStartServer called");
    return NULL;
}

void* FigRoutingCopyDisplayMenuVideoContext(void) {
    if (verbose) puts("STUB: FigRoutingCopyDisplayMenuVideoContext called");
    return NULL;
}

void* FigRoutingManagerInit(void) {
    if (verbose) puts("STUB: FigRoutingManagerInit called");
    return NULL;
}

void* FigRoutingRemoteCopyDisplayMenuVideoContext(void) {
    if (verbose) puts("STUB: FigRoutingRemoteCopyDisplayMenuVideoContext called");
    return NULL;
}

void* FigRunDeferredNotificationsOnCurrentRunLoop(void) {
    if (verbose) puts("STUB: FigRunDeferredNotificationsOnCurrentRunLoop called");
    return NULL;
}

void* FigRuntimeGetAttachmentBearerCallbacksWithTypeID(void) {
    if (verbose) puts("STUB: FigRuntimeGetAttachmentBearerCallbacksWithTypeID called");
    return NULL;
}

void* FigRuntimeRegisterAttachmentBearerWithTypeID(void) {
    if (verbose) puts("STUB: FigRuntimeRegisterAttachmentBearerWithTypeID called");
    return NULL;
}

void* FigRuntimeUnregisterAttachmentBearerWithTypeID(void) {
    if (verbose) puts("STUB: FigRuntimeUnregisterAttachmentBearerWithTypeID called");
    return NULL;
}

void* FigSampleBufferCreateCopyWithNewFormatDescription(void) {
    if (verbose) puts("STUB: FigSampleBufferCreateCopyWithNewFormatDescription called");
    return NULL;
}

void* FigSampleBufferCreateCopyWithNoData(void) {
    if (verbose) puts("STUB: FigSampleBufferCreateCopyWithNoData called");
    return NULL;
}

void* FigSampleBufferCreateForCaptionGroup(void) {
    if (verbose) puts("STUB: FigSampleBufferCreateForCaptionGroup called");
    return NULL;
}

void* FigSampleBufferGetCaptionGroup(void) {
    if (verbose) puts("STUB: FigSampleBufferGetCaptionGroup called");
    return NULL;
}

void* FigSampleBufferGetDecryptor(void) {
    if (verbose) puts("STUB: FigSampleBufferGetDecryptor called");
    return NULL;
}

void* FigSampleBufferRelease(void) {
    if (verbose) puts("STUB: FigSampleBufferRelease called");
    return NULL;
}

void* FigSampleBufferRetain(void) {
    if (verbose) puts("STUB: FigSampleBufferRetain called");
    return NULL;
}

void* FigSampleBufferSetDecryptor(void) {
    if (verbose) puts("STUB: FigSampleBufferSetDecryptor called");
    return NULL;
}

void* FigScheduledFileIOCloseForRead(void) {
    if (verbose) puts("STUB: FigScheduledFileIOCloseForRead called");
    return NULL;
}

void* FigScheduledFileIOComputeBufferSpecs(void) {
    if (verbose) puts("STUB: FigScheduledFileIOComputeBufferSpecs called");
    return NULL;
}

void* FigScheduledFileIOExpediteAllClientRequests(void) {
    if (verbose) puts("STUB: FigScheduledFileIOExpediteAllClientRequests called");
    return NULL;
}

void* FigScheduledFileIOFinish(void) {
    if (verbose) puts("STUB: FigScheduledFileIOFinish called");
    return NULL;
}

void* FigScheduledFileIOGetEOF(void) {
    if (verbose) puts("STUB: FigScheduledFileIOGetEOF called");
    return NULL;
}

void* FigScheduledFileIOOpenForRead(void) {
    if (verbose) puts("STUB: FigScheduledFileIOOpenForRead called");
    return NULL;
}

void* FigScheduledFileIOOpenWithByteStreamForRead(void) {
    if (verbose) puts("STUB: FigScheduledFileIOOpenWithByteStreamForRead called");
    return NULL;
}

void* FigScheduledFileIORequestRead(void) {
    if (verbose) puts("STUB: FigScheduledFileIORequestRead called");
    return NULL;
}

void* FigScheduledFileIOReschduleRequests(void) {
    if (verbose) puts("STUB: FigScheduledFileIOReschduleRequests called");
    return NULL;
}

void* FigScheduledIOCreateForReadingByteStream(void) {
    if (verbose) puts("STUB: FigScheduledIOCreateForReadingByteStream called");
    return NULL;
}

void* FigScheduledIOCreateForReadingFile(void) {
    if (verbose) puts("STUB: FigScheduledIOCreateForReadingFile called");
    return NULL;
}

void* FigScheduledIOGetClassID(void) {
    if (verbose) puts("STUB: FigScheduledIOGetClassID called");
    return NULL;
}

void* FigScheduledIOGetFigBaseObject(void) {
    if (verbose) puts("STUB: FigScheduledIOGetFigBaseObject called");
    return NULL;
}

void* FigScheduledIOGetTypeID(void) {
    if (verbose) puts("STUB: FigScheduledIOGetTypeID called");
    return NULL;
}

void* FigSecCopyOutmostBundleSignedWithSameCertificateAsCodeAtURL(void) {
    if (verbose) puts("STUB: FigSecCopyOutmostBundleSignedWithSameCertificateAsCodeAtURL called");
    return NULL;
}

void* FigSemaphoreCreate(void) {
    if (verbose) puts("STUB: FigSemaphoreCreate called");
    return NULL;
}

void* FigSemaphoreDestroy(void) {
    if (verbose) puts("STUB: FigSemaphoreDestroy called");
    return NULL;
}

void* FigSemaphoreSignal(void) {
    if (verbose) puts("STUB: FigSemaphoreSignal called");
    return NULL;
}

void* FigSemaphoreWaitRelative(void) {
    if (verbose) puts("STUB: FigSemaphoreWaitRelative called");
    return NULL;
}

void* FigSetItIsOKToLogURLs(void) {
    if (verbose) puts("STUB: FigSetItIsOKToLogURLs called");
    return NULL;
}

void* FigSignalErrorAt(void) {
    if (verbose) puts("STUB: FigSignalErrorAt called");
    return NULL;
}

void* FigSignalErrorAt2(void) {
    if (verbose) puts("STUB: FigSignalErrorAt2 called");
    return NULL;
}

void* FigSimpleMutexCheckIsLockedOnThisThread(void) {
    if (verbose) puts("STUB: FigSimpleMutexCheckIsLockedOnThisThread called");
    return NULL;
}

void* FigSimpleMutexCheckIsNotLockedOnThisThread(void) {
    if (verbose) puts("STUB: FigSimpleMutexCheckIsNotLockedOnThisThread called");
    return NULL;
}

void* FigSimpleMutexCreate(void) {
    if (verbose) puts("STUB: FigSimpleMutexCreate called");
    return NULL;
}

void* FigSimpleMutexDestroy(void) {
    if (verbose) puts("STUB: FigSimpleMutexDestroy called");
    return NULL;
}

void* FigSimpleMutexLock(void) {
    if (verbose) puts("STUB: FigSimpleMutexLock called");
    return NULL;
}

void* FigSimpleMutexTryLock(void) {
    if (verbose) puts("STUB: FigSimpleMutexTryLock called");
    return NULL;
}

void* FigSimpleMutexUnlock(void) {
    if (verbose) puts("STUB: FigSimpleMutexUnlock called");
    return NULL;
}

void* FigSimpleQueueRelease(void) {
    if (verbose) puts("STUB: FigSimpleQueueRelease called");
    return NULL;
}

void* FigSimpleQueueRetain(void) {
    if (verbose) puts("STUB: FigSimpleQueueRetain called");
    return NULL;
}

void* FigSimpleQueueSetQueueIsFullErrorLoggingEnabled(void) {
    if (verbose) puts("STUB: FigSimpleQueueSetQueueIsFullErrorLoggingEnabled called");
    return NULL;
}

void* FigSyncConvertTime(void) {
    if (verbose) puts("STUB: FigSyncConvertTime called");
    return NULL;
}

void* FigSyncGetNotificationCenter(void) {
    if (verbose) puts("STUB: FigSyncGetNotificationCenter called");
    return NULL;
}

void* FigSyncGetRelativeRate(void) {
    if (verbose) puts("STUB: FigSyncGetRelativeRate called");
    return NULL;
}

void* FigSyncGetRelativeRateAndAnchorTime(void) {
    if (verbose) puts("STUB: FigSyncGetRelativeRateAndAnchorTime called");
    return NULL;
}

void* FigSyncGetTime(void) {
    if (verbose) puts("STUB: FigSyncGetTime called");
    return NULL;
}

void* FigSyncMightDrift(void) {
    if (verbose) puts("STUB: FigSyncMightDrift called");
    return NULL;
}

void* FigTextMarkupAspectRatioCopyAsSimplifiedDictionary(void) {
    if (verbose) puts("STUB: FigTextMarkupAspectRatioCopyAsSimplifiedDictionary called");
    return NULL;
}

void* FigTextMarkupAspectRatioMakeFromSimplifiedDictionary(void) {
    if (verbose) puts("STUB: FigTextMarkupAspectRatioMakeFromSimplifiedDictionary called");
    return NULL;
}

void* FigTextMarkupCreateARGBColorArrayFromCGColor(void) {
    if (verbose) puts("STUB: FigTextMarkupCreateARGBColorArrayFromCGColor called");
    return NULL;
}

void* FigTextMarkupCreateBlackARGBColorArray(void) {
    if (verbose) puts("STUB: FigTextMarkupCreateBlackARGBColorArray called");
    return NULL;
}

void* FigTextMarkupCreateClearARGBColorArray(void) {
    if (verbose) puts("STUB: FigTextMarkupCreateClearARGBColorArray called");
    return NULL;
}

void* FigTextMarkupCreateWhiteARGBColorArray(void) {
    if (verbose) puts("STUB: FigTextMarkupCreateWhiteARGBColorArray called");
    return NULL;
}

void* FigTextMarkupDimensionCopyAsSimplifiedDictionary(void) {
    if (verbose) puts("STUB: FigTextMarkupDimensionCopyAsSimplifiedDictionary called");
    return NULL;
}

void* FigTextMarkupDimensionMakeFromSimplifiedDictionary(void) {
    if (verbose) puts("STUB: FigTextMarkupDimensionMakeFromSimplifiedDictionary called");
    return NULL;
}

void* FigTextMarkupMapGenericFontFamilyToMACaptionAppearanceFontStyle(void) {
    if (verbose) puts("STUB: FigTextMarkupMapGenericFontFamilyToMACaptionAppearanceFontStyle called");
    return NULL;
}

void* FigTextMarkupSizeCopyAsSimplifiedDictionary(void) {
    if (verbose) puts("STUB: FigTextMarkupSizeCopyAsSimplifiedDictionary called");
    return NULL;
}

void* FigTextMarkupSizeMakeFromSimplifiedDictionary(void) {
    if (verbose) puts("STUB: FigTextMarkupSizeMakeFromSimplifiedDictionary called");
    return NULL;
}

void* FigThreadAbort(void) {
    if (verbose) puts("STUB: FigThreadAbort called");
    return NULL;
}

void* FigThreadAbortForTeardown(void) {
    if (verbose) puts("STUB: FigThreadAbortForTeardown called");
    return NULL;
}

void* FigThreadAbortQueue(void) {
    if (verbose) puts("STUB: FigThreadAbortQueue called");
    return NULL;
}

void* FigThreadAbortQueueForTeardown(void) {
    if (verbose) puts("STUB: FigThreadAbortQueueForTeardown called");
    return NULL;
}

void* FigThreadAllocKey(void) {
    if (verbose) puts("STUB: FigThreadAllocKey called");
    return NULL;
}

void* FigThreadCopyProperty(void) {
    if (verbose) puts("STUB: FigThreadCopyProperty called");
    return NULL;
}

void* FigThreadCreate(void) {
    if (verbose) puts("STUB: FigThreadCreate called");
    return NULL;
}

void* FigThreadExit(void) {
    if (verbose) puts("STUB: FigThreadExit called");
    return NULL;
}

void* FigThreadGetCurrent(void) {
    if (verbose) puts("STUB: FigThreadGetCurrent called");
    return NULL;
}

void* FigThreadGetGlobalNetworkBufferingBackgroundThrottledRunloop(void) {
    if (verbose) puts("STUB: FigThreadGetGlobalNetworkBufferingBackgroundThrottledRunloop called");
    return NULL;
}

void* FigThreadGetGlobalNetworkBufferingLowPriorityQueue(void) {
    if (verbose) puts("STUB: FigThreadGetGlobalNetworkBufferingLowPriorityQueue called");
    return NULL;
}

void* FigThreadGetGlobalNetworkBufferingLowPriorityRunloop(void) {
    if (verbose) puts("STUB: FigThreadGetGlobalNetworkBufferingLowPriorityRunloop called");
    return NULL;
}

void* FigThreadGetGlobalNetworkBufferingQueue(void) {
    if (verbose) puts("STUB: FigThreadGetGlobalNetworkBufferingQueue called");
    return NULL;
}

void* FigThreadGetGlobalNetworkBufferingRunloop(void) {
    if (verbose) puts("STUB: FigThreadGetGlobalNetworkBufferingRunloop called");
    return NULL;
}

void* FigThreadGetMachThreadPriorityValue(void) {
    if (verbose) puts("STUB: FigThreadGetMachThreadPriorityValue called");
    return NULL;
}

void* FigThreadGetMaxLogicalCoreCount(void) {
    if (verbose) puts("STUB: FigThreadGetMaxLogicalCoreCount called");
    return NULL;
}

void* FigThreadGetPlatformID(void) {
    if (verbose) puts("STUB: FigThreadGetPlatformID called");
    return NULL;
}

void* FigThreadGetValue(void) {
    if (verbose) puts("STUB: FigThreadGetValue called");
    return NULL;
}

void* FigThreadIsValid(void) {
    if (verbose) puts("STUB: FigThreadIsValid called");
    return NULL;
}

void* FigThreadJoin(void) {
    if (verbose) puts("STUB: FigThreadJoin called");
    return NULL;
}

void* FigThreadMakeDispatchQueueAbortable(void) {
    if (verbose) puts("STUB: FigThreadMakeDispatchQueueAbortable called");
    return NULL;
}

void* FigThreadRegisterAbortAction(void) {
    if (verbose) puts("STUB: FigThreadRegisterAbortAction called");
    return NULL;
}

void* FigThreadRunOnce(void) {
    if (verbose) puts("STUB: FigThreadRunOnce called");
    return NULL;
}

void* FigThreadSetProperty(void) {
    if (verbose) puts("STUB: FigThreadSetProperty called");
    return NULL;
}

void* FigThreadSetValue(void) {
    if (verbose) puts("STUB: FigThreadSetValue called");
    return NULL;
}

void* FigThreadUnregisterAbortAction(void) {
    if (verbose) puts("STUB: FigThreadUnregisterAbortAction called");
    return NULL;
}

void* FigTimeCodeFormatDescriptionCopyAsBigEndianTimeCodeDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: FigTimeCodeFormatDescriptionCopyAsBigEndianTimeCodeDescriptionBlockBuffer called");
    return NULL;
}

void* FigTimeCodeFormatDescriptionCreateFromBigEndianTimeCodeDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: FigTimeCodeFormatDescriptionCreateFromBigEndianTimeCodeDescriptionBlockBuffer called");
    return NULL;
}

void* FigTimeCodeFormatDescriptionCreateFromBigEndianTimeCodeDescriptionData(void) {
    if (verbose) puts("STUB: FigTimeCodeFormatDescriptionCreateFromBigEndianTimeCodeDescriptionData called");
    return NULL;
}

void* FigTimebaseAddTimer(void) {
    if (verbose) puts("STUB: FigTimebaseAddTimer called");
    return NULL;
}

void* FigTimebaseAddTimerDispatchSource(void) {
    if (verbose) puts("STUB: FigTimebaseAddTimerDispatchSource called");
    return NULL;
}

void* FigTimebaseCreateWithMasterClock(void) {
    if (verbose) puts("STUB: FigTimebaseCreateWithMasterClock called");
    return NULL;
}

void* FigTimebaseCreateWithMasterTimebase(void) {
    if (verbose) puts("STUB: FigTimebaseCreateWithMasterTimebase called");
    return NULL;
}

void* FigTimebaseGetEffectiveRate(void) {
    if (verbose) puts("STUB: FigTimebaseGetEffectiveRate called");
    return NULL;
}

void* FigTimebaseGetMaster(void) {
    if (verbose) puts("STUB: FigTimebaseGetMaster called");
    return NULL;
}

void* FigTimebaseGetMasterClock(void) {
    if (verbose) puts("STUB: FigTimebaseGetMasterClock called");
    return NULL;
}

void* FigTimebaseGetMasterTimebase(void) {
    if (verbose) puts("STUB: FigTimebaseGetMasterTimebase called");
    return NULL;
}

void* FigTimebaseGetRate(void) {
    if (verbose) puts("STUB: FigTimebaseGetRate called");
    return NULL;
}

void* FigTimebaseGetTime(void) {
    if (verbose) puts("STUB: FigTimebaseGetTime called");
    return NULL;
}

void* FigTimebaseGetTimeAndRate(void) {
    if (verbose) puts("STUB: FigTimebaseGetTimeAndRate called");
    return NULL;
}

void* FigTimebaseGetTimeWithTimeScale(void) {
    if (verbose) puts("STUB: FigTimebaseGetTimeWithTimeScale called");
    return NULL;
}

void* FigTimebaseGetTypeID(void) {
    if (verbose) puts("STUB: FigTimebaseGetTypeID called");
    return NULL;
}

void* FigTimebaseGetUltimateMasterClock(void) {
    if (verbose) puts("STUB: FigTimebaseGetUltimateMasterClock called");
    return NULL;
}

void* FigTimebaseRelease(void) {
    if (verbose) puts("STUB: FigTimebaseRelease called");
    return NULL;
}

void* FigTimebaseRemoveTimer(void) {
    if (verbose) puts("STUB: FigTimebaseRemoveTimer called");
    return NULL;
}

void* FigTimebaseRemoveTimerDispatchSource(void) {
    if (verbose) puts("STUB: FigTimebaseRemoveTimerDispatchSource called");
    return NULL;
}

void* FigTimebaseRetain(void) {
    if (verbose) puts("STUB: FigTimebaseRetain called");
    return NULL;
}

void* FigTimebaseSetAnchorTime(void) {
    if (verbose) puts("STUB: FigTimebaseSetAnchorTime called");
    return NULL;
}

void* FigTimebaseSetMasterClock(void) {
    if (verbose) puts("STUB: FigTimebaseSetMasterClock called");
    return NULL;
}

void* FigTimebaseSetMasterTimebase(void) {
    if (verbose) puts("STUB: FigTimebaseSetMasterTimebase called");
    return NULL;
}

void* FigTimebaseSetRate(void) {
    if (verbose) puts("STUB: FigTimebaseSetRate called");
    return NULL;
}

void* FigTimebaseSetRateAndAnchorTime(void) {
    if (verbose) puts("STUB: FigTimebaseSetRateAndAnchorTime called");
    return NULL;
}

void* FigTimebaseSetTime(void) {
    if (verbose) puts("STUB: FigTimebaseSetTime called");
    return NULL;
}

void* FigTimebaseSetTimerDispatchSourceNextFireTime(void) {
    if (verbose) puts("STUB: FigTimebaseSetTimerDispatchSourceNextFireTime called");
    return NULL;
}

void* FigTimebaseSetTimerDispatchSourceToFireImmediately(void) {
    if (verbose) puts("STUB: FigTimebaseSetTimerDispatchSourceToFireImmediately called");
    return NULL;
}

void* FigTimebaseSetTimerNextFireTime(void) {
    if (verbose) puts("STUB: FigTimebaseSetTimerNextFireTime called");
    return NULL;
}

void* FigTimebaseSetTimerToFireImmediately(void) {
    if (verbose) puts("STUB: FigTimebaseSetTimerToFireImmediately called");
    return NULL;
}

void* FigTraceLoggingDumpToFile(void) {
    if (verbose) puts("STUB: FigTraceLoggingDumpToFile called");
    return NULL;
}

void* FigTraceLoggingEnd(void) {
    if (verbose) puts("STUB: FigTraceLoggingEnd called");
    return NULL;
}

void* FigTraceLoggingPause(void) {
    if (verbose) puts("STUB: FigTraceLoggingPause called");
    return NULL;
}

void* FigTraceLoggingResume(void) {
    if (verbose) puts("STUB: FigTraceLoggingResume called");
    return NULL;
}

void* FigTraceLoggingStart(void) {
    if (verbose) puts("STUB: FigTraceLoggingStart called");
    return NULL;
}

void* FigTransportConnectToReceiver(void) {
    if (verbose) puts("STUB: FigTransportConnectToReceiver called");
    return NULL;
}

void* FigTransportConnectionGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigTransportConnectionGetCMBaseObject called");
    return NULL;
}

void* FigTransportConnectionGetClassID(void) {
    if (verbose) puts("STUB: FigTransportConnectionGetClassID called");
    return NULL;
}

void* FigTransportConnectionGetTypeID(void) {
    if (verbose) puts("STUB: FigTransportConnectionGetTypeID called");
    return NULL;
}

void* FigTransportConnectionTCPCreate(void) {
    if (verbose) puts("STUB: FigTransportConnectionTCPCreate called");
    return NULL;
}

void* FigTransportConnectionUSBCreate(void) {
    if (verbose) puts("STUB: FigTransportConnectionUSBCreate called");
    return NULL;
}

void* FigTransportCopyRegisteredObject(void) {
    if (verbose) puts("STUB: FigTransportCopyRegisteredObject called");
    return NULL;
}

void* FigTransportCreateBBufFromPointer(void) {
    if (verbose) puts("STUB: FigTransportCreateBBufFromPointer called");
    return NULL;
}

void* FigTransportCreateBBufFromRemoteObjectRef(void) {
    if (verbose) puts("STUB: FigTransportCreateBBufFromRemoteObjectRef called");
    return NULL;
}

void* FigTransportCreateCFNumberFromLocalObjectRef(void) {
    if (verbose) puts("STUB: FigTransportCreateCFNumberFromLocalObjectRef called");
    return NULL;
}

void* FigTransportCreateCFNumberFromRemoteObjectRef(void) {
    if (verbose) puts("STUB: FigTransportCreateCFNumberFromRemoteObjectRef called");
    return NULL;
}

void* FigTransportDisconnectFromReceiver(void) {
    if (verbose) puts("STUB: FigTransportDisconnectFromReceiver called");
    return NULL;
}

void* FigTransportDiscoveryAddEventHandler(void) {
    if (verbose) puts("STUB: FigTransportDiscoveryAddEventHandler called");
    return NULL;
}

void* FigTransportDiscoveryGetSharedDiscovery(void) {
    if (verbose) puts("STUB: FigTransportDiscoveryGetSharedDiscovery called");
    return NULL;
}

void* FigTransportDiscoveryGetTypeID(void) {
    if (verbose) puts("STUB: FigTransportDiscoveryGetTypeID called");
    return NULL;
}

void* FigTransportDiscoveryRemoveEventHandler(void) {
    if (verbose) puts("STUB: FigTransportDiscoveryRemoveEventHandler called");
    return NULL;
}

void* FigTransportDiscoveryStart(void) {
    if (verbose) puts("STUB: FigTransportDiscoveryStart called");
    return NULL;
}

void* FigTransportDiscoveryStop(void) {
    if (verbose) puts("STUB: FigTransportDiscoveryStop called");
    return NULL;
}

void* FigTransportGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigTransportGetCMBaseObject called");
    return NULL;
}

void* FigTransportGetClassID(void) {
    if (verbose) puts("STUB: FigTransportGetClassID called");
    return NULL;
}

void* FigTransportGetLocalObjectRefFromCFNumber(void) {
    if (verbose) puts("STUB: FigTransportGetLocalObjectRefFromCFNumber called");
    return NULL;
}

void* FigTransportGetLocalRootObject(void) {
    if (verbose) puts("STUB: FigTransportGetLocalRootObject called");
    return NULL;
}

void* FigTransportGetRemoteObjectRefFromBBuf(void) {
    if (verbose) puts("STUB: FigTransportGetRemoteObjectRefFromBBuf called");
    return NULL;
}

void* FigTransportGetRemoteObjectRefFromCFNumber(void) {
    if (verbose) puts("STUB: FigTransportGetRemoteObjectRefFromCFNumber called");
    return NULL;
}

void* FigTransportGetRemoteRootObject(void) {
    if (verbose) puts("STUB: FigTransportGetRemoteRootObject called");
    return NULL;
}

void* FigTransportGetTypeID(void) {
    if (verbose) puts("STUB: FigTransportGetTypeID called");
    return NULL;
}

void* FigTransportInitialize(void) {
    if (verbose) puts("STUB: FigTransportInitialize called");
    return NULL;
}

void* FigTransportInitializeWithConnection(void) {
    if (verbose) puts("STUB: FigTransportInitializeWithConnection called");
    return NULL;
}

void* FigTransportIsConnected(void) {
    if (verbose) puts("STUB: FigTransportIsConnected called");
    return NULL;
}

void* FigTransportIsHiSpeed(void) {
    if (verbose) puts("STUB: FigTransportIsHiSpeed called");
    return NULL;
}

void* FigTransportPackageGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigTransportPackageGetCMBaseObject called");
    return NULL;
}

void* FigTransportPackageGetClassID(void) {
    if (verbose) puts("STUB: FigTransportPackageGetClassID called");
    return NULL;
}

void* FigTransportPackageGetTypeID(void) {
    if (verbose) puts("STUB: FigTransportPackageGetTypeID called");
    return NULL;
}

void* FigTransportParcelCreateBBufRepresentation(void) {
    if (verbose) puts("STUB: FigTransportParcelCreateBBufRepresentation called");
    return NULL;
}

void* FigTransportParcelFree(void) {
    if (verbose) puts("STUB: FigTransportParcelFree called");
    return NULL;
}

void* FigTransportParcelGetBytes(void) {
    if (verbose) puts("STUB: FigTransportParcelGetBytes called");
    return NULL;
}

void* FigTransportParcelGetMessageData(void) {
    if (verbose) puts("STUB: FigTransportParcelGetMessageData called");
    return NULL;
}

void* FigTransportParcelGetMessageType(void) {
    if (verbose) puts("STUB: FigTransportParcelGetMessageType called");
    return NULL;
}

void* FigTransportParcelMake(void) {
    if (verbose) puts("STUB: FigTransportParcelMake called");
    return NULL;
}

void* FigTransportParcelSetMessageData(void) {
    if (verbose) puts("STUB: FigTransportParcelSetMessageData called");
    return NULL;
}

void* FigTransportParcelSetMessageType(void) {
    if (verbose) puts("STUB: FigTransportParcelSetMessageType called");
    return NULL;
}

void* FigTransportRegisterObject(void) {
    if (verbose) puts("STUB: FigTransportRegisterObject called");
    return NULL;
}

void* FigTransportRegisterObjectWithFlags(void) {
    if (verbose) puts("STUB: FigTransportRegisterObjectWithFlags called");
    return NULL;
}

void* FigTransportSendAsyncMessage(void) {
    if (verbose) puts("STUB: FigTransportSendAsyncMessage called");
    return NULL;
}

void* FigTransportSendSyncMessageCreatingReply(void) {
    if (verbose) puts("STUB: FigTransportSendSyncMessageCreatingReply called");
    return NULL;
}

void* FigTransportServiceGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigTransportServiceGetCMBaseObject called");
    return NULL;
}

void* FigTransportServiceGetClassID(void) {
    if (verbose) puts("STUB: FigTransportServiceGetClassID called");
    return NULL;
}

void* FigTransportServiceGetTypeID(void) {
    if (verbose) puts("STUB: FigTransportServiceGetTypeID called");
    return NULL;
}

void* FigTransportSessionGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigTransportSessionGetCMBaseObject called");
    return NULL;
}

void* FigTransportSessionGetClassID(void) {
    if (verbose) puts("STUB: FigTransportSessionGetClassID called");
    return NULL;
}

void* FigTransportSessionGetTypeID(void) {
    if (verbose) puts("STUB: FigTransportSessionGetTypeID called");
    return NULL;
}

void* FigTransportSetLocalVersion(void) {
    if (verbose) puts("STUB: FigTransportSetLocalVersion called");
    return NULL;
}

void* FigTransportSetSharedTransport(void) {
    if (verbose) puts("STUB: FigTransportSetSharedTransport called");
    return NULL;
}

void* FigTransportStartAcceptingConnections(void) {
    if (verbose) puts("STUB: FigTransportStartAcceptingConnections called");
    return NULL;
}

void* FigTransportStopAcceptingConnections(void) {
    if (verbose) puts("STUB: FigTransportStopAcceptingConnections called");
    return NULL;
}

void* FigTransportStreamGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigTransportStreamGetCMBaseObject called");
    return NULL;
}

void* FigTransportStreamGetClassID(void) {
    if (verbose) puts("STUB: FigTransportStreamGetClassID called");
    return NULL;
}

void* FigTransportStreamGetTypeID(void) {
    if (verbose) puts("STUB: FigTransportStreamGetTypeID called");
    return NULL;
}

void* FigTransportUninitialize(void) {
    if (verbose) puts("STUB: FigTransportUninitialize called");
    return NULL;
}

void* FigTransportUnregisterObject(void) {
    if (verbose) puts("STUB: FigTransportUnregisterObject called");
    return NULL;
}

void* FigUInt32NearestPowerOfTwo(void) {
    if (verbose) puts("STUB: FigUInt32NearestPowerOfTwo called");
    return NULL;
}

void* FigUInt64MulDiv(void) {
    if (verbose) puts("STUB: FigUInt64MulDiv called");
    return NULL;
}

void* FigUSleep(void) {
    if (verbose) puts("STUB: FigUSleep called");
    return NULL;
}

void* FigUserBreak(void) {
    if (verbose) puts("STUB: FigUserBreak called");
    return NULL;
}

void* FigUserCrash(void) {
    if (verbose) puts("STUB: FigUserCrash called");
    return NULL;
}

void* FigVideoCreateBlockBufferAndFormatDescriptionForHoistingInStreamParameterSets(void) {
    if (verbose) puts("STUB: FigVideoCreateBlockBufferAndFormatDescriptionForHoistingInStreamParameterSets called");
    return NULL;
}

void* FigVideoFormatDescriptionContainsWideColor(void) {
    if (verbose) puts("STUB: FigVideoFormatDescriptionContainsWideColor called");
    return NULL;
}

void* FigVideoFormatDescriptionCopyAsBigEndianImageDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: FigVideoFormatDescriptionCopyAsBigEndianImageDescriptionBlockBuffer called");
    return NULL;
}

void* FigVideoFormatDescriptionCreateFromBigEndianImageDescriptionBlockBuffer(void) {
    if (verbose) puts("STUB: FigVideoFormatDescriptionCreateFromBigEndianImageDescriptionBlockBuffer called");
    return NULL;
}

void* FigVideoFormatDescriptionCreateFromBigEndianImageDescriptionData(void) {
    if (verbose) puts("STUB: FigVideoFormatDescriptionCreateFromBigEndianImageDescriptionData called");
    return NULL;
}

void* FigVideoFormatDescriptionCreateFromSPSAndPPS(void) {
    if (verbose) puts("STUB: FigVideoFormatDescriptionCreateFromSPSAndPPS called");
    return NULL;
}

void* FigVideoFormatDescriptionCreateKeysCommonWithImageBuffers(void) {
    if (verbose) puts("STUB: FigVideoFormatDescriptionCreateKeysCommonWithImageBuffers called");
    return NULL;
}

void* FigVideoFormatDescriptionCreateUpdatedFromHEVCParameterSets(void) {
    if (verbose) puts("STUB: FigVideoFormatDescriptionCreateUpdatedFromHEVCParameterSets called");
    return NULL;
}

void* FigVideoFormatDescriptionCreateWithSampleDescriptionExtensionAtom(void) {
    if (verbose) puts("STUB: FigVideoFormatDescriptionCreateWithSampleDescriptionExtensionAtom called");
    return NULL;
}

void* FigVideoFormatDescriptionGetDimensions(void) {
    if (verbose) puts("STUB: FigVideoFormatDescriptionGetDimensions called");
    return NULL;
}

void* FigVolumeControllerCopySharedControllerRemote(void) {
    if (verbose) puts("STUB: FigVolumeControllerCopySharedControllerRemote called");
    return NULL;
}

void* FigVolumeControllerGetCMBaseObject(void) {
    if (verbose) puts("STUB: FigVolumeControllerGetCMBaseObject called");
    return NULL;
}

void* FigVolumeControllerGetClassID(void) {
    if (verbose) puts("STUB: FigVolumeControllerGetClassID called");
    return NULL;
}

void* FigVolumeControllerGetLocalPlaybackVolume(void) {
    if (verbose) puts("STUB: FigVolumeControllerGetLocalPlaybackVolume called");
    return NULL;
}

void* FigVolumeControllerGetTypeID(void) {
    if (verbose) puts("STUB: FigVolumeControllerGetTypeID called");
    return NULL;
}

void* FigVolumeControllerSetLocalPlaybackVolume(void) {
    if (verbose) puts("STUB: FigVolumeControllerSetLocalPlaybackVolume called");
    return NULL;
}

void* FigVolumeControllerStartServer(void) {
    if (verbose) puts("STUB: FigVolumeControllerStartServer called");
    return NULL;
}

void* FigXPCConnectionCopyRemoteExecutableURL(void) {
    if (verbose) puts("STUB: FigXPCConnectionCopyRemoteExecutableURL called");
    return NULL;
}

void* FigXPCConnectionHasEntitlement(void) {
    if (verbose) puts("STUB: FigXPCConnectionHasEntitlement called");
    return NULL;
}

void* FigXPCCreateBasicMessage(void) {
    if (verbose) puts("STUB: FigXPCCreateBasicMessage called");
    return NULL;
}

void* FigXPCEndpointGroupServerRetainEndpointGroupForID(void) {
    if (verbose) puts("STUB: FigXPCEndpointGroupServerRetainEndpointGroupForID called");
    return NULL;
}

void* FigXPCEndpointServerRetainEndpointForID(void) {
    if (verbose) puts("STUB: FigXPCEndpointServerRetainEndpointForID called");
    return NULL;
}

void* FigXPCHandleStdCopyPropertyMessage(void) {
    if (verbose) puts("STUB: FigXPCHandleStdCopyPropertyMessage called");
    return NULL;
}

void* FigXPCHandleStdSetPropertyMessage(void) {
    if (verbose) puts("STUB: FigXPCHandleStdSetPropertyMessage called");
    return NULL;
}

void* FigXPCMessageCopyCFArray(void) {
    if (verbose) puts("STUB: FigXPCMessageCopyCFArray called");
    return NULL;
}

void* FigXPCMessageCopyCFData(void) {
    if (verbose) puts("STUB: FigXPCMessageCopyCFData called");
    return NULL;
}

void* FigXPCMessageCopyCFDate(void) {
    if (verbose) puts("STUB: FigXPCMessageCopyCFDate called");
    return NULL;
}

void* FigXPCMessageCopyCFDictionary(void) {
    if (verbose) puts("STUB: FigXPCMessageCopyCFDictionary called");
    return NULL;
}

void* FigXPCMessageCopyCFObject(void) {
    if (verbose) puts("STUB: FigXPCMessageCopyCFObject called");
    return NULL;
}

void* FigXPCMessageCopyCFString(void) {
    if (verbose) puts("STUB: FigXPCMessageCopyCFString called");
    return NULL;
}

void* FigXPCMessageCopyCFURL(void) {
    if (verbose) puts("STUB: FigXPCMessageCopyCFURL called");
    return NULL;
}

void* FigXPCMessageCopySenderExecutableURL(void) {
    if (verbose) puts("STUB: FigXPCMessageCopySenderExecutableURL called");
    return NULL;
}

void* FigXPCMessageCreateBlockBufferData(void) {
    if (verbose) puts("STUB: FigXPCMessageCreateBlockBufferData called");
    return NULL;
}

void* FigXPCMessageCreateBlockBufferDataWithSize(void) {
    if (verbose) puts("STUB: FigXPCMessageCreateBlockBufferDataWithSize called");
    return NULL;
}

void* FigXPCMessageGetCMTime(void) {
    if (verbose) puts("STUB: FigXPCMessageGetCMTime called");
    return NULL;
}

void* FigXPCMessageGetCMTimeMapping(void) {
    if (verbose) puts("STUB: FigXPCMessageGetCMTimeMapping called");
    return NULL;
}

void* FigXPCMessageGetCMTimeRange(void) {
    if (verbose) puts("STUB: FigXPCMessageGetCMTimeRange called");
    return NULL;
}

void* FigXPCMessageGetOpCode(void) {
    if (verbose) puts("STUB: FigXPCMessageGetOpCode called");
    return NULL;
}

void* FigXPCMessageSetAndConsumeVMData(void) {
    if (verbose) puts("STUB: FigXPCMessageSetAndConsumeVMData called");
    return NULL;
}

void* FigXPCMessageSetBlockBuffer(void) {
    if (verbose) puts("STUB: FigXPCMessageSetBlockBuffer called");
    return NULL;
}

void* FigXPCMessageSetCFArray(void) {
    if (verbose) puts("STUB: FigXPCMessageSetCFArray called");
    return NULL;
}

void* FigXPCMessageSetCFData(void) {
    if (verbose) puts("STUB: FigXPCMessageSetCFData called");
    return NULL;
}

void* FigXPCMessageSetCFDate(void) {
    if (verbose) puts("STUB: FigXPCMessageSetCFDate called");
    return NULL;
}

void* FigXPCMessageSetCFDictionary(void) {
    if (verbose) puts("STUB: FigXPCMessageSetCFDictionary called");
    return NULL;
}

void* FigXPCMessageSetCFObject(void) {
    if (verbose) puts("STUB: FigXPCMessageSetCFObject called");
    return NULL;
}

void* FigXPCMessageSetCFString(void) {
    if (verbose) puts("STUB: FigXPCMessageSetCFString called");
    return NULL;
}

void* FigXPCMessageSetCFURL(void) {
    if (verbose) puts("STUB: FigXPCMessageSetCFURL called");
    return NULL;
}

void* FigXPCMessageSetCMTime(void) {
    if (verbose) puts("STUB: FigXPCMessageSetCMTime called");
    return NULL;
}

void* FigXPCMessageSetCMTimeMapping(void) {
    if (verbose) puts("STUB: FigXPCMessageSetCMTimeMapping called");
    return NULL;
}

void* FigXPCMessageSetCMTimeRange(void) {
    if (verbose) puts("STUB: FigXPCMessageSetCMTimeRange called");
    return NULL;
}

void* FigXPCMessageSetVMData(void) {
    if (verbose) puts("STUB: FigXPCMessageSetVMData called");
    return NULL;
}

void* FigXPCRelease(void) {
    if (verbose) puts("STUB: FigXPCRelease called");
    return NULL;
}

void* FigXPCRemoteClientAssociateObject(void) {
    if (verbose) puts("STUB: FigXPCRemoteClientAssociateObject called");
    return NULL;
}

void* FigXPCRemoteClientCreate(void) {
    if (verbose) puts("STUB: FigXPCRemoteClientCreate called");
    return NULL;
}

void* FigXPCRemoteClientCreateWithXPCEndpoint(void) {
    if (verbose) puts("STUB: FigXPCRemoteClientCreateWithXPCEndpoint called");
    return NULL;
}

void* FigXPCRemoteClientCreateWithXPCService(void) {
    if (verbose) puts("STUB: FigXPCRemoteClientCreateWithXPCService called");
    return NULL;
}

void* FigXPCRemoteClientDisassociateObject(void) {
    if (verbose) puts("STUB: FigXPCRemoteClientDisassociateObject called");
    return NULL;
}

void* FigXPCRemoteClientDispose(void) {
    if (verbose) puts("STUB: FigXPCRemoteClientDispose called");
    return NULL;
}

void* FigXPCRemoteClientGetServerPID(void) {
    if (verbose) puts("STUB: FigXPCRemoteClientGetServerPID called");
    return NULL;
}

void* FigXPCRemoteClientKillServerOnTimeout(void) {
    if (verbose) puts("STUB: FigXPCRemoteClientKillServerOnTimeout called");
    return NULL;
}

void* FigXPCRemoteClientLookupAndRetainAssociatedObject(void) {
    if (verbose) puts("STUB: FigXPCRemoteClientLookupAndRetainAssociatedObject called");
    return NULL;
}

void* FigXPCRemoteClientRetainCopiedObject(void) {
    if (verbose) puts("STUB: FigXPCRemoteClientRetainCopiedObject called");
    return NULL;
}

void* FigXPCRemoteClientSendAsyncMessage(void) {
    if (verbose) puts("STUB: FigXPCRemoteClientSendAsyncMessage called");
    return NULL;
}

void* FigXPCRemoteClientSendSyncMessage(void) {
    if (verbose) puts("STUB: FigXPCRemoteClientSendSyncMessage called");
    return NULL;
}

void* FigXPCRemoteClientSendSyncMessageCreatingReply(void) {
    if (verbose) puts("STUB: FigXPCRemoteClientSendSyncMessageCreatingReply called");
    return NULL;
}

void* FigXPCRetain(void) {
    if (verbose) puts("STUB: FigXPCRetain called");
    return NULL;
}

void* FigXPCSendStdCopyPropertyMessage(void) {
    if (verbose) puts("STUB: FigXPCSendStdCopyPropertyMessage called");
    return NULL;
}

void* FigXPCSendStdSetPropertyMessage(void) {
    if (verbose) puts("STUB: FigXPCSendStdSetPropertyMessage called");
    return NULL;
}

void* FigXPCServerAssociateCopiedObjectWithConnection(void) {
    if (verbose) puts("STUB: FigXPCServerAssociateCopiedObjectWithConnection called");
    return NULL;
}

void* FigXPCServerAssociateCopiedObjectWithNeighborProcess(void) {
    if (verbose) puts("STUB: FigXPCServerAssociateCopiedObjectWithNeighborProcess called");
    return NULL;
}

void* FigXPCServerAssociateCopiedObjectWithNeighborProcessByPID(void) {
    if (verbose) puts("STUB: FigXPCServerAssociateCopiedObjectWithNeighborProcessByPID called");
    return NULL;
}

void* FigXPCServerAssociateObjectWithConnection(void) {
    if (verbose) puts("STUB: FigXPCServerAssociateObjectWithConnection called");
    return NULL;
}

void* FigXPCServerAssociateObjectWithNeighborProcess(void) {
    if (verbose) puts("STUB: FigXPCServerAssociateObjectWithNeighborProcess called");
    return NULL;
}

void* FigXPCServerAssociateObjectWithNeighborProcessByPID(void) {
    if (verbose) puts("STUB: FigXPCServerAssociateObjectWithNeighborProcessByPID called");
    return NULL;
}

void* FigXPCServerConfirmNeighborProcessByPID(void) {
    if (verbose) puts("STUB: FigXPCServerConfirmNeighborProcessByPID called");
    return NULL;
}

void* FigXPCServerCopyXPCEndpoint(void) {
    if (verbose) puts("STUB: FigXPCServerCopyXPCEndpoint called");
    return NULL;
}

void* FigXPCServerDisassociateObjectWithConnection(void) {
    if (verbose) puts("STUB: FigXPCServerDisassociateObjectWithConnection called");
    return NULL;
}

void* FigXPCServerDisassociateObjectWithNeighborClientConnection(void) {
    if (verbose) puts("STUB: FigXPCServerDisassociateObjectWithNeighborClientConnection called");
    return NULL;
}

void* FigXPCServerDisassociateObjectWithNeighborClientPID(void) {
    if (verbose) puts("STUB: FigXPCServerDisassociateObjectWithNeighborClientPID called");
    return NULL;
}

void* FigXPCServerGetConnectionRefcon(void) {
    if (verbose) puts("STUB: FigXPCServerGetConnectionRefcon called");
    return NULL;
}

void* FigXPCServerLookupAndRetainAssociatedObject(void) {
    if (verbose) puts("STUB: FigXPCServerLookupAndRetainAssociatedObject called");
    return NULL;
}

void* FigXPCServerRetainNeighborObjectFromID(void) {
    if (verbose) puts("STUB: FigXPCServerRetainNeighborObjectFromID called");
    return NULL;
}

void* FigXPCServerRetainNeighborObjectFromIDWithConnection(void) {
    if (verbose) puts("STUB: FigXPCServerRetainNeighborObjectFromIDWithConnection called");
    return NULL;
}

void* FigXPCServerRetainNeighborObjectFromIDWithProcessID(void) {
    if (verbose) puts("STUB: FigXPCServerRetainNeighborObjectFromIDWithProcessID called");
    return NULL;
}

void* FigXPCServerSetConnectionRefcon(void) {
    if (verbose) puts("STUB: FigXPCServerSetConnectionRefcon called");
    return NULL;
}

void* FigXPCServerStart(void) {
    if (verbose) puts("STUB: FigXPCServerStart called");
    return NULL;
}

void* FigXPCServerStartWithClientXPCConnection(void) {
    if (verbose) puts("STUB: FigXPCServerStartWithClientXPCConnection called");
    return NULL;
}

void* FigXPCServerStartWithNewXPCEndpoint(void) {
    if (verbose) puts("STUB: FigXPCServerStartWithNewXPCEndpoint called");
    return NULL;
}

void* H264Bridge_CopyDataWithEmulationPreventionBytesRemoved(void) {
    if (verbose) puts("STUB: H264Bridge_CopyDataWithEmulationPreventionBytesRemoved called");
    return NULL;
}

void* NeroTransportConnectToReceiver(void) {
    if (verbose) puts("STUB: NeroTransportConnectToReceiver called");
    return NULL;
}

void* NeroTransportConnectionGetCMBaseObject(void) {
    if (verbose) puts("STUB: NeroTransportConnectionGetCMBaseObject called");
    return NULL;
}

void* NeroTransportConnectionGetClassID(void) {
    if (verbose) puts("STUB: NeroTransportConnectionGetClassID called");
    return NULL;
}

void* NeroTransportConnectionGetTypeID(void) {
    if (verbose) puts("STUB: NeroTransportConnectionGetTypeID called");
    return NULL;
}

void* NeroTransportCopyRegisteredObject(void) {
    if (verbose) puts("STUB: NeroTransportCopyRegisteredObject called");
    return NULL;
}

void* NeroTransportCreate(void) {
    if (verbose) puts("STUB: NeroTransportCreate called");
    return NULL;
}

void* NeroTransportDisconnectFromReceiver(void) {
    if (verbose) puts("STUB: NeroTransportDisconnectFromReceiver called");
    return NULL;
}

void* NeroTransportGetTypeID(void) {
    if (verbose) puts("STUB: NeroTransportGetTypeID called");
    return NULL;
}

void* NeroTransportInvalidate(void) {
    if (verbose) puts("STUB: NeroTransportInvalidate called");
    return NULL;
}

void* NeroTransportIsConnected(void) {
    if (verbose) puts("STUB: NeroTransportIsConnected called");
    return NULL;
}

void* NeroTransportIsHiSpeed(void) {
    if (verbose) puts("STUB: NeroTransportIsHiSpeed called");
    return NULL;
}

void* NeroTransportRegisterObject(void) {
    if (verbose) puts("STUB: NeroTransportRegisterObject called");
    return NULL;
}

void* NeroTransportRegisterObjectWithFlags(void) {
    if (verbose) puts("STUB: NeroTransportRegisterObjectWithFlags called");
    return NULL;
}

void* NeroTransportSendAsyncMessage(void) {
    if (verbose) puts("STUB: NeroTransportSendAsyncMessage called");
    return NULL;
}

void* NeroTransportSendSyncMessageCreatingReply(void) {
    if (verbose) puts("STUB: NeroTransportSendSyncMessageCreatingReply called");
    return NULL;
}

void* NeroTransportStartAcceptingConnections(void) {
    if (verbose) puts("STUB: NeroTransportStartAcceptingConnections called");
    return NULL;
}

void* NeroTransportStopAcceptingConnections(void) {
    if (verbose) puts("STUB: NeroTransportStopAcceptingConnections called");
    return NULL;
}

void* NeroTransportUnregisterObject(void) {
    if (verbose) puts("STUB: NeroTransportUnregisterObject called");
    return NULL;
}

void* _FigIsCurrentDispatchQueue(void) {
    if (verbose) puts("STUB: _FigIsCurrentDispatchQueue called");
    return NULL;
}

void* _FigIsNotCurrentDispatchQueue(void) {
    if (verbose) puts("STUB: _FigIsNotCurrentDispatchQueue called");
    return NULL;
}

void* fig_get_epochTimeStamp(void) {
    if (verbose) puts("STUB: fig_get_epochTimeStamp called");
    return NULL;
}

void* fig_get_offsetTimeStamp(void) {
    if (verbose) puts("STUB: fig_get_offsetTimeStamp called");
    return NULL;
}

void* fig_get_timestamp(void) {
    if (verbose) puts("STUB: fig_get_timestamp called");
    return NULL;
}

void* fig_log(void) {
    if (verbose) puts("STUB: fig_log called");
    return NULL;
}

void* fig_logP(void) {
    if (verbose) puts("STUB: fig_logP called");
    return NULL;
}

void* fig_log_CF1(void) {
    if (verbose) puts("STUB: fig_log_CF1 called");
    return NULL;
}

void* fig_log_CF1P(void) {
    if (verbose) puts("STUB: fig_log_CF1P called");
    return NULL;
}

void* fig_log_CF2(void) {
    if (verbose) puts("STUB: fig_log_CF2 called");
    return NULL;
}

void* fig_log_handle(void) {
    if (verbose) puts("STUB: fig_log_handle called");
    return NULL;
}

void* fig_log_with_return_address(void) {
    if (verbose) puts("STUB: fig_log_with_return_address called");
    return NULL;
}

void* fig_note_initialize_autolog_work(void) {
    if (verbose) puts("STUB: fig_note_initialize_autolog_work called");
    return NULL;
}

void* fig_note_initialize_logging(void) {
    if (verbose) puts("STUB: fig_note_initialize_logging called");
    return NULL;
}

void* fig_note_initialize_timestamps(void) {
    if (verbose) puts("STUB: fig_note_initialize_timestamps called");
    return NULL;
}

void* fig_note_initialize_work(void) {
    if (verbose) puts("STUB: fig_note_initialize_work called");
    return NULL;
}

void* fig_post_a_symptom_guts(void) {
    if (verbose) puts("STUB: fig_post_a_symptom_guts called");
    return NULL;
}
