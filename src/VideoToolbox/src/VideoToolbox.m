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


#include <VideoToolbox/VideoToolbox.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* COLOR_Y2R24(void)
{
    if (verbose) puts("STUB: COLOR_Y2R24 called");
    return NULL;
}

void* COLOR_Y2R32(void)
{
    if (verbose) puts("STUB: COLOR_Y2R32 called");
    return NULL;
}

void* COLOR_Y420uToUYVY_scalar(void)
{
    if (verbose) puts("STUB: COLOR_Y420uToUYVY_scalar called");
    return NULL;
}

void* COLOR_Y420uToY422(void)
{
    if (verbose) puts("STUB: COLOR_Y420uToY422 called");
    return NULL;
}

void* H263QT_InitSharedGlobals(void)
{
    if (verbose) puts("STUB: H263QT_InitSharedGlobals called");
    return NULL;
}

void* H263QT_KillSharedGlobals(void)
{
    if (verbose) puts("STUB: H263QT_KillSharedGlobals called");
    return NULL;
}

void* H264VideoDecoder_CreateInstance(void)
{
    if (verbose) puts("STUB: H264VideoDecoder_CreateInstance called");
    return NULL;
}

void* JVTLibDecoDispose(void)
{
    if (verbose) puts("STUB: JVTLibDecoDispose called");
    return NULL;
}

void* MPEG4VideoDecoder_CreateInstance(void)
{
    if (verbose) puts("STUB: MPEG4VideoDecoder_CreateInstance called");
    return NULL;
}

void* VCH263VideoDecoder_CreateInstance(void)
{
    if (verbose) puts("STUB: VCH263VideoDecoder_CreateInstance called");
    return NULL;
}

void* VCH263VideoEncoder_CreateInstance(void)
{
    if (verbose) puts("STUB: VCH263VideoEncoder_CreateInstance called");
    return NULL;
}

void* VTBuildPixelBufferPools(void)
{
    if (verbose) puts("STUB: VTBuildPixelBufferPools called");
    return NULL;
}

void* VTBuildPixelBufferPools2(void)
{
    if (verbose) puts("STUB: VTBuildPixelBufferPools2 called");
    return NULL;
}

void* VTCompressionSessionBeginPass(void)
{
    if (verbose) puts("STUB: VTCompressionSessionBeginPass called");
    return NULL;
}

void* VTCompressionSessionCompleteFrames(void)
{
    if (verbose) puts("STUB: VTCompressionSessionCompleteFrames called");
    return NULL;
}

void* VTCompressionSessionCopyProperty(void)
{
    if (verbose) puts("STUB: VTCompressionSessionCopyProperty called");
    return NULL;
}

void* VTCompressionSessionCopySerializableProperties(void)
{
    if (verbose) puts("STUB: VTCompressionSessionCopySerializableProperties called");
    return NULL;
}

void* VTCompressionSessionCopySupportedPropertyDictionary(void)
{
    if (verbose) puts("STUB: VTCompressionSessionCopySupportedPropertyDictionary called");
    return NULL;
}

void* VTCompressionSessionCreate(void)
{
    if (verbose) puts("STUB: VTCompressionSessionCreate called");
    return NULL;
}

void* VTCompressionSessionEncodeFrame(void)
{
    if (verbose) puts("STUB: VTCompressionSessionEncodeFrame called");
    return NULL;
}

void* VTCompressionSessionEncodeFrameWithOutputHandler(void)
{
    if (verbose) puts("STUB: VTCompressionSessionEncodeFrameWithOutputHandler called");
    return NULL;
}

void* VTCompressionSessionEndPass(void)
{
    if (verbose) puts("STUB: VTCompressionSessionEndPass called");
    return NULL;
}

void* VTCompressionSessionGetPixelBufferPool(void)
{
    if (verbose) puts("STUB: VTCompressionSessionGetPixelBufferPool called");
    return NULL;
}

void* VTCompressionSessionGetTimeRangesForNextPass(void)
{
    if (verbose) puts("STUB: VTCompressionSessionGetTimeRangesForNextPass called");
    return NULL;
}

void* VTCompressionSessionGetTypeID(void)
{
    if (verbose) puts("STUB: VTCompressionSessionGetTypeID called");
    return NULL;
}

void* VTCompressionSessionInvalidate(void)
{
    if (verbose) puts("STUB: VTCompressionSessionInvalidate called");
    return NULL;
}

void* VTCompressionSessionPrepareToEncodeFrames(void)
{
    if (verbose) puts("STUB: VTCompressionSessionPrepareToEncodeFrames called");
    return NULL;
}

void* VTCompressionSessionRelease(void)
{
    if (verbose) puts("STUB: VTCompressionSessionRelease called");
    return NULL;
}

void* VTCompressionSessionSetProperties(void)
{
    if (verbose) puts("STUB: VTCompressionSessionSetProperties called");
    return NULL;
}

void* VTCompressionSessionSetProperty(void)
{
    if (verbose) puts("STUB: VTCompressionSessionSetProperty called");
    return NULL;
}

void* VTCopyHEVCDecoderCapabilitiesDictionary(void)
{
    if (verbose) puts("STUB: VTCopyHEVCDecoderCapabilitiesDictionary called");
    return NULL;
}

void* VTCopyIORegistryValueForKeyFromDeviceRegistryID(void)
{
    if (verbose) puts("STUB: VTCopyIORegistryValueForKeyFromDeviceRegistryID called");
    return NULL;
}

void* VTCopyMetalDeviceArray(void)
{
    if (verbose) puts("STUB: VTCopyMetalDeviceArray called");
    return NULL;
}

void* VTCopyRenderIDArrayForIORegistryKey(void)
{
    if (verbose) puts("STUB: VTCopyRenderIDArrayForIORegistryKey called");
    return NULL;
}

void* VTCopySupportedPropertyDictionaryForEncoder(void)
{
    if (verbose) puts("STUB: VTCopySupportedPropertyDictionaryForEncoder called");
    return NULL;
}

void* VTCopySupportedPropertyForVideoEncoderID(void)
{
    if (verbose) puts("STUB: VTCopySupportedPropertyForVideoEncoderID called");
    return NULL;
}

void* VTCopyTemporalFilterList(void)
{
    if (verbose) puts("STUB: VTCopyTemporalFilterList called");
    return NULL;
}

void* VTCopyVideoEncoderList(void)
{
    if (verbose) puts("STUB: VTCopyVideoEncoderList called");
    return NULL;
}

void* VTCreateCGImageFromCVPixelBuffer(void)
{
    if (verbose) puts("STUB: VTCreateCGImageFromCVPixelBuffer called");
    return NULL;
}

void* VTCreateCGImageFromIOSurfaceAndAttributes(void)
{
    if (verbose) puts("STUB: VTCreateCGImageFromIOSurfaceAndAttributes called");
    return NULL;
}

void* VTCreateDictionaryRepresentationForMovedRect(void)
{
    if (verbose) puts("STUB: VTCreateDictionaryRepresentationForMovedRect called");
    return NULL;
}

void* VTCreatePixelBufferPoolAttributesWithName(void)
{
    if (verbose) puts("STUB: VTCreatePixelBufferPoolAttributesWithName called");
    return NULL;
}

void* VTDecoderSessionEmitDecodedFrame(void)
{
    if (verbose) puts("STUB: VTDecoderSessionEmitDecodedFrame called");
    return NULL;
}

void* VTDecoderSessionEmitDecodedTile(void)
{
    if (verbose) puts("STUB: VTDecoderSessionEmitDecodedTile called");
    return NULL;
}

void* VTDecoderSessionGetDestinationPixelBufferAttributes(void)
{
    if (verbose) puts("STUB: VTDecoderSessionGetDestinationPixelBufferAttributes called");
    return NULL;
}

void* VTDecoderSessionGetPixelBufferPool(void)
{
    if (verbose) puts("STUB: VTDecoderSessionGetPixelBufferPool called");
    return NULL;
}

void* VTDecoderSessionSetPixelBufferAttributes(void)
{
    if (verbose) puts("STUB: VTDecoderSessionSetPixelBufferAttributes called");
    return NULL;
}

void* VTDecoderSessionTrace(void)
{
    if (verbose) puts("STUB: VTDecoderSessionTrace called");
    return NULL;
}

void* VTDecompressionSessionCanAcceptFormatDescription(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionCanAcceptFormatDescription called");
    return NULL;
}

void* VTDecompressionSessionCopyBlackPixelBuffer(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionCopyBlackPixelBuffer called");
    return NULL;
}

void* VTDecompressionSessionCopyProperty(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionCopyProperty called");
    return NULL;
}

void* VTDecompressionSessionCopySerializableProperties(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionCopySerializableProperties called");
    return NULL;
}

void* VTDecompressionSessionCopySupportedPropertyDictionary(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionCopySupportedPropertyDictionary called");
    return NULL;
}

void* VTDecompressionSessionCreate(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionCreate called");
    return NULL;
}

void* VTDecompressionSessionCreateWithOptions(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionCreateWithOptions called");
    return NULL;
}

void* VTDecompressionSessionDecodeFrame(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionDecodeFrame called");
    return NULL;
}

void* VTDecompressionSessionDecodeFrameWithOutputHandler(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionDecodeFrameWithOutputHandler called");
    return NULL;
}

void* VTDecompressionSessionFinishDelayedFrames(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionFinishDelayedFrames called");
    return NULL;
}

void* VTDecompressionSessionGetMinAndMaxOutputPresentationTimeStampOfFramesBeingDecoded(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionGetMinAndMaxOutputPresentationTimeStampOfFramesBeingDecoded called");
    return NULL;
}

void* VTDecompressionSessionGetMinOutputPresentationTimeStampOfFramesBeingDecoded(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionGetMinOutputPresentationTimeStampOfFramesBeingDecoded called");
    return NULL;
}

void* VTDecompressionSessionGetTypeID(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionGetTypeID called");
    return NULL;
}

void* VTDecompressionSessionInvalidate(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionInvalidate called");
    return NULL;
}

void* VTDecompressionSessionSetProperties(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionSetProperties called");
    return NULL;
}

void* VTDecompressionSessionSetProperty(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionSetProperty called");
    return NULL;
}

void* VTDecompressionSessionWaitForAsynchronousFrames(void)
{
    if (verbose) puts("STUB: VTDecompressionSessionWaitForAsynchronousFrames called");
    return NULL;
}

void* VTDistributedCompressionGetSegmentRanges(void)
{
    if (verbose) puts("STUB: VTDistributedCompressionGetSegmentRanges called");
    return NULL;
}

void* VTDistributedCompressionGetSegmentsToReencode(void)
{
    if (verbose) puts("STUB: VTDistributedCompressionGetSegmentsToReencode called");
    return NULL;
}

void* VTDistributedPreprocessingGetOverlap(void)
{
    if (verbose) puts("STUB: VTDistributedPreprocessingGetOverlap called");
    return NULL;
}

void* VTDoesAnyDeviceSupportIORegistryKey(void)
{
    if (verbose) puts("STUB: VTDoesAnyDeviceSupportIORegistryKey called");
    return NULL;
}

void* VTDoesMetalDeviceSupportIORegistryKey(void)
{
    if (verbose) puts("STUB: VTDoesMetalDeviceSupportIORegistryKey called");
    return NULL;
}

void* VTEncoderPreprocessingSessionEmitPreprocessedFrame(void)
{
    if (verbose) puts("STUB: VTEncoderPreprocessingSessionEmitPreprocessedFrame called");
    return NULL;
}

void* VTEncoderPreprocessingSessionGetResolutionOutputPixelBufferPool(void)
{
    if (verbose) puts("STUB: VTEncoderPreprocessingSessionGetResolutionOutputPixelBufferPool called");
    return NULL;
}

void* VTEncoderPreprocessingSessionSetResolutionOutputPixelBufferAttributes(void)
{
    if (verbose) puts("STUB: VTEncoderPreprocessingSessionSetResolutionOutputPixelBufferAttributes called");
    return NULL;
}

void* VTEncoderPreprocessingSessionSetSourcePixelBufferAttributes(void)
{
    if (verbose) puts("STUB: VTEncoderPreprocessingSessionSetSourcePixelBufferAttributes called");
    return NULL;
}

void* VTEncoderSessionCreateVideoFormatDescription(void)
{
    if (verbose) puts("STUB: VTEncoderSessionCreateVideoFormatDescription called");
    return NULL;
}

void* VTEncoderSessionCreateVideoFormatDescriptionFromHEVCParameterSets(void)
{
    if (verbose) puts("STUB: VTEncoderSessionCreateVideoFormatDescriptionFromHEVCParameterSets called");
    return NULL;
}

void* VTEncoderSessionDequeueDecodeTimeStamp(void)
{
    if (verbose) puts("STUB: VTEncoderSessionDequeueDecodeTimeStamp called");
    return NULL;
}

void* VTEncoderSessionEmitEncodedFrame(void)
{
    if (verbose) puts("STUB: VTEncoderSessionEmitEncodedFrame called");
    return NULL;
}

void* VTEncoderSessionEmitEncodedTile(void)
{
    if (verbose) puts("STUB: VTEncoderSessionEmitEncodedTile called");
    return NULL;
}

void* VTEncoderSessionEnqueuePresentationTimeStamp(void)
{
    if (verbose) puts("STUB: VTEncoderSessionEnqueuePresentationTimeStamp called");
    return NULL;
}

void* VTEncoderSessionSetPixelBufferAttributes(void)
{
    if (verbose) puts("STUB: VTEncoderSessionSetPixelBufferAttributes called");
    return NULL;
}

void* VTEncoderSessionSetTimeRangesForNextPass(void)
{
    if (verbose) puts("STUB: VTEncoderSessionSetTimeRangesForNextPass called");
    return NULL;
}

void* VTFillPixelBufferBorderWithBlack(void)
{
    if (verbose) puts("STUB: VTFillPixelBufferBorderWithBlack called");
    return NULL;
}

void* VTFillPixelBufferWithBlack(void)
{
    if (verbose) puts("STUB: VTFillPixelBufferWithBlack called");
    return NULL;
}

void* VTFrameSiloAddSampleBuffer(void)
{
    if (verbose) puts("STUB: VTFrameSiloAddSampleBuffer called");
    return NULL;
}

void* VTFrameSiloCallBlockForEachSampleBuffer(void)
{
    if (verbose) puts("STUB: VTFrameSiloCallBlockForEachSampleBuffer called");
    return NULL;
}

void* VTFrameSiloCallFunctionForEachSampleBuffer(void)
{
    if (verbose) puts("STUB: VTFrameSiloCallFunctionForEachSampleBuffer called");
    return NULL;
}

void* VTFrameSiloCreate(void)
{
    if (verbose) puts("STUB: VTFrameSiloCreate called");
    return NULL;
}

void* VTFrameSiloGetProgressOfCurrentPass(void)
{
    if (verbose) puts("STUB: VTFrameSiloGetProgressOfCurrentPass called");
    return NULL;
}

void* VTFrameSiloGetTypeID(void)
{
    if (verbose) puts("STUB: VTFrameSiloGetTypeID called");
    return NULL;
}

void* VTFrameSiloInvalidate(void)
{
    if (verbose) puts("STUB: VTFrameSiloInvalidate called");
    return NULL;
}

void* VTFrameSiloSetTimeRangesForNextPass(void)
{
    if (verbose) puts("STUB: VTFrameSiloSetTimeRangesForNextPass called");
    return NULL;
}

void* VTGetGVADecoderAvailability(void)
{
    if (verbose) puts("STUB: VTGetGVADecoderAvailability called");
    return NULL;
}

void* VTGetHEVCCapabilitesForFormatDescription(void)
{
    if (verbose) puts("STUB: VTGetHEVCCapabilitesForFormatDescription called");
    return NULL;
}

void* VTImageRotationSessionCopyProperty(void)
{
    if (verbose) puts("STUB: VTImageRotationSessionCopyProperty called");
    return NULL;
}

void* VTImageRotationSessionCopySerializableProperties(void)
{
    if (verbose) puts("STUB: VTImageRotationSessionCopySerializableProperties called");
    return NULL;
}

void* VTImageRotationSessionCopySupportedPropertyDictionary(void)
{
    if (verbose) puts("STUB: VTImageRotationSessionCopySupportedPropertyDictionary called");
    return NULL;
}

void* VTImageRotationSessionCreate(void)
{
    if (verbose) puts("STUB: VTImageRotationSessionCreate called");
    return NULL;
}

void* VTImageRotationSessionGetTypeID(void)
{
    if (verbose) puts("STUB: VTImageRotationSessionGetTypeID called");
    return NULL;
}

void* VTImageRotationSessionInvalidate(void)
{
    if (verbose) puts("STUB: VTImageRotationSessionInvalidate called");
    return NULL;
}

void* VTImageRotationSessionRelease(void)
{
    if (verbose) puts("STUB: VTImageRotationSessionRelease called");
    return NULL;
}

void* VTImageRotationSessionRetain(void)
{
    if (verbose) puts("STUB: VTImageRotationSessionRetain called");
    return NULL;
}

void* VTImageRotationSessionSetProperties(void)
{
    if (verbose) puts("STUB: VTImageRotationSessionSetProperties called");
    return NULL;
}

void* VTImageRotationSessionSetProperty(void)
{
    if (verbose) puts("STUB: VTImageRotationSessionSetProperty called");
    return NULL;
}

void* VTImageRotationSessionTransferImage(void)
{
    if (verbose) puts("STUB: VTImageRotationSessionTransferImage called");
    return NULL;
}

void* VTImageRotationSessionTransferSubImage(void)
{
    if (verbose) puts("STUB: VTImageRotationSessionTransferSubImage called");
    return NULL;
}

void* VTIsHardwareDecodeSupported(void)
{
    if (verbose) puts("STUB: VTIsHardwareDecodeSupported called");
    return NULL;
}

void* VTIsMetalDeviceExternal(void)
{
    if (verbose) puts("STUB: VTIsMetalDeviceExternal called");
    return NULL;
}

void* VTIsMetalDeviceSlotted(void)
{
    if (verbose) puts("STUB: VTIsMetalDeviceSlotted called");
    return NULL;
}

void* VTMakeMovedRectWithDictionaryRepresentation(void)
{
    if (verbose) puts("STUB: VTMakeMovedRectWithDictionaryRepresentation called");
    return NULL;
}

void* VTMetalTransferSessionCreate(void)
{
    if (verbose) puts("STUB: VTMetalTransferSessionCreate called");
    return NULL;
}

void* VTMetalTransferSessionTransferImageSync(void)
{
    if (verbose) puts("STUB: VTMetalTransferSessionTransferImageSync called");
    return NULL;
}

void* VTMultiPassStorageClose(void)
{
    if (verbose) puts("STUB: VTMultiPassStorageClose called");
    return NULL;
}

void* VTMultiPassStorageCopyDataAtTimeStamp(void)
{
    if (verbose) puts("STUB: VTMultiPassStorageCopyDataAtTimeStamp called");
    return NULL;
}

void* VTMultiPassStorageCopyIdentifier(void)
{
    if (verbose) puts("STUB: VTMultiPassStorageCopyIdentifier called");
    return NULL;
}

void* VTMultiPassStorageCreate(void)
{
    if (verbose) puts("STUB: VTMultiPassStorageCreate called");
    return NULL;
}

void* VTMultiPassStorageGetTimeStamp(void)
{
    if (verbose) puts("STUB: VTMultiPassStorageGetTimeStamp called");
    return NULL;
}

void* VTMultiPassStorageGetTimeStampAndDuration(void)
{
    if (verbose) puts("STUB: VTMultiPassStorageGetTimeStampAndDuration called");
    return NULL;
}

void* VTMultiPassStorageGetTypeID(void)
{
    if (verbose) puts("STUB: VTMultiPassStorageGetTypeID called");
    return NULL;
}

void* VTMultiPassStorageInvalidate(void)
{
    if (verbose) puts("STUB: VTMultiPassStorageInvalidate called");
    return NULL;
}

void* VTMultiPassStorageSetDataAtTimeStamp(void)
{
    if (verbose) puts("STUB: VTMultiPassStorageSetDataAtTimeStamp called");
    return NULL;
}

void* VTMultiPassStorageSetIdentifier(void)
{
    if (verbose) puts("STUB: VTMultiPassStorageSetIdentifier called");
    return NULL;
}

void* VTMultiPassStorageWriteMerged(void)
{
    if (verbose) puts("STUB: VTMultiPassStorageWriteMerged called");
    return NULL;
}

void* VTMultiPassStorageWriteSegment(void)
{
    if (verbose) puts("STUB: VTMultiPassStorageWriteSegment called");
    return NULL;
}

void* VTPixelBufferAttributesMediatorAddLayer(void)
{
    if (verbose) puts("STUB: VTPixelBufferAttributesMediatorAddLayer called");
    return NULL;
}

void* VTPixelBufferAttributesMediatorCopyProperty(void)
{
    if (verbose) puts("STUB: VTPixelBufferAttributesMediatorCopyProperty called");
    return NULL;
}

void* VTPixelBufferAttributesMediatorCreate(void)
{
    if (verbose) puts("STUB: VTPixelBufferAttributesMediatorCreate called");
    return NULL;
}

void* VTPixelBufferAttributesMediatorGetTypeID(void)
{
    if (verbose) puts("STUB: VTPixelBufferAttributesMediatorGetTypeID called");
    return NULL;
}

void* VTPixelBufferAttributesMediatorNotificationBarrier(void)
{
    if (verbose) puts("STUB: VTPixelBufferAttributesMediatorNotificationBarrier called");
    return NULL;
}

void* VTPixelBufferAttributesMediatorRemoveLayer(void)
{
    if (verbose) puts("STUB: VTPixelBufferAttributesMediatorRemoveLayer called");
    return NULL;
}

void* VTPixelBufferAttributesMediatorRemoveRequestedPixelBufferAttributesForKey(void)
{
    if (verbose) puts("STUB: VTPixelBufferAttributesMediatorRemoveRequestedPixelBufferAttributesForKey called");
    return NULL;
}

void* VTPixelBufferAttributesMediatorSetProperty(void)
{
    if (verbose) puts("STUB: VTPixelBufferAttributesMediatorSetProperty called");
    return NULL;
}

void* VTPixelBufferAttributesMediatorSetRequestedPixelBufferAttributesForKey(void)
{
    if (verbose) puts("STUB: VTPixelBufferAttributesMediatorSetRequestedPixelBufferAttributesForKey called");
    return NULL;
}

void* VTPixelBufferConformerCopyBlackPixelBuffer(void)
{
    if (verbose) puts("STUB: VTPixelBufferConformerCopyBlackPixelBuffer called");
    return NULL;
}

void* VTPixelBufferConformerCopyConformedPixelBuffer(void)
{
    if (verbose) puts("STUB: VTPixelBufferConformerCopyConformedPixelBuffer called");
    return NULL;
}

void* VTPixelBufferConformerCreateWithAttributes(void)
{
    if (verbose) puts("STUB: VTPixelBufferConformerCreateWithAttributes called");
    return NULL;
}

void* VTPixelBufferConformerGetAttributes(void)
{
    if (verbose) puts("STUB: VTPixelBufferConformerGetAttributes called");
    return NULL;
}

void* VTPixelBufferConformerGetColorProperties(void)
{
    if (verbose) puts("STUB: VTPixelBufferConformerGetColorProperties called");
    return NULL;
}

void* VTPixelBufferConformerGetTypeID(void)
{
    if (verbose) puts("STUB: VTPixelBufferConformerGetTypeID called");
    return NULL;
}

void* VTPixelBufferConformerIsConformantPixelBuffer(void)
{
    if (verbose) puts("STUB: VTPixelBufferConformerIsConformantPixelBuffer called");
    return NULL;
}

void* VTPixelBufferConformerSetAttributes(void)
{
    if (verbose) puts("STUB: VTPixelBufferConformerSetAttributes called");
    return NULL;
}

void* VTPixelBufferConformerSetColorProperties(void)
{
    if (verbose) puts("STUB: VTPixelBufferConformerSetColorProperties called");
    return NULL;
}

void* VTPixelTransferSessionCanTransfer(void)
{
    if (verbose) puts("STUB: VTPixelTransferSessionCanTransfer called");
    return NULL;
}

void* VTPixelTransferSessionCopyProperty(void)
{
    if (verbose) puts("STUB: VTPixelTransferSessionCopyProperty called");
    return NULL;
}

void* VTPixelTransferSessionCopySerializableProperties(void)
{
    if (verbose) puts("STUB: VTPixelTransferSessionCopySerializableProperties called");
    return NULL;
}

void* VTPixelTransferSessionCopySupportedPropertyDictionary(void)
{
    if (verbose) puts("STUB: VTPixelTransferSessionCopySupportedPropertyDictionary called");
    return NULL;
}

void* VTPixelTransferSessionCreate(void)
{
    if (verbose) puts("STUB: VTPixelTransferSessionCreate called");
    return NULL;
}

void* VTPixelTransferSessionCreateScalingParameters(void)
{
    if (verbose) puts("STUB: VTPixelTransferSessionCreateScalingParameters called");
    return NULL;
}

void* VTPixelTransferSessionGetTypeID(void)
{
    if (verbose) puts("STUB: VTPixelTransferSessionGetTypeID called");
    return NULL;
}

void* VTPixelTransferSessionInvalidate(void)
{
    if (verbose) puts("STUB: VTPixelTransferSessionInvalidate called");
    return NULL;
}

void* VTPixelTransferSessionSetProperties(void)
{
    if (verbose) puts("STUB: VTPixelTransferSessionSetProperties called");
    return NULL;
}

void* VTPixelTransferSessionSetProperty(void)
{
    if (verbose) puts("STUB: VTPixelTransferSessionSetProperty called");
    return NULL;
}

void* VTPixelTransferSessionTransferImage(void)
{
    if (verbose) puts("STUB: VTPixelTransferSessionTransferImage called");
    return NULL;
}

void* VTPreprocessingSessionAddResolution(void)
{
    if (verbose) puts("STUB: VTPreprocessingSessionAddResolution called");
    return NULL;
}

void* VTPreprocessingSessionCompleteFrames(void)
{
    if (verbose) puts("STUB: VTPreprocessingSessionCompleteFrames called");
    return NULL;
}

void* VTPreprocessingSessionCopyProperty(void)
{
    if (verbose) puts("STUB: VTPreprocessingSessionCopyProperty called");
    return NULL;
}

void* VTPreprocessingSessionCopySerializableProperties(void)
{
    if (verbose) puts("STUB: VTPreprocessingSessionCopySerializableProperties called");
    return NULL;
}

void* VTPreprocessingSessionCopySupportedPropertyDictionary(void)
{
    if (verbose) puts("STUB: VTPreprocessingSessionCopySupportedPropertyDictionary called");
    return NULL;
}

void* VTPreprocessingSessionCreate(void)
{
    if (verbose) puts("STUB: VTPreprocessingSessionCreate called");
    return NULL;
}

void* VTPreprocessingSessionGetTypeID(void)
{
    if (verbose) puts("STUB: VTPreprocessingSessionGetTypeID called");
    return NULL;
}

void* VTPreprocessingSessionInvalidate(void)
{
    if (verbose) puts("STUB: VTPreprocessingSessionInvalidate called");
    return NULL;
}

void* VTPreprocessingSessionPreprocessFrame(void)
{
    if (verbose) puts("STUB: VTPreprocessingSessionPreprocessFrame called");
    return NULL;
}

void* VTPreprocessingSessionSetProperties(void)
{
    if (verbose) puts("STUB: VTPreprocessingSessionSetProperties called");
    return NULL;
}

void* VTPreprocessingSessionSetProperty(void)
{
    if (verbose) puts("STUB: VTPreprocessingSessionSetProperty called");
    return NULL;
}

void* VTRegisterGVAPixelTransferCapability(void)
{
    if (verbose) puts("STUB: VTRegisterGVAPixelTransferCapability called");
    return NULL;
}

void* VTRegisterPixelTransferCapability(void)
{
    if (verbose) puts("STUB: VTRegisterPixelTransferCapability called");
    return NULL;
}

void* VTRegisterProfessionalVideoWorkflowVideoDecoders(void)
{
    if (verbose) puts("STUB: VTRegisterProfessionalVideoWorkflowVideoDecoders called");
    return NULL;
}

void* VTRegisterProfessionalVideoWorkflowVideoEncoders(void)
{
    if (verbose) puts("STUB: VTRegisterProfessionalVideoWorkflowVideoEncoders called");
    return NULL;
}

void* VTRegisterVideoDecoder(void)
{
    if (verbose) puts("STUB: VTRegisterVideoDecoder called");
    return NULL;
}

void* VTRegisterVideoDecoderBundleDirectory(void)
{
    if (verbose) puts("STUB: VTRegisterVideoDecoderBundleDirectory called");
    return NULL;
}

void* VTRegisterVideoDecoderWithInfo(void)
{
    if (verbose) puts("STUB: VTRegisterVideoDecoderWithInfo called");
    return NULL;
}

void* VTRegisterVideoEncoder(void)
{
    if (verbose) puts("STUB: VTRegisterVideoEncoder called");
    return NULL;
}

void* VTRegisterVideoEncoderBundleDirectory(void)
{
    if (verbose) puts("STUB: VTRegisterVideoEncoderBundleDirectory called");
    return NULL;
}

void* VTRegisterVideoEncoderWithInfo(void)
{
    if (verbose) puts("STUB: VTRegisterVideoEncoderWithInfo called");
    return NULL;
}

void* VTRemoteCompressionServer_ServiceConnectionHandler(void)
{
    if (verbose) puts("STUB: VTRemoteCompressionServer_ServiceConnectionHandler called");
    return NULL;
}

void* VTRemoteDecompressionServer_ServiceEventHandler(void)
{
    if (verbose) puts("STUB: VTRemoteDecompressionServer_ServiceEventHandler called");
    return NULL;
}

void* VTRemoteVideoDecoderGetClassID(void)
{
    if (verbose) puts("STUB: VTRemoteVideoDecoderGetClassID called");
    return NULL;
}

void* VTRemoteVideoEncoderGetClassID(void)
{
    if (verbose) puts("STUB: VTRemoteVideoEncoderGetClassID called");
    return NULL;
}

void* VTSelectAndCreateTemporalFilterInstance(void)
{
    if (verbose) puts("STUB: VTSelectAndCreateTemporalFilterInstance called");
    return NULL;
}

void* VTSelectAndCreateVideoDecoderInstance(void)
{
    if (verbose) puts("STUB: VTSelectAndCreateVideoDecoderInstance called");
    return NULL;
}

void* VTSelectAndCreateVideoEncoderInstance(void)
{
    if (verbose) puts("STUB: VTSelectAndCreateVideoEncoderInstance called");
    return NULL;
}

void* VTSessionCopyProperty(void)
{
    if (verbose) puts("STUB: VTSessionCopyProperty called");
    return NULL;
}

void* VTSessionCopySerializableProperties(void)
{
    if (verbose) puts("STUB: VTSessionCopySerializableProperties called");
    return NULL;
}

void* VTSessionCopySupportedPropertyDictionary(void)
{
    if (verbose) puts("STUB: VTSessionCopySupportedPropertyDictionary called");
    return NULL;
}

void* VTSessionGetCallbacksWithTypeID(void)
{
    if (verbose) puts("STUB: VTSessionGetCallbacksWithTypeID called");
    return NULL;
}

void* VTSessionRegisterCallbacksForTypeID(void)
{
    if (verbose) puts("STUB: VTSessionRegisterCallbacksForTypeID called");
    return NULL;
}

void* VTSessionSetProperties(void)
{
    if (verbose) puts("STUB: VTSessionSetProperties called");
    return NULL;
}

void* VTSessionSetProperty(void)
{
    if (verbose) puts("STUB: VTSessionSetProperty called");
    return NULL;
}

void* VTTemporalFilterPluginGetCMBaseObject(void)
{
    if (verbose) puts("STUB: VTTemporalFilterPluginGetCMBaseObject called");
    return NULL;
}

void* VTTemporalFilterPluginGetClassID(void)
{
    if (verbose) puts("STUB: VTTemporalFilterPluginGetClassID called");
    return NULL;
}

void* VTTemporalFilterPluginGetTypeID(void)
{
    if (verbose) puts("STUB: VTTemporalFilterPluginGetTypeID called");
    return NULL;
}

void* VTTemporalFilterPluginSessionConcludeInputFrame(void)
{
    if (verbose) puts("STUB: VTTemporalFilterPluginSessionConcludeInputFrame called");
    return NULL;
}

void* VTTemporalFilterPluginSessionEmitOutputFrame(void)
{
    if (verbose) puts("STUB: VTTemporalFilterPluginSessionEmitOutputFrame called");
    return NULL;
}

void* VTTemporalFilterPluginSessionGetClientInputPixelBufferAttributesHint(void)
{
    if (verbose) puts("STUB: VTTemporalFilterPluginSessionGetClientInputPixelBufferAttributesHint called");
    return NULL;
}

void* VTTemporalFilterPluginSessionGetClientOutputPixelBufferAttributes(void)
{
    if (verbose) puts("STUB: VTTemporalFilterPluginSessionGetClientOutputPixelBufferAttributes called");
    return NULL;
}

void* VTTemporalFilterPluginSessionGetOutputPixelBufferPool(void)
{
    if (verbose) puts("STUB: VTTemporalFilterPluginSessionGetOutputPixelBufferPool called");
    return NULL;
}

void* VTTemporalFilterPluginSessionSetInputPixelBufferAttributes(void)
{
    if (verbose) puts("STUB: VTTemporalFilterPluginSessionSetInputPixelBufferAttributes called");
    return NULL;
}

void* VTTemporalFilterPluginSessionSetOutputPixelBufferAttributes(void)
{
    if (verbose) puts("STUB: VTTemporalFilterPluginSessionSetOutputPixelBufferAttributes called");
    return NULL;
}

void* VTTemporalFilterSessionCompleteFrames(void)
{
    if (verbose) puts("STUB: VTTemporalFilterSessionCompleteFrames called");
    return NULL;
}

void* VTTemporalFilterSessionCopyProperty(void)
{
    if (verbose) puts("STUB: VTTemporalFilterSessionCopyProperty called");
    return NULL;
}

void* VTTemporalFilterSessionCopySerializableProperties(void)
{
    if (verbose) puts("STUB: VTTemporalFilterSessionCopySerializableProperties called");
    return NULL;
}

void* VTTemporalFilterSessionCopySupportedPropertyDictionary(void)
{
    if (verbose) puts("STUB: VTTemporalFilterSessionCopySupportedPropertyDictionary called");
    return NULL;
}

void* VTTemporalFilterSessionCreate(void)
{
    if (verbose) puts("STUB: VTTemporalFilterSessionCreate called");
    return NULL;
}

void* VTTemporalFilterSessionGetTypeID(void)
{
    if (verbose) puts("STUB: VTTemporalFilterSessionGetTypeID called");
    return NULL;
}

void* VTTemporalFilterSessionInvalidate(void)
{
    if (verbose) puts("STUB: VTTemporalFilterSessionInvalidate called");
    return NULL;
}

void* VTTemporalFilterSessionProcessFrame(void)
{
    if (verbose) puts("STUB: VTTemporalFilterSessionProcessFrame called");
    return NULL;
}

void* VTTemporalFilterSessionSessionSetProperty(void)
{
    if (verbose) puts("STUB: VTTemporalFilterSessionSessionSetProperty called");
    return NULL;
}

void* VTTemporalFilterSessionSetProperties(void)
{
    if (verbose) puts("STUB: VTTemporalFilterSessionSetProperties called");
    return NULL;
}

void* VTTileCompressionSessionCompleteTiles(void)
{
    if (verbose) puts("STUB: VTTileCompressionSessionCompleteTiles called");
    return NULL;
}

void* VTTileCompressionSessionCopyProperty(void)
{
    if (verbose) puts("STUB: VTTileCompressionSessionCopyProperty called");
    return NULL;
}

void* VTTileCompressionSessionCopySerializableProperties(void)
{
    if (verbose) puts("STUB: VTTileCompressionSessionCopySerializableProperties called");
    return NULL;
}

void* VTTileCompressionSessionCopySupportedPropertyDictionary(void)
{
    if (verbose) puts("STUB: VTTileCompressionSessionCopySupportedPropertyDictionary called");
    return NULL;
}

void* VTTileCompressionSessionCreate(void)
{
    if (verbose) puts("STUB: VTTileCompressionSessionCreate called");
    return NULL;
}

void* VTTileCompressionSessionEncodeTile(void)
{
    if (verbose) puts("STUB: VTTileCompressionSessionEncodeTile called");
    return NULL;
}

void* VTTileCompressionSessionGetTypeID(void)
{
    if (verbose) puts("STUB: VTTileCompressionSessionGetTypeID called");
    return NULL;
}

void* VTTileCompressionSessionInvalidate(void)
{
    if (verbose) puts("STUB: VTTileCompressionSessionInvalidate called");
    return NULL;
}

void* VTTileCompressionSessionPrepareToEncodeTiles(void)
{
    if (verbose) puts("STUB: VTTileCompressionSessionPrepareToEncodeTiles called");
    return NULL;
}

void* VTTileCompressionSessionSetProperties(void)
{
    if (verbose) puts("STUB: VTTileCompressionSessionSetProperties called");
    return NULL;
}

void* VTTileCompressionSessionSetProperty(void)
{
    if (verbose) puts("STUB: VTTileCompressionSessionSetProperty called");
    return NULL;
}

void* VTTileDecoderSessionEmitDecodedTile(void)
{
    if (verbose) puts("STUB: VTTileDecoderSessionEmitDecodedTile called");
    return NULL;
}

void* VTTileDecoderSessionSetTileDecodeRequirements(void)
{
    if (verbose) puts("STUB: VTTileDecoderSessionSetTileDecodeRequirements called");
    return NULL;
}

void* VTTileDecompressionSessionCopyProperty(void)
{
    if (verbose) puts("STUB: VTTileDecompressionSessionCopyProperty called");
    return NULL;
}

void* VTTileDecompressionSessionCopySerializableProperties(void)
{
    if (verbose) puts("STUB: VTTileDecompressionSessionCopySerializableProperties called");
    return NULL;
}

void* VTTileDecompressionSessionCopySupportedPropertyDictionary(void)
{
    if (verbose) puts("STUB: VTTileDecompressionSessionCopySupportedPropertyDictionary called");
    return NULL;
}

void* VTTileDecompressionSessionCreate(void)
{
    if (verbose) puts("STUB: VTTileDecompressionSessionCreate called");
    return NULL;
}

void* VTTileDecompressionSessionDecodeTile(void)
{
    if (verbose) puts("STUB: VTTileDecompressionSessionDecodeTile called");
    return NULL;
}

void* VTTileDecompressionSessionFinishDelayedTiles(void)
{
    if (verbose) puts("STUB: VTTileDecompressionSessionFinishDelayedTiles called");
    return NULL;
}

void* VTTileDecompressionSessionGetTypeID(void)
{
    if (verbose) puts("STUB: VTTileDecompressionSessionGetTypeID called");
    return NULL;
}

void* VTTileDecompressionSessionInvalidate(void)
{
    if (verbose) puts("STUB: VTTileDecompressionSessionInvalidate called");
    return NULL;
}

void* VTTileDecompressionSessionSetProperties(void)
{
    if (verbose) puts("STUB: VTTileDecompressionSessionSetProperties called");
    return NULL;
}

void* VTTileDecompressionSessionSetProperty(void)
{
    if (verbose) puts("STUB: VTTileDecompressionSessionSetProperty called");
    return NULL;
}

void* VTTileEncoderSessionCreateVideoFormatDescription(void)
{
    if (verbose) puts("STUB: VTTileEncoderSessionCreateVideoFormatDescription called");
    return NULL;
}

void* VTTileEncoderSessionSetTileAttributes(void)
{
    if (verbose) puts("STUB: VTTileEncoderSessionSetTileAttributes called");
    return NULL;
}

void* VTTileEncoderSessionSetTileEncodeRequirements(void)
{
    if (verbose) puts("STUB: VTTileEncoderSessionSetTileEncodeRequirements called");
    return NULL;
}

void* VTVideoDecoderGetCMBaseObject(void)
{
    if (verbose) puts("STUB: VTVideoDecoderGetCMBaseObject called");
    return NULL;
}

void* VTVideoDecoderGetClassID(void)
{
    if (verbose) puts("STUB: VTVideoDecoderGetClassID called");
    return NULL;
}

void* VTVideoDecoderGetFigBaseObject(void)
{
    if (verbose) puts("STUB: VTVideoDecoderGetFigBaseObject called");
    return NULL;
}

void* VTVideoDecoderGetTypeID(void)
{
    if (verbose) puts("STUB: VTVideoDecoderGetTypeID called");
    return NULL;
}

void* VTVideoEncoderGetCMBaseObject(void)
{
    if (verbose) puts("STUB: VTVideoEncoderGetCMBaseObject called");
    return NULL;
}

void* VTVideoEncoderGetClassID(void)
{
    if (verbose) puts("STUB: VTVideoEncoderGetClassID called");
    return NULL;
}

void* VTVideoEncoderGetFigBaseObject(void)
{
    if (verbose) puts("STUB: VTVideoEncoderGetFigBaseObject called");
    return NULL;
}

void* VTVideoEncoderGetTypeID(void)
{
    if (verbose) puts("STUB: VTVideoEncoderGetTypeID called");
    return NULL;
}

void* VTXPCMessageCopyCMSampleBuffer(void)
{
    if (verbose) puts("STUB: VTXPCMessageCopyCMSampleBuffer called");
    return NULL;
}

void* VTXPCMessageCopyCVPixelBuffer(void)
{
    if (verbose) puts("STUB: VTXPCMessageCopyCVPixelBuffer called");
    return NULL;
}

void* VTXPCMessageCopyVideoFormatDescription(void)
{
    if (verbose) puts("STUB: VTXPCMessageCopyVideoFormatDescription called");
    return NULL;
}

void* VTXPCMessageSetCMSampleBuffer(void)
{
    if (verbose) puts("STUB: VTXPCMessageSetCMSampleBuffer called");
    return NULL;
}

void* VTXPCMessageSetCVPixelBuffer(void)
{
    if (verbose) puts("STUB: VTXPCMessageSetCVPixelBuffer called");
    return NULL;
}

void* VTXPCMessageSetVideoFormatDescription(void)
{
    if (verbose) puts("STUB: VTXPCMessageSetVideoFormatDescription called");
    return NULL;
}
