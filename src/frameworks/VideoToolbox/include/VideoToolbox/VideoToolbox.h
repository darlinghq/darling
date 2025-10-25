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


#ifndef _VideoToolbox_H_
#define _VideoToolbox_H_

#import <Foundation/Foundation.h>
#import <CoreFoundation/CFBase.h>
#import <VideoToolbox/FigMetalTransferHelper.h>

extern const CFStringRef kVTCompressionPropertyKey_DataRateLimits;
extern const CFStringRef kVTCompressionPropertyKey_ExpectedFrameRate;
extern const CFStringRef kVTCompressionPropertyKey_MaxFrameDelayCount;
extern const CFStringRef kVTCompressionPropertyKey_MaxKeyFrameIntervalDuration;
extern const CFStringRef kVTProfileLevel_H264_High_AutoLevel;
extern const CFStringRef kVTVideoDecoderSpecification_RequireHardwareAcceleratedVideoDecoder;
extern const CFStringRef kVTVideoEncoderSpecification_RequireHardwareAcceleratedVideoEncoder;

void* COLOR_Y2R24(void);
void* COLOR_Y2R32(void);
void* COLOR_Y420uToUYVY_scalar(void);
void* COLOR_Y420uToY422(void);
void* H263QT_InitSharedGlobals(void);
void* H263QT_KillSharedGlobals(void);
void* H264VideoDecoder_CreateInstance(void);
void* JVTLibDecoDispose(void);
void* MPEG4VideoDecoder_CreateInstance(void);
void* VCH263VideoDecoder_CreateInstance(void);
void* VCH263VideoEncoder_CreateInstance(void);
void* VTBuildPixelBufferPools(void);
void* VTBuildPixelBufferPools2(void);
void* VTCompressionSessionBeginPass(void);
void* VTCompressionSessionCompleteFrames(void);
void* VTCompressionSessionCopyProperty(void);
void* VTCompressionSessionCopySerializableProperties(void);
void* VTCompressionSessionCopySupportedPropertyDictionary(void);
void* VTCompressionSessionCreate(void);
void* VTCompressionSessionEncodeFrame(void);
void* VTCompressionSessionEncodeFrameWithOutputHandler(void);
void* VTCompressionSessionEndPass(void);
void* VTCompressionSessionGetPixelBufferPool(void);
void* VTCompressionSessionGetTimeRangesForNextPass(void);
void* VTCompressionSessionGetTypeID(void);
void* VTCompressionSessionInvalidate(void);
void* VTCompressionSessionPrepareToEncodeFrames(void);
void* VTCompressionSessionRelease(void);
void* VTCompressionSessionSetProperties(void);
void* VTCompressionSessionSetProperty(void);
void* VTCopyHEVCDecoderCapabilitiesDictionary(void);
void* VTCopyIORegistryValueForKeyFromDeviceRegistryID(void);
void* VTCopyMetalDeviceArray(void);
void* VTCopyRenderIDArrayForIORegistryKey(void);
void* VTCopySupportedPropertyDictionaryForEncoder(void);
void* VTCopySupportedPropertyForVideoEncoderID(void);
void* VTCopyTemporalFilterList(void);
void* VTCopyVideoEncoderList(void);
void* VTCreateCGImageFromCVPixelBuffer(void);
void* VTCreateCGImageFromIOSurfaceAndAttributes(void);
void* VTCreateDictionaryRepresentationForMovedRect(void);
void* VTCreatePixelBufferPoolAttributesWithName(void);
void* VTDecoderSessionEmitDecodedFrame(void);
void* VTDecoderSessionEmitDecodedTile(void);
void* VTDecoderSessionGetDestinationPixelBufferAttributes(void);
void* VTDecoderSessionGetPixelBufferPool(void);
void* VTDecoderSessionSetPixelBufferAttributes(void);
void* VTDecoderSessionTrace(void);
void* VTDecompressionSessionCanAcceptFormatDescription(void);
void* VTDecompressionSessionCopyBlackPixelBuffer(void);
void* VTDecompressionSessionCopyProperty(void);
void* VTDecompressionSessionCopySerializableProperties(void);
void* VTDecompressionSessionCopySupportedPropertyDictionary(void);
void* VTDecompressionSessionCreate(void);
void* VTDecompressionSessionCreateWithOptions(void);
void* VTDecompressionSessionDecodeFrame(void);
void* VTDecompressionSessionDecodeFrameWithOutputHandler(void);
void* VTDecompressionSessionFinishDelayedFrames(void);
void* VTDecompressionSessionGetMinAndMaxOutputPresentationTimeStampOfFramesBeingDecoded(void);
void* VTDecompressionSessionGetMinOutputPresentationTimeStampOfFramesBeingDecoded(void);
void* VTDecompressionSessionGetTypeID(void);
void* VTDecompressionSessionInvalidate(void);
void* VTDecompressionSessionSetProperties(void);
void* VTDecompressionSessionSetProperty(void);
void* VTDecompressionSessionWaitForAsynchronousFrames(void);
void* VTDistributedCompressionGetSegmentRanges(void);
void* VTDistributedCompressionGetSegmentsToReencode(void);
void* VTDistributedPreprocessingGetOverlap(void);
void* VTDoesAnyDeviceSupportIORegistryKey(void);
void* VTDoesMetalDeviceSupportIORegistryKey(void);
void* VTEncoderPreprocessingSessionEmitPreprocessedFrame(void);
void* VTEncoderPreprocessingSessionGetResolutionOutputPixelBufferPool(void);
void* VTEncoderPreprocessingSessionSetResolutionOutputPixelBufferAttributes(void);
void* VTEncoderPreprocessingSessionSetSourcePixelBufferAttributes(void);
void* VTEncoderSessionCreateVideoFormatDescription(void);
void* VTEncoderSessionCreateVideoFormatDescriptionFromHEVCParameterSets(void);
void* VTEncoderSessionDequeueDecodeTimeStamp(void);
void* VTEncoderSessionEmitEncodedFrame(void);
void* VTEncoderSessionEmitEncodedTile(void);
void* VTEncoderSessionEnqueuePresentationTimeStamp(void);
void* VTEncoderSessionSetPixelBufferAttributes(void);
void* VTEncoderSessionSetTimeRangesForNextPass(void);
void* VTFillPixelBufferBorderWithBlack(void);
void* VTFillPixelBufferWithBlack(void);
void* VTFrameSiloAddSampleBuffer(void);
void* VTFrameSiloCallBlockForEachSampleBuffer(void);
void* VTFrameSiloCallFunctionForEachSampleBuffer(void);
void* VTFrameSiloCreate(void);
void* VTFrameSiloGetProgressOfCurrentPass(void);
void* VTFrameSiloGetTypeID(void);
void* VTFrameSiloInvalidate(void);
void* VTFrameSiloSetTimeRangesForNextPass(void);
void* VTGetGVADecoderAvailability(void);
void* VTGetHEVCCapabilitesForFormatDescription(void);
void* VTImageRotationSessionCopyProperty(void);
void* VTImageRotationSessionCopySerializableProperties(void);
void* VTImageRotationSessionCopySupportedPropertyDictionary(void);
void* VTImageRotationSessionCreate(void);
void* VTImageRotationSessionGetTypeID(void);
void* VTImageRotationSessionInvalidate(void);
void* VTImageRotationSessionRelease(void);
void* VTImageRotationSessionRetain(void);
void* VTImageRotationSessionSetProperties(void);
void* VTImageRotationSessionSetProperty(void);
void* VTImageRotationSessionTransferImage(void);
void* VTImageRotationSessionTransferSubImage(void);
void* VTIsHardwareDecodeSupported(void);
void* VTIsMetalDeviceExternal(void);
void* VTIsMetalDeviceSlotted(void);
void* VTMakeMovedRectWithDictionaryRepresentation(void);
void* VTMetalTransferSessionCreate(void);
void* VTMetalTransferSessionTransferImageSync(void);
void* VTMultiPassStorageClose(void);
void* VTMultiPassStorageCopyDataAtTimeStamp(void);
void* VTMultiPassStorageCopyIdentifier(void);
void* VTMultiPassStorageCreate(void);
void* VTMultiPassStorageGetTimeStamp(void);
void* VTMultiPassStorageGetTimeStampAndDuration(void);
void* VTMultiPassStorageGetTypeID(void);
void* VTMultiPassStorageInvalidate(void);
void* VTMultiPassStorageSetDataAtTimeStamp(void);
void* VTMultiPassStorageSetIdentifier(void);
void* VTMultiPassStorageWriteMerged(void);
void* VTMultiPassStorageWriteSegment(void);
void* VTPixelBufferAttributesMediatorAddLayer(void);
void* VTPixelBufferAttributesMediatorCopyProperty(void);
void* VTPixelBufferAttributesMediatorCreate(void);
void* VTPixelBufferAttributesMediatorGetTypeID(void);
void* VTPixelBufferAttributesMediatorNotificationBarrier(void);
void* VTPixelBufferAttributesMediatorRemoveLayer(void);
void* VTPixelBufferAttributesMediatorRemoveRequestedPixelBufferAttributesForKey(void);
void* VTPixelBufferAttributesMediatorSetProperty(void);
void* VTPixelBufferAttributesMediatorSetRequestedPixelBufferAttributesForKey(void);
void* VTPixelBufferConformerCopyBlackPixelBuffer(void);
void* VTPixelBufferConformerCopyConformedPixelBuffer(void);
void* VTPixelBufferConformerCreateWithAttributes(void);
void* VTPixelBufferConformerGetAttributes(void);
void* VTPixelBufferConformerGetColorProperties(void);
void* VTPixelBufferConformerGetTypeID(void);
void* VTPixelBufferConformerIsConformantPixelBuffer(void);
void* VTPixelBufferConformerSetAttributes(void);
void* VTPixelBufferConformerSetColorProperties(void);
void* VTPixelTransferSessionCanTransfer(void);
void* VTPixelTransferSessionCopyProperty(void);
void* VTPixelTransferSessionCopySerializableProperties(void);
void* VTPixelTransferSessionCopySupportedPropertyDictionary(void);
void* VTPixelTransferSessionCreate(void);
void* VTPixelTransferSessionCreateScalingParameters(void);
void* VTPixelTransferSessionGetTypeID(void);
void* VTPixelTransferSessionInvalidate(void);
void* VTPixelTransferSessionSetProperties(void);
void* VTPixelTransferSessionSetProperty(void);
void* VTPixelTransferSessionTransferImage(void);
void* VTPreprocessingSessionAddResolution(void);
void* VTPreprocessingSessionCompleteFrames(void);
void* VTPreprocessingSessionCopyProperty(void);
void* VTPreprocessingSessionCopySerializableProperties(void);
void* VTPreprocessingSessionCopySupportedPropertyDictionary(void);
void* VTPreprocessingSessionCreate(void);
void* VTPreprocessingSessionGetTypeID(void);
void* VTPreprocessingSessionInvalidate(void);
void* VTPreprocessingSessionPreprocessFrame(void);
void* VTPreprocessingSessionSetProperties(void);
void* VTPreprocessingSessionSetProperty(void);
void* VTRegisterGVAPixelTransferCapability(void);
void* VTRegisterPixelTransferCapability(void);
void* VTRegisterProfessionalVideoWorkflowVideoDecoders(void);
void* VTRegisterProfessionalVideoWorkflowVideoEncoders(void);
void* VTRegisterVideoDecoder(void);
void* VTRegisterVideoDecoderBundleDirectory(void);
void* VTRegisterVideoDecoderWithInfo(void);
void* VTRegisterVideoEncoder(void);
void* VTRegisterVideoEncoderBundleDirectory(void);
void* VTRegisterVideoEncoderWithInfo(void);
void* VTRemoteCompressionServer_ServiceConnectionHandler(void);
void* VTRemoteDecompressionServer_ServiceEventHandler(void);
void* VTRemoteVideoDecoderGetClassID(void);
void* VTRemoteVideoEncoderGetClassID(void);
void* VTSelectAndCreateTemporalFilterInstance(void);
void* VTSelectAndCreateVideoDecoderInstance(void);
void* VTSelectAndCreateVideoEncoderInstance(void);
void* VTSessionCopyProperty(void);
void* VTSessionCopySerializableProperties(void);
void* VTSessionCopySupportedPropertyDictionary(void);
void* VTSessionGetCallbacksWithTypeID(void);
void* VTSessionRegisterCallbacksForTypeID(void);
void* VTSessionSetProperties(void);
void* VTSessionSetProperty(void);
void* VTTemporalFilterPluginGetCMBaseObject(void);
void* VTTemporalFilterPluginGetClassID(void);
void* VTTemporalFilterPluginGetTypeID(void);
void* VTTemporalFilterPluginSessionConcludeInputFrame(void);
void* VTTemporalFilterPluginSessionEmitOutputFrame(void);
void* VTTemporalFilterPluginSessionGetClientInputPixelBufferAttributesHint(void);
void* VTTemporalFilterPluginSessionGetClientOutputPixelBufferAttributes(void);
void* VTTemporalFilterPluginSessionGetOutputPixelBufferPool(void);
void* VTTemporalFilterPluginSessionSetInputPixelBufferAttributes(void);
void* VTTemporalFilterPluginSessionSetOutputPixelBufferAttributes(void);
void* VTTemporalFilterSessionCompleteFrames(void);
void* VTTemporalFilterSessionCopyProperty(void);
void* VTTemporalFilterSessionCopySerializableProperties(void);
void* VTTemporalFilterSessionCopySupportedPropertyDictionary(void);
void* VTTemporalFilterSessionCreate(void);
void* VTTemporalFilterSessionGetTypeID(void);
void* VTTemporalFilterSessionInvalidate(void);
void* VTTemporalFilterSessionProcessFrame(void);
void* VTTemporalFilterSessionSessionSetProperty(void);
void* VTTemporalFilterSessionSetProperties(void);
void* VTTileCompressionSessionCompleteTiles(void);
void* VTTileCompressionSessionCopyProperty(void);
void* VTTileCompressionSessionCopySerializableProperties(void);
void* VTTileCompressionSessionCopySupportedPropertyDictionary(void);
void* VTTileCompressionSessionCreate(void);
void* VTTileCompressionSessionEncodeTile(void);
void* VTTileCompressionSessionGetTypeID(void);
void* VTTileCompressionSessionInvalidate(void);
void* VTTileCompressionSessionPrepareToEncodeTiles(void);
void* VTTileCompressionSessionSetProperties(void);
void* VTTileCompressionSessionSetProperty(void);
void* VTTileDecoderSessionEmitDecodedTile(void);
void* VTTileDecoderSessionSetTileDecodeRequirements(void);
void* VTTileDecompressionSessionCopyProperty(void);
void* VTTileDecompressionSessionCopySerializableProperties(void);
void* VTTileDecompressionSessionCopySupportedPropertyDictionary(void);
void* VTTileDecompressionSessionCreate(void);
void* VTTileDecompressionSessionDecodeTile(void);
void* VTTileDecompressionSessionFinishDelayedTiles(void);
void* VTTileDecompressionSessionGetTypeID(void);
void* VTTileDecompressionSessionInvalidate(void);
void* VTTileDecompressionSessionSetProperties(void);
void* VTTileDecompressionSessionSetProperty(void);
void* VTTileEncoderSessionCreateVideoFormatDescription(void);
void* VTTileEncoderSessionSetTileAttributes(void);
void* VTTileEncoderSessionSetTileEncodeRequirements(void);
void* VTVideoDecoderGetCMBaseObject(void);
void* VTVideoDecoderGetClassID(void);
void* VTVideoDecoderGetFigBaseObject(void);
void* VTVideoDecoderGetTypeID(void);
void* VTVideoEncoderGetCMBaseObject(void);
void* VTVideoEncoderGetClassID(void);
void* VTVideoEncoderGetFigBaseObject(void);
void* VTVideoEncoderGetTypeID(void);
void* VTXPCMessageCopyCMSampleBuffer(void);
void* VTXPCMessageCopyCVPixelBuffer(void);
void* VTXPCMessageCopyVideoFormatDescription(void);
void* VTXPCMessageSetCMSampleBuffer(void);
void* VTXPCMessageSetCVPixelBuffer(void);
void* VTXPCMessageSetVideoFormatDescription(void);

#endif
