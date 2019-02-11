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


#ifndef _ImageIO_H_
#define _ImageIO_H_

#include <CoreFoundation/CoreFoundation.h>

#define IMAGEIO_EXTERN extern

IMAGEIO_EXTERN const CFStringRef kCGImageDestinationLossyCompressionQuality;
IMAGEIO_EXTERN const CFStringRef kCGImageSourceCreateThumbnailFromImageAlways;
IMAGEIO_EXTERN const CFStringRef kCGImageSourceThumbnailMaxPixelSize;

void* CGCopyBasicPropertiesOfImageAtPath(void);
void* CGImageCopyFileWithGPSInformation(void);
void* CGImageCopyFileWithParameters(void);
void* CGImageCopyJPEGData(void);
void* CGImageCopyJPEGDataAndColorSpace(void);
void* CGImageCopySourceData(void);
void* CGImageCreateByMatchingToColorSpace(void);
void* CGImageCreateByMatchingToDisplay(void);
void* CGImageCreateByScaling(void);
void* CGImageCreateColorSpaceFromDataArray(void);
void* CGImageCreateCopyWithAlphaInfo(void);
void* CGImageCreateCopyWithParameters(void);
void* CGImageCreateEXIFDataFromProperties(void);
void* CGImageCreateEXIFJPEGData(void);
void* CGImageCreateFromIOSurface(void);
void* CGImageCreateMetadataFromDataArray(void);
void* CGImageCreateThumb(void);
void* CGImageCreateWithWBMPData(void);
void* CGImageDestinationAddAuxiliaryDataInfo(void);
void* CGImageDestinationAddImage(void);
void* CGImageDestinationAddImageAndMetadata(void);
void* CGImageDestinationAddImageFromSource(void);
void* CGImageDestinationAddImageFromSourceWithMetadata(void);
void* CGImageDestinationAddImageWithMetadata(void);
void* CGImageDestinationCopyImageFormats(void);
void* CGImageDestinationCopyImageSource(void);
void* CGImageDestinationCopyTypeIdentifiers(void);
void* CGImageDestinationCreateWithData(void);
void* CGImageDestinationCreateWithDataConsumer(void);
void* CGImageDestinationCreateWithURL(void);
void* CGImageDestinationFinalize(void);
void* CGImageDestinationGetTypeID(void);
void* CGImageDestinationSetProperties(void);
void* CGImageIOCreateXMPDataFromMetaData(void);
void* CGImageIsInsideSRGB(void);
void* CGImageMetadataCopyStringValueWithPath(void);
void* CGImageMetadataCopyTagMatchingImageProperty(void);
void* CGImageMetadataCopyTagWithPath(void);
void* CGImageMetadataCopyTags(void);
void* CGImageMetadataCreateFromData(void);
void* CGImageMetadataCreateFromMetadataProperties(void);
void* CGImageMetadataCreateFromXMPData(void);
void* CGImageMetadataCreateMetadataProperties(void);
void* CGImageMetadataCreateMutable(void);
void* CGImageMetadataCreateMutableCopy(void);
void* CGImageMetadataCreateSerializedData(void);
void* CGImageMetadataCreateSidecar(void);
void* CGImageMetadataCreateXMPData(void);
void* CGImageMetadataEnumerateTagsUsingBlock(void);
void* CGImageMetadataGetTypeID(void);
void* CGImageMetadataPropertyCreate(void);
void* CGImageMetadataPropertyGetPropertyName(void);
void* CGImageMetadataPropertyGetSchema(void);
void* CGImageMetadataPropertyGetSuggestedPrefix(void);
void* CGImageMetadataPropertyGetTypeID(void);
void* CGImageMetadataPropertyGetValue(void);
void* CGImageMetadataPropertySetValue(void);
void* CGImageMetadataRegisterNamespaceForPrefix(void);
void* CGImageMetadataRemoveTagWithPath(void);
void* CGImageMetadataSerialize(void);
void* CGImageMetadataSetTagWithPath(void);
void* CGImageMetadataSetValueMatchingImageProperty(void);
void* CGImageMetadataSetValueWithPath(void);
void* CGImageMetadataTagCopyName(void);
void* CGImageMetadataTagCopyNamespace(void);
void* CGImageMetadataTagCopyPrefix(void);
void* CGImageMetadataTagCopyQualifiers(void);
void* CGImageMetadataTagCopyValue(void);
void* CGImageMetadataTagCreate(void);
void* CGImageMetadataTagGetType(void);
void* CGImageMetadataTagGetTypeID(void);
void* CGImageMetadataValueAddQualifier(void);
void* CGImageMetadataValueCreate(void);
void* CGImageMetadataValueGetNamespaceURI(void);
void* CGImageMetadataValueGetQualifiers(void);
void* CGImageMetadataValueGetSuggestedNamespacePrefix(void);
void* CGImageMetadataValueGetTypeID(void);
void* CGImageMetadataValueGetValue(void);
void* CGImageMetadataValueIsAlternate(void);
void* CGImageMetadataValueIsAlternateText(void);
void* CGImageMetadataValueSetIsAlternate(void);
void* CGImageMetadataValueSetIsAlternateText(void);
void* CGImageMetadataValueSetNamespaceURI(void);
void* CGImageMetadataValueSetSuggestedNamespacePrefix(void);
void* CGImagePluginGetMatchToProfileOption(void);
void* CGImagePluginGetMetadata(void);
void* CGImagePluginGetOptions(void);
void* CGImagePluginGetProperties(void);
void* CGImagePluginGetSession(void);
void* CGImagePluginInitJPEGAtOffset(void);
void* CGImagePluginInitThumbJPEGAtOffset(void);
void* CGImagePluginReadRawIPTCProps(void);
void* CGImagePluginReadRawXMPProps(void);
void* CGImagePluginReadRawXMPSidecarProps(void);
void* CGImagePluginSetClipPath(void);
void* CGImagePluginSetImage(void);
void* CGImagePluginSetImageBlockProc(void);
void* CGImagePluginSetJpegProvider(void);
void* CGImageReadSessionCreateFILE(void);
void* CGImageReadSessionGetBytePointer(void);
void* CGImageReadSessionGetBytesAtOffset(void);
void* CGImageReadSessionGetFilePath(void);
void* CGImageReadSessionGetSize(void);
void* CGImageReadSessionGetTypeID(void);
void* CGImageReadSessionIsFinal(void);
void* CGImageReadSessionReleaseBytePointer(void);
void* CGImageReadSessionReleaseCachedImageBlockData(void);
void* CGImageSaveToFile(void);
void* CGImageSourceAddProperty(void);
void* CGImageSourceCopyAuxiliaryDataInfoAtIndex(void);
void* CGImageSourceCopyMetadataAtIndex(void);
void* CGImageSourceCopyMetadataPropertiesAtIndex(void);
void* CGImageSourceCopyProperties(void);
void* CGImageSourceCopyPropertiesAtIndex(void);
void* CGImageSourceCopyTypeExtensions(void);
void* CGImageSourceCopyTypeIdentifiers(void);
void* CGImageSourceCreateIOSurfaceAtIndex(void);
void* CGImageSourceCreateImageAtIndex(void);
void* CGImageSourceCreateIncremental(void);
void* CGImageSourceCreateThumbnailAtIndex(void);
void* CGImageSourceCreateWithData(void);
void* CGImageSourceCreateWithDataProvider(void);
void* CGImageSourceCreateWithFile(void);
void* CGImageSourceCreateWithURL(void);
void* CGImageSourceGetCount(void);
void* CGImageSourceGetImageCacheAtIndex(void);
void* CGImageSourceGetPrimaryImageIndex(void);
void* CGImageSourceGetStatus(void);
void* CGImageSourceGetStatusAtIndex(void);
void* CGImageSourceGetType(void);
void* CGImageSourceGetTypeID(void);
void* CGImageSourceGetTypeWithData(void);
void* CGImageSourceGetTypeWithDataProvider(void);
void* CGImageSourceGetTypeWithExtension(void);
void* CGImageSourceGetTypeWithFile(void);
void* CGImageSourceGetTypeWithURL(void);
void* CGImageSourceIsColorOptimizedForSharing(void);
void* CGImageSourceRemoveCacheAtIndex(void);
void* CGImageSourceUpdateData(void);
void* CGImageSourceUpdateDataProvider(void);
void* CGImageWriteEXIFJPEGToPath(void);
void* CGImageWriteEXIFJPEGWithMetadata(void);
void* CopyMetadataFromFileAtPath(void);
void* CopyMetadataFromSequentialDataProviderCallbacks(void);
void* GetMetadataPropertyMatchingSchemaAndPropertyName(void);
void* ImageIODebug(void);
void* ImageIOSetLoggingProc(void);

#endif
