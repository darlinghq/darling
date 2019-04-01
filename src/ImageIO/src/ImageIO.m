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


#include <ImageIO/ImageIO.h>
#include <stdlib.h>
#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>

const CFStringRef kCGImageDestinationLossyCompressionQuality = CFSTR("kCGImageDestinationLossyCompressionQuality");
const CFStringRef kCGImageSourceCreateThumbnailFromImageAlways = CFSTR("kCGImageSourceCreateThumbnailFromImageAlways");
const CFStringRef kCGImageSourceThumbnailMaxPixelSize = CFSTR("kCGImageSourceThumbnailMaxPixelSize");
const CFStringRef kCGImagePropertyDPIHeight = CFSTR("DPIHeight");
const CFStringRef kCGImagePropertyDPIWidth = CFSTR("DPIWidth");
const CFStringRef kCGImageSourceCreateThumbnailFromImageIfAbsent = CFSTR("kCGImageSourceCreateThumbnailFromImageIfAbsent");
const CFStringRef kCGImagePropertyPixelHeight = CFSTR("PixelHeight");
const CFStringRef kCGImagePropertyPixelWidth = CFSTR("PixelWidth");
const CFStringRef kCGImagePropertyPNGDictionary = CFSTR("{PNG}");
const CFStringRef kCGImagePropertyPNGGamma = CFSTR("Gamma");
const CFStringRef kCGImageSourceCreateThumbnailWithTransform = CFSTR("kCGImageSourceCreateThumbnailWithTransform");
const CFStringRef kCGImageSourceShouldCache = CFSTR("kCGImageSourceShouldCache");

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CGCopyBasicPropertiesOfImageAtPath(void) {
    if (verbose) puts("STUB: CGCopyBasicPropertiesOfImageAtPath called");
    return NULL;
}

void* CGImageCopyFileWithGPSInformation(void) {
    if (verbose) puts("STUB: CGImageCopyFileWithGPSInformation called");
    return NULL;
}

void* CGImageCopyFileWithParameters(void) {
    if (verbose) puts("STUB: CGImageCopyFileWithParameters called");
    return NULL;
}

void* CGImageCopyJPEGData(void) {
    if (verbose) puts("STUB: CGImageCopyJPEGData called");
    return NULL;
}

void* CGImageCopyJPEGDataAndColorSpace(void) {
    if (verbose) puts("STUB: CGImageCopyJPEGDataAndColorSpace called");
    return NULL;
}

void* CGImageCopySourceData(void) {
    if (verbose) puts("STUB: CGImageCopySourceData called");
    return NULL;
}

void* CGImageCreateByMatchingToColorSpace(void) {
    if (verbose) puts("STUB: CGImageCreateByMatchingToColorSpace called");
    return NULL;
}

void* CGImageCreateByMatchingToDisplay(void) {
    if (verbose) puts("STUB: CGImageCreateByMatchingToDisplay called");
    return NULL;
}

void* CGImageCreateByScaling(void) {
    if (verbose) puts("STUB: CGImageCreateByScaling called");
    return NULL;
}

void* CGImageCreateColorSpaceFromDataArray(void) {
    if (verbose) puts("STUB: CGImageCreateColorSpaceFromDataArray called");
    return NULL;
}

void* CGImageCreateCopyWithAlphaInfo(void) {
    if (verbose) puts("STUB: CGImageCreateCopyWithAlphaInfo called");
    return NULL;
}

void* CGImageCreateCopyWithParameters(void) {
    if (verbose) puts("STUB: CGImageCreateCopyWithParameters called");
    return NULL;
}

void* CGImageCreateEXIFDataFromProperties(void) {
    if (verbose) puts("STUB: CGImageCreateEXIFDataFromProperties called");
    return NULL;
}

void* CGImageCreateEXIFJPEGData(void) {
    if (verbose) puts("STUB: CGImageCreateEXIFJPEGData called");
    return NULL;
}

void* CGImageCreateFromIOSurface(void) {
    if (verbose) puts("STUB: CGImageCreateFromIOSurface called");
    return NULL;
}

void* CGImageCreateMetadataFromDataArray(void) {
    if (verbose) puts("STUB: CGImageCreateMetadataFromDataArray called");
    return NULL;
}

void* CGImageCreateThumb(void) {
    if (verbose) puts("STUB: CGImageCreateThumb called");
    return NULL;
}

void* CGImageCreateWithWBMPData(void) {
    if (verbose) puts("STUB: CGImageCreateWithWBMPData called");
    return NULL;
}

void* CGImageDestinationAddAuxiliaryDataInfo(void) {
    if (verbose) puts("STUB: CGImageDestinationAddAuxiliaryDataInfo called");
    return NULL;
}

void* CGImageDestinationAddImage(void) {
    if (verbose) puts("STUB: CGImageDestinationAddImage called");
    return NULL;
}

void* CGImageDestinationAddImageAndMetadata(void) {
    if (verbose) puts("STUB: CGImageDestinationAddImageAndMetadata called");
    return NULL;
}

void* CGImageDestinationAddImageFromSource(void) {
    if (verbose) puts("STUB: CGImageDestinationAddImageFromSource called");
    return NULL;
}

void* CGImageDestinationAddImageFromSourceWithMetadata(void) {
    if (verbose) puts("STUB: CGImageDestinationAddImageFromSourceWithMetadata called");
    return NULL;
}

void* CGImageDestinationAddImageWithMetadata(void) {
    if (verbose) puts("STUB: CGImageDestinationAddImageWithMetadata called");
    return NULL;
}

void* CGImageDestinationCopyImageFormats(void) {
    if (verbose) puts("STUB: CGImageDestinationCopyImageFormats called");
    return NULL;
}

void* CGImageDestinationCopyImageSource(void) {
    if (verbose) puts("STUB: CGImageDestinationCopyImageSource called");
    return NULL;
}

void* CGImageDestinationCopyTypeIdentifiers(void) {
    if (verbose) puts("STUB: CGImageDestinationCopyTypeIdentifiers called");
    return NULL;
}

void* CGImageDestinationCreateWithData(void) {
    if (verbose) puts("STUB: CGImageDestinationCreateWithData called");
    return NULL;
}

void* CGImageDestinationCreateWithDataConsumer(void) {
    if (verbose) puts("STUB: CGImageDestinationCreateWithDataConsumer called");
    return NULL;
}

void* CGImageDestinationCreateWithURL(void) {
    if (verbose) puts("STUB: CGImageDestinationCreateWithURL called");
    return NULL;
}

void* CGImageDestinationFinalize(void) {
    if (verbose) puts("STUB: CGImageDestinationFinalize called");
    return NULL;
}

void* CGImageDestinationGetTypeID(void) {
    if (verbose) puts("STUB: CGImageDestinationGetTypeID called");
    return NULL;
}

void* CGImageDestinationSetProperties(void) {
    if (verbose) puts("STUB: CGImageDestinationSetProperties called");
    return NULL;
}

void* CGImageIOCreateXMPDataFromMetaData(void) {
    if (verbose) puts("STUB: CGImageIOCreateXMPDataFromMetaData called");
    return NULL;
}

void* CGImageIsInsideSRGB(void) {
    if (verbose) puts("STUB: CGImageIsInsideSRGB called");
    return NULL;
}

void* CGImageMetadataCopyStringValueWithPath(void) {
    if (verbose) puts("STUB: CGImageMetadataCopyStringValueWithPath called");
    return NULL;
}

void* CGImageMetadataCopyTagMatchingImageProperty(void) {
    if (verbose) puts("STUB: CGImageMetadataCopyTagMatchingImageProperty called");
    return NULL;
}

void* CGImageMetadataCopyTagWithPath(void) {
    if (verbose) puts("STUB: CGImageMetadataCopyTagWithPath called");
    return NULL;
}

void* CGImageMetadataCopyTags(void) {
    if (verbose) puts("STUB: CGImageMetadataCopyTags called");
    return NULL;
}

void* CGImageMetadataCreateFromData(void) {
    if (verbose) puts("STUB: CGImageMetadataCreateFromData called");
    return NULL;
}

void* CGImageMetadataCreateFromMetadataProperties(void) {
    if (verbose) puts("STUB: CGImageMetadataCreateFromMetadataProperties called");
    return NULL;
}

void* CGImageMetadataCreateFromXMPData(void) {
    if (verbose) puts("STUB: CGImageMetadataCreateFromXMPData called");
    return NULL;
}

void* CGImageMetadataCreateMetadataProperties(void) {
    if (verbose) puts("STUB: CGImageMetadataCreateMetadataProperties called");
    return NULL;
}

void* CGImageMetadataCreateMutable(void) {
    if (verbose) puts("STUB: CGImageMetadataCreateMutable called");
    return NULL;
}

void* CGImageMetadataCreateMutableCopy(void) {
    if (verbose) puts("STUB: CGImageMetadataCreateMutableCopy called");
    return NULL;
}

void* CGImageMetadataCreateSerializedData(void) {
    if (verbose) puts("STUB: CGImageMetadataCreateSerializedData called");
    return NULL;
}

void* CGImageMetadataCreateSidecar(void) {
    if (verbose) puts("STUB: CGImageMetadataCreateSidecar called");
    return NULL;
}

void* CGImageMetadataCreateXMPData(void) {
    if (verbose) puts("STUB: CGImageMetadataCreateXMPData called");
    return NULL;
}

void* CGImageMetadataEnumerateTagsUsingBlock(void) {
    if (verbose) puts("STUB: CGImageMetadataEnumerateTagsUsingBlock called");
    return NULL;
}

void* CGImageMetadataGetTypeID(void) {
    if (verbose) puts("STUB: CGImageMetadataGetTypeID called");
    return NULL;
}

void* CGImageMetadataPropertyCreate(void) {
    if (verbose) puts("STUB: CGImageMetadataPropertyCreate called");
    return NULL;
}

void* CGImageMetadataPropertyGetPropertyName(void) {
    if (verbose) puts("STUB: CGImageMetadataPropertyGetPropertyName called");
    return NULL;
}

void* CGImageMetadataPropertyGetSchema(void) {
    if (verbose) puts("STUB: CGImageMetadataPropertyGetSchema called");
    return NULL;
}

void* CGImageMetadataPropertyGetSuggestedPrefix(void) {
    if (verbose) puts("STUB: CGImageMetadataPropertyGetSuggestedPrefix called");
    return NULL;
}

void* CGImageMetadataPropertyGetTypeID(void) {
    if (verbose) puts("STUB: CGImageMetadataPropertyGetTypeID called");
    return NULL;
}

void* CGImageMetadataPropertyGetValue(void) {
    if (verbose) puts("STUB: CGImageMetadataPropertyGetValue called");
    return NULL;
}

void* CGImageMetadataPropertySetValue(void) {
    if (verbose) puts("STUB: CGImageMetadataPropertySetValue called");
    return NULL;
}

void* CGImageMetadataRegisterNamespaceForPrefix(void) {
    if (verbose) puts("STUB: CGImageMetadataRegisterNamespaceForPrefix called");
    return NULL;
}

void* CGImageMetadataRemoveTagWithPath(void) {
    if (verbose) puts("STUB: CGImageMetadataRemoveTagWithPath called");
    return NULL;
}

void* CGImageMetadataSerialize(void) {
    if (verbose) puts("STUB: CGImageMetadataSerialize called");
    return NULL;
}

void* CGImageMetadataSetTagWithPath(void) {
    if (verbose) puts("STUB: CGImageMetadataSetTagWithPath called");
    return NULL;
}

void* CGImageMetadataSetValueMatchingImageProperty(void) {
    if (verbose) puts("STUB: CGImageMetadataSetValueMatchingImageProperty called");
    return NULL;
}

void* CGImageMetadataSetValueWithPath(void) {
    if (verbose) puts("STUB: CGImageMetadataSetValueWithPath called");
    return NULL;
}

void* CGImageMetadataTagCopyName(void) {
    if (verbose) puts("STUB: CGImageMetadataTagCopyName called");
    return NULL;
}

void* CGImageMetadataTagCopyNamespace(void) {
    if (verbose) puts("STUB: CGImageMetadataTagCopyNamespace called");
    return NULL;
}

void* CGImageMetadataTagCopyPrefix(void) {
    if (verbose) puts("STUB: CGImageMetadataTagCopyPrefix called");
    return NULL;
}

void* CGImageMetadataTagCopyQualifiers(void) {
    if (verbose) puts("STUB: CGImageMetadataTagCopyQualifiers called");
    return NULL;
}

void* CGImageMetadataTagCopyValue(void) {
    if (verbose) puts("STUB: CGImageMetadataTagCopyValue called");
    return NULL;
}

void* CGImageMetadataTagCreate(void) {
    if (verbose) puts("STUB: CGImageMetadataTagCreate called");
    return NULL;
}

void* CGImageMetadataTagGetType(void) {
    if (verbose) puts("STUB: CGImageMetadataTagGetType called");
    return NULL;
}

void* CGImageMetadataTagGetTypeID(void) {
    if (verbose) puts("STUB: CGImageMetadataTagGetTypeID called");
    return NULL;
}

void* CGImageMetadataValueAddQualifier(void) {
    if (verbose) puts("STUB: CGImageMetadataValueAddQualifier called");
    return NULL;
}

void* CGImageMetadataValueCreate(void) {
    if (verbose) puts("STUB: CGImageMetadataValueCreate called");
    return NULL;
}

void* CGImageMetadataValueGetNamespaceURI(void) {
    if (verbose) puts("STUB: CGImageMetadataValueGetNamespaceURI called");
    return NULL;
}

void* CGImageMetadataValueGetQualifiers(void) {
    if (verbose) puts("STUB: CGImageMetadataValueGetQualifiers called");
    return NULL;
}

void* CGImageMetadataValueGetSuggestedNamespacePrefix(void) {
    if (verbose) puts("STUB: CGImageMetadataValueGetSuggestedNamespacePrefix called");
    return NULL;
}

void* CGImageMetadataValueGetTypeID(void) {
    if (verbose) puts("STUB: CGImageMetadataValueGetTypeID called");
    return NULL;
}

void* CGImageMetadataValueGetValue(void) {
    if (verbose) puts("STUB: CGImageMetadataValueGetValue called");
    return NULL;
}

void* CGImageMetadataValueIsAlternate(void) {
    if (verbose) puts("STUB: CGImageMetadataValueIsAlternate called");
    return NULL;
}

void* CGImageMetadataValueIsAlternateText(void) {
    if (verbose) puts("STUB: CGImageMetadataValueIsAlternateText called");
    return NULL;
}

void* CGImageMetadataValueSetIsAlternate(void) {
    if (verbose) puts("STUB: CGImageMetadataValueSetIsAlternate called");
    return NULL;
}

void* CGImageMetadataValueSetIsAlternateText(void) {
    if (verbose) puts("STUB: CGImageMetadataValueSetIsAlternateText called");
    return NULL;
}

void* CGImageMetadataValueSetNamespaceURI(void) {
    if (verbose) puts("STUB: CGImageMetadataValueSetNamespaceURI called");
    return NULL;
}

void* CGImageMetadataValueSetSuggestedNamespacePrefix(void) {
    if (verbose) puts("STUB: CGImageMetadataValueSetSuggestedNamespacePrefix called");
    return NULL;
}

void* CGImagePluginGetMatchToProfileOption(void) {
    if (verbose) puts("STUB: CGImagePluginGetMatchToProfileOption called");
    return NULL;
}

void* CGImagePluginGetMetadata(void) {
    if (verbose) puts("STUB: CGImagePluginGetMetadata called");
    return NULL;
}

void* CGImagePluginGetOptions(void) {
    if (verbose) puts("STUB: CGImagePluginGetOptions called");
    return NULL;
}

void* CGImagePluginGetProperties(void) {
    if (verbose) puts("STUB: CGImagePluginGetProperties called");
    return NULL;
}

void* CGImagePluginGetSession(void) {
    if (verbose) puts("STUB: CGImagePluginGetSession called");
    return NULL;
}

void* CGImagePluginInitJPEGAtOffset(void) {
    if (verbose) puts("STUB: CGImagePluginInitJPEGAtOffset called");
    return NULL;
}

void* CGImagePluginInitThumbJPEGAtOffset(void) {
    if (verbose) puts("STUB: CGImagePluginInitThumbJPEGAtOffset called");
    return NULL;
}

void* CGImagePluginReadRawIPTCProps(void) {
    if (verbose) puts("STUB: CGImagePluginReadRawIPTCProps called");
    return NULL;
}

void* CGImagePluginReadRawXMPProps(void) {
    if (verbose) puts("STUB: CGImagePluginReadRawXMPProps called");
    return NULL;
}

void* CGImagePluginReadRawXMPSidecarProps(void) {
    if (verbose) puts("STUB: CGImagePluginReadRawXMPSidecarProps called");
    return NULL;
}

void* CGImagePluginSetClipPath(void) {
    if (verbose) puts("STUB: CGImagePluginSetClipPath called");
    return NULL;
}

void* CGImagePluginSetImage(void) {
    if (verbose) puts("STUB: CGImagePluginSetImage called");
    return NULL;
}

void* CGImagePluginSetImageBlockProc(void) {
    if (verbose) puts("STUB: CGImagePluginSetImageBlockProc called");
    return NULL;
}

void* CGImagePluginSetJpegProvider(void) {
    if (verbose) puts("STUB: CGImagePluginSetJpegProvider called");
    return NULL;
}

void* CGImageReadSessionCreateFILE(void) {
    if (verbose) puts("STUB: CGImageReadSessionCreateFILE called");
    return NULL;
}

void* CGImageReadSessionGetBytePointer(void) {
    if (verbose) puts("STUB: CGImageReadSessionGetBytePointer called");
    return NULL;
}

void* CGImageReadSessionGetBytesAtOffset(void) {
    if (verbose) puts("STUB: CGImageReadSessionGetBytesAtOffset called");
    return NULL;
}

void* CGImageReadSessionGetFilePath(void) {
    if (verbose) puts("STUB: CGImageReadSessionGetFilePath called");
    return NULL;
}

void* CGImageReadSessionGetSize(void) {
    if (verbose) puts("STUB: CGImageReadSessionGetSize called");
    return NULL;
}

void* CGImageReadSessionGetTypeID(void) {
    if (verbose) puts("STUB: CGImageReadSessionGetTypeID called");
    return NULL;
}

void* CGImageReadSessionIsFinal(void) {
    if (verbose) puts("STUB: CGImageReadSessionIsFinal called");
    return NULL;
}

void* CGImageReadSessionReleaseBytePointer(void) {
    if (verbose) puts("STUB: CGImageReadSessionReleaseBytePointer called");
    return NULL;
}

void* CGImageReadSessionReleaseCachedImageBlockData(void) {
    if (verbose) puts("STUB: CGImageReadSessionReleaseCachedImageBlockData called");
    return NULL;
}

void* CGImageSaveToFile(void) {
    if (verbose) puts("STUB: CGImageSaveToFile called");
    return NULL;
}

void* CGImageSourceAddProperty(void) {
    if (verbose) puts("STUB: CGImageSourceAddProperty called");
    return NULL;
}

void* CGImageSourceCopyAuxiliaryDataInfoAtIndex(void) {
    if (verbose) puts("STUB: CGImageSourceCopyAuxiliaryDataInfoAtIndex called");
    return NULL;
}

void* CGImageSourceCopyMetadataAtIndex(void) {
    if (verbose) puts("STUB: CGImageSourceCopyMetadataAtIndex called");
    return NULL;
}

void* CGImageSourceCopyMetadataPropertiesAtIndex(void) {
    if (verbose) puts("STUB: CGImageSourceCopyMetadataPropertiesAtIndex called");
    return NULL;
}

void* CGImageSourceCopyProperties(void) {
    if (verbose) puts("STUB: CGImageSourceCopyProperties called");
    return NULL;
}

void* CGImageSourceCopyPropertiesAtIndex(void) {
    if (verbose) puts("STUB: CGImageSourceCopyPropertiesAtIndex called");
    return NULL;
}

void* CGImageSourceCopyTypeExtensions(void) {
    if (verbose) puts("STUB: CGImageSourceCopyTypeExtensions called");
    return NULL;
}

void* CGImageSourceCopyTypeIdentifiers(void) {
    if (verbose) puts("STUB: CGImageSourceCopyTypeIdentifiers called");
    return NULL;
}

void* CGImageSourceCreateIOSurfaceAtIndex(void) {
    if (verbose) puts("STUB: CGImageSourceCreateIOSurfaceAtIndex called");
    return NULL;
}

void* CGImageSourceCreateImageAtIndex(void) {
    if (verbose) puts("STUB: CGImageSourceCreateImageAtIndex called");
    return NULL;
}

void* CGImageSourceCreateIncremental(void) {
    if (verbose) puts("STUB: CGImageSourceCreateIncremental called");
    return NULL;
}

void* CGImageSourceCreateThumbnailAtIndex(void) {
    if (verbose) puts("STUB: CGImageSourceCreateThumbnailAtIndex called");
    return NULL;
}

void* CGImageSourceCreateWithData(void) {
    if (verbose) puts("STUB: CGImageSourceCreateWithData called");
    return NULL;
}

void* CGImageSourceCreateWithDataProvider(void) {
    if (verbose) puts("STUB: CGImageSourceCreateWithDataProvider called");
    return NULL;
}

void* CGImageSourceCreateWithFile(void) {
    if (verbose) puts("STUB: CGImageSourceCreateWithFile called");
    return NULL;
}

void* CGImageSourceCreateWithURL(void) {
    if (verbose) puts("STUB: CGImageSourceCreateWithURL called");
    return NULL;
}

void* CGImageSourceGetCount(void) {
    if (verbose) puts("STUB: CGImageSourceGetCount called");
    return NULL;
}

void* CGImageSourceGetImageCacheAtIndex(void) {
    if (verbose) puts("STUB: CGImageSourceGetImageCacheAtIndex called");
    return NULL;
}

void* CGImageSourceGetPrimaryImageIndex(void) {
    if (verbose) puts("STUB: CGImageSourceGetPrimaryImageIndex called");
    return NULL;
}

void* CGImageSourceGetStatus(void) {
    if (verbose) puts("STUB: CGImageSourceGetStatus called");
    return NULL;
}

void* CGImageSourceGetStatusAtIndex(void) {
    if (verbose) puts("STUB: CGImageSourceGetStatusAtIndex called");
    return NULL;
}

void* CGImageSourceGetType(void) {
    if (verbose) puts("STUB: CGImageSourceGetType called");
    return NULL;
}

void* CGImageSourceGetTypeID(void) {
    if (verbose) puts("STUB: CGImageSourceGetTypeID called");
    return NULL;
}

void* CGImageSourceGetTypeWithData(void) {
    if (verbose) puts("STUB: CGImageSourceGetTypeWithData called");
    return NULL;
}

void* CGImageSourceGetTypeWithDataProvider(void) {
    if (verbose) puts("STUB: CGImageSourceGetTypeWithDataProvider called");
    return NULL;
}

void* CGImageSourceGetTypeWithExtension(void) {
    if (verbose) puts("STUB: CGImageSourceGetTypeWithExtension called");
    return NULL;
}

void* CGImageSourceGetTypeWithFile(void) {
    if (verbose) puts("STUB: CGImageSourceGetTypeWithFile called");
    return NULL;
}

void* CGImageSourceGetTypeWithURL(void) {
    if (verbose) puts("STUB: CGImageSourceGetTypeWithURL called");
    return NULL;
}

void* CGImageSourceIsColorOptimizedForSharing(void) {
    if (verbose) puts("STUB: CGImageSourceIsColorOptimizedForSharing called");
    return NULL;
}

void* CGImageSourceRemoveCacheAtIndex(void) {
    if (verbose) puts("STUB: CGImageSourceRemoveCacheAtIndex called");
    return NULL;
}

void* CGImageSourceUpdateData(void) {
    if (verbose) puts("STUB: CGImageSourceUpdateData called");
    return NULL;
}

void* CGImageSourceUpdateDataProvider(void) {
    if (verbose) puts("STUB: CGImageSourceUpdateDataProvider called");
    return NULL;
}

void* CGImageWriteEXIFJPEGToPath(void) {
    if (verbose) puts("STUB: CGImageWriteEXIFJPEGToPath called");
    return NULL;
}

void* CGImageWriteEXIFJPEGWithMetadata(void) {
    if (verbose) puts("STUB: CGImageWriteEXIFJPEGWithMetadata called");
    return NULL;
}

void* CopyMetadataFromFileAtPath(void) {
    if (verbose) puts("STUB: CopyMetadataFromFileAtPath called");
    return NULL;
}

void* CopyMetadataFromSequentialDataProviderCallbacks(void) {
    if (verbose) puts("STUB: CopyMetadataFromSequentialDataProviderCallbacks called");
    return NULL;
}

void* GetMetadataPropertyMatchingSchemaAndPropertyName(void) {
    if (verbose) puts("STUB: GetMetadataPropertyMatchingSchemaAndPropertyName called");
    return NULL;
}

void* ImageIODebug(void) {
    if (verbose) puts("STUB: ImageIODebug called");
    return NULL;
}

void* ImageIOSetLoggingProc(void) {
    if (verbose) puts("STUB: ImageIOSetLoggingProc called");
    return NULL;
}
